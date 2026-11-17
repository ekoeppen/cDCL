// ==============================
// File:			TDCLSimpleServer.cp
// Project:			Desktop Connection Library
//
// Created:			15/1/2002
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
// The Original Code is TDCLSimpleServer.cp.
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
	// Delete the connection.
	if (mPipe)
	{
		delete mPipe;
	}
}

// -------------------------------------------------------------------------- //
//  * SetLink( TDCLLink* )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::SetLink( TDCLLink* inLink )
{
	AcquireConfigurationLock();
	mLink = inLink;
	ReleaseConfigurationLock();
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStartListening( void )
{
	mCommLayer->StartListening(this);
}

// -------------------------------------------------------------------------- //
//  * DoStop( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStop( void )
{
	if ((mLink) && (mLink->IsConnected()))
	{
		try {
			(void) mLink->Disconnect();	// No need to know whether it was
										// actually connected.
		} catch (...) {
			// Silently catch exceptions
		}
	}

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
			// Silently catch exceptions.
		}

		// Tell the link it was disconnected.
		if (mLink)
		{
			(void) mLink->Close();
		}

		// Delete the interface.
		delete mPipe;
		mPipe = nil;
	}

	mCommLayer->StopListening();
}

// -------------------------------------------------------------------------- //
//  * HandleIncomingConnection( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
TDCLServer::EState
TDCLSimpleServer::HandleIncomingConnection( TDCLCommLayer* /* inCommLayer */ )
{
	// If the link is connected, refuse the connection.
	if ((mLink) && (mLink->IsConnected()))
	{
		mCommLayer->Refuse();
	} else {
		// Otherwise, accept it.

		// If a TDCLPipe object was present (from an unknown state), delete it
		// here.
		if (mPipe)
		{
			delete mPipe;
		}

		// Accept the connection.
		mPipe = mCommLayer->Accept();
				
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
TDCLSimpleServer::HandleLinkDisconnection( TDCLLink* /* inLink */ )
{
	if (mPipe)
	{	
		mPipe->Disconnect();

		// Delete the communication interface.
		delete mPipe;
		mPipe = nil;
	} else {
		throw DCLBadStateError;	// Should not happen.
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
