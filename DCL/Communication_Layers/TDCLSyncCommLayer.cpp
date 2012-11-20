// ==============================
// Fichier:			TDCLSyncCommLayer.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			28/3/2003
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
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
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

// ------------------------------------------------------------------------- //
//  *ÊStartListening( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::StartListening( TDCLServer* inServer )
{
	// Exception si on est dŽjˆ en train d'Žcouter.
	if (GetServer())
	{
		throw DCLBadStateError;
	}
	
	// Enregistrement du serveur.
	SetServer( inServer );
	
	// DŽmarrage du processus lŽger.
	// Celui-ci appellera DoStartListening.
	Start();
}

// ------------------------------------------------------------------------- //
//  *ÊStopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::StopListening( void )
{
	// On ne fait rien si on n'est pas en train d'Žcouter.
	if (GetServer())
	{
		// On appelle la mŽthode qui fait vraiment le travail.
		try {
			DoStopListening();
		
			// RŽveil du processus lŽger
			WakeUp();

			// On attend que le processus lŽger termine.
			while (GetThreadState() != IDCLThreads::kStopped)
			{
				Yield();
			}
		} catch ( ... ) {
			// On reste silencieux.
		}
		
		SetServer( nil );
	}
}

// ------------------------------------------------------------------------- //
//  *ÊRun( void )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::Run( void )
{
	// On commence ˆ Žcouter.
	DoStartListening();
	
	// AyŽ, on Žcoute.
	GetServer()->WaitingConnection( this );

	// On attend qu'une requte soit prŽsente.
	while ( WaitForIncomingRequest() )
	{
		// Une requte vient d'arriver. On prŽvient le serveur.
		TDCLServer* theServer = GetServer();
		
		if (theServer)
		{
			theServer->RequestPresent( this );
		} else {
			break;
		}

		(void) Sleep();	// On attend que la requte soit acceptŽe ou refusŽe.
	}
}

// ------------------------------------------------------------------------- //
//  * Accept( void )
// ------------------------------------------------------------------------- //
TDCLPipe*
TDCLSyncCommLayer::Accept( void )
{
	TDCLPipe* thePipe = DoAccept();

	// RŽveil du processus lŽger (pour la prochaine requte).
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
	
	// RŽveil du processus lŽger (pour la prochaine requte).
	WakeUp();
}

// --------------------------------------------------------------------------------	//
//  *ÊHandleException( TDCLException* )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::HandleException( TDCLException* inException )
{
	// On transmet au serveur.
	GetServer()->HandleCommLayerException( this, inException );
}

// --------------------------------------------------------------------------------	//
//  *ÊTSyncPipe( IDCLThreads*, TDCLSyncCommLayer* )
// --------------------------------------------------------------------------------	//
TDCLSyncCommLayer::TSyncPipe::TSyncPipe(
					IDCLThreads* inThreadsIntf, TDCLSyncCommLayer* inCommLayer )
	:
		TDCLPipe( inCommLayer ),
		TDCLThread( inThreadsIntf )
{
}

// --------------------------------------------------------------------------------	//
//  *ÊDisconnect( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::Disconnect( void )
{
	if (GetLink())
	{
		// On appelle la mŽthode qui fait vraiment le travail.
		DoDisconnect();
		
		// On rel‰che le processus s'il attendait
		WakeUp();
		
		// On attend que le processus lŽger termine.
		while (GetThreadState() != IDCLThreads::kStopped)
		{
			Yield();
		}
	}
}

// --------------------------------------------------------------------------------	//
//  *ÊConnected( TDCLLink* )
// --------------------------------------------------------------------------------	//
TDCLCommLayer*
TDCLSyncCommLayer::TSyncPipe::Connected( TDCLLink* inLink )
{
	// Appel de la mŽthode par dŽfaut.
	TDCLCommLayer* theResult = TDCLPipe::Connected( inLink );
	
	// DŽmarrage du processus lŽger.
	Start();
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  *ÊClearDataPresent( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::ClearDataPresent( void )
{
	WakeUp();
}

// --------------------------------------------------------------------------------	//
//  *ÊRun( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::Run( void )
{
	do {
		(void) Sleep();	// On attend que le lien nous demande si on veut des donnŽes.

		if ( WaitForIncomingData() )
		{
			// Des donnŽes viennent d'arriver. On prŽvient le lien.
			GetLink()->DataPresent();
		} else {
			break;
		}
	} while ( true );
}

// --------------------------------------------------------------------------------	//
//  *ÊHandleException( TDCLException* )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::HandleException( TDCLException* inException )
{
	// On transmet ˆ la couche de communication.
	((TDCLSyncCommLayer*) GetCommLayer())->HandleException( inException );
}

// ============================================================= //
// Sendmail may be safely run set-user-id to root.               //
//                 -- Eric Allman, "Sendmail Installation Guide" //
// ============================================================= //
