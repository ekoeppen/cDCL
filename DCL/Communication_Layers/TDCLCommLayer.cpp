// ==============================
// File:			TDCLCommLayer.cpp
// Project:			Desktop Connection Library
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
// The Original Code is TDCLCommLayer.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Communication_Layers/TDCLCommLayer.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Interfaces/TDCLEventBus.h>
#include <DCL/Server/TDCLServer.h>

TDCLCommLayer::TDCLCommLayer(
	Boolean inFlushAvailable /* = false */,
	KUInt32 inMaxPipeCount /* = kDefaultPipeCount */ )
	:
		mFlushAvailable( inFlushAvailable ),
		mMaxPipeCount( inMaxPipeCount ),
		mServer( nil ),
		mSessionBus()
{
}

void
TDCLCommLayer::Idle( void )
{
}

void
TDCLCommLayer::StartListening( TDCLServer* inServer )
{
	if ( mServer )
	{
		throw DCLBadStateError;
	}

	mServer = inServer;
	mSessionBus = inServer ? inServer->GetSharedSessionBus() : nullptr;

	ResetStopSource();
	DoStartListening();

	if ( inServer )
	{
		inServer->WaitingConnection( this );
	}
}

void
TDCLCommLayer::StopListening( void )
{
	if ( mServer )
	{
		(void) RequestStop();

		try
		{
			DoStopListening();
		}
		catch ( ... )
		{
		}

		mServer = nil;
		mSessionBus.reset();
	}
}
