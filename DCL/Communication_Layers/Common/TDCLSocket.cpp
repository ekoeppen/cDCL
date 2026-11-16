// ==============================
// File:			TDCLSocket.cpp
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
// The Original Code is TDCLSocket.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacik <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>

// ISO C++
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <deque>
#include <utility>

#if !TARGET_OS_WIN32
#include <netinet/tcp.h>
#include <unistd.h>
#endif

// DCL
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Server/TDCLServer.h>

class TDCLSocket::TSocketSession
{
public:
	TSocketSession(
		dcl_socket_t inSocket,
		TSocketOps inOps,
		std::shared_ptr<TDCLSessionBus> inBus,
		std::shared_ptr<std::atomic<KUInt32>> inActiveCount )
		: mSocket( inSocket ),
		  mOps( inOps ),
		  mBus( std::move( inBus ) ),
		  mActiveCount( std::move( inActiveCount ) )
	{
	}

	~TSocketSession( void )
	{
		StopAndJoin();
	}

	TDCLSessionId Start( void )
	{
		(void) mWakeWait.EnsureOpen();
		mActiveCount->fetch_add( 1, std::memory_order_relaxed );

		mDownstreamGuard = mBus->CreateSession(
		    [this]( std::vector<std::uint8_t> inBytes ) {
			    return QueueOutbound( std::move( inBytes ) );
		    },
		    [this]( void ) {
			    RequestClose();
		    } );

		const TDCLSessionId id = mDownstreamGuard.GetSessionId();
		mSessionId = id;
		mStopSource = std::stop_source{};
		const std::stop_token token = mStopSource.get_token();
		mThread = std::jthread( [this, token]( std::stop_token ) { WorkerLoop( token ); } );
		return id;
	}

	void StopAndJoin( void ) noexcept
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mCloseRequested = true;
			mCancelRequested = true;
			if ( mIterStopSource )
			{
				(void) mIterStopSource->request_stop();
			}
		}
		(void) mStopSource.request_stop();
		mWakeWait.Trigger();

		if ( mThread.joinable() )
		{
			if ( std::this_thread::get_id() == mThread.get_id() )
			{
				// Never self-join; detach not needed because StopAndJoin is called
				// externally from Destroy()/CloseAllSessions().
				return;
			}
			mThread.join();
		}

		mDownstreamGuard.Reset();
		mWakeWait.Close();
		if ( mSocket != kInvalidSocket )
		{
			mOps.closeSocket( mSocket );
			mSocket = kInvalidSocket;
		}
		if ( mCounted.exchange( false ) )
		{
			mActiveCount->fetch_sub( 1, std::memory_order_relaxed );
		}
	}

	bool IsFinished( void ) const noexcept
	{
		return mFinished.load( std::memory_order_acquire );
	}

private:
	bool QueueOutbound( std::vector<std::uint8_t>&& inBytes )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( mCloseRequested || mCancelRequested )
		{
			return false;
		}
		if ( !inBytes.empty() )
		{
			mOutboundQueue.push_back( std::move( inBytes ) );
			if ( mIterStopSource )
			{
				(void) mIterStopSource->request_stop();
			}
		}
		return true;
	}

	void RequestClose( void )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		mCloseRequested = true;
		if ( mIterStopSource )
		{
			(void) mIterStopSource->request_stop();
		}
	}

	void WorkerLoop( std::stop_token inStopToken )
	{
		std::vector<std::uint8_t> readBuf( 8192 );
		std::size_t frontOffset = 0;

		while ( !inStopToken.stop_requested() )
		{
			// 1. Drain outbound queue.
			bool writeBlocked = false;
			for ( ;; )
			{
				std::vector<std::uint8_t> chunk;
				std::size_t offset = 0;
				{
					std::lock_guard<std::mutex> lock( mMutex );
					if ( mCancelRequested || mOutboundQueue.empty() )
					{
						break;
					}
					chunk = std::move( mOutboundQueue.front() );
					offset = frontOffset;
				}

				while ( offset < chunk.size() && !inStopToken.stop_requested() )
				{
#if TARGET_OS_WIN32
					int toSend = static_cast<int>( chunk.size() - offset );
					int sent = ::send( mSocket, reinterpret_cast<const char*>( chunk.data() + offset ), toSend, 0 );
#else
					std::size_t toSend = chunk.size() - offset;
					ssize_t sent = ::send( mSocket, chunk.data() + offset, toSend, 0 );
#endif
					if ( sent > 0 )
					{
						offset += static_cast<std::size_t>( sent );
					}
					else
					{
						const int err = mOps.getSocketError();
						if ( mOps.isInterrupted( err ) )
						{
							continue;
						}
						if ( mOps.isWouldBlock( err ) )
						{
							writeBlocked = true;
							break;
						}
						mBus->PublishError( mSessionId, mOps.makeSendError( err ) );
						goto done;
					}
				}

				std::lock_guard<std::mutex> lock( mMutex );
				if ( offset >= chunk.size() )
				{
					mOutboundQueue.pop_front();
					frontOffset = 0;
				}
				else
				{
					mOutboundQueue.front() = std::move( chunk );
					frontOffset = offset;
					break;
				}
			}

			// 2. Check close request after outbound flush.
			{
				std::lock_guard<std::mutex> lock( mMutex );
				if ( mCloseRequested && ( mCancelRequested || mOutboundQueue.empty() ) )
				{
					goto done;
				}
			}

			// 3. Read all currently available bytes.
			std::vector<std::uint8_t> inbound;
			for ( ;; )
			{
#if TARGET_OS_WIN32
				int n = ::recv( mSocket, reinterpret_cast<char*>( readBuf.data() ), static_cast<int>( readBuf.size() ), 0 );
#else
				ssize_t n = ::recv( mSocket, readBuf.data(), readBuf.size(), 0 );
#endif
				if ( n > 0 )
				{
					inbound.insert( inbound.end(), readBuf.data(), readBuf.data() + n );
				}
				else if ( n == 0 )
				{
					if ( !inbound.empty() )
					{
						mBus->PublishBytes( mSessionId, std::move( inbound ) );
					}
					mBus->PublishClosed( mSessionId );
					goto done;
				}
				else
				{
					const int err = mOps.getSocketError();
					if ( mOps.isInterrupted( err ) )
					{
						continue;
					}
					if ( mOps.isWouldBlock( err ) )
					{
						break;
					}
					if ( !inbound.empty() )
					{
						mBus->PublishBytes( mSessionId, std::move( inbound ) );
					}
					mBus->PublishError( mSessionId, mOps.makeRecvError( err ) );
					goto done;
				}
			}

			if ( !inbound.empty() )
			{
				mBus->PublishBytes( mSessionId, std::move( inbound ) );
			}

			// 4. Wait for socket readiness or outbound/close wake.
			std::stop_source iterStop;
			std::stop_token iterToken = iterStop.get_token();
			{
				std::lock_guard<std::mutex> lock( mMutex );
				if ( mCancelRequested || ( mCloseRequested && mOutboundQueue.empty() ) || ( !writeBlocked && !mOutboundQueue.empty() ) )
				{
					continue;
				}
				mIterStopSource = &iterStop;
			}

			std::stop_callback cb( inStopToken, [&iterStop]() { (void)iterStop.request_stop(); } );
			int waitErr = 0;
			const auto dir = writeBlocked ? TDCLWakeableWait::EDirection::kWrite : TDCLWakeableWait::EDirection::kRead;
			const auto res = mWakeWait.WaitReady(
			    (dcl_wait_handle_t)mSocket,
			    dir,
			    writeBlocked ? std::chrono::milliseconds( 50 ) : std::chrono::milliseconds( -1 ),
			    iterToken,
			    &waitErr );

			{
				std::lock_guard<std::mutex> lock( mMutex );
				mIterStopSource = nullptr;
			}

			if ( res == TDCLWakeableWait::EResult::kError )
			{
				mBus->PublishError( mSessionId, mOps.makeRecvError( waitErr ) );
				break;
			}
		}

	done:
		mDownstreamGuard.Reset();
		mWakeWait.Close();
		if ( mSocket != kInvalidSocket )
		{
			mOps.closeSocket( mSocket );
			mSocket = kInvalidSocket;
		}
		if ( mCounted.exchange( false ) )
		{
			mActiveCount->fetch_sub( 1, std::memory_order_relaxed );
		}
		mFinished.store( true, std::memory_order_release );
	}

	dcl_socket_t mSocket;
	TSocketOps mOps;
	std::shared_ptr<TDCLSessionBus> mBus;
	std::shared_ptr<std::atomic<KUInt32>> mActiveCount;
	std::atomic<bool> mCounted{ true };
	std::atomic<bool> mFinished{ false };

	TDCLSessionId mSessionId;
	TDCLSessionBus::DownstreamGuard mDownstreamGuard;
	TDCLWakeableWait mWakeWait;

	std::mutex mMutex;
	std::deque<std::vector<std::uint8_t>> mOutboundQueue;
	bool mCloseRequested = false;
	bool mCancelRequested = false;
	std::stop_source* mIterStopSource = nullptr;

	std::stop_source mStopSource;
	std::jthread mThread;
};

TDCLSocket::TDCLSocket( unsigned short inPort /* = kIANAAssignedPort */,
                        KUInt32 inMaxPipeCount /* = kDefaultPipeCount */,
                        long inTimeout /* = kDefaultTimeout */ )
    : TDCLCommLayer( false /* inFlushAvailable */, inMaxPipeCount ),
      mServerSocket( kInvalidSocket ),
      mActiveSessionCount( std::make_shared<std::atomic<KUInt32>>( 0 ) ),
      mBackLogSize( 0 ),
      mTimeout( inTimeout ),
      mWakeWait(),
      mSocketsAreClosed( true ),
      mPort( inPort ),
      mRequestPending( false )
{
	if ( ( inMaxPipeCount == 0 ) || ( inMaxPipeCount > kMaxSocketBackLog ) )
	{
		mBackLogSize = kMaxSocketBackLog;
	}
	else
	{
		mBackLogSize = inMaxPipeCount;
	}
}

TDCLSocket::~TDCLSocket( void )
{
	Destroy();
}

void
TDCLSocket::CloseListenSockets( void )
{
	if ( !mSocketsAreClosed )
	{
		if ( mServerSocket != kInvalidSocket )
		{
			CloseSocket( mServerSocket );
			mServerSocket = kInvalidSocket;
		}

		mWakeWait.Close();
		mSocketsAreClosed = true;
	}
}

void
TDCLSocket::StopListening( void )
{
	TDCLCommLayer::StopListening();
	CloseListenSockets();
}

void
TDCLSocket::Destroy( void )
{
	StopListening();
	CloseAllSessions();
}

void
TDCLSocket::CloseAllSessions( void )
{
	std::vector<std::shared_ptr<TSocketSession>> sessions;
	{
		std::lock_guard<std::mutex> lock( mSessionsMutex );
		sessions.swap( mSessions );
	}
	for ( auto& s : sessions )
	{
		s->StopAndJoin();
	}
}

void
TDCLSocket::PruneFinishedSessionsLocked( void )
{
	mSessions.erase(
	    std::remove_if( mSessions.begin(), mSessions.end(),
	                    []( const std::shared_ptr<TSocketSession>& s ) { return !s || s->IsFinished(); } ),
	    mSessions.end() );
}

bool
TDCLSocket::IsCurrentThread( void ) const noexcept
{
	return std::this_thread::get_id() == mListenThreadId.load();
}

void
TDCLSocket::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

long
TDCLSocket::GetTimeout( void )
{
	return mTimeout;
}

int
TDCLSocket::SetListenSocketOptions( dcl_socket_t inSocket )
{
	int theReuseFlag = 1;
	return ::setsockopt( inSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&theReuseFlag, sizeof( theReuseFlag ) );
}

dcl_socket_t
TDCLSocket::SocketBindAndListen( void )
{
	dcl_socket_t theSocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( theSocket == kInvalidSocket )
	{
		return kInvalidSocket;
	}

	struct sockaddr_in listenName;
	(void)::memset( &listenName, 0, sizeof( listenName ) );
	listenName.sin_family = AF_INET;
	listenName.sin_port = htons( mPort );
	listenName.sin_addr.s_addr = INADDR_ANY;

	int theResult = SetListenSocketOptions( theSocket );
	if ( theResult < 0 )
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	theResult = ::bind( theSocket, (struct sockaddr*)&listenName, sizeof( listenName ) );
	if ( theResult < 0 )
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	theResult = ::listen( theSocket, mBackLogSize );
	if ( theResult < 0 )
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	return theSocket;
}

void
TDCLSocket::DoStartListening( void )
{
	if ( mServerSocket == kInvalidSocket )
	{
		mServerSocket = SocketBindAndListen();
		if ( mServerSocket == kInvalidSocket )
		{
			int theErr = GetSocketError();
			ThrowSocketError( theErr );
			return;
		}

		try
		{
			if ( SetNonBlocking( mServerSocket, true ) != 0 )
			{
				ThrowSocketError( GetSocketError() );
			}

			int wakeErr = mWakeWait.EnsureOpen();
			if ( wakeErr != 0 )
			{
				ThrowSocketError( wakeErr );
			}

			mSocketsAreClosed = false;
		}
		catch ( ... )
		{
			mWakeWait.Close();
			CloseSocket( mServerSocket );
			mServerSocket = kInvalidSocket;
			throw;
		}
	}

	StartListenThread();
}

void
TDCLSocket::StartListenThread( void )
{
	{
		std::lock_guard<std::mutex> lock( mListenMutex );
		mRequestPending = false;
	}
	const std::stop_token token = GetStopToken();
	mListenThread = std::jthread( [this, token]( std::stop_token ) { ListenWorkerEntry( token ); } );
}

void
TDCLSocket::ClearRequestPending( void )
{
	{
		std::lock_guard<std::mutex> lock( mListenMutex );
		mRequestPending = false;
	}
	mListenCV.notify_all();
}

void
TDCLSocket::DoStopListening( void )
{
	mWakeWait.Trigger();
	{
		std::lock_guard<std::mutex> lock( mListenMutex );
		mRequestPending = false;
	}
	mListenCV.notify_all();
	if ( mListenThread.joinable() && !IsCurrentThread() )
	{
		mListenThread.join();
	}
}

void
TDCLSocket::ListenWorkerEntry( std::stop_token inStopToken )
{
	mListenThreadId.store( std::this_thread::get_id() );
	try
	{
		while ( !inStopToken.stop_requested() )
		{
			{
				std::unique_lock<std::mutex> lock( mListenMutex );
				mListenCV.wait( lock, [this, &inStopToken]() {
					return !mRequestPending || inStopToken.stop_requested();
				} );
			}
			if ( inStopToken.stop_requested() )
			{
				break;
			}
			if ( WaitForIncomingRequest() )
			{
				{
					std::lock_guard<std::mutex> lock( mListenMutex );
					mRequestPending = true;
				}
				if ( TDCLServer* srv = GetServer() )
				{
					srv->RequestPresent( this );
				}
			}
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
	mListenThreadId.store( std::thread::id() );
}

Boolean
TDCLSocket::WaitForIncomingRequest( void )
{
	int theErr = 0;
	TDCLWakeableWait::EResult theRes = mWakeWait.WaitReady(
	    (dcl_wait_handle_t)mServerSocket,
	    TDCLWakeableWait::EDirection::kRead,
	    std::chrono::milliseconds( -1 ),
	    GetStopToken(),
	    &theErr );

	if ( theRes == TDCLWakeableWait::EResult::kReady )
	{
		return true;
	}

	if ( theRes == TDCLWakeableWait::EResult::kError )
	{
		ThrowSelectError( theErr );
	}

	return false;
}

TDCLSessionId
TDCLSocket::CreateSessionFromSocket( dcl_socket_t inSocket )
{
	auto bus = GetSessionBus();
	if ( !bus )
	{
		CloseSocket( inSocket );
		return {};
	}

	auto session = std::make_shared<TSocketSession>(
	    inSocket,
	    GetSocketOps(),
	    std::move( bus ),
	    mActiveSessionCount );
	const TDCLSessionId id = session->Start();

	std::lock_guard<std::mutex> lock( mSessionsMutex );
	PruneFinishedSessionsLocked();
	mSessions.push_back( std::move( session ) );
	return id;
}

TDCLSessionId
TDCLSocket::AcceptSession( void )
{
	auto clearPending = [this]() {
		{
			std::lock_guard<std::mutex> lock( mListenMutex );
			mRequestPending = false;
		}
		mListenCV.notify_all();
	};

	if ( mServerSocket == kInvalidSocket )
	{
		clearPending();
		return {};
	}

	dcl_socket_t theSocket = kInvalidSocket;
	for ( ;; )
	{
		struct sockaddr_in clientAddress;
		socklen_t nameSize = sizeof( clientAddress );
		theSocket = ::accept( mServerSocket, (struct sockaddr*)&clientAddress, &nameSize );

		if ( theSocket != kInvalidSocket )
		{
			break;
		}

		int theErr = GetSocketError();
		if ( IsInterruptedError( theErr ) )
		{
			continue;
		}

		if ( IsWouldBlockError( theErr ) || IsAbortedAcceptError( theErr ) )
		{
			clearPending();
			return {};
		}

		clearPending();
		ThrowSocketError( theErr );
		return {};
	}

	clearPending();

	KUInt32 theMaxPipeCount = GetMaxPipeCount();
	if ( ( theMaxPipeCount == 0 ) || ( mActiveSessionCount->load( std::memory_order_relaxed ) < theMaxPipeCount ) )
	{
		int theTCPFlag = 1;
		int theResult =
		    ::setsockopt( theSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&theTCPFlag, sizeof( theTCPFlag ) );
		if ( theResult >= 0 )
		{
			theResult = SetNonBlocking( theSocket, true );
		}

		if ( theResult < 0 )
		{
			int theErr = GetSocketError();
			CloseSocket( theSocket );
			ThrowSocketError( theErr );
		}

		return CreateSessionFromSocket( theSocket );
	}

	CloseSocket( theSocket );
	return {};
}

void
TDCLSocket::Refuse( void )
{
	if ( mServerSocket != kInvalidSocket )
	{
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof( clientAddress );
	dcl_socket_t theSocket = ::accept( mServerSocket, (struct sockaddr*)&clientAddress, &nameSize );

	if ( theSocket != kInvalidSocket )
	{
		CloseSocket( theSocket );
	}
	}

	{
		std::lock_guard<std::mutex> lock( mListenMutex );
		mRequestPending = false;
	}
	mListenCV.notify_all();
}
