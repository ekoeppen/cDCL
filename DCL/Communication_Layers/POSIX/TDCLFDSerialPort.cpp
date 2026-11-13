// ==============================
// Fichier:			TDCLFDSerialPort.cp
// Projet:			Desktop Connection Library
//
// Créé le:			4/4/2003
// Tabulation:		4 espaces
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
// The Original Code is TDCLFDSerialPort.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Simon Stapleton <simon@tufty.co.uk>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLFDSerialPort.cp,v 1.9 2006/02/13 01:55:30 pguyot Exp $
// ===========

// C++
#include <chrono>

// POSIX & ANSI
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>

// DCL
#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Communication_Layers/TDCLHammerPipe.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

// -------------------------------------------------------------------------- //
//  * TDCLFDSerialPort( IDCLThreads*, const char*, long, Boolean )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud /* = 38400 */,
				Boolean inUseMNP /* = true */ )
	:
		TDCLSyncCommLayer( inThreadsIntf, true ),
		mIsConnected( false ),
		mSerialFD( -1 ),
		mDevPath( nil ),
		mBaud( inBaud ),
		mPipeMode( inUseMNP ? kModeMNP : kModePlain ),
		mReset( false ),
		mDisconnecting( false ),
		mRequestPresent( false ),
		mStateMutex(),
		mStateCV(),
		mWakeWait(),
		mTimeout( kDefaultTimeout )
{
	if (inDevPath)
	{
		Init( inDevPath );
	}
}

// -------------------------------------------------------------------------- //
//  * TDCLFDSerialPort( IDCLThreads*, const char*, long, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud,
				KUInt32 inMode )
	:
		TDCLSyncCommLayer( inThreadsIntf, true ),
		mIsConnected( false ),
		mSerialFD( -1 ),
		mDevPath( nil ),
		mBaud( inBaud ),
		mPipeMode( inMode ),
		mReset( false ),
		mDisconnecting( false ),
		mRequestPresent( false ),
		mStateMutex(),
		mStateCV(),
		mWakeWait(),
		mTimeout( kDefaultTimeout )
{
	if (inDevPath)
	{
		Init( inDevPath );
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLFDSerialPort( void )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::~TDCLFDSerialPort( void )
{
	StopListening();

	if (mSerialFD != -1)
	{
		(void)::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions );
		CloseSerialFD( mSerialFD );
		mSerialFD = -1;
	}

	if (mDevPath)
	{
		::free( mDevPath );
	}
}

// -------------------------------------------------------------------------- //
//  * Init( const char* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Init( const char* inDevPath )
{
	// Copie du nom.
	size_t theLength = ::strlen( inDevPath ) + 1;	// Taille avec terminateur.
	mDevPath = (char*) ::malloc( theLength );
	(void) ::memcpy( mDevPath, inDevPath, theLength );
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoStartListening( void )
{
	if (mSerialFD == -1)
	{
		// Ouverture et configuration du port.
		OpenAndConfigure();
	}

	int wakeErr = mWakeWait.EnsureOpen();
	if (wakeErr != 0)
	{
		(void)::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions );
		CloseSerialFD( mSerialFD );
		mSerialFD = -1;
		throw DCLPlatformUnknownError( wakeErr );
	}
}

// -------------------------------------------------------------------------- //
//  * StopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::StopListening( void )
{
	TDCLSyncCommLayer::StopListening();

	std::lock_guard<std::mutex> lock( mStateMutex );
	mRequestPresent = false;
	if ((!mIsConnected) && (mSerialFD != -1))
	{
		(void)::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions );
		CloseSerialFD( mSerialFD );
		mSerialFD = -1;
	}
	mWakeWait.Close();
}

// -------------------------------------------------------------------------- //
//  * DoStopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoStopListening( void )
{
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mRequestPresent = false;
	}
	mStateCV.notify_all();
	mWakeWait.Trigger();
}

// -------------------------------------------------------------------------- //
//  * WaitForIncomingRequest( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::WaitForIncomingRequest( void )
{
	{
		std::unique_lock<std::mutex> lock( mStateMutex );
		mStateCV.wait( lock, GetStopToken(), [this] {
			return !mRequestPresent;
		} );
		if (StopRequested())
		{
			return false;
		}
	}

	Boolean theResult = Select();

	if (theResult)
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		if (StopRequested())
		{
			return false;
		}
		mRequestPresent = true;
	}

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * Select( long )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::Select( long inTimeoutMs )
{
	for (;;)
	{
		int theFD = -1;
		{
			std::unique_lock<std::mutex> lock( mStateMutex );
			mStateCV.wait( lock, GetStopToken(), [this] {
				return !mReset;
			} );
			if (StopRequested())
			{
				return false;
			}
			theFD = mSerialFD;
		}

		if (theFD < 0)
		{
			return false;
		}

		int theErr = 0;
		TDCLWakeableWait::EResult theRes = mWakeWait.WaitReady(
			theFD,
			TDCLWakeableWait::EDirection::kRead,
			std::chrono::milliseconds( inTimeoutMs ),
			GetStopToken(),
			&theErr );

		if (theRes == TDCLWakeableWait::EResult::kReady)
		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			if (mReset)
			{
				continue;
			}
			return !StopRequested();
		}

		if (theRes == TDCLWakeableWait::EResult::kCancelled)
		{
			return false;
		}

		if (theRes == TDCLWakeableWait::EResult::kTimeout)
		{
			return false;
		}

		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			if (mReset)
			{
				continue;
			}
		}
		return false;
	}
}

// -------------------------------------------------------------------------- //
//  * DoAccept( void )
// -------------------------------------------------------------------------- //
TDCLPipe*
TDCLFDSerialPort::DoAccept( void )
{
	TDCLPipe* theResult = nil;
	
	// Si une requête est présente, on l'accepte.
	if (mRequestPresent)
	{
		mIsConnected = true;
        switch ( mPipeMode )
        {
            case kModePlain:
                theResult = new TDCLBufferedPipe( new TFDSerialPortPipe( this ) );
                break;
            case kModeMNP:
                theResult = new TDCLMNPPipe( new TFDSerialPortPipe( this ) );
                break;
            case kModeHammer:
                theResult = new TDCLHammerPipe( new TFDSerialPortPipe( this ) );
                break;
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * DoRefuse( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoRefuse( void )
{
	// On ferme et on rouvre.
	Reset();
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long 
TDCLFDSerialPort::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * Disconnected( TPipe* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Disconnected( TDCLPipe* /* inPipe */ )
{
	mIsConnected = false;

	// Select fera la réinitialisation au temps voulu.
	Reset();
}

// -------------------------------------------------------------------------- //
//  * TFDSerialPortPipe( TDCLFDSerialPort* )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TFDSerialPortPipe::TFDSerialPortPipe(
												TDCLFDSerialPort* inCommLayer )
	:
		TSyncPipe(inCommLayer->GetThreadsIntf(), inCommLayer),
		mSerialFD( inCommLayer->GetSerialFD() ),
		mTimeout( inCommLayer->GetTimeout() ),
		mDisconnected( false ),
		mCancelledBeforeDisconnect( false ),
		mDataWakeWait(),
		mIOWakeWait()
{
}

// -------------------------------------------------------------------------- //
//  * ~TFDSerialPortPipe( void )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TFDSerialPortPipe::~TFDSerialPortPipe( void ) 
{
	if(!mDisconnected)
	{
		try {
			Disconnect();
		} catch ( ... ) {
			// Silence!
		}
	}

	mDataWakeWait.Close();
	mIOWakeWait.Close();
}

// -------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	size_t toRead = (size_t) *ioCount;
	char* theBuffer = (char*) outBuffer;
	const bool hasTimeout = ( mTimeout > 0 );
	auto deadline = hasTimeout
		? ( std::chrono::steady_clock::now() + std::chrono::seconds( mTimeout ) )
		: std::chrono::steady_clock::time_point{};

	while ( toRead > 0 )
	{
		if ( StopRequested() )
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLUserCancelled;
		}

		ssize_t readBytes = ::read( mSerialFD, theBuffer, toRead );
		if ( readBytes > 0 )
		{
			toRead -= (size_t) readBytes;
			theBuffer += readBytes;
			if ( hasTimeout )
			{
				deadline = std::chrono::steady_clock::now() + std::chrono::seconds( mTimeout );
			}
			continue;
		}

		if ( readBytes == 0 )
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLEOF;
		}

		int theErr = errno;
		if ( ( theErr != EINTR ) && ( theErr != EAGAIN )
#if ( EWOULDBLOCK != EAGAIN )
			 && ( theErr != EWOULDBLOCK )
#endif
		)
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLPlatformUnknownError( theErr );
		}

		std::chrono::milliseconds waitTimeout( -1 );
		if ( hasTimeout )
		{
			auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
				deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				*ioCount -= (KUInt32) toRead;
				throw DCLTimeout;
			}
			waitTimeout = remaining;
		}

		int waitErr = 0;
		TDCLWakeableWait::EResult waitRes = mIOWakeWait.WaitReady(
			mSerialFD,
			TDCLWakeableWait::EDirection::kRead,
			waitTimeout,
			GetStopToken(),
			&waitErr );

		if ( waitRes == TDCLWakeableWait::EResult::kCancelled )
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLUserCancelled;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kTimeout )
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLTimeout;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kError )
		{
			*ioCount -= (KUInt32) toRead;
			throw DCLPlatformUnknownError( waitErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// -------------------------------------------------------------------------- //
void 
TDCLFDSerialPort::TFDSerialPortPipe::Write(
										const void* inBuffer, KUInt32* ioCount )
{
	size_t toWrite = (size_t) *ioCount;
	const char* theBuffer = (const char*) inBuffer;
	const bool hasTimeout = ( mTimeout > 0 );
	auto deadline = hasTimeout
		? ( std::chrono::steady_clock::now() + std::chrono::seconds( mTimeout ) )
		: std::chrono::steady_clock::time_point{};

	while ( toWrite > 0 )
	{
		if ( StopRequested() )
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLUserCancelled;
		}

		ssize_t sentBytes = ::write( mSerialFD, theBuffer, toWrite );
		if ( sentBytes > 0 )
		{
			toWrite -= (size_t) sentBytes;
			theBuffer += sentBytes;
			if ( hasTimeout )
			{
				deadline = std::chrono::steady_clock::now() + std::chrono::seconds( mTimeout );
			}
			continue;
		}

		if ( sentBytes == 0 )
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLUnknownError;
		}

		int theErr = errno;
		if ( ( theErr != EINTR ) && ( theErr != EAGAIN )
#if ( EWOULDBLOCK != EAGAIN )
			 && ( theErr != EWOULDBLOCK )
#endif
		)
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLPlatformUnknownError( theErr );
		}

		std::chrono::milliseconds waitTimeout( -1 );
		if ( hasTimeout )
		{
			auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
				deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				*ioCount -= (KUInt32) toWrite;
				throw DCLTimeout;
			}
			waitTimeout = remaining;
		}

		int waitErr = 0;
		TDCLWakeableWait::EResult waitRes = mIOWakeWait.WaitReady(
			mSerialFD,
			TDCLWakeableWait::EDirection::kWrite,
			waitTimeout,
			GetStopToken(),
			&waitErr );

		if ( waitRes == TDCLWakeableWait::EResult::kCancelled )
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLUserCancelled;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kTimeout )
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLTimeout;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kError )
		{
			*ioCount -= (KUInt32) toWrite;
			throw DCLPlatformUnknownError( waitErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * BytesAvailable( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::TFDSerialPortPipe::BytesAvailable( void ) 
{
	if (mSerialFD < 0)
	{
		return false;
	}

	int theErr = 0;
	TDCLWakeableWait::EResult theRes = TDCLWakeableWait::PollNow(
		mSerialFD,
		TDCLWakeableWait::EDirection::kRead,
		&theErr );

	if (theRes == TDCLWakeableWait::EResult::kError)
	{
		throw DCLPlatformNotAvailable( theErr );
	}

	return (theRes == TDCLWakeableWait::EResult::kReady);
}

// -------------------------------------------------------------------------- //
//  * WaitForIncomingData( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::TFDSerialPortPipe::WaitForIncomingData( void )
{
	int theErr = 0;
	TDCLWakeableWait::EResult theRes = mDataWakeWait.WaitReady(
		mSerialFD,
		TDCLWakeableWait::EDirection::kRead,
		std::chrono::milliseconds( -1 ),
		GetStopToken(),
		&theErr );

	return (theRes == TDCLWakeableWait::EResult::kReady);
}

// -------------------------------------------------------------------------- //
//  * Disconnect( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::Disconnect( void )
{
	mCancelledBeforeDisconnect = StopRequested();
	TSyncPipe::Disconnect();
	mDataWakeWait.Close();
	mIOWakeWait.Close();
}

// -------------------------------------------------------------------------- //
//  * DoDisconnect( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::DoDisconnect( void ) 
{
	mDataWakeWait.Trigger();
	mIOWakeWait.Trigger();

	if (!mDisconnected) 
	{
		mDisconnected = true;
		
		if (mSerialFD >= 0)
		{
			if (mCancelledBeforeDisconnect)
			{
				(void)::tcflush( mSerialFD, TCOFLUSH );
			}
			else
			{
				// Flush remaining outgoing bytes.
				if (::tcdrain(mSerialFD) == -1)
				{
					throw DCLPlatformUnknownError( errno );
				}
			}
		}
	}

	// On déconnecte au niveau de la couche de communication (qui fermera
	// derrière nous).
	((TDCLFDSerialPort*) GetCommLayer())->Disconnected(this);
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long
TDCLFDSerialPort::TFDSerialPortPipe::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// -------------------------------------------------------------------------- //
//  * CloseSerialFD( int )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::CloseSerialFD( int inFD )
{
#if	!TARGET_OS_CYGWIN
	// Exclusive mode is attached to the tty, not the fd: clear it so
	// reopening (without privileges) remains possible if the tty outlives
	// close (e.g. a pseudo-terminal whose master remains open).
	(void) ::ioctl( inFD, TIOCNXCL );
#endif
	(void) ::close( inFD );
}

// -------------------------------------------------------------------------- //
//  * OpenAndConfigure( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::OpenAndConfigure( void )
{
	// Ouverture (non bloquant, lecture et écriture).
	int theFD = ::open( mDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK );
	
	if (theFD == -1)
	{
		throw DCLPlatformUnknownError( errno );
	}
	
#if	!TARGET_OS_CYGWIN
	// Prevent further open calls with TIOCEXCL (see tty(4)).
	// Exclusive mode is cleared by CloseSerialFD before each close.
	if (::ioctl(theFD, TIOCEXCL) == -1)
	{
		int theErrno = errno;
		(void) ::close( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}
#endif

	// Configure the interface in non-blocking mode for wakeable I/O.
	int flags = ::fcntl(theFD, F_GETFL, 0 /* ignored */);
#if	TARGET_OS_CYGWIN
	flags |= O_NONBLOCK;
#else
	flags &= ~O_ASYNC;
	flags |= O_NONBLOCK;
#endif
	flags |= O_APPEND;
	int theErr = ::fcntl(theFD, F_SETFL, flags);
	if (theErr < 0)
	{
		// Fermeture.
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}
	
	// Enregistrement des options courantes pour pouvoir les remettre après.
	if (::tcgetattr(theFD, &mOriginalOptions) == -1)
	{
		// Fermeture.
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}
	
	// Nouvelles options.
	struct termios theOptions = mOriginalOptions;

	// 8 bits, pas de parité, un bit d'arrêt, pas de CTS/RTS (?)
	theOptions.c_iflag = IGNBRK | INPCK;
	theOptions.c_oflag = 0;
	theOptions.c_cflag = ((((CS8 | CREAD | HUPCL | CLOCAL) & ~PARENB) & ~PARODD) & ~CSTOPB);
	theOptions.c_lflag = 0;
	
	// Vitesse.
	// (cfsetispeed & cfsetospeed sont des macros sur certaines plateformes)
    switch(mBaud) {
        #ifdef B2400
        case 2400:
            theErr = cfsetospeed(&theOptions, B2400);
            break;
        #endif
        #ifdef B4800
        case 4800:
            theErr = cfsetospeed(&theOptions, B4800);
            break;
        #endif
        #ifdef B9600
        case 9600:
            theErr = cfsetospeed(&theOptions, B9600);
            break;
        #endif
        #ifdef B19200
        case 19200:
            theErr = cfsetospeed(&theOptions, B19200);
            break;
        #endif
        #ifdef B38400
        case 38400:
            theErr = cfsetospeed(&theOptions, B38400);
            break;
        #endif
        #ifdef B57600
        case 57600:
            theErr = cfsetospeed(&theOptions, B57600);
            break;
        #endif
        #ifdef B115200
        case 115200:
            theErr = cfsetospeed(&theOptions, B115200);
            break;
        #endif
        #ifdef B230400
        case 230400:
            theErr = cfsetospeed(&theOptions, B230400);
            break;
        #endif
        default:
			CloseSerialFD( theFD );
			throw DCLPlatformUnknownError( EINVAL );
        }

	if (theErr < 0)
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}
	
	// B0 copies the speed from cfsetospeed above
	theErr = cfsetispeed( &theOptions, B0 );
	if (theErr < 0)
	{
		int theErrno = errno;
		CloseSerialFD( theFD );
		throw DCLPlatformUnknownError( theErrno );
	}
	
	// Accès bloquant jusqu'à ce qu'un caractère soit lu ou qu'une temporisation
	// d'une seconde soit arrivée.
	theOptions.c_cc[VMIN] = 1;
	theOptions.c_cc[VTIME] = 10;

	// On règle les options.
	theErr = ::tcsetattr( theFD, TCSANOW, &theOptions );
	if (theErr < 0)
    {
		// Fermeture
		int theErrno = errno;
		CloseSerialFD( theFD );
		
		throw DCLPlatformUnknownError( theErrno );
	}
	
	// On enregistre le descripteur de fichier.
	mSerialFD = theFD;
}

// -------------------------------------------------------------------------- //
//  * Reset( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Reset( void )
{
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		mReset = true;
		mWakeWait.Trigger();

		if (mSerialFD != -1)
		{
			(void)::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions );
			CloseSerialFD( mSerialFD );
			mSerialFD = -1;
		}

		if (GetServer() && !StopRequested())
		{
			OpenAndConfigure();
		}

		mReset = false;
		mRequestPresent = false;
	}

	mStateCV.notify_all();
}

// ======================================================== //
// Blinding speed can compensate for a lot of deficiencies. //
//                 -- David Nichols                         //
// ======================================================== //
