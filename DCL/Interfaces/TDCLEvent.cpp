// ==============================
// Fichier:			TDCLEvent.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			28/3/2003
// Tabulation:		4 espaces
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
// The Original Code is TDCLEvent.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLEvent.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEvent.h>

// -------------------------------------------------------------------------- //
//  * TDCLEvent( EEventID, void* )
// -------------------------------------------------------------------------- //
TDCLEvent::TDCLEvent(
				EEventKind inEventKind,
				EEventID inEventID,
				void* inData /* = nil */ )
	:
		mEventKind( inEventKind ),
		mEventID( inEventID ),
		mEventData( inData ),
		mNextEvent( nil )
{
	// DŽfense d'afficher.
}

// -------------------------------------------------------------------------- //
//  * TDCLEvent( EEventID, void* )
// -------------------------------------------------------------------------- //
TDCLEvent::TDCLEvent( const TDCLEvent& inCopy )
	:
		mEventKind( inCopy.mEventKind ),
		mEventID( inCopy.mEventID ),
		mEventData( inCopy.mEventData ),
		mNextEvent( nil )
{
	// Loi du 29 juillet 1881
}

// -------------------------------------------------------------------------- //
//  * ~TDCLEvent( void )
// -------------------------------------------------------------------------- //
TDCLEvent::~TDCLEvent( void )
{
	// This space for rent.
}

// =========================================================== //
// Does a good farmer neglect a crop he has planted?           //
// Does a good teacher overlook even the most humble student?  //
// Does a good father allow a single child to starve?          //
// Does a good programmer refuse to maintain his code?         //
//                 -- Geoffrey James, "The Tao of Programming" //
// =========================================================== //
