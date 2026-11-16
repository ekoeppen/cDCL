// ==============================
// File:			TDCLSingleLinkServer.cpp
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
// The Original Code is TDCLSingleLinkServer.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot.
// Portions created by the Initial Developers are Copyright (C) 2002-2004 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLSingleLinkServer.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Link/TDCLLink.h>

TDCLSingleLinkServer::TDCLSingleLinkServer( TDCLApplication* inApplication )
	:
		TDCLServer( inApplication ),
		mLink()
{
}

TDCLSingleLinkServer::~TDCLSingleLinkServer( void )
{
	if ( mLink )
	{
		(void)mLink->RequestStop();
		(void)mLink->Wait();
	}
}

TDCLLink*
TDCLSingleLinkServer::SetLink( std::unique_ptr<TDCLLink> inLink )
{
	auto lock = AcquireConfigurationLock();
	mLink = std::move( inLink );
	return mLink.get();
}

void
TDCLSingleLinkServer::DoStop( void )
{
	if ( ( mLink ) && ( mLink->IsConnected() ) )
	{
		try {
			(void)mLink->Disconnect();
		} catch ( ... ) {
		}
	}

	DoStopListening();
}

void
TDCLSingleLinkServer::DoRequestStop( void )
{
	if ( mLink )
	{
		(void)mLink->RequestStop();
	}
	DoRequestStopCommLayers();
}

bool
TDCLSingleLinkServer::IsShutdownWorkerThread( void ) const
{
	return TDCLServer::IsShutdownWorkerThread()
		|| ( mLink && mLink->IsCurrentThread() );
}

void
TDCLSingleLinkServer::DoKill( void )
{
	if ( mLink )
	{
		(void)mLink->RequestStop();
		(void)mLink->Wait();
	}

	DoStopListening();
}

TDCLServer::EState
TDCLSingleLinkServer::HandleIncomingConnection( TDCLCommLayer* inCommLayer )
{
	if ( ( mLink ) && ( mLink->IsConnected() ) )
	{
		inCommLayer->Refuse();
	}
	else
	{
		if ( mLink )
		{
			(void)mLink->Wait();
		}

		const TDCLSessionId sessionId = inCommLayer->AcceptSession();
		if ( !sessionId )
		{
			return kRunning;
		}

		if ( mLink )
		{
			if ( !mLink->Connect( sessionId, this, inCommLayer ) )
			{
				GetSessionBus()->CloseSession( sessionId );
				throw DCLBadStateError;
			}
		}
		else
		{
			GetSessionBus()->CloseSession( sessionId );
		}
	}

	return kRunning;
}

TDCLServer::EState
TDCLSingleLinkServer::HandleLinkDisconnection( TDCLLink* /* inLink */ )
{
	return kRunning;
}
