// ==============================
// File:			TDCLSimpleServer.h
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
// The Original Code is TDCLSimpleServer.h.
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
// $Id: TDCLSimpleServer.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLSIMPLESERVER_H
#define _TDCLSIMPLESERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>

class TDCLLink;

///
/// Class for a simple server that listens on only a single communication
/// interface and serves only one connection at a time.
/// This is an NPI/NTK-style server.
///
/// The link must be created first (because the link assumes the server already
/// exists).
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	no tests defined.
///
class TDCLSimpleServer
	:
		public TDCLServer
{
public:
	///
	/// Constructor from an application and a communication layer.
	///
	/// \param inApplication	application notified of server activity.
	/// \param inCommLayer		communication layer used for the server.
	///
	TDCLSimpleServer(
			TDCLApplication* inApplication,
			TDCLCommLayer* inCommLayer
		);

	///
	/// Sets the link.
	/// This method may only be called before Start() has been called.
	/// Calling it once Start() has been called (even after a subsequent Stop()
	/// or Kill()) throws TDCLBadStateError.
	///
	/// \param inLink		link to use when a connection is established.
	/// 	hrows TDCLBadStateError if Start() has already been called.
	///
	void				SetLink( TDCLLink* inLink );
	
	///
	/// Destructor.
	///
	virtual	~TDCLSimpleServer( void );
	
protected:

	///
	/// Starts listening.
	/// This method is called when the server is started (from the server
	/// thread).
	///
	virtual	void		DoStartListening( void );

	///
	/// Stops the server.
	///
	virtual	void		DoStop( void );
	
	///
	/// Forcefully closes all connections.
	///
	virtual	void		DoKill( void );

	///
	/// Handles the arrival of a new connection.
	///
	/// \param inCommLayer	communication interface that received the request.
	///						(ignored)
	/// \throws TDCLException if a problem occurred.
	///
	virtual EState		HandleIncomingConnection(
								TDCLCommLayer* inCommLayer );

	///
	/// Method called on the server thread to indicate that a link has been
	/// disconnected and the communication interface must be closed.
	///
	/// \param inLink	the link that was disconnected.
	/// \return the new state of the server.
	/// \throws TDCLException if a problem occurred.
	///
	virtual	EState		HandleLinkDisconnection( TDCLLink* inLink );

	///
	/// Method called by the server when nothing is happening.
	/// It allows calling Idle on communication interfaces that need it.
	///
	virtual	void		DoIdle( void );

	///
	/// Accessor for the link.
	///
	/// \return the link to use when a connection is established.
	///
	inline TDCLLink*		GetLink( void )
		{
			return mLink;
		}
private:
	///
	/// Copy constructor intentionally unavailable.
	///
	/// \param inCopy		object to copy
	///
	TDCLSimpleServer( const TDCLSimpleServer& inCopy );

	///
	/// Assignment operator intentionally unavailable.
	///
	/// \param inCopy		object to copy
	///
	TDCLSimpleServer& operator = ( const TDCLSimpleServer& inCopy );

	/// \name Member variables

	TDCLLink*			mLink;			///< Reference to the single link.
	TDCLCommLayer*		mCommLayer;		///< Reference to the communication
										///< interface.
	TDCLPipe*			mPipe;			///< Reference to the connection, \c nil
										///< if not connected.
};

#endif
		// _TDCLSIMPLESERVER_H

// ======================================================== //
// Blinding speed can compensate for a lot of deficiencies. //
//                 -- David Nichols                         //
// ======================================================== //
