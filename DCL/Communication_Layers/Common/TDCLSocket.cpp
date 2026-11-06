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
// The Original Code is TDCLSocket.cp.
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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#if !TARGET_OS_WIN32
#include <netinet/tcp.h>
#include <sys/time.h>
#endif

#include <K/Tests/KDebug.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

TDCLSocket::TDCLSocket(
		IDCLThreads*	inThreadsIntf,
		unsigned short	inPort /* = kIANAAssignedPort */,
		KUInt32			inMaxPipeCount /* = kDefaultPipeCount */,
		long			inTimeout /* = kDefaultTimeout */ )
	:
		TDCLSyncCommLayer(
				inThreadsIntf,
				false /* inFlushAvailable */,
				inMaxPipeCount ),
		mServerSocket( kInvalidSocket ),
		mCurrentPipeCount( 0 ),
		mBackLogSize( 0 ),
		mTimeout( inTimeout ),
		mPublicPairMember( kInvalidSocket ),
		mPrivatePairMember( kInvalidSocket ),
		mSocketsAreClosed( true ),
		mPort( inPort )
{
	if ((inMaxPipeCount == 0) || (inMaxPipeCount > kMaxSocketBackLog))
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
TDCLSocket::Destroy( void )
{
	StopListening();

	if (!mSocketsAreClosed)
	{
		if (mServerSocket != kInvalidSocket)
		{
			CloseSocket( mServerSocket );
			mServerSocket = kInvalidSocket;
		}

		if (mPublicPairMember != kInvalidSocket)
		{
			CloseSocket( mPublicPairMember );
			mPublicPairMember = kInvalidSocket;
		}

		if (mPrivatePairMember != kInvalidSocket)
		{
			CloseSocket( mPrivatePairMember );
			mPrivatePairMember = kInvalidSocket;
		}

		mSocketsAreClosed = true;
	}
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
	if (mCurrentPipeCount > 0)
	{
		mCurrentPipeCount--;
	}
}

dcl_socket_t
TDCLSocket::SocketBindAndListen( void )
{
	dcl_socket_t theSocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (theSocket == kInvalidSocket)
	{
		return kInvalidSocket;
	}

	struct sockaddr_in listenName;
	(void) ::memset( &listenName, 0, sizeof(listenName) );
	listenName.sin_family = AF_INET;
	listenName.sin_port = htons( mPort );
	listenName.sin_addr.s_addr = INADDR_ANY;

	int theReuseFlag = 1;
	int theResult = ::setsockopt(
				theSocket,
				SOL_SOCKET,
				SO_REUSEADDR,
				(const char*) &theReuseFlag,
				sizeof(theReuseFlag) );
	if (theResult < 0)
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	theResult = ::bind(
				theSocket,
				(struct sockaddr*) &listenName,
				sizeof(listenName) );
	if (theResult < 0)
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	theResult = ::listen( theSocket, mBackLogSize );
	if (theResult < 0)
	{
		CloseSocket( theSocket );
		return kInvalidSocket;
	}

	return theSocket;
}

void
TDCLSocket::DoStartListening( void )
{
	if (mServerSocket == kInvalidSocket)
	{
		mServerSocket = SocketBindAndListen();
		if (mServerSocket == kInvalidSocket)
		{
			int theErr = GetSocketError();
			ThrowSocketError( theErr );
			return;
		}

		dcl_socket_t theSocketPair[2];
		if (MakeSocketPair( theSocketPair ) != 0)
		{
			int theErr = GetSocketError();
			ThrowSocketError( theErr );
		}

		mPublicPairMember = theSocketPair[0];
		mPrivatePairMember = theSocketPair[1];

		if (SetNonBlocking( mPrivatePairMember, true ) != 0)
		{
			int theErr = GetSocketError();
			ThrowSocketError( theErr );
		}

		mSocketsAreClosed = false;
	}
}

void
TDCLSocket::DoStopListening( void )
{
	if (mPublicPairMember != kInvalidSocket)
	{
		char someByte = 0;
		(void) ::send(
					mPublicPairMember,
					&someByte,
					sizeof(someByte),
					0 );
	}
}

Boolean
TDCLSocket::WaitForIncomingRequest( void )
{
	Boolean theResult = false;
	fd_set socketSet;
	FD_ZERO( &socketSet );
	FD_SET( mServerSocket, &socketSet );
	FD_SET( mPrivatePairMember, &socketSet );

	int maxSocketNb = (int)(mServerSocket > mPrivatePairMember ? mServerSocket : mPrivatePairMember);
	int readySockets = ::select( maxSocketNb + 1, &socketSet, NULL, NULL, NULL );

	if (readySockets == -1)
	{
		int theErr = GetSocketError();
		ThrowSelectError( theErr );
	}
	else if (readySockets > 0)
	{
		if (FD_ISSET( mPrivatePairMember, &socketSet ))
		{
			char someByte;
			while (::recv( mPrivatePairMember, &someByte, sizeof(someByte), 0 ) > 0)
			{
			}

			if (mServerSocket != kInvalidSocket)
			{
				CloseSocket( mServerSocket );
				mServerSocket = kInvalidSocket;
			}

			if (mPublicPairMember != kInvalidSocket)
			{
				CloseSocket( mPublicPairMember );
				mPublicPairMember = kInvalidSocket;
			}

			if (mPrivatePairMember != kInvalidSocket)
			{
				CloseSocket( mPrivatePairMember );
				mPrivatePairMember = kInvalidSocket;
			}

			mSocketsAreClosed = true;
		}
		else
		{
			theResult = true;
		}
	}

	return theResult;
}

TDCLPipe*
TDCLSocket::DoAccept( void )
{
	TDCLPipe* thePipe = nil;
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof(clientAddress);
	dcl_socket_t theSocket = ::accept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );

	if (theSocket == kInvalidSocket)
	{
		int theErr = GetSocketError();
		ThrowSocketError( theErr );
	}
	else
	{
		KUInt32 theMaxPipeCount = GetMaxPipeCount();
		if ((theMaxPipeCount == 0) || (mCurrentPipeCount < theMaxPipeCount))
		{
			int theTCPFlag = 1;
			int theResult = ::setsockopt(
								theSocket,
								IPPROTO_TCP,
								TCP_NODELAY,
								(const char*) &theTCPFlag,
								sizeof(theTCPFlag) );
			if (theResult >= 0)
			{
				theResult = SetNonBlocking( theSocket, false );
			}

			if (theResult < 0)
			{
				int theErr = GetSocketError();
				CloseSocket( theSocket );
				ThrowSocketError( theErr );
			}

			thePipe = new TDCLBufferedPipe(
						CreateSocketPipe( theSocket, mTimeout ) );
			mCurrentPipeCount++;
		}
		else
		{
			CloseSocket( theSocket );
		}
	}

	return thePipe;
}

void
TDCLSocket::DoRefuse( void )
{
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof(clientAddress);
	dcl_socket_t theSocket = ::accept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );

	if (theSocket != kInvalidSocket)
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

TDCLSocket::TSocketPipe::TSocketPipe(
		TDCLSocket*		inCommLayer,
		dcl_socket_t	inClientSocket,
		long			inTimeout )
	:
		TSyncPipe( inCommLayer->GetThreadsIntf(), inCommLayer ),
		mSocketCommLayer( inCommLayer ),
		mClientSocket( inClientSocket ),
		mTimeout( inTimeout ),
		mPublicPairMember( kInvalidSocket ),
		mPrivatePairMember( kInvalidSocket ),
		mClientSocketIsClosed( false )
{
	SetTimeout( inTimeout );

	dcl_socket_t theSocketPair[2];
	if (mSocketCommLayer->MakeSocketPair( theSocketPair ) != 0)
	{
		mSocketCommLayer->ThrowSocketError( mSocketCommLayer->GetSocketError() );
	}
	mPublicPairMember = theSocketPair[0];
	mPrivatePairMember = theSocketPair[1];

	if (mSocketCommLayer->SetNonBlocking( mPrivatePairMember, true ) != 0)
	{
		mSocketCommLayer->ThrowSocketError( mSocketCommLayer->GetSocketError() );
	}
}

TDCLSocket::TSocketPipe::~TSocketPipe( void )
{
	if (!mClientSocketIsClosed)
	{
		if (mClientSocket != kInvalidSocket)
		{
			(void) Disconnect();
		}
	}

	if (mPublicPairMember != kInvalidSocket)
	{
		mSocketCommLayer->CloseSocket( mPublicPairMember );
		mPublicPairMember = kInvalidSocket;
	}

	if (mPrivatePairMember != kInvalidSocket)
	{
		mSocketCommLayer->CloseSocket( mPrivatePairMember );
		mPrivatePairMember = kInvalidSocket;
	}
}

Boolean
TDCLSocket::TSocketPipe::BytesAvailable( void )
{
	fd_set socketSet;
	FD_ZERO( &socketSet );
	FD_SET( mClientSocket, &socketSet );

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int readySockets = ::select( (int) mClientSocket + 1, &socketSet, NULL, NULL, &timeout );
	if (readySockets == -1)
	{
		int theErr = mSocketCommLayer->GetSocketError();
		mSocketCommLayer->ThrowSelectError( theErr );
	}

	return (readySockets > 0);
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
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kSocketDefaultTimeout;
	}

	int theErr = mSocketCommLayer->SetSocketTimeout( mClientSocket, timeoutInSecs );
	if (theErr < 0)
	{
		mSocketCommLayer->ThrowTimeoutError( mSocketCommLayer->GetSocketError() );
	}
}

Boolean
TDCLSocket::TSocketPipe::WaitForIncomingData( void )
{
	Boolean theResult = false;
	fd_set socketSet;
	FD_ZERO( &socketSet );
	FD_SET( mClientSocket, &socketSet );
	FD_SET( mPrivatePairMember, &socketSet );

	int maxSocketNb = (int)(mClientSocket > mPrivatePairMember ? mClientSocket : mPrivatePairMember);
	int readySockets = ::select( maxSocketNb + 1, &socketSet, NULL, NULL, NULL );

	if (readySockets == -1)
	{
		int theErr = mSocketCommLayer->GetSocketError();
		mSocketCommLayer->ThrowSelectError( theErr );
	}
	else if (readySockets > 0)
	{
		if (FD_ISSET( mPrivatePairMember, &socketSet ))
		{
			char someByte;
			while (::recv( mPrivatePairMember, &someByte, sizeof(someByte), 0 ) > 0)
			{
			}
		}
		else
		{
			theResult = true;
		}
	}

	return theResult;
}

void
TDCLSocket::TSocketPipe::DoDisconnect( void )
{
	if (mPublicPairMember != kInvalidSocket)
	{
		char someByte = 0;
		(void) ::send(
					mPublicPairMember,
					&someByte,
					sizeof(someByte),
					0 );
	}

	if (mClientSocket != kInvalidSocket)
	{
		mSocketCommLayer->CloseSocket( mClientSocket );
		mClientSocket = kInvalidSocket;
	}

	mClientSocketIsClosed = true;
}

void
TDCLSocket::TSocketPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	size_t toRead = (size_t) *ioCount;
	char* theBuffer = (char*) outBuffer;
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kSocketDefaultTimeout;
	}
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);

	while (toRead > 0)
	{
		int readBytes = ::recv( mClientSocket, theBuffer, toRead, 0 );
		if (readBytes < 0)
		{
			int theErr = mSocketCommLayer->GetSocketError();
			try
			{
				mSocketCommLayer->ThrowRecvError( theErr );
			}
			catch ( ... )
			{
				*ioCount -= toRead;
				throw;
			}

			if ((timeoutInSecs != kNoTimeout) &&
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				*ioCount -= toRead;
				throw DCLTimeout;
			}
		}
		else
		{
			if ((timeoutInSecs != kNoTimeout) &&
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				*ioCount -= toRead;
				throw DCLTimeout;
			}

			if (readBytes == 0)
			{
				*ioCount -= toRead;
				break;
			}

			lastOperation = ::time(NULL);
			toRead -= readBytes;
			theBuffer += readBytes;
		}
		theThreadsIntf->Yield();
	}
}

void
TDCLSocket::TSocketPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	size_t toWrite = (size_t) *ioCount;
	const char* theBuffer = (const char*) inBuffer;
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kSocketDefaultTimeout;
	}
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);

	while (toWrite > 0)
	{
		int sentBytes = ::send( mClientSocket, theBuffer, toWrite, 0 );
		if (sentBytes < 0)
		{
			int theErr = mSocketCommLayer->GetSocketError();
			try
			{
				mSocketCommLayer->ThrowSendError( theErr );
			}
			catch ( ... )
			{
				*ioCount -= toWrite;
				throw;
			}

			if ((timeoutInSecs != kNoTimeout) &&
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				*ioCount -= toWrite;
				throw DCLTimeout;
			}
		}
		else
		{
			if ((timeoutInSecs != kNoTimeout) &&
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				*ioCount -= toWrite;
				throw DCLTimeout;
			}

			if (sentBytes == 0)
			{
				*ioCount -= toWrite;
				throw DCLEOF;
			}

			lastOperation = ::time(NULL);
			toWrite -= sentBytes;
			theBuffer += sentBytes;
		}
		theThreadsIntf->Yield();
	}
}