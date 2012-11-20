// ==============================
// Fichier:			TDCLBSDSocket.cp
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
// The Original Code is TDCLBSDSocket.cp.
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
// $Id: TDCLBSDSocket.cp,v 1.13 2004/11/24 18:11:12 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

// OS X n'est pas conforme POSIX pour le coup (est-ce le seul?)
// (ceci doit être avant sys/socket.h)
#ifndef _BSD_SOCKLEN_T_
	#define _BSD_SOCKLEN_T_ int
#endif

// ANSI C & POSIX.
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fcntl.h>
// Conflit entre les constantes BSD et les constates OT
// (elles ont les mêmes valeurs sauf pour TCP_KEEPALIVE, mais je veux la version BSD ici.)
#ifdef TCP_NODELAY
#undef TCP_NODELAY
#endif
#ifdef TCP_MAXSEG
#undef TCP_MAXSEG
#endif
#ifdef TCP_KEEPALIVE
#undef TCP_KEEPALIVE
#endif
#include <netinet/tcp.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// K
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

// ------------------------------------------------------------------------- //
//  * Déverminage
// ------------------------------------------------------------------------- //

#undef KERROR_ENABLED
#if defined(error_TDCLBSDSocket) && error_TDCLBSDSocket
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLBSDSocket) && trace_TDCLBSDSocket
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLBSDSocket) && debug_TDCLBSDSocket
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

// ------------------------------------------------------------------------- //
//  * TDCLBSDSocket( IDCLThreads*, unsigned short, KUInt32, long )
// ------------------------------------------------------------------------- //
TDCLBSDSocket::TDCLBSDSocket(
			IDCLThreads*	inThreadsIntf,
			unsigned short	inPort /* = kIANAAssignedPort */,
			KUInt32			inMaxPipeCount /* = kDefaultPipeCount */,
			long			inTimeout /* = kDefaultTimeout */ )
		:
			TDCLSyncCommLayer(
						inThreadsIntf,
						false /* inFlushAvailable */,
						inMaxPipeCount ),
			mServerSocket( -1 ),
			mCurrentPipeCount( 0 ),
			mBackLogSize( 0 ),
			mTimeout( inTimeout ),
			mSocketsAreClosed( true ),
			mPort( inPort )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLBSDSocket( void )
// ------------------------------------------------------------------------- //
TDCLBSDSocket::~TDCLBSDSocket( void )
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
TDCLBSDSocket::IsAvailable( void )
{
	Boolean theResult = false;
	
	int tmpsocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (tmpsocket >= 0)
	{
		theResult = true;
		(void) ::close( tmpsocket );
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * SocketBindAndListen( void )
// ------------------------------------------------------------------------- //
int
TDCLBSDSocket::SocketBindAndListen( void )
{
	int theSocket;
	do {
		theSocket = ::socket( AF_INET, SOCK_STREAM, 0 );

		if (theSocket < 0)
		{
			break;
		}

		struct sockaddr_in listenName;

		listenName.sin_family = AF_INET;			// addressing family
		listenName.sin_port = htons(mPort);			// port number
		listenName.sin_addr.s_addr = INADDR_ANY;	// IP address

		int acceptedSocketLen = sizeof(listenName);

		int theResult = ::bind(
							theSocket,
							(struct sockaddr*) &listenName,
							acceptedSocketLen );
		if (theResult < 0)
		{
			// Fermeture de l'interface de communication.
			(void) ::close( theSocket );	
			theSocket = theResult;
			break;
		}
		
		KUInt32 theBackLogSize = mBackLogSize;
		if ((theBackLogSize == 0) || (theBackLogSize > kBSDSocketMaxBackLogSize))
		{
			theBackLogSize = kBSDSocketMaxBackLogSize;
		}
		theResult = ::listen( theSocket, (int) theBackLogSize );
		if (theResult < 0)
		{
			// Fermeture de l'interface de communication.
			(void) ::close( theSocket );
			theSocket = theResult;
			
			break;
		}
	} while ( false );
	
	return theSocket;
}

// ------------------------------------------------------------------------- //
//  * DoStartListening( void )
// ------------------------------------------------------------------------- //
void
TDCLBSDSocket::DoStartListening( void )
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
	}	// Sinon, on ne fait rien
}

// ------------------------------------------------------------------------- //
//  * DoStopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLBSDSocket::DoStopListening( void )
{
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
//  * WaitForIncomingRequest( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLBSDSocket::WaitForIncomingRequest( void )
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
TDCLBSDSocket::DoAccept( void )
{
	TDCLPipe* thePipe = nil;
	
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof(clientAddress);
		
	int theSocket = ::accept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );
	
	if (theSocket < 0)
	{
		int theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	} else {
		KUInt32 theMaxPipeCount = GetMaxPipeCount();
		if ((theMaxPipeCount == 0) || (mCurrentPipeCount < theMaxPipeCount))
		{
			// Configuration de l'interface.
			// On veut ne pas attendre entre les paquets TCP/IP
			// (remarque: on utilise la classe TDCLBufferedPipe pour éviter les
			// appels trop répétés).
			int theTCPFlag = 1;
			int theResult = ::setsockopt(
								theSocket,
								IPPROTO_TCP,
								TCP_NODELAY,
								(char*) &theTCPFlag,
								sizeof(theTCPFlag) );
			if (theResult >= 0)
			{
				// On veut aussi que la socket soit bloquante.
				int flags = ::fcntl( theSocket, F_GETFL, 0 /* ignored */ );
				if (flags & O_NONBLOCK)
				{
					flags &= ~ O_NONBLOCK;
					theResult = ::fcntl( theSocket, F_SETFL, flags );
				}
			}

			if (theResult < 0)
			{
				int theErr = errno;
				throw DCLPlatformUnknownError( theErr );
			}
			
			thePipe = new TDCLBufferedPipe(
					new TBSDSocketPipe(
							this,
							theSocket,
							mTimeout ) );

			// Incrément du compteur
			mCurrentPipeCount++;			
		} else {
			if (::close( theSocket ) < 0)
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
TDCLBSDSocket::DoRefuse( void )
{
	struct sockaddr_in clientAddress;
	socklen_t nameSize = sizeof(clientAddress);
		
	int theSocket = ::accept(
				mServerSocket,
				(struct sockaddr*) &clientAddress,
				&nameSize );
	
	if (theSocket < 0)
	{
		int theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	} else {
		if (::close( theSocket ) < 0)
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
TDCLBSDSocket::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// ------------------------------------------------------------------------- //
//  * GetTimeout( void )
// ------------------------------------------------------------------------- //
long 
TDCLBSDSocket::GetTimeout( void )
{
	return mTimeout;
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TPipe* )
// ------------------------------------------------------------------------- //
void
TDCLBSDSocket::Disconnected( TDCLPipe* /* inPipe */ )
{
	mCurrentPipeCount--;
}

// ------------------------------------------------------------------------- //
//  * TBSDSocketPipe( TDCLBSDSocket*, int, long )
// ------------------------------------------------------------------------- //
TDCLBSDSocket::TBSDSocketPipe::TBSDSocketPipe(
				TDCLBSDSocket* inCommLayer,
				int inClientSocket,
				long inTimeout )
	:
		TSyncPipe( inCommLayer->GetThreadsIntf(), inCommLayer ),
		mClientSocket( inClientSocket ),
		mTimeout( inTimeout ),
		mPublicPairMember( -1 ),
		mPrivatePairMember( -1 ),
		mClientSocketIsClosed( true )
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
//  * ~TBSDSocketPipe( void )
// ------------------------------------------------------------------------- //
TDCLBSDSocket::TBSDSocketPipe::~TBSDSocketPipe( void ) 
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
TDCLBSDSocket::TBSDSocketPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	// Nombre d'octets voulus.
	size_t toRead = (size_t) *ioCount;

	// Curseur sur les données.
	char* theBuffer = (char*) outBuffer;	
	
	// Temporisation		
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kBSDSocketDefaultTimeout;
	}
		
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);

	// Lecture tant qu'on n'attend pas plus de timeout entre deux lectures.
	while( toRead > 0 )
	{
		// Nombre d'octets effectivement lus.
		int readBytes = ::recv( mClientSocket, theBuffer, toRead, 0 );

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
TDCLBSDSocket::TBSDSocketPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	// Nombre d'octets voulus.
	size_t toWrite = (size_t) *ioCount;
		
	// Curseur sur les données.
	const char* theBuffer = (const char*) inBuffer;	

	// Temporisation		
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kBSDSocketDefaultTimeout;
	}
		
	IDCLThreads* theThreadsIntf = GetThreadsIntf();
	time_t lastOperation = ::time(NULL);
		
	while( toWrite > 0 )
	{
		int sentBytes = ::send( mClientSocket, (char*) theBuffer, toWrite, 0 );

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
TDCLBSDSocket::TBSDSocketPipe::WaitForIncomingData( void )
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
TDCLBSDSocket::TBSDSocketPipe::BytesAvailable( void )
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
TDCLBSDSocket::TBSDSocketPipe::DoDisconnect( void ) 
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

	(void) ::close( mClientSocket );
	// Ceci fera échouer select dans le futur.

	mClientSocketIsClosed = true;
}

// ------------------------------------------------------------------------- //
//  * GetTimeout( void )
// ------------------------------------------------------------------------- //
long
TDCLBSDSocket::TBSDSocketPipe::GetTimeout( void )
{
	return mTimeout;
}

// ------------------------------------------------------------------------- //
//  * SetTimeout( long )
// ------------------------------------------------------------------------- //
void
TDCLBSDSocket::TBSDSocketPipe::SetTimeout( long inTimeout )
{
	// On copie la valeur.
	mTimeout = inTimeout;
	long timeoutInSecs = mTimeout;
	if (timeoutInSecs == kDefaultTimeout)
	{
		timeoutInSecs = kBSDSocketDefaultTimeout;
	}
	
	// Ensuite, on règle la temporisation sur l'interface.
	struct timeval theTimeoutStructure;
	theTimeoutStructure.tv_sec = timeoutInSecs;
	theTimeoutStructure.tv_usec = 0;
	int theErr = ::setsockopt(
					mClientSocket,
					SOL_SOCKET,
					SO_RCVTIMEO,
					&theTimeoutStructure,
					sizeof(theTimeoutStructure));
	
	if (theErr < 0)
	{
		theErr = errno;
		switch (theErr)
		{
			case ENOPROTOOPT:
				mTimeout = kDefaultTimeout;
				break;
			
			case EBADF:
			case EDOM:
			case ENOTSOCK:
				throw DCLPlatformBadParamError( theErr );

			case EINVAL:
			case EISCONN:
				throw DCLPlatformBadStateError( theErr );

			case ENOMEM:
				throw DCLPlatformMemError( theErr );

			case ENOBUFS:

// MacOS X n'est pas conforme POSIX
#ifdef ENOSR
			case ENOSR:
#endif
				throw DCLPlatformLimitReachedError( theErr );
			
			case EFAULT:
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

// ================================= //
// FORTRAN rots the brain.           //
//                 -- John McQuillin //
// ================================= //
