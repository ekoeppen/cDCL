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

// C++
#include <thread>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Link/TDCLLink.h>
#include <DCL/Server/TDCLServer.h>

// -------------------------------------------------------------------------- //
//  * TDCLSyncCommLayer( Boolean, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLSyncCommLayer::TDCLSyncCommLayer(
					Boolean	inFlushAvailable /* = false */,
					KUInt32	inMaxPipeCount /* = kDefaultPipeCount */
				)
	:
		TDCLCommLayer( inFlushAvailable, inMaxPipeCount ),
		mWorkerWoken( false ),
		mWorkerThreadId()
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLSyncCommLayer( void )
// -------------------------------------------------------------------------- //
TDCLSyncCommLayer::~TDCLSyncCommLayer( void )
{
	(void) RequestStop();
	WakeWorker();
	JoinWorker();
}

bool
TDCLSyncCommLayer::IsCurrentThread( void ) const noexcept
{
	return std::this_thread::get_id() == mWorkerThreadId.load();
}

void
TDCLSyncCommLayer::WorkerEntry( void )
{
	mWorkerThreadId.store( std::this_thread::get_id() );
	try {
		Run();
	} catch ( TDCLException& inException ) {
		try {
			HandleException( &inException );
		} catch ( ... ) {
		}
	} catch ( ... ) {
		try {
			HandleException( nil );
		} catch ( ... ) {
		}
	}
	mWorkerThreadId.store( std::thread::id() );
}

void
TDCLSyncCommLayer::WakeWorker( void )
{
	{
		std::lock_guard<std::mutex> lock( mWorkerMutex );
		mWorkerWoken = true;
	}
	mWorkerCV.notify_one();
}

void
TDCLSyncCommLayer::WaitForWorkerWake( void )
{
	std::unique_lock<std::mutex> lock( mWorkerMutex );
	(void)mWorkerCV.wait( lock, GetStopToken(), [ this ] { return mWorkerWoken; } );
	mWorkerWoken = false;
}

void
TDCLSyncCommLayer::JoinWorker( void )
{
	if ( mWorkerThread.joinable() && ( mWorkerThread.get_id() != std::this_thread::get_id() ) )
	{
		mWorkerThread.join();
	}
}

// ------------------------------------------------------------------------- //
//  * StartListening( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLSyncCommLayer::StartListening( TDCLServer* inServer )
{
	// Exception si on est déjà en train d'écouter.
	if (GetServer())
	{
		throw DCLBadStateError;
	}

	JoinWorker();

	// Reset the communication layer's stop token.
	ResetStopSource();

	// Enregistrement du serveur.
	SetServer( inServer );

	{
		std::lock_guard<std::mutex> lock( mWorkerMutex );
		mWorkerWoken = false;
	}

	// Démarrage du processus léger.
	// Celui-ci appellera DoStartListening.
	mWorkerThread = std::jthread( [ this ]( std::stop_token ) { WorkerEntry(); } );
}

// ------------------------------------------------------------------------- //
//  * StopListening( void )
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
			WakeWorker();

			// On attend que le processus léger termine.
			JoinWorker();
		} catch ( ... ) {
			// On reste silencieux.
		}
		
		SetServer( nil );
	}
}

// ------------------------------------------------------------------------- //
//  * Run( void )
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

		WaitForWorkerWake();	// On attend que la requête soit acceptée ou refusée.
	}
}

// ------------------------------------------------------------------------- //
//  * Accept( void )
// ------------------------------------------------------------------------- //
std::unique_ptr<TDCLPipe>
TDCLSyncCommLayer::Accept( void )
{
	std::unique_ptr<TDCLPipe> thePipe = DoAccept();

	// Réveil du processus léger (pour la prochaine requête).
	WakeWorker();

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
	WakeWorker();
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
//  * TSyncPipe( TDCLSyncCommLayer* )
// --------------------------------------------------------------------------------	//
TDCLSyncCommLayer::TSyncPipe::TSyncPipe( TDCLSyncCommLayer* inCommLayer )
	:
		TDCLPipe( inCommLayer ),
		mWorkerWoken( false ),
		mWorkerThreadId()
{
}

// --------------------------------------------------------------------------------	//
//  * ~TSyncPipe( void )
// --------------------------------------------------------------------------------	//
TDCLSyncCommLayer::TSyncPipe::~TSyncPipe( void )
{
	(void) RequestStop();
	WakeWorker();
	JoinWorker();
}

bool
TDCLSyncCommLayer::TSyncPipe::IsCurrentThread( void ) const noexcept
{
	return std::this_thread::get_id() == mWorkerThreadId.load();
}

void
TDCLSyncCommLayer::TSyncPipe::WorkerEntry( void )
{
	struct ThreadIdGuard
	{
		std::atomic<std::thread::id>& mId;
		ThreadIdGuard( std::atomic<std::thread::id>& inId )
			: mId( inId )
		{
			mId.store( std::this_thread::get_id() );
		}
		~ThreadIdGuard()
		{
			mId.store( std::thread::id() );
		}
	} guard( mWorkerThreadId );

	try {
		Run();
	} catch ( TDCLException& inException ) {
		try {
			HandleException( &inException );
		} catch ( ... ) {
		}
	} catch ( ... ) {
		try {
			HandleException( nil );
		} catch ( ... ) {
		}
	}
}

void
TDCLSyncCommLayer::TSyncPipe::WakeWorker( void )
{
	{
		std::lock_guard<std::mutex> lock( mWorkerMutex );
		mWorkerWoken = true;
	}
	mWorkerCV.notify_one();
}

void
TDCLSyncCommLayer::TSyncPipe::WaitForWorkerWake( void )
{
	std::unique_lock<std::mutex> lock( mWorkerMutex );
	(void)mWorkerCV.wait( lock, GetStopToken(), [ this ] { return mWorkerWoken; } );
	mWorkerWoken = false;
}

void
TDCLSyncCommLayer::TSyncPipe::JoinWorker( void )
{
	if ( mWorkerThread.joinable() && ( mWorkerThread.get_id() != std::this_thread::get_id() ) )
	{
		mWorkerThread.join();
	}
}

// --------------------------------------------------------------------------------	//
//  * Disconnect( void )
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
		WakeWorker();
		
		// On attend que le processus léger termine.
		JoinWorker();
	}
}

// --------------------------------------------------------------------------------	//
//  * Connected( TDCLLink* )
// --------------------------------------------------------------------------------	//
TDCLCommLayer*
TDCLSyncCommLayer::TSyncPipe::Connected( TDCLLink* inLink )
{
	JoinWorker();

	ResetStopSource();

	// Appel de la méthode par défaut.
	TDCLCommLayer* theResult = TDCLPipe::Connected( inLink );

	{
		std::lock_guard<std::mutex> lock( mWorkerMutex );
		mWorkerWoken = false;
	}

	// Démarrage du processus léger.
	mWorkerThread = std::jthread( [ this ]( std::stop_token ) { WorkerEntry(); } );

	// A stop requested by the link before ResetStopSource() would be lost: re-request it.
	if (inLink && inLink->StopRequested())
	{
		(void) RequestStop();
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * ClearDataPresent( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::ClearDataPresent( void )
{
	WakeWorker();
}

// --------------------------------------------------------------------------------	//
//  * Run( void )
// --------------------------------------------------------------------------------	//
void
TDCLSyncCommLayer::TSyncPipe::Run( void )
{
	do {
		WaitForWorkerWake();	// On attend que le lien nous demande si on veut des données.

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
