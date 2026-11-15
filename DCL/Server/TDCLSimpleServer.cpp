// ==============================
// File:			TDCLSimpleServer.cpp
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
// The Original Code is TDCLSimpleServer.cpp.
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
#include <DCL/Server/TDCLSimpleServer.h>

// -------------------------------------------------------------------------- //
//  * TDCLSimpleServer( TDCLApplication*, unique_ptr<TDCLCommLayer>, unique_ptr<TDCLLink> )
// -------------------------------------------------------------------------- //
TDCLSimpleServer::TDCLSimpleServer(
			TDCLApplication* inApplication,
			std::unique_ptr<TDCLCommLayer> inCommLayer,
			std::unique_ptr<TDCLLink> inLink )
		:
			TDCLSingleLinkServer( inApplication ),
			mCommLayer( std::move( inCommLayer ) )
{
	if ( inLink )
	{
		(void)SetLink( std::move( inLink ) );
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLSimpleServer( void )
// -------------------------------------------------------------------------- //
TDCLSimpleServer::~TDCLSimpleServer( void )
{
	Kill();
	DoRequestStop();
	DoKill();
}

// -------------------------------------------------------------------------- //
//  * SetLink( TDCLLink* )
// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStartListening( void )
{
	if ( mCommLayer )
	{
		mCommLayer->StartListening( this );
	}
}

// -------------------------------------------------------------------------- //
//  * DoStopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoStopListening( void )
{
	if ( mCommLayer )
	{
		mCommLayer->StopListening();
	}
}

// -------------------------------------------------------------------------- //
//  * DoRequestStopCommLayers( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoRequestStopCommLayers( void )
{
	if ( mCommLayer )
	{
		(void)mCommLayer->RequestStop();
	}
}

// -------------------------------------------------------------------------- //
//  * DoIdle( void )
// -------------------------------------------------------------------------- //
void
TDCLSimpleServer::DoIdle( void )
{
	if ( mCommLayer )
	{
		mCommLayer->Idle();
	}
}
