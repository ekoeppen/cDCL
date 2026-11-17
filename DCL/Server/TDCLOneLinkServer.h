// ==============================
// File:			TDCLOneLinkServer.h
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
// The Original Code is TDCLOneLinkServer.h.
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
// $Id: TDCLOneLinkServer.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLONELINKSERVER_H
#define _TDCLONELINKSERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

///
/// Class for a server with a single link and multiple communication layers.
/// This is an NCU-style server.
///
/// The link must be created first (because the link assumes the server already
/// exists).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	no tests defined.
///
class TDCLOneLinkServer
	:
		public TDCLServer
{
public:
	///
	/// Constructor from an application.
	///
	/// \param inApplication	application notified of server activity.
	///
	TDCLOneLinkServer( TDCLApplication* inApplication );
	
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
	virtual	~TDCLOneLinkServer( void );

	///
	/// Adds a communication layer.
	/// This method may only be called before Start() has been called.
	/// Calling it once Start() has been called (even after a subsequent Stop()
	/// or Kill()) throws TDCLBadStateError.
	/// Listening begins when Start() runs.
	/// The communication layer must be deleted by the application.
	///
	/// \param inLayer	the layer to add to the server.
	/// 	hrows TDCLBadStateError if Start() has already been called.
	/// 	hrows TDCLBadParamError if inLayer is \c nil.
	///
	void		AddCommLayer( TDCLCommLayer* inLayer );
	
	///
	/// Function used to determine whether the server is "active" or not.
	///
	/// \return true if the server is already connected, false otherwise.
	///
	TDCLLink*			GetConnectionLink( void );

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
	TDCLOneLinkServer( const TDCLOneLinkServer& inCopy );

	///
	/// Assignment operator intentionally unavailable.
	///
	/// \param inCopy		object to copy
	///
	TDCLOneLinkServer& operator = ( const TDCLOneLinkServer& inCopy );

	/// \name Member variables
	
	TDCLLink*					mLink;			///< Reference to the single
												///< link.
	TDCLPipe*					mPipe;			///< Reference to the
												///< connection, \c nil if not
												///< connected.
	TDCLCommLayer**				mCommLayers;	///< List of communication
												///< layers.
	KUInt32						mNbCommLayers;	///< Number of elements in the
												///< previous list.
};

#endif
		// _TDCLONELINKSERVER_H

// =========================== //
// Memory fault -- brain fried //
// =========================== //
