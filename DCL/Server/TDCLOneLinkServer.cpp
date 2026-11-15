// ==============================
// Fichier:			TDCLOneLinkServer.cp
// Projet:			Desktop Connection Library
//
// Créé le:			3/4/2003
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
// The Original Code is TDCLOneLinkServer.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLOneLinkServer.cp,v 1.6 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLOneLinkServer.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Link/TDCLLink.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Interfaces/TDCLApplication.h>

// -------------------------------------------------------------------------- //
//  * TDCLOneLinkServer( TDCLApplication* )
// -------------------------------------------------------------------------- //
TDCLOneLinkServer::TDCLOneLinkServer( TDCLApplication* inApplication )
		:
			TDCLServer( inApplication ),
			mLink( nil ),
			mPipe( nil ),
			mCommLayers( nil ),
			mNbCommLayers( 0 ),
			mListMutex( nil )
{
	// Allocation de la liste des interfaces de communication.
	mCommLayers = ( TDCLCommLayer**  )::malloc( 0 * sizeof( TDCLCommLayer* ) );
	
	// Mutex sur cette liste.
	mListMutex = inApplication->GetThreadsIntf()->CreateSemaphore();
}

// -------------------------------------------------------------------------- //
//  * ~TDCLOneLinkServer( void )
// -------------------------------------------------------------------------- //
TDCLOneLinkServer::~TDCLOneLinkServer( void )
{	
	// Suppression de la connexion.
	if (mPipe)
	{
		delete mPipe;
	}
	
	// Suppression du sémaphore.
	if (mListMutex)
	{
		delete mListMutex;
	}

	// Suppression de la liste des interfaces de communication.
	// Ces interfaces devront être supprimées par l'application.
	if (mCommLayers)
	{
		::free( mCommLayers );
	}
}

// -------------------------------------------------------------------------- //
//  * AddCommLayer( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::AddCommLayer( TDCLCommLayer* inLayer )
{
	// Acquisition
	mListMutex->Acquire();

	// Redimensionnement de la liste.
	KUInt32 indexNewCL = mNbCommLayers++;
	mCommLayers = (TDCLCommLayer**) ::realloc(
					mCommLayers, mNbCommLayers * sizeof( TDCLCommLayer* ) );

	// Ajout du nouvel élément.
	mCommLayers[indexNewCL] = inLayer;

	inLayer->StartListening(this);
	
	// Libération
	mListMutex->Release();
}

// -------------------------------------------------------------------------- //
//  * RemoveCommLayer( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::RemoveCommLayer( TDCLCommLayer* inLayer )
{
	// Acquisition
	mListMutex->Acquire();

	// Déterminons où se trouve l'élément dans la liste.
	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		if (mCommLayers[indexCL] == inLayer)
		{
			// Trouvé.
			inLayer->StopListening();
			
			// Copie des éléments qui suivent.
			(void) ::memmove(
						(void*) &mCommLayers[indexCL],
						(const void*) &mCommLayers[indexCL+1],
						(nbCL - indexCL - 1) * sizeof( TDCLCommLayer* ));
			
			// Redimensionnement
			--mNbCommLayers;
			mCommLayers = (TDCLCommLayer**) ::realloc(
							mCommLayers,
							mNbCommLayers * sizeof( TDCLCommLayer* ) );
			break;
		}
	}
	
	// Libération
	mListMutex->Release();
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStartListening( void )
{
	// On appelle StartListening sur chacune des interfaces de connexions.
	// Acquisition
	mListMutex->Acquire();
	
	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->StartListening(this);
	}

	// Libération
	mListMutex->Release();
}

// -------------------------------------------------------------------------- //
//  * DoStop( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStop( void )
{
	// Disconnect the link:
	if ((mLink) && mLink->IsConnected())
	{
		try {
			(void) mLink->Disconnect();	// On se fiche de savoir s'il était
										// vraiment connecté.
		} catch (...) {
			// Silently catch exceptions
		}
	}

	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->StopListening();
	}
}

// -------------------------------------------------------------------------- //
//  * DoKill( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoKill( void )
{
	if (mPipe)
	{	
		try {
			mPipe->Disconnect();
		} catch (...) {
			// On fait le silence sur les exceptions.
		}

		// On dit au lien qu'il a été déconnecté.
		if (mLink)
		{
			(void) mLink->Close();
		}

		// Suppression de l'interface.
		delete mPipe;
		mPipe = nil;
	}

	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->StopListening();
	}
}

// -------------------------------------------------------------------------- //
//  * HandleIncomingConnection( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
TDCLServer::EState
TDCLOneLinkServer::HandleIncomingConnection( TDCLCommLayer* inCommLayer )
{
	// Si le lien est connecté, on refuse la connexion.
	if ((mLink) && (mLink->IsConnected()))
	{
		inCommLayer->Refuse();
	} else {
		// Sinon, on l'accepte.

		// Si un objet TDCLPipe était présent (d'un état inconnu), on le
		// supprime ici.
		if (mPipe)
		{
			delete mPipe;
		}

		// On accepte la connexion.
		mPipe = inCommLayer->Accept();
				
		// On dit au lien de se connecter.
		if ((mLink) && (!mLink->Connect( mPipe, this )))
		{
			throw DCLBadStateError;	// Ne devrait pas arriver.
		}
	}
	
	return kRunning;		
}

// -------------------------------------------------------------------------- //
//  * HandleLinkDisconnection( TDCLLink* )
// -------------------------------------------------------------------------- //
TDCLServer::EState
TDCLOneLinkServer::HandleLinkDisconnection( TDCLLink* /* inLink */ )
{
	if (mPipe)
	{	
		try {
			mPipe->Disconnect();
		} catch ( ... ) {
		}

		// Suppression de l'interface de communication.
		delete mPipe;
		mPipe = nil;
	}

	return kRunning;		
}

// -------------------------------------------------------------------------- //
//  * DoIdle( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoIdle( void )
{
	// On appelle Idle sur chacune des interfaces de connexions.
	// Acquisition
	mListMutex->Acquire();
	
	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->Idle();
	}

	// Libération
	mListMutex->Release();
}

// ========================================================================== //
// So you see Antonio, why worry about one little core dump, eh?  In reality  //
// all core dumps happen at the same instant, so the core dump you will have  //
// tomorrow, why, it already happened.  You see, it's just a little universal //
// recursive joke which threads our lives through the infinite potential of   //
// the instant.  So go to sleep, Antonio, your thread could break any moment  //
// and cast you out of the safe security of the instant into the dark void of //
// eternity, the anti-time.  So go to sleep...                                //
// ========================================================================== //
