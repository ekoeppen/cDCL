// ==============================
// File:			TDCLOneLinkServer.cp
// Project:			Desktop Connection Library
//
// Created:			3/4/2003
// Tab width:		4 spaces
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

// DCL
#include <DCL/Link/TDCLLink.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
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
			mNbCommLayers( 0 )
{
	mCommLayers = ( TDCLCommLayer**  )::malloc( 0 * sizeof( TDCLCommLayer* ) );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLOneLinkServer( void )
// -------------------------------------------------------------------------- //
TDCLOneLinkServer::~TDCLOneLinkServer( void )
{	
	if (mPipe)
	{
		delete mPipe;
	}

	// Delete the list of communication interfaces.
	// These interfaces must be deleted by the application.
	if (mCommLayers)
	{
		::free( mCommLayers );
	}
}

// -------------------------------------------------------------------------- //
//  * SetLink( TDCLLink* )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::SetLink( TDCLLink* inLink )
{
	AcquireConfigurationLock();
	mLink = inLink;
	ReleaseConfigurationLock();
}

// -------------------------------------------------------------------------- //
//  * AddCommLayer( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::AddCommLayer( TDCLCommLayer* inLayer )
{
	AcquireConfigurationLock();

	if (inLayer == nil)
	{
		ReleaseConfigurationLock();
		throw DCLBadParamError;
	}

	KUInt32 indexNewCL = mNbCommLayers;
	TDCLCommLayer** newLayers = (TDCLCommLayer**) ::realloc(
					mCommLayers, (mNbCommLayers + 1) * sizeof( TDCLCommLayer* ) );
	if (newLayers == nil)
	{
		ReleaseConfigurationLock();
		throw DCLMemError;
	}

	mCommLayers = newLayers;
	mNbCommLayers++;
	mCommLayers[indexNewCL] = inLayer;

	ReleaseConfigurationLock();
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStartListening( void )
{
	// Call StartListening on each of the connection interfaces.
	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->StartListening(this);
	}
}

// -------------------------------------------------------------------------- //
//  * DoStop( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStop( void )
{
	if ((mLink) && mLink->IsConnected())
	{
		try {
			(void) mLink->Disconnect();	// Don't care whether it was
										// actually connected.
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
			// Silently catch exceptions.
		}

		// Tell the link it was disconnected.
		if (mLink)
		{
			(void) mLink->Close();
		}

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
	// If the link is connected, refuse the connection.
	if ((mLink) && (mLink->IsConnected()))
	{
		inCommLayer->Refuse();
	} else {
		// Otherwise, accept it.

		// If a TDCLPipe object was present (from an unknown state), delete it
		// here.
		if (mPipe)
		{
			delete mPipe;
		}

		mPipe = inCommLayer->Accept();
				
		// Tell the link to connect.
		if ((mLink) && (!mLink->Connect( mPipe, this )))
		{
			throw DCLBadStateError;	// Should not happen.
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

		// Delete the communication interface.
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
	// Call Idle on each of the connection interfaces.
	KUInt32 nbCL = mNbCommLayers;
	KUInt32 indexCL;
	for (indexCL = 0; indexCL < nbCL; indexCL++)
	{
		mCommLayers[indexCL]->Idle();
	}
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
