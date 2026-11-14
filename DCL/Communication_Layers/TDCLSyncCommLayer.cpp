// ==============================
// Fichier:			TDCLSyncCommLayer.cp
// Projet:			Desktop Connection Library
//
// Créé le:			28/3/2003
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
// The Original Code is TDCLSyncCommLayer.cp.
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
// $Id: TDCLSyncCommLayer.cp,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.h>
#include <DCL/Link/TDCLLink.h>
#include <DCL/Server/TDCLServer.h>

// -------------------------------------------------------------------------- //
//  * TDCLSyncCommLayer( IDCLThreads*, Boolean, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLSyncCommLayer::TDCLSyncCommLayer(
					IDCLThreads* inThreadsIntf,
					Boolean	inFlushAvailable /* = false */,
					KUInt32	inMaxPipeCount /* = kDefaultPipeCount */
				)
	:
		TDCLCommLayer( inFlushAvailable, inMaxPipeCount ),
		TDCLThread( inThreadsIntf )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLSyncCommLayer( void )
// -------------------------------------------------------------------------- //
TDCLSyncCommLayer::~TDCLSyncCommLayer( void )
{
	(void) RequestStop();
	try {
		Join();
	} catch ( TDCLThreadDeadLockException& ) {
	}
}

// ------------------------------------------------------------------------- //
//  * RequestStop( void )
// ------------------------------------------------------------------------- //
bool
TDCLSyncCommLayer::RequestStop( void )
{
	bool theResult = TDCLCommLayer::RequestStop();
	if (TDCLThread::RequestStop())
	{
		theResult = true;
	}
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * StartListening( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::StartListening( TDCLServer* inServer )
{
	// Exception si on est déjà en train d'écouter.
	if (GetServer())
	{
		throw DCLBadStateError;
	}
	
	// Reset the communication layer's stop token.
	TDCLCommLayer::ResetStopSource();

	// Enregistrement du serveur.
	SetServer( inServer );
	
	// Démarrage du processus léger.
	// Celui-ci appellera DoStartListening.
	Start();
}

// ------------------------------------------------------------------------- //
//  * StopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::StopListening( void )
{
	// On ne fait rien si on n'est pas en train d'écouter.
	if (GetServer())
	{
		// On appelle la méthode qui fait vraiment le travail.
		try {
			(void) RequestStop();

			DoStopListening();
		
			// Réveil du processus léger
			WakeUp();

			// On attend que le processus léger termine.
			try {
				Join();
			} catch ( TDCLThreadDeadLockException& ) {
				// Avoid self-join if called from the worker thread.
			}
		} catch ( ... ) {
			// On reste silencieux.
		}
		
		SetServer( nil );
	}
}

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::Run( void )
{
	// On commence à écouter.
	DoStartListening();
	
	// Ayé, on écoute.
	GetServer()->WaitingConnection( this );

	// On attend qu'une requête soit présente.
	while ( !StopRequested() && WaitForIncomingRequest() )
	{
		// Une requête vient d'arriver. On prévient le serveur.
		TDCLServer* theServer = GetServer();
		
		if (theServer)
		{
			theServer->RequestPresent( this );
		} else {
			break;
		}

		(void) Sleep();	// On attend que la requête soit acceptée ou refusée.
	}
}

// ------------------------------------------------------------------------- //
//  * Accept( void )
// ------------------------------------------------------------------------- //
TDCLPipe*
TDCLSyncCommLayer::Accept( void )
{
	TDCLPipe* thePipe = DoAccept();

	// Réveil du processus léger (pour la prochaine requête).
	WakeUp();

	return thePipe;
}

// ------------------------------------------------------------------------- //
//  * Refuse( void )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::Refuse( void )
{
	DoRefuse();
	
	// Réveil du processus léger (pour la prochaine requête).
	WakeUp();
}

// --------------------------------------------------------------------------------	//
//  * HandleException( TDCLException* )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::HandleException( TDCLException* inException )
{
	// On transmet au serveur.
	GetServer()->HandleCommLayerException( this, inException );
}

// --------------------------------------------------------------------------------	//
//  * TSyncPipe( IDCLThreads*, TDCLSyncCommLayer* )
// --------------------------------------------------------------------------------	//
TDCLSyncCommLayer::TSyncPipe::TSyncPipe(
					IDCLThreads* inThreadsIntf, TDCLSyncCommLayer* inCommLayer )
	:
		TDCLPipe( inCommLayer ),
		TDCLThread( inThreadsIntf )
{
}

// --------------------------------------------------------------------------------	//
//  * ~TSyncPipe( void )
// --------------------------------------------------------------------------------	//
TDCLSyncCommLayer::TSyncPipe::~TSyncPipe( void )
{
	(void) RequestStop();
	try {
		Join();
	} catch ( TDCLThreadDeadLockException& ) {
	}
}

// --------------------------------------------------------------------------------	//
//  * RequestStop( void )
// --------------------------------------------------------------------------------	//
bool
TDCLSyncCommLayer::TSyncPipe::RequestStop( void )
{
	bool theResult = TDCLPipe::RequestStop();
	if (TDCLThread::RequestStop())
	{
		theResult = true;
	}
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * Disconnect( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::Disconnect( void )
{
	(void) RequestStop();

	if (GetLink())
	{
		// On appelle la méthode qui fait vraiment le travail.
		DoDisconnect();
		
		// On relâche le processus s'il attendait
		WakeUp();
		
		// On attend que le processus léger termine.
		try {
			Join();
		} catch ( TDCLThreadDeadLockException& ) {
			// Avoid self-join if called from the worker thread.
		}
	}
}

// --------------------------------------------------------------------------------	//
//  * Connected( TDCLLink* )
// --------------------------------------------------------------------------------	//
TDCLCommLayer*
TDCLSyncCommLayer::TSyncPipe::Connected( TDCLLink* inLink )
{
	TDCLPipe::ResetStopSource();

	// Appel de la méthode par défaut.
	TDCLCommLayer* theResult = TDCLPipe::Connected( inLink );
	
	// Démarrage du processus léger.
	Start();

	// A stop requested by the link before ResetStopSource() went to the
	// old stop source and would be lost: re-request it.
	if (inLink && inLink->StopRequested())
	{
		(void) RequestStop();
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * ClearDataPresent( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::ClearDataPresent( void )
{
	WakeUp();
}

// --------------------------------------------------------------------------------	//
//  * Run( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::Run( void )
{
	do {
		(void) Sleep();	// On attend que le lien nous demande si on veut des données.

		if ( StopRequested() )
		{
			break;
		}

		if ( WaitForIncomingData() )
		{
			// Des données viennent d'arriver. On prévient le lien.
			GetLink()->DataPresent();
		} else {
			break;
		}
	} while ( !StopRequested() );
}

// --------------------------------------------------------------------------------	//
//  * HandleException( TDCLException* )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::HandleException( TDCLException* inException )
{
	// On transmet à la couche de communication.
	((TDCLSyncCommLayer*) GetCommLayer())->HandleException( inException );
}

// ============================================================= //
// Sendmail may be safely run set-user-id to root.               //
//                 -- Eric Allman, "Sendmail Installation Guide" //
// ============================================================= //
