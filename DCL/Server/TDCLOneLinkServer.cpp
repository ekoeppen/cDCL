// ==============================
// File:			TDCLOneLinkServer.cpp
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
// The Original Code is TDCLOneLinkServer.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2002-2004 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLOneLinkServer.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>

// -------------------------------------------------------------------------- //
//  * TDCLOneLinkServer( TDCLApplication* )
// -------------------------------------------------------------------------- //
TDCLOneLinkServer::TDCLOneLinkServer( TDCLApplication* inApplication )
		:
			TDCLSingleLinkServer( inApplication )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLOneLinkServer( void )
// -------------------------------------------------------------------------- //
TDCLOneLinkServer::~TDCLOneLinkServer( void )
{
	Kill();
	DoRequestStop();
	DoKill();
}

// -------------------------------------------------------------------------- //
//  * AddCommLayer( std::unique_ptr<TDCLCommLayer> )
// -------------------------------------------------------------------------- //
TDCLCommLayer*
TDCLOneLinkServer::AddCommLayer( std::unique_ptr<TDCLCommLayer> inLayer )
{
	auto lock = AcquireConfigurationLock();

	if ( !inLayer )
	{
		throw DCLBadParamError;
	}

	TDCLCommLayer* theLayerPtr = inLayer.get();
	mCommLayers.push_back( std::move( inLayer ) );
	return theLayerPtr;
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStartListening( void )
{
	for ( const auto& layer : mCommLayers )
	{
		layer->StartListening( this );
	}
}

// -------------------------------------------------------------------------- //
//  * DoStopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoStopListening( void )
{
	for ( const auto& layer : mCommLayers )
	{
		layer->StopListening();
	}
}

// -------------------------------------------------------------------------- //
//  * DoRequestStopCommLayers( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoRequestStopCommLayers( void )
{
	for ( const auto& layer : mCommLayers )
	{
		(void)layer->RequestStop();
	}
}

// -------------------------------------------------------------------------- //
//  * DoIdle( void )
// -------------------------------------------------------------------------- //
void
TDCLOneLinkServer::DoIdle( void )
{
	for ( const auto& layer : mCommLayers )
	{
		layer->Idle();
	}
}
