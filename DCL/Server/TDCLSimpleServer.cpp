// ==============================
// Fichier:			TDCLSimpleServer.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			15/1/2002
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
// The Original Code is TDCLSimpleServer.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLSimpleServer.cp,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLSimpleServer.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/TDCLLink.h>

// -------------------------------------------------------------------------- //
//  * TDCLSimpleServer( TDCLApplication*, TDCLPipe* )
// -------------------------------------------------------------------------- //
TDCLSimpleServer::TDCLSimpleServer(
			TDCLApplication* inApplication,
			TDCLCommLayer* inCommLayer
		)
		:
			TDCLServer( inApplication ),
			mLink( nil ),
			mCommLayer( inCommLayer ),
			mPipe( nil )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLSimpleServer( void )
// -------------------------------------------------------------------------- //
TDCLSimpleServer::~TDCLSimpleServer( void )
{	
	// Suppression de la connexion.
	if (mPipe)
	{
		delete mPipe;
	}
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStartListening( void )
{
	// On commence ˆ Žcouter
	mCommLayer->StartListening(this);
}

// -------------------------------------------------------------------------- //
//  * DoStop( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStop( void )
{
	// Disconnect the link:
	if ((mLink) && (mLink->IsConnected()))
	{
		try {
			(void) mLink->Disconnect();	// On n'a pas besoin de savoir s'il
										// Žtait vraiment connectŽ.
		} catch (...) {
			// Silently catch exceptions
		}
	}

	// Fermeture du serveur.
	mCommLayer->StopListening();
}

// -------------------------------------------------------------------------- //
//  * DoKill( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoKill( void )
{
	if (mPipe)
	{	
		try {
			mPipe->Disconnect();
		} catch (...) {
			// On fait le silence sur les exceptions.
		}

		// On dit au lien qu'il a ŽtŽ dŽconnectŽ.
		if (mLink)
		{
			(void) mLink->Close();
		}

		// Suppression de l'interface.
		delete mPipe;
		mPipe = nil;
	}

	// Fermeture du serveur.
	mCommLayer->StopListening();
}

// -------------------------------------------------------------------------- //
//  * HandleIncomingConnection( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
TDCLServer::EState
TDCLSimpleServer::HandleIncomingConnection( TDCLCommLayer* /* inCommLayer */ )
{
	// Si le lien est connectŽ, on refuse la connexion.
	if ((mLink) && (mLink->IsConnected()))
	{
		mCommLayer->Refuse();
	} else {
		// Sinon, on l'accepte.

		// Si un objet TDCLPipe Žtait prŽsent (d'un Žtat inconnu), on le
		// supprime ici.
		if (mPipe)
		{
			delete mPipe;
		}

		// On accepte la connexion.
		mPipe = mCommLayer->Accept();
				
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
TDCLSimpleServer::HandleLinkDisconnection( TDCLLink* /* inLink */ )
{
	if (mPipe)
	{	
		mPipe->Disconnect();

		// Suppression de l'interface de communication.
		delete mPipe;
		mPipe = nil;
	} else {
		throw DCLBadStateError;	// Ne devrait pas arriver.
	}

	return kRunning;		
}

// -------------------------------------------------------------------------- //
//  * DoIdle( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoIdle( void )
{
	mCommLayer->Idle();	// Comm layer may need some time to get
						// incoming connections.
}

// ========================================================================= //
// Simulations are like miniskirts, they show a lot and hide the essentials. //
//                 -- Hubert Kirrman                                         //
// ========================================================================= //
