// ==============================
// Fichier:			TDCLXADSP.cp
// Projet:			Desktop Connection Library
//
// Créé le:			29/07/2002
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
// The Original Code is TDCLXADSP.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLXADSP.cp,v 1.12 2004/11/24 18:11:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS_X/TDCLXADSP.h>

// ANSI C & POSIX
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <fcntl.h>

// AppleTalk & co. dans Kernel.Framework (dans /usr/include)
#include <netat/appletalk.h>
#include <netat/nbp.h>
#include <netat/adsp.h>

// AppleTalk dans AppleTalk.Framework
// (n'est pas dans le système mais dans Darwin)
extern "C" {
#include "at_proto.h"
}

// K
#include <K/Misc/UPStrings.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Interfaces/MacOS/UDCLMacMachineName.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>

// ------------------------------------------------------------------------- //
//  * TDCLXADSP( IDCLThreads*, const char*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLXADSP::TDCLXADSP(
			IDCLThreads*	inThreadsIntf,
			const char*		inMachineName /* = nil */,
			KUInt32			inMaxPipeCount /* = kMaxADSPPipeCount */,
			long			inTimeout )
		:
			TDCLSyncCommLayer(
						inThreadsIntf,
						false /* inFlushAvailable */,
						inMaxPipeCount ),
			mServerSocket( -1 ),
			mCurrentPipeCount( 0 ),
			mBackLogSize( 0 ),
			mTimeout( inTimeout ),
			mSocketsAreClosed( true )
{
	// Correction du nombre maximal de connexions
	if ((inMaxPipeCount == 0) || (inMaxPipeCount > kMaxADSPPipeCount))
	{
		SetMaxPipeCount( kMaxADSPPipeCount );
	}
	
	const char* theMachineName;
	
	// Récupération du nom de la machine si nécessaire.
	if (inMachineName == nil)
	{
		theMachineName = UDCLMacMachineName::GetMachineName();
	} else {
		theMachineName = inMachineName;
	}

	// Ensuite, on l'associe à l'adresse NBP kivabien.
	const char *theType = "Docker";
	const char *theZone = "*";

	int theErr = ::nbp_make_entity(
						&mMachineEntity, (char*) theMachineName,
						(char*) theType, (char*) theZone);
	
	// Nettoyage
	if (inMachineName == nil)
	{
		::free( (void*) theMachineName );
	}
	
	if (theErr < 0)
	{
		// On lance une exception.
		theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLXADSP( void )
// ------------------------------------------------------------------------- //
TDCLXADSP::~TDCLXADSP( void )
{
	StopListening();
	
	if (!mSocketsAreClosed)
	{
		if(mServerSocket >= 0)
		{
			(void) ::close( mServerSocket );
		}

		if(mPublicPairMember >= 0)
		{
			(void) ::close( mPublicPairMember );
		}

		if(mPrivatePairMember >= 0)
		{
			(void) ::close( mPrivatePairMember );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * IsAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLXADSP::IsAvailable( void )
{
	int status = ::checkATStack();

	switch(status) {
		case RUNNING:
			return true;

		case LOADED:
		case NOTLOADED:
		case OTHERERROR:
		default:
			return false;
	}
}

// ------------------------------------------------------------------------- //
//  * DoStartListening( void )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::DoStartListening( void )
{
	if (mServerSocket < 0)
	{
		int theErr;
		mServerSocket = SocketBindAndListen();
		if (mServerSocket < 0)
		{
			// On lance une exception.
			theErr = errno;
			switch (theErr)
			{
				case EDESTADDRREQ:
				case EADDRNOTAVAIL:
				case EAFNOSUPPORT:
				case EBADF:
				case ENOTSOCK:
				case EOPNOTSUPP:
				case EINVAL:
				case ENAMETOOLONG:
					throw DCLPlatformBadParamError( theErr );

				case EISCONN:				
					throw DCLPlatformBadStateError( theErr );

				case EPROTONOSUPPORT:
				case EPROTOTYPE:
					throw DCLPlatformNotAvailable( theErr );
				
				case EMFILE:
				case ENFILE:
				case ENOBUFS:
// MacOS X n'est pas conforme POSIX
#ifdef ENOSR
				case ENOSR:
#endif
					throw DCLPlatformLimitReachedError( theErr );
				
				case EADDRINUSE:
#warning handle EADDRINUSE properly
					KDEBUG( "Address is in use" );

				case EFAULT:
				case EACCES:
					throw DCLPlatformUnknownError( theErr );
			}
        }

		// Création de la paire de socket.	
		int theSocketPair[2];
		if (::socketpair( AF_UNIX, SOCK_STREAM, PF_UNSPEC, theSocketPair ) != 0)
		{
			throw DCLPlatformUnknownError( errno );
		}
		
		mPublicPairMember = theSocketPair[0];
		mPrivatePairMember = theSocketPair[1];

		// La partie privée doit être non bloquante.
		int flags = ::fcntl(mPrivatePairMember, F_GETFL, 0 /* ignored */);
		flags |= O_NONBLOCK;
		if (::fcntl(mPrivatePairMember, F_SETFL, flags) != 0)
		{
			throw DCLPlatformUnknownError( errno );
		}
		
		mSocketsAreClosed = false;

		theErr = ::nbp_register(
							&mMachineEntity,
							mServerSocket,
							NULL );
		
		if (theErr < 0)
		{
			theErr = errno;
			throw DCLPlatformUnknownError( theErr );
		}
	}	// Sinon, on ne fait rien
}

// ------------------------------------------------------------------------- //
//  * DoStopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::DoStopListening( void )
{
	if ( mServerSocket >= 0 )
	{
		// Nicolas nous raconte: fd is not currently used
		// Je suppose que ça veut dire qu'on s'en fout de mServerSocket
		
		// Ici, l'erreur n'a pas tellement d'importance.
		(void) ::nbp_remove( &mMachineEntity, mServerSocket );
	}

	if (mPublicPairMember >= 0)
	{
		// On écrit dans la paire pour sortir.
		char someByte = 0;
		(void) ::send(
					mPublicPairMember,
					&someByte,
					sizeof( someByte ),
					0 /* flags */ );
	}

	// La fermeture se fera dans WaitForIncomingRequest.
}

// ------------------------------------------------------------------------- //
//  * SocketBindAndListen( void )
// ------------------------------------------------------------------------- //
int
TDCLXADSP::SocketBindAndListen( void )
{
	int theSocket;
	do {
        theSocket = ::ADSPsocket( AF_APPLETALK, SOCK_STREAM, 0 );

		if (theSocket < 0)
		{
			break;
		}

		int theResult = ::ADSPbind( theSocket, NULL, 0 );
		if (theResult < 0)
        {
			// Fermeture de l'interface de communication.
			(void) ::ADSPclose(theSocket);
			theSocket = theResult;
			break;
        }
		
		// The argument backlog specifies the maximum length of the queue of
		// pending connections. With the current implementation of the ADSP
		// protocol module, this queue length is always 1 so the backlog
		// value has no effect.
		
		// (on met quand même inBacklogSize, au cas où la doc serait en
		// retard).
		theResult = ::ADSPlisten( theSocket, (int) mBackLogSize );
		if (theResult < 0)
        {
			// Fermeture de l'interface de communication.
			(void) ::ADSPclose(theSocket);	
			theSocket = theResult;
			break;
        }
	} while ( false );

	return theSocket;
}

// ------------------------------------------------------------------------- //
//  * WaitForIncomingRequest( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLXADSP::WaitForIncomingRequest( void )
{
	Boolean theResult = false;
	
	// Petit select pour attendre.
	fd_set socketSet;
	FD_ZERO( &socketSet );
	FD_SET( mServerSocket, &socketSet );
	FD_SET( mPrivatePairMember, &socketSet );
	int maxSocketNb =
		mServerSocket > mPrivatePairMember
			? mServerSocket : mPrivatePairMember;
	int readySockets = ::select(
							maxSocketNb + 1, &socketSet, NULL, NULL, NULL );

	if (readySockets == -1)
	{
		// Erreur.
		int theErr = errno;

		switch (theErr)
		{
			case EINVAL:
				throw DCLPlatformBadParamError( theErr );
			
			case EBADF:
			case EINTR:
				break;
			
			default:
				throw DCLPlatformUnknownError( theErr );
 		}
	} else if (readySockets > 0) {
		if (FD_ISSET( mPrivatePairMember, &socketSet ))
		{
			// On vide la paire.
			char someByte;
			while (
				::recv(
					mPrivatePairMember,
					&someByte,
					sizeof(someByte), 0 ) >= 0)
			{
				// This space for rent.
			};

			// Puis on ferme.
			if(mServerSocket >= 0)
			{
				(void) ::ADSPclose( mServerSocket );
			}

			if(mPublicPairMember >= 0)
			{
				(void) ::close( mPublicPairMember );
			}

			if(mPrivatePairMember >= 0)
			{
				(void) ::close( mPrivatePairMember );
			}
			
			mSocketsAreClosed = true;
		} else {
			theResult = true;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * DoAccept( void )
// ------------------------------------------------------------------------- //
TDCLPipe*
TDCLXADSP::DoAccept( void )
{
	TDCLPipe* thePipe = nil;
	
	int nameSize = sizeof(at_inet_t);
	at_inet_t clientAddress;

	int theADSPSocket = ::ADSPaccept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );
		
	if (theADSPSocket < 0)
	{
		int theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	} else {
		KUInt32 theMaxPipeCount = GetMaxPipeCount();
		if ((theMaxPipeCount == 0) || (mCurrentPipeCount < theMaxPipeCount))
		{
			thePipe = new TDCLBufferedPipe(
					new TXADSPPipe(
							this,
							theADSPSocket,
							mTimeout ) );

			// Incrément du compteur
			mCurrentPipeCount++;			
		} else {
			if (::ADSPclose( theADSPSocket ) < 0)
			{
				int theErr = errno;
				throw DCLPlatformUnknownError( theErr );
			}
		}
	}

	return thePipe;
}

// ------------------------------------------------------------------------- //
//  * DoRefuse( void )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::DoRefuse( void )
{
	int nameSize = sizeof(at_inet_t);
	at_inet_t clientAddress;

	int theADSPSocket = ::ADSPaccept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );
	
	if (theADSPSocket < 0)
	{
		int theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	} else {
		if (::ADSPclose( theADSPSocket ) < 0)
		{
			int theErr = errno;
			throw DCLPlatformUnknownError( theErr );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * SetTimeout( long )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// ------------------------------------------------------------------------- //
//  * GetTimeout( void )
// ------------------------------------------------------------------------- //
long 
TDCLXADSP::GetTimeout( void )
{
	return mTimeout;
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TPipe* )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::Disconnected( TDCLPipe* /* inPipe */ )
{
	mCurrentPipeCount--;
}

// ------------------------------------------------------------------------- //
//  * TXADSPPipe( TDCLXADSP*, int, long )
// ------------------------------------------------------------------------- //
TDCLXADSP::TXADSPPipe::TXADSPPipe(
				TDCLXADSP* inCommLayer,
				int inClientSocket,
				long inTimeout )
	:
		TSyncPipe( inCommLayer->GetThreadsIntf(), inCommLayer ),
		mClientSocket( inClientSocket ),
		mTimeout( inTimeout ),
		mPublicPairMember( -1 ),
		mPrivatePairMember( -1 ),
		mClientSocketIsClosed( true ),
		mHasNextByte( false )
{
	SetTimeout( inTimeout );

	// Création de la paire de socket.	
	int theSocketPair[2];
	if (::socketpair( AF_UNIX, SOCK_STREAM, PF_UNSPEC, theSocketPair ) != 0)
	{
		throw DCLPlatformUnknownError( errno );
	}
	
	mPublicPairMember = theSocketPair[0];
	mPrivatePairMember = theSocketPair[1];

	// La partie privée doit être non bloquante.
	int flags = ::fcntl(mPrivatePairMember, F_GETFL, 0 /* ignored */);
	flags |= O_NONBLOCK;
	if (::fcntl(mPrivatePairMember, F_SETFL, flags) != 0)
	{
		throw DCLPlatformUnknownError( errno );
	}
}

// ------------------------------------------------------------------------- //
//  * ~TXADSPPipe( void )
// ------------------------------------------------------------------------- //
TDCLXADSP::TXADSPPipe::~TXADSPPipe( void ) 
{
	if (!mClientSocketIsClosed)
	{
		if(mClientSocket >= 0)
		{
			(void) Disconnect();
		}
	}

	if(mPublicPairMember >= 0)
	{
		(void) ::close( mPublicPairMember );
	}

	if(mPrivatePairMember >= 0)
	{
		(void) ::close( mPrivatePairMember );
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void*, size_t )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::TXADSPPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	// Nombre d'octets voulus.
	size_t toRead = (size_t) *ioCount;

	// Curseur sur les données.
	char* theBuffer = (char*) outBuffer;	
	
	// Temporisation		
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kXADSPDefaultTimeout;
	}
		
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);
	
	// Premier octet.
	if (mHasNextByte)
	{
		
	}

	// Lecture tant qu'on n'attend pas plus de timeout entre deux lectures.
	while( toRead > 0 )
	{
		// Nombre d'octets effectivement lus.
		int readBytes = ::ADSPrecv( mClientSocket, theBuffer, (int) toRead, 0 );

		if (readBytes < 0)
		{
			// Erreur.
			// Mise à jour du nombre d'octets lus.
			*ioCount -= toRead;

			int theErr = errno;
			switch (theErr)
			{
				case EINTR:
				case EAGAIN:
#if (EWOULDBLOCK != EAGAIN)
				case EWOULDBLOCK:
#endif
					KDEBUG( "Looping in socket code!" );
					break;

				case ECONNRESET:
					throw DCLPlatformEOF( theErr );

				case ENOTCONN:
					throw DCLPlatformBadStateError( theErr );

				case EBADF:
				case EINVAL:
				case ENOTSOCK:
					throw DCLPlatformBadParamError( theErr );

				case EOPNOTSUPP:
					throw DCLPlatformNotAvailable( theErr );

				case ETIMEDOUT:
					throw DCLPlatformTimeout( theErr );

				case ENOMEM:
					throw DCLPlatformMemError( theErr );

				case ENOBUFS:
// MacOS X n'est pas conforme POSIX
#ifdef ENOSR
				case ENOSR:
#endif
					throw DCLPlatformLimitReachedError( theErr );
				
				case EFAULT:
				case EIO:
				default:
					throw DCLPlatformUnknownError( theErr );
			}
		} else {
			// On vérifie que le délai n'est pas dépassé.
			if ((timeoutInSecs != kNoTimeout) && 
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				// Mise à jour du nombre d'octets lus.
				*ioCount -= toRead;
				throw DCLTimeout;
			}
			
			if (readBytes == 0)
			{
				// EOF.
				*ioCount -= toRead;
				break;
			}
			
			// On a eu quelques octets.
			lastOperation = ::time(NULL);
			
			// Octets suivants.
			toRead -= readBytes;
			theBuffer += readBytes;
		}

		// On partage le CPU.
		theThreadsIntf->Yield();
	}
}

// ------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void 
TDCLXADSP::TXADSPPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	// Nombre d'octets voulus.
	size_t toWrite = (size_t) *ioCount;
		
	// Curseur sur les données.
	const char* theBuffer = (const char*) inBuffer;	

	// Temporisation		
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kXADSPDefaultTimeout;
	}
		
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);
		
	while( toWrite > 0 )
	{
		int sentBytes = ::ADSPsend( mClientSocket, (char*) theBuffer, (int) toWrite, 0 );

		if (sentBytes < 0)
		{
			// Erreur.
			// Mise à jour du nombre d'octets écrits.
			*ioCount -= toWrite;

			int theErr = errno;
			switch (theErr)
			{
				case EINTR:
				case EAGAIN:
#if (EWOULDBLOCK != EAGAIN)
				case EWOULDBLOCK:
#endif
					KDEBUG( "Looping in socket code!" );
					break;

				case ECONNRESET:
					throw DCLPlatformEOF( theErr );

				case EDESTADDRREQ:
				case ENOTCONN:
				case EPIPE:
					throw DCLPlatformBadStateError( theErr );

				case EBADF:
				case EMSGSIZE:
				case EINVAL:
				case ENOTSOCK:
					throw DCLPlatformBadParamError( theErr );

				case EOPNOTSUPP:
					throw DCLPlatformNotAvailable( theErr );

				case ETIMEDOUT:
					throw DCLPlatformTimeout( theErr );

				case ENOMEM:
					throw DCLPlatformMemError( theErr );

				case ENOBUFS:
// MacOS X n'est pas conforme POSIX
#ifdef ENOSR
				case ENOSR:
#endif
					throw DCLPlatformLimitReachedError( theErr );
				
				case EACCES:
				case EFAULT:
				case EIO:
				case ENETDOWN:
				case ENETUNREACH:
				default:
					throw DCLPlatformUnknownError( theErr );
			}
		} else {
			// On vérifie que le délai n'est pas dépassé.
			if ((timeoutInSecs != kNoTimeout) && 
				(((long)(::time(NULL) - lastOperation)) > timeoutInSecs))
			{
				// Mise à jour du nombre d'octets écrits.
				*ioCount -= toWrite;

				throw DCLTimeout;
			}
			
			if (sentBytes == 0)
			{
				// Déconnexion.
				*ioCount -= toWrite;
				throw DCLEOF;
			}
			
			// On a eu quelques octets.
			lastOperation = ::time(NULL);
			
			// Octets suivants.
			toWrite -= sentBytes;
			theBuffer += sentBytes;
		}

		// On partage le CPU.
		theThreadsIntf->Yield();
	}
}

// ------------------------------------------------------------------------- //
//  * WaitForIncomingData( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLXADSP::TXADSPPipe::WaitForIncomingData( void )
{
	Boolean theResult = false;
	
	// Petit select pour attendre.
	fd_set socketSet;
	FD_ZERO( &socketSet );
	FD_SET( mClientSocket, &socketSet );
	FD_SET( mPrivatePairMember, &socketSet );

	int maxSocketNb =
		mClientSocket > mPrivatePairMember
			? mClientSocket : mPrivatePairMember;

	KDEBUG3(
		"WaitForIncomingData::before select (max= %i, client=%i, prv=%i)",
		maxSocketNb, mClientSocket, mPrivatePairMember );

	int readySockets = ::select(
							maxSocketNb + 1, &socketSet, NULL, NULL, NULL );

	if (readySockets == -1)
	{
		// Erreur.
		int theErr = errno;

		switch (theErr)
		{
			case EINVAL:
				throw DCLPlatformBadParamError( theErr );
			
			case EBADF:
				KDEBUG( "WaitForIncomingData: EBADF" );
				break;

			case EINTR:
				KDEBUG( "WaitForIncomingData: EINTR" );
				break;
						
			default:
				throw DCLPlatformUnknownError( theErr );
 		}
	} else if (readySockets > 0) {
		if (FD_ISSET( mPrivatePairMember, &socketSet ))
		{
			KDEBUG( "WaitForIncomingData: private isset" );

			// On vide la paire.
			char someByte;
			while (
				::recv(
						mPrivatePairMember,
						&someByte,
						sizeof(someByte), 0 ) >= 0)
			{
				// Kilroy was here.
			};
		} else {
			KDEBUG( "WaitForIncomingData: theResult = true" );
			theResult = true;
		}
	}
	
	KDEBUG1( "...WaitForIncomingData: theResult = %c", theResult );

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * BytesAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLXADSP::TXADSPPipe::BytesAvailable( void )
{
	timeval timeout;
	fd_set socketSet;

	FD_ZERO( &socketSet );
	FD_SET( mClientSocket, &socketSet );

	timeout.tv_sec = 0;		// secondes
	timeout.tv_usec = 0;	// microsecondes

	// Délai de temporisation à 0 pour savoir si on peut lire maintenant.
	int readySockets =
		::select( mClientSocket + 1, &socketSet, NULL, NULL, &timeout );

	if (readySockets == -1)
	{
		// Erreur.
		int theErr = errno;

		switch (theErr)
		{
			case EINVAL:
				throw DCLPlatformBadParamError( theErr );
			
			case EBADF:
				KDEBUG( "BytesAvailable: EBADF" );
				break;

			case EINTR:
				KDEBUG( "BytesAvailable: EINTR" );
				break;
			
			default:
				throw DCLPlatformUnknownError( theErr );
 		}
	}

	KDEBUG1( "BytesAvailable: %i", readySockets );

	return (readySockets > 0);
}

// ------------------------------------------------------------------------- //
//  * DoDisconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::TXADSPPipe::DoDisconnect( void ) 
{
	// On écrit dans la paire pour sortir.
	if(mPublicPairMember >= 0)
	{
		char someByte = 0;
		(void) ::send(
					mPublicPairMember,
					&someByte,
					sizeof( someByte ),
					0 /* flags */ );
	}

	(void) ::ADSPclose( mClientSocket );
	// Ceci fera échouer select dans le futur.

	mClientSocketIsClosed = true;
}

// ------------------------------------------------------------------------- //
//  * GetTimeout( void )
// ------------------------------------------------------------------------- //
long
TDCLXADSP::TXADSPPipe::GetTimeout( void )
{
	return mTimeout;
}

// ------------------------------------------------------------------------- //
//  * SetTimeout( long )
// ------------------------------------------------------------------------- //
void
TDCLXADSP::TXADSPPipe::SetTimeout( long inTimeout )
{
	// On copie la valeur.
	mTimeout = inTimeout;
}

// ================================= //
// FORTRAN rots the brain.           //
//                 -- John McQuillin //
// ================================= //
