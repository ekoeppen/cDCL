// ==============================
// File:			TDCLOneLinkServer.h
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
// The Original Code is TDCLOneLinkServer.h.
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

#ifndef _TDCLONELINKSERVER_H
#define _TDCLONELINKSERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLSingleLinkServer.h>

// C++
#include <memory>
#include <vector>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Link/TDCLLink.h>

///
/// Class for a server managing a single link and multiple communication layers
/// owned by the server.
///
class TDCLOneLinkServer
	:
		public TDCLSingleLinkServer
{
public:
	///
	/// Constructor from an application.
	///
	TDCLOneLinkServer( TDCLApplication* inApplication );

	///
	/// Destructor.
	///
	virtual ~TDCLOneLinkServer( void );

	///
	/// Adds a communication layer of which the server takes ownership.
	/// This method may only be called before Start() has been called.
	/// Calling it once Start() has been called (even after a subsequent Stop()
	/// or Kill()) throws TDCLBadStateError.
	/// Listening begins when Start() runs.
	///
	/// \param inLayer	the layer to add to the server.
	/// \return non-owning pointer to the added layer.
	/// \throws TDCLBadStateError if Start() has already been called.
	/// \throws TDCLBadParamError if inLayer is null.
	///
	TDCLCommLayer*				AddCommLayer( std::unique_ptr<TDCLCommLayer> inLayer );

protected:
	virtual void	DoStartListening( void ) override;
	virtual void	DoStopListening( void ) override;
	virtual void	DoRequestStopCommLayers( void ) override;
	virtual void	DoIdle( void ) override;

private:
	TDCLOneLinkServer( const TDCLOneLinkServer& inCopy ) = delete;
	TDCLOneLinkServer& operator = ( const TDCLOneLinkServer& inCopy ) = delete;

	std::vector<std::unique_ptr<TDCLCommLayer>>	mCommLayers;
};

#endif
// _TDCLONELINKSERVER_H
