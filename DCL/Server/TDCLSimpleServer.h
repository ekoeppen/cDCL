// ==============================
// File:			TDCLSimpleServer.h
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
// The Original Code is TDCLSimpleServer.h.
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

#ifndef _TDCLSIMPLESERVER_H
#define _TDCLSIMPLESERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLSingleLinkServer.h>

// C++
#include <memory>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Link/TDCLLink.h>

///
/// Class for a simple server with a single communication layer and a single
/// link, both owned by the server.
///
class TDCLSimpleServer
	:
		public TDCLSingleLinkServer
{
public:
	///
	/// Constructor from an application, a communication layer, and an
	/// optional link.
	///
	TDCLSimpleServer(
				TDCLApplication* inApplication,
				std::unique_ptr<TDCLCommLayer> inCommLayer,
				std::unique_ptr<TDCLLink> inLink = nullptr );

	///
	/// Destructor.
	/// Stops the server and destroys the link and then the communication layer.
	///
	virtual ~TDCLSimpleServer( void );

	///
	/// Non-owning accessor for the owned communication layer.
	///
	inline TDCLCommLayer*	GetCommLayer( void ) const
		{
			return mCommLayer.get();
		}

protected:
	virtual void	DoStartListening( void ) override;
	virtual void	DoStopListening( void ) override;
	virtual void	DoRequestStopCommLayers( void ) override;
	virtual void	DoIdle( void ) override;

private:
	TDCLSimpleServer( const TDCLSimpleServer& inCopy ) = delete;
	TDCLSimpleServer& operator = ( const TDCLSimpleServer& inCopy ) = delete;

	std::unique_ptr<TDCLCommLayer>	mCommLayer;
};

#endif
// _TDCLSIMPLESERVER_H
