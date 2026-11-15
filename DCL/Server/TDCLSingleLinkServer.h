// ==============================
// File:			TDCLSingleLinkServer.h
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
// The Original Code is TDCLSingleLinkServer.h.
//
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2002-2004 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLSINGLELINKSERVER_H
#define _TDCLSINGLELINKSERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

// C++
#include <memory>

///
/// Shared base class for servers managing a single owned TDCLLink.
///
class TDCLSingleLinkServer
	:
		public TDCLServer
{
public:
	///
	/// Constructor from an application.
	///
	TDCLSingleLinkServer( TDCLApplication* inApplication );

	///
	/// Transfers ownership of the link to the server and returns a non-owning pointer.
	/// This method may only be called before Start() has been called.
	/// Calling it once Start() has been called (even after a subsequent Stop()
	/// or Kill()) throws TDCLBadStateError.
	///
	/// \param inLink		link to use when a connection is established.
	/// \return non-owning pointer to the installed link.
	/// \throws TDCLBadStateError if Start() has already been called.
	///
	TDCLLink*			SetLink( std::unique_ptr<TDCLLink> inLink );

	///
	/// Destructor.
	///
	virtual	~TDCLSingleLinkServer( void );

protected:
	///
	/// Stops all communication layers owned by the subclass.
	///
	virtual	void		DoStopListening( void ) = 0;

	///
	/// Requests cooperative stop on all communication layers owned by the subclass.
	///
	virtual	void		DoRequestStopCommLayers( void ) = 0;

	///
	/// Stops the server cleanly.
	///
	virtual	void		DoStop( void ) override;

	///
	/// Requests cooperative stop on the active link and comm layers.
	///
	virtual	void		DoRequestStop( void ) override;

	///
	/// Returns \c true if called from the server thread or owned link thread.
	///
	virtual	bool		IsShutdownWorkerThread( void ) const override;

	///
	/// Forcefully closes the active link and stops listening.
	///
	virtual	void		DoKill( void ) override;

	///
	/// Handles an incoming connection by accepting into the single link or refusing.
	///
	virtual	EState		HandleIncomingConnection(
							TDCLCommLayer* inCommLayer ) override;

	///
	/// Handles link disconnection.
	///
	virtual	EState		HandleLinkDisconnection( TDCLLink* inLink ) override;

	///
	/// Non-owning accessor for the owned link.
	///
	inline	TDCLLink*	GetLink( void ) const
		{
			return mLink.get();
		}

private:
	std::unique_ptr<TDCLLink>	mLink;
};

#endif
// _TDCLSINGLELINKSERVER_H
