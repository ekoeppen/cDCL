// ==============================
// File:			TDCLFDSerialPort.cpp
// Project:			Desktop Connection Library
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is TDCLFDSerialPort.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacik <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Simon Stapleton <simon@tufty.co.uk>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>

// C++
#include <algorithm>
#include <chrono>
#include <utility>

// POSIX & ANSI
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Server/TDCLServer.h>

TDCLFDSerialPort::TDCLFDSerialPort(
	const char* inDevPath,
	long inBaud /* = 38400 */,
	Boolean inUseMNP /* = true */ )
	: TDCLCommLayer( true ),
	  mSerialFD( -1 ),
	  mDevPath( nil ),
	  mBaud( inBaud ),
	  mPipeMode( inUseMNP ? kModeMNP : kModePlain ),
	  mTimeout( kDefaultTimeout ),
	  mWakeWait(),
	  mPortState( EPortState::kStopped ),
	  mRefuseRequested( false ),
	  mCloseRequested( false ),
	  mCancelFlushRequested( false ),
	  mIterStopSource( nullptr ),
	  mOutboundFrontOffset( 0 )
{
	if ( inDevPath )
	{
		Init( inDevPath );
	}
}

TDCLFDSerialPort::TDCLFDSerialPort(
	const char* inDevPath,
	long inBaud,
	KUInt32 inMode )
	: TDCLCommLayer( true ),
	  mSerialFD( -1 ),
	  mDevPath( nil ),
	  mBaud( inBaud ),
	  mPipeMode( inMode ),
	  mTimeout( kDefaultTimeout ),
	  mWakeWait(),
	  mPortState( EPortState::kStopped ),
	  mRefuseRequested( false ),
	  mCloseRequested( false ),
	  mCancelFlushRequested( false ),
	  mIterStopSource( nullptr ),
	  mOutboundFrontOffset( 0 )
{
	if ( inDevPath )
	{
		Init( inDevPath );
	}
}

TDCLFDSerialPort::~TDCLFDSerialPort( void )
{
	StopListening();
	CloseCurrentFD( false, true );

	if ( mDevPath )
	{
		::free( mDevPath );
		mDevPath = nil;
	}
}

void
TDCLFDSerialPort::Init( const char* inDevPath )
{
	size_t theLength = ::strlen( inDevPath ) + 1;
	mDevPath = (char*)::malloc( theLength );
	(void)::memcpy( mDevPath, inDevPath, theLength );
}

bool
TDCLFDSerialPort::IsCurrentThread( void ) const noexcept
{
	return std::this_thread::get_id() == mWorkerThreadId.load();
}

void
TDCLFDSerialPort::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

long
TDCLFDSerialPort::GetTimeout( void )
{
	return mTimeout;
}

void
TDCLFDSerialPort::DoStartListening( void )
{
	if ( mSerialFD == -1 )
	{
		OpenAndConfigure();
	}

	int wakeErr = mWakeWait.EnsureOpen();
	if ( wakeErr != 0 )
	{
		CloseCurrentFD( false, true );
		throw DCLPlatformUnknownError( wakeErr );
	}

	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mPortState = EPortState::kListening;
		mRefuseRequested = false;
		mRefuseException = nullptr;
		mCloseRequested = false;
		mCancelFlushRequested = false;
		mOutboundQueue.clear();
		mOutboundFrontOffset = 0;
	}

	const std::stop_token token = GetStopToken();
	mWorkerThread = std::jthread( [this, token]( std::stop_token ) { WorkerEntry( token ); } );
}

void
TDCLFDSerialPort::StopListening( void )
{
	TDCLCommLayer::StopListening();
	CloseCurrentFD( false, true );
	mWakeWait.Close();
}

void
TDCLFDSerialPort::DoStopListening( void )
{
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mCancelFlushRequested = true;
		if ( mIterStopSource )
		{
			(void)mIterStopSource->request_stop();
		}
	}
	mWakeWait.Trigger();
	mStateCV.notify_all();

	if ( mWorkerThread.joinable() && !IsCurrentThread() )
	{
		mWorkerThread.join();
	}

	mDownstreamGuard.Reset();
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mPortState = EPortState::kStopped;
		mActiveSessionId = TDCLSessionId{};
		mOutboundQueue.clear();
		mOutboundFrontOffset = 0;
	}
}

TDCLSessionId
TDCLFDSerialPort::AcceptSession( void )
{
	auto bus = GetSessionBus();
	if ( !bus )
	{
		return {};
	}

	std::unique_lock<std::mutex> lock( mStateMutex );
	if ( mPortState != EPortState::kPendingAccept )
	{
		return {};
	}

	mCloseRequested = false;
	mCancelFlushRequested = false;
	mOutboundQueue.clear();
	mOutboundFrontOffset = 0;
	lock.unlock();

	mDownstreamGuard = bus->CreateSession(
	    [this]( std::vector<std::uint8_t> inBytes ) {
		    return QueueOutbound( mActiveSessionId, std::move( inBytes ) );
	    },
	    [this]( void ) {
		    RequestCloseSession( mActiveSessionId );
	    } );
	const TDCLSessionId newId = mDownstreamGuard.GetSessionId();

	lock.lock();
	mActiveSessionId = newId;
	mPortState = EPortState::kConnected;
	mStateCV.notify_all();
	return newId;
}

void
TDCLFDSerialPort::Refuse( void )
{
	std::unique_lock<std::mutex> lock( mStateMutex );
	if ( mPortState == EPortState::kStopped )
	{
		return;
	}
	mRefuseRequested = true;
	mRefuseException = nullptr;
	if ( mIterStopSource )
	{
		(void)mIterStopSource->request_stop();
	}
	mWakeWait.Trigger();
	mStateCV.notify_all();

	if ( !IsCurrentThread() )
	{
		mStateCV.wait( lock, GetStopToken(), [this]() {
			return !mRefuseRequested || mPortState == EPortState::kStopped;
		} );
	}

	std::exception_ptr ex = mRefuseException;
	mRefuseException = nullptr;
	lock.unlock();

	if ( ex )
	{
		std::rethrow_exception( ex );
	}
}

bool
TDCLFDSerialPort::QueueOutbound( TDCLSessionId /* inSessionId */, std::vector<std::uint8_t>&& inBytes )
{
	std::lock_guard<std::mutex> lock( mStateMutex );
	if ( mPortState != EPortState::kConnected || mCloseRequested || mCancelFlushRequested )
	{
		return false;
	}
	if ( !inBytes.empty() )
	{
		mOutboundQueue.push_back( std::move( inBytes ) );
		if ( mIterStopSource )
		{
			(void)mIterStopSource->request_stop();
		}
	}
	return true;
}

void
TDCLFDSerialPort::RequestCloseSession( TDCLSessionId /* inSessionId */ )
{
	std::lock_guard<std::mutex> lock( mStateMutex );
	mCloseRequested = true;
	if ( mIterStopSource )
	{
		(void)mIterStopSource->request_stop();
	}
	mStateCV.notify_all();
}

void
TDCLFDSerialPort::WorkerEntry( std::stop_token inStopToken )
{
	mWorkerThreadId.store( std::this_thread::get_id() );
	std::vector<std::uint8_t> readBuf( 4096 );
	auto bus = GetSessionBus();

	try
	{
		while ( !inStopToken.stop_requested() )
		{
			EPortState state;
			bool refuse = false;
			{
				std::lock_guard<std::mutex> lock( mStateMutex );
				state = mPortState;
				refuse = mRefuseRequested;
			}

			if ( refuse )
			{
				mDownstreamGuard.Reset();
				std::exception_ptr ex;
				try
				{
					ResetSerialFD( false, true );
				}
				catch ( ... )
				{
					ex = std::current_exception();
				}
				std::lock_guard<std::mutex> lock( mStateMutex );
				mOutboundQueue.clear();
				mOutboundFrontOffset = 0;
				mCloseRequested = false;
				mActiveSessionId = TDCLSessionId{};
				mRefuseException = ex;
				mRefuseRequested = false;
				mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
				mStateCV.notify_all();
				if ( mSerialFD < 0 )
				{
					break;
				}
				continue;
			}

			if ( state == EPortState::kListening )
			{
				std::stop_source iterStop;
				std::stop_token iterToken = iterStop.get_token();
				int fd = -1;
				{
					std::lock_guard<std::mutex> lock( mStateMutex );
					if ( mRefuseRequested || inStopToken.stop_requested() )
					{
						continue;
					}
					fd = mSerialFD;
					mIterStopSource = &iterStop;
				}
				if ( fd < 0 )
				{
					break;
				}

				std::stop_callback cb( inStopToken, [&iterStop]() { (void)iterStop.request_stop(); } );
				int err = 0;
				const auto res = mWakeWait.WaitReady(
				    fd,
				    TDCLWakeableWait::EDirection::kRead,
				    std::chrono::milliseconds( -1 ),
				    iterToken,
				    &err );

				bool notifyRequest = false;
				{
					std::lock_guard<std::mutex> lock( mStateMutex );
					mIterStopSource = nullptr;
					if ( res == TDCLWakeableWait::EResult::kReady && !mRefuseRequested && !inStopToken.stop_requested() )
					{
						mPortState = EPortState::kPendingAccept;
						notifyRequest = true;
					}
				}
				if ( notifyRequest )
				{
					if ( TDCLServer* srv = GetServer() )
					{
						srv->RequestPresent( this );
					}
				}
			}
			else if ( state == EPortState::kPendingAccept )
			{
				std::unique_lock<std::mutex> lock( mStateMutex );
				mStateCV.wait( lock, inStopToken, [this]() {
					return mPortState != EPortState::kPendingAccept || mRefuseRequested;
				} );
			}
			else if ( state == EPortState::kConnected )
			{
				// 1. Drain outbound queue.
				bool writeBlocked = false;
				for ( ;; )
				{
					std::vector<std::uint8_t> chunk;
					std::size_t offset = 0;
					{
						std::lock_guard<std::mutex> lock( mStateMutex );
						if ( mCancelFlushRequested || mRefuseRequested || mOutboundQueue.empty() )
						{
							break;
						}
						chunk = std::move( mOutboundQueue.front() );
						offset = mOutboundFrontOffset;
					}

					while ( offset < chunk.size() && !inStopToken.stop_requested() )
					{
						ssize_t written = ::write( mSerialFD, chunk.data() + offset, chunk.size() - offset );
						if ( written > 0 )
						{
							offset += static_cast<std::size_t>( written );
						}
						else
						{
							const int err = errno;
							if ( err == EINTR )
							{
								continue;
							}
							if ( err == EAGAIN
#if ( EWOULDBLOCK != EAGAIN )
							     || err == EWOULDBLOCK
#endif
							)
							{
								writeBlocked = true;
								break;
							}
							if ( bus )
							{
								bus->PublishError( mActiveSessionId, std::make_exception_ptr( DCLPlatformUnknownError( err ) ) );
							}
							mDownstreamGuard.Reset();
							ResetSerialFD( false, true );
							{
								std::lock_guard<std::mutex> lock( mStateMutex );
								mActiveSessionId = TDCLSessionId{};
								mOutboundQueue.clear();
								mOutboundFrontOffset = 0;
								mCloseRequested = false;
								mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
							}
							goto next_iter;
						}
					}

					std::lock_guard<std::mutex> lock( mStateMutex );
					if ( offset >= chunk.size() )
					{
						mOutboundQueue.pop_front();
						mOutboundFrontOffset = 0;
					}
					else
					{
						mOutboundQueue.front() = std::move( chunk );
						mOutboundFrontOffset = offset;
						break;
					}
				}

				// 2. Handle session close request.
				bool doClose = false;
				bool drainOnClose = false;
				{
					std::lock_guard<std::mutex> lock( mStateMutex );
					if ( mCloseRequested && ( mCancelFlushRequested || mOutboundQueue.empty() ) )
					{
						doClose = true;
						drainOnClose = !mCancelFlushRequested && !inStopToken.stop_requested();
					}
				}
				if ( doClose )
				{
					mDownstreamGuard.Reset();
					ResetSerialFD( drainOnClose, false );
					std::lock_guard<std::mutex> lock( mStateMutex );
					mActiveSessionId = TDCLSessionId{};
					mOutboundQueue.clear();
					mOutboundFrontOffset = 0;
					mCloseRequested = false;
					mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
					goto next_iter;
				}

				// 3. Read available bytes.
				std::vector<std::uint8_t> inbound;
				for ( ;; )
				{
					ssize_t n = ::read( mSerialFD, readBuf.data(), readBuf.size() );
					if ( n > 0 )
					{
						inbound.insert( inbound.end(), readBuf.data(), readBuf.data() + n );
					}
					else if ( n == 0 )
					{
						if ( bus && !inbound.empty() )
						{
							bus->PublishBytes( mActiveSessionId, std::move( inbound ) );
						}
						if ( bus )
						{
							bus->PublishClosed( mActiveSessionId );
						}
						mDownstreamGuard.Reset();
						ResetSerialFD( false, true );
						std::lock_guard<std::mutex> lock( mStateMutex );
						mActiveSessionId = TDCLSessionId{};
						mOutboundQueue.clear();
						mOutboundFrontOffset = 0;
						mCloseRequested = false;
						mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
						goto next_iter;
					}
					else
					{
						const int err = errno;
						if ( err == EINTR )
						{
							continue;
						}
						if ( err == EAGAIN
#if ( EWOULDBLOCK != EAGAIN )
						     || err == EWOULDBLOCK
#endif
						)
						{
							break;
						}
						if ( bus && !inbound.empty() )
						{
							bus->PublishBytes( mActiveSessionId, std::move( inbound ) );
						}
						if ( bus )
						{
							bus->PublishError( mActiveSessionId, std::make_exception_ptr( DCLPlatformUnknownError( err ) ) );
						}
						mDownstreamGuard.Reset();
						ResetSerialFD( false, true );
						std::lock_guard<std::mutex> lock( mStateMutex );
						mActiveSessionId = TDCLSessionId{};
						mOutboundQueue.clear();
						mOutboundFrontOffset = 0;
						mCloseRequested = false;
						mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
						goto next_iter;
					}
				}

				if ( bus && !inbound.empty() )
				{
					bus->PublishBytes( mActiveSessionId, std::move( inbound ) );
				}

				// 4. Wait for serial FD readiness or outbound/close/refuse wakeup.
				std::stop_source iterStop;
				std::stop_token iterToken = iterStop.get_token();
				{
					std::lock_guard<std::mutex> lock( mStateMutex );
					if ( mRefuseRequested || mCancelFlushRequested ||
					     ( mCloseRequested && mOutboundQueue.empty() ) ||
					     ( !writeBlocked && !mOutboundQueue.empty() ) )
					{
						continue;
					}
					mIterStopSource = &iterStop;
				}

				std::stop_callback cb( inStopToken, [&iterStop]() { (void)iterStop.request_stop(); } );
				int waitErr = 0;
				const auto dir = writeBlocked ? TDCLWakeableWait::EDirection::kWrite : TDCLWakeableWait::EDirection::kRead;
				const auto res = mWakeWait.WaitReady(
				    mSerialFD,
				    dir,
				    writeBlocked ? std::chrono::milliseconds( 50 ) : std::chrono::milliseconds( -1 ),
				    iterToken,
				    &waitErr );

				{
					std::lock_guard<std::mutex> lock( mStateMutex );
					mIterStopSource = nullptr;
				}

				if ( res == TDCLWakeableWait::EResult::kError )
				{
					if ( bus )
					{
						bus->PublishError( mActiveSessionId, std::make_exception_ptr( DCLPlatformUnknownError( waitErr ) ) );
					}
					mDownstreamGuard.Reset();
					ResetSerialFD( false, true );
					std::lock_guard<std::mutex> lock( mStateMutex );
					mActiveSessionId = TDCLSessionId{};
					mPortState = ( mSerialFD >= 0 ) ? EPortState::kListening : EPortState::kStopped;
				}
			}
			else
			{
				break;
			}
		next_iter:;
		}
	}
	catch ( TDCLUserCancelledException& )
	{
	}
	catch ( TDCLException& ex )
	{
		if ( TDCLServer* srv = GetServer() )
		{
			srv->HandleCommLayerException( this, &ex );
		}
	}
	catch ( ... )
	{
		if ( TDCLServer* srv = GetServer() )
		{
			srv->HandleCommLayerException( this, nil );
		}
	}

	mDownstreamGuard.Reset();
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mRefuseRequested = false;
		mPortState = EPortState::kStopped;
		mStateCV.notify_all();
	}
	mWorkerThreadId.store( std::thread::id() );
}

void
TDCLFDSerialPort::CloseSerialFD( int inFD )
{
	(void)::ioctl( inFD, TIOCNXCL );
	(void)::close( inFD );
}

void
TDCLFDSerialPort::CloseCurrentFD( bool inDrainOutput, bool inFlushInput )
{
	if ( mSerialFD != -1 )
	{
		if ( inDrainOutput )
		{
			(void)::tcdrain( mSerialFD );
		}
		else if ( inFlushInput )
		{
			(void)::tcflush( mSerialFD, TCIOFLUSH );
		}
		else
		{
			(void)::tcflush( mSerialFD, TCOFLUSH );
		}
		(void)::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions );
		CloseSerialFD( mSerialFD );
		mSerialFD = -1;
	}
}

void
TDCLFDSerialPort::ResetSerialFD( bool inDrainOutput, bool inFlushInput )
{
	CloseCurrentFD( inDrainOutput, inFlushInput );
	if ( GetServer() && !StopRequested() )
	{
		OpenAndConfigure();
	}
}

void
TDCLFDSerialPort::OpenAndConfigure( void )
{
	int theFD = ::open( mDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if ( theFD == -1 )
	{
		throw DCLPlatformUnknownError( errno );
	}

	if ( ::ioctl( theFD, TIOCEXCL ) == -1 )
	{
		int theErrno = errno;
		(void)::close( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	int flags = ::fcntl( theFD, F_GETFL, 0 );
	flags &= ~O_ASYNC;
	flags |= O_NONBLOCK;
	flags |= O_APPEND;
	int theErr = ::fcntl( theFD, F_SETFL, flags );
	if ( theErr < 0 )
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	if ( ::tcgetattr( theFD, &mOriginalOptions ) == -1 )
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	struct termios theOptions = mOriginalOptions;
	theOptions.c_iflag = IGNBRK | INPCK;
	theOptions.c_oflag = 0;
	theOptions.c_cflag = ((((CS8 | CREAD | HUPCL | CLOCAL) & ~PARENB) & ~PARODD) & ~CSTOPB);
	theOptions.c_lflag = 0;

	switch ( mBaud )
	{
#ifdef B2400
	case 2400:
		theErr = cfsetospeed( &theOptions, B2400 );
		break;
#endif
#ifdef B4800
	case 4800:
		theErr = cfsetospeed( &theOptions, B4800 );
		break;
#endif
#ifdef B9600
	case 9600:
		theErr = cfsetospeed( &theOptions, B9600 );
		break;
#endif
#ifdef B19200
	case 19200:
		theErr = cfsetospeed( &theOptions, B19200 );
		break;
#endif
#ifdef B38400
	case 38400:
		theErr = cfsetospeed( &theOptions, B38400 );
		break;
#endif
#ifdef B57600
	case 57600:
		theErr = cfsetospeed( &theOptions, B57600 );
		break;
#endif
#ifdef B115200
	case 115200:
		theErr = cfsetospeed( &theOptions, B115200 );
		break;
#endif
#ifdef B230400
	case 230400:
		theErr = cfsetospeed( &theOptions, B230400 );
		break;
#endif
	default:
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( EINVAL );
	}

	if ( theErr < 0 )
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	theErr = cfsetispeed( &theOptions, B0 );
	if ( theErr < 0 )
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	theOptions.c_cc[VMIN] = 1;
	theOptions.c_cc[VTIME] = 10;

	theErr = ::tcsetattr( theFD, TCSANOW, &theOptions );
	if ( theErr < 0 )
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}

	mSerialFD = theFD;
}
