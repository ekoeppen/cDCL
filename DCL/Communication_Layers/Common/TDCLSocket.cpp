// ==============================
// Fichier:			TDCLSocket.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			29/3/2003
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
// The Original Code is TDCLSocket.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLSocket.cp,v 1.11 2004/11/24 18:10:56 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>

#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if !TARGET_OS_WIN32
#include <netinet/tcp.h>
#include <unistd.h>
#endif

#include <K/Tests/KDebug.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

TDCLSocket::TDCLSocket( IDCLThreads* inThreadsIntf, unsigned short inPort /* = kIANAAssignedPort */,
                        KUInt32 inMaxPipeCount /* = kDefaultPipeCount */, long inTimeout /* = kDefaultTimeout */ )
    : TDCLSyncCommLayer( inThreadsIntf, false /* inFlushAvailable */, inMaxPipeCount ),
      mServerSocket( kInvalidSocket ),
      mCurrentPipeCount( 0 ),
      mBackLogSize( 0 ),
      mTimeout( inTimeout ),
      mWakeWait(),
      mSocketsAreClosed( true ),
      mPort( inPort )
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
	TDCLSyncCommLayer::StopListening();
	CloseListenSockets();
}

void
TDCLSocket::Destroy( void )
{
	StopListening();
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

void
TDCLSocket::Disconnected( TDCLPipe* /* inPipe */ )
{
	if ( mCurrentPipeCount > 0 )
	{
		mCurrentPipeCount--;
	}
}

int
TDCLSocket::SetListenSocketOptions( dcl_socket_t inSocket )
{
	// Allow rebinding while old connections are in TIME_WAIT.
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
}

void
TDCLSocket::DoStopListening( void )
{
	mWakeWait.Trigger();
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

TDCLPipe*
TDCLSocket::DoAccept( void )
{
	TDCLPipe* thePipe = nil;
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
			// Interrupted: retry.
			continue;
		}

		if ( IsWouldBlockError( theErr ) || IsAbortedAcceptError( theErr ) )
		{
			// No pending connection (the client left before being
			// accepted): no connection, not an error.
			return nil;
		}

		ThrowSocketError( theErr );
		return nil;
	}

	KUInt32 theMaxPipeCount = GetMaxPipeCount();
	if ( ( theMaxPipeCount == 0 ) || ( mCurrentPipeCount < theMaxPipeCount ) )
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

		thePipe = new TDCLBufferedPipe( CreateSocketPipe( theSocket, mTimeout ) );
		mCurrentPipeCount++;
	}
	else
	{
		CloseSocket( theSocket );
	}

	return thePipe;
}

void
TDCLSocket::DoRefuse( void )
{
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof( clientAddress );
	dcl_socket_t theSocket = ::accept( mServerSocket, (struct sockaddr*)&clientAddress, &nameSize );

	if ( theSocket != kInvalidSocket )
	{
		CloseSocket( theSocket );
	}
}

TDCLPipe*
TDCLSocket::CreateSocketPipe( dcl_socket_t inSocket, long inTimeout )
{
	return new TSocketPipe( this, inSocket, inTimeout );
}

// ------------------------------------------------------------------------- //
// TSocketPipe Implementation
// ------------------------------------------------------------------------- //

// Pipes may outlive their communication layer after disconnecting.
static void
ClosePipeSocket( dcl_socket_t inSocket )
{
#if TARGET_OS_WIN32
	(void)::closesocket( inSocket );
#else
	(void)::close( inSocket );
#endif
}

TDCLSocket::TSocketPipe::TSocketPipe( TDCLSocket* inCommLayer, dcl_socket_t inClientSocket, long inTimeout )
    : TSyncPipe( inCommLayer->GetThreadsIntf(), inCommLayer ),
      mSocketCommLayer( inCommLayer ),
      mClientSocket( inClientSocket ),
      mTimeout( inTimeout ),
      mDataWakeWait(),
      mIOWakeWait(),
      mClientSocketIsClosed( false )
{
	SetTimeout( inTimeout );

	if ( mSocketCommLayer->SetNonBlocking( mClientSocket, true ) != 0 )
	{
		mSocketCommLayer->ThrowSocketError( mSocketCommLayer->GetSocketError() );
	}
}

TDCLSocket::TSocketPipe::~TSocketPipe( void )
{
	if ( !mClientSocketIsClosed )
	{
		if ( mClientSocket != kInvalidSocket )
		{
			(void)Disconnect();
		}
	}

	mDataWakeWait.Close();
	mIOWakeWait.Close();
}

void
TDCLSocket::TSocketPipe::Disconnect( void )
{
	// Keep the client socket valid until the worker has left WaitReady().
	TSyncPipe::Disconnect();

	if ( mClientSocket != kInvalidSocket )
	{
		ClosePipeSocket( mClientSocket );
		mClientSocket = kInvalidSocket;
	}
	mDataWakeWait.Close();
	mIOWakeWait.Close();
	mClientSocketIsClosed = true;
}

Boolean
TDCLSocket::TSocketPipe::BytesAvailable( void )
{
	if ( mClientSocket == kInvalidSocket )
	{
		return false;
	}

	int theErr = 0;
	TDCLWakeableWait::EResult theRes = TDCLWakeableWait::PollNow(
	    (dcl_wait_handle_t)mClientSocket,
	    TDCLWakeableWait::EDirection::kRead,
	    &theErr );

	if ( theRes == TDCLWakeableWait::EResult::kError )
	{
		mSocketCommLayer->ThrowSelectError( theErr );
	}

	return ( theRes == TDCLWakeableWait::EResult::kReady );
}

long
TDCLSocket::TSocketPipe::GetTimeout( void )
{
	return mTimeout;
}

void
TDCLSocket::TSocketPipe::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

Boolean
TDCLSocket::TSocketPipe::WaitForIncomingData( void )
{
	int theErr = 0;
	TDCLWakeableWait::EResult theRes = mDataWakeWait.WaitReady(
	    (dcl_wait_handle_t)mClientSocket,
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
		mSocketCommLayer->ThrowSelectError( theErr );
	}

	return false;
}

void
TDCLSocket::TSocketPipe::DoDisconnect( void )
{
	mDataWakeWait.Trigger();
	mIOWakeWait.Trigger();
}

void
TDCLSocket::TSocketPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	size_t toRead = (size_t)*ioCount;
	char* theBuffer = (char*)outBuffer;
	long timeoutInSecs = mTimeout;
	if ( timeoutInSecs == kDefaultTimeout )
	{
		timeoutInSecs = kSocketDefaultTimeout;
	}

	const bool hasTimeout = ( timeoutInSecs != kNoTimeout );
	auto deadline = hasTimeout
	    ? ( std::chrono::steady_clock::now() + std::chrono::seconds( timeoutInSecs ) )
	    : std::chrono::steady_clock::time_point{};

	while ( toRead > 0 )
	{
		if ( StopRequested() )
		{
			*ioCount -= (KUInt32)toRead;
			throw DCLUserCancelled;
		}

#if TARGET_OS_WIN32
		int chunkSize = (int)std::min<size_t>( toRead, (size_t)INT_MAX );
		int readBytes = ::recv( mClientSocket, theBuffer, chunkSize, 0 );
#else
		ssize_t readBytes = ::recv( mClientSocket, theBuffer, toRead, 0 );
#endif
		if ( readBytes > 0 )
		{
			toRead -= (size_t)readBytes;
			theBuffer += readBytes;
			if ( hasTimeout )
			{
				deadline = std::chrono::steady_clock::now() + std::chrono::seconds( timeoutInSecs );
			}
			continue;
		}

		if ( readBytes == 0 )
		{
			*ioCount -= (KUInt32)toRead;
			break;
		}

		int theErr = mSocketCommLayer->GetSocketError();
		if ( !mSocketCommLayer->IsWouldBlockError( theErr ) )
		{
			try
			{
				mSocketCommLayer->ThrowRecvError( theErr );
			}
			catch ( ... )
			{
				*ioCount -= (KUInt32)toRead;
				throw;
			}
		}

		std::chrono::milliseconds waitTimeout( -1 );
		if ( hasTimeout )
		{
			auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
			    deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				*ioCount -= (KUInt32)toRead;
				throw DCLTimeout;
			}
			waitTimeout = remaining;
		}

		int waitErr = 0;
		TDCLWakeableWait::EResult waitRes = mIOWakeWait.WaitReady(
		    (dcl_wait_handle_t)mClientSocket,
		    TDCLWakeableWait::EDirection::kRead,
		    waitTimeout,
		    GetStopToken(),
		    &waitErr );

		if ( waitRes == TDCLWakeableWait::EResult::kCancelled )
		{
			*ioCount -= (KUInt32)toRead;
			throw DCLUserCancelled;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kTimeout )
		{
			*ioCount -= (KUInt32)toRead;
			throw DCLTimeout;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kError )
		{
			try
			{
				mSocketCommLayer->ThrowSelectError( waitErr );
			}
			catch ( ... )
			{
				*ioCount -= (KUInt32)toRead;
				throw;
			}
		}
	}
}

void
TDCLSocket::TSocketPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	size_t toWrite = (size_t)*ioCount;
	const char* theBuffer = (const char*)inBuffer;
	long timeoutInSecs = mTimeout;
	if ( timeoutInSecs == kDefaultTimeout )
	{
		timeoutInSecs = kSocketDefaultTimeout;
	}

	const bool hasTimeout = ( timeoutInSecs != kNoTimeout );
	auto deadline = hasTimeout
	    ? ( std::chrono::steady_clock::now() + std::chrono::seconds( timeoutInSecs ) )
	    : std::chrono::steady_clock::time_point{};

	while ( toWrite > 0 )
	{
		if ( StopRequested() )
		{
			*ioCount -= (KUInt32)toWrite;
			throw DCLUserCancelled;
		}

#if TARGET_OS_WIN32
		int chunkSize = (int)std::min<size_t>( toWrite, (size_t)INT_MAX );
		int sentBytes = ::send( mClientSocket, theBuffer, chunkSize, 0 );
#else
		ssize_t sentBytes = ::send( mClientSocket, theBuffer, toWrite, 0 );
#endif
		if ( sentBytes > 0 )
		{
			toWrite -= (size_t)sentBytes;
			theBuffer += sentBytes;
			if ( hasTimeout )
			{
				deadline = std::chrono::steady_clock::now() + std::chrono::seconds( timeoutInSecs );
			}
			continue;
		}

		if ( sentBytes == 0 )
		{
			*ioCount -= (KUInt32)toWrite;
			throw DCLEOF;
		}

		int theErr = mSocketCommLayer->GetSocketError();
		if ( !mSocketCommLayer->IsWouldBlockError( theErr ) )
		{
			try
			{
				mSocketCommLayer->ThrowSendError( theErr );
			}
			catch ( ... )
			{
				*ioCount -= (KUInt32)toWrite;
				throw;
			}
		}

		std::chrono::milliseconds waitTimeout( -1 );
		if ( hasTimeout )
		{
			auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
			    deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				*ioCount -= (KUInt32)toWrite;
				throw DCLTimeout;
			}
			waitTimeout = remaining;
		}

		int waitErr = 0;
		TDCLWakeableWait::EResult waitRes = mIOWakeWait.WaitReady(
		    (dcl_wait_handle_t)mClientSocket,
		    TDCLWakeableWait::EDirection::kWrite,
		    waitTimeout,
		    GetStopToken(),
		    &waitErr );

		if ( waitRes == TDCLWakeableWait::EResult::kCancelled )
		{
			*ioCount -= (KUInt32)toWrite;
			throw DCLUserCancelled;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kTimeout )
		{
			*ioCount -= (KUInt32)toWrite;
			throw DCLTimeout;
		}
		if ( waitRes == TDCLWakeableWait::EResult::kError )
		{
			try
			{
				mSocketCommLayer->ThrowSelectError( waitErr );
			}
			catch ( ... )
			{
				*ioCount -= (KUInt32)toWrite;
				throw;
			}
		}
	}
}
