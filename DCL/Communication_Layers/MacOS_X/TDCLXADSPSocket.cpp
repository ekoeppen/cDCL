// ==============================
// Fichier:			TDCLXADSPSocket.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			25/4/2004
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
// The Original Code is TDCLXADSPSocket.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLXADSPSocket.cp,v 1.5 2004/11/24 14:08:55 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS_X/TDCLXADSPSocket.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

// ANSI C & POSIX.
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <time.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// AppleTalk & co. dans Kernel.Framework (/usr/include)
#include <netat/appletalk.h>
#include <netat/nbp.h>
#include <netat/adsp.h>

// AppleTalk dans AppleTalk.Framework
// (n'est pas dans le système mais dans Darwin)
extern "C" {
#include "at_proto.h"
}

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLXADSPSocket( IDCLThreads*, int )
// -------------------------------------------------------------------------- //
TDCLXADSPSocket::TDCLXADSPSocket( IDCLThreads* inThreadsIntf, int inADSPSocket )
	:
		TDCLThread( inThreadsIntf ),
		mADSPSocket( inADSPSocket ),
		mSocketsAreClosed( true ),
		mException( nil ),
		mHadException( false )
{
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
	
	Start();
}

// -------------------------------------------------------------------------- //
//  * ~TDCLXADSPSocket( void )
// -------------------------------------------------------------------------- //
TDCLXADSPSocket::~TDCLXADSPSocket( void )
{
	Close();
}

// -------------------------------------------------------------------------- //
//  * Close( void )
// -------------------------------------------------------------------------- //
void
TDCLXADSPSocket::Close( void )
{
	// On ferme.
	if (!mSocketsAreClosed)
	{
		if (mPublicPairMember >= 0)
		{
			(void) ::close( mPublicPairMember );
		}
		if (mPrivatePairMember >= 0)
		{
			(void) ::close( mPrivatePairMember );
		}
		if (mADSPSocket >= 0)
		{
			(void) ::ADSPclose( mADSPSocket );
		}
		
		mSocketsAreClosed = true;
	}
}

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
TDCLXADSPSocket::Run( void )
{
	Boolean someSocketWasClosed = false;
	do {
		char adspBuffer[1];
		
		// Lecture avec ASYNCread.
		int nbRead = ::ASYNCread( mADSPSocket, adspBuffer, sizeof(adspBuffer));

		if (nbRead == -1)
		{
			// Erreur.
			someSocketWasClosed = HandleError();
		} else if (nbRead > 0) {
			// On a lu des octets.
			// On écrit tout sur la paire.
			if (::send(
					mPrivatePairMember,
					adspBuffer,
					(size_t) nbRead,
					0 /* flags */ ) != (ssize_t) nbRead)
			{
				someSocketWasClosed = HandleError();
			}
		} else {
			// Boucle select + ASYNCread_complete.
			Boolean gotADSPdata = false;
			do {
				// On fait un select puis un ASYNCread_complete.
				fd_set socketSet;
				FD_ZERO( &socketSet );
				FD_SET( mADSPSocket, &socketSet );
				FD_SET( mPrivatePairMember, &socketSet );

				int maxSocketNb =
					mADSPSocket > mPrivatePairMember
					? mADSPSocket : mPrivatePairMember;

				int readySockets = ::select( maxSocketNb + 1, &socketSet,
					NULL, NULL, NULL );
	
				if (readySockets == -1)
				{
					someSocketWasClosed = HandleError();
				} else {
					if (FD_ISSET( mADSPSocket, &socketSet ))
					{
						gotADSPdata = true;

						// Appel de ASYNCread_complete.
						nbRead = ::ASYNCread_complete(
												mADSPSocket,
												adspBuffer,
												sizeof(adspBuffer));
						
						if (nbRead == -1) {
							// Erreur.
							someSocketWasClosed = HandleError();
						} else if (nbRead > 0) {
							// On a lu quelques octets.
							// On les écrit sur la paire.
							if (::send(
									mPrivatePairMember,
									adspBuffer,
									(size_t) nbRead,
									0 /* flags */ ) != (ssize_t) nbRead)
							{
								someSocketWasClosed = HandleError();
							}
						}
					} // ISSET mADSPSocket
					
					if (FD_ISSET( mPrivatePairMember, &socketSet ))
					{
						// On vide la socket de la paire dans la socket ADSP.
						char theBuffer[ADSP_MAX_DATA_LEN];
						do {
							int amountRcvd = ::recv(
								mPrivatePairMember,
								theBuffer,
								sizeof(theBuffer), 0 );
							
							if (amountRcvd > 0)
							{
								int amountSent = ::ADSPsend(
										mADSPSocket,
										theBuffer,
										amountRcvd,
										0 /* send only standard data, not attention */);
								
								if (amountSent != amountRcvd)
								{
									if (amountSent == -1)
									{
										someSocketWasClosed = HandleError();
									}
								}
							}
							
							if (amountRcvd != sizeof(theBuffer))
							{
								if (amountRcvd == -1)
								{
									someSocketWasClosed = HandleError();
								}
								
								break;
							}
						} while (true);
					}
				} // if (readySockets == -1) ...
			} while ( !gotADSPdata && !someSocketWasClosed );
		}
	} while ( !someSocketWasClosed );
	
	// On ferme.
	Close();
}

// ------------------------------------------------------------------------- //
//  * HandleError( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLXADSPSocket::HandleError( void ) const
{
	Boolean theResult = false;

	// Erreur.
	int theErr = errno;

	switch (theErr)
	{
		case EINVAL:
			throw DCLPlatformBadParamError( theErr );
	
		case EBADF:
			KDEBUG( "EBADF" );
			theResult = true;
			break;
		
		case EINTR:
			// ?
			KDEBUG( "EINTR" );
			break;						
		
		case EAGAIN:
			// ?
			KDEBUG( "EAGAIN" );
			break;						
		
		default:
			throw DCLPlatformUnknownError( theErr );
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLXADSPSocket::HandleException( TDCLException* inException )
{
	// On ferme.
	Close();
	
	// On note l'exception.
	mException = inException;
	
	// On note qu'on a eu une exception.
	mHadException = true;
}

// ================================ //
// Never trust an operating system. //
// ================================ //
