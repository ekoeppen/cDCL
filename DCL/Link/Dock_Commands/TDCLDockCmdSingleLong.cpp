// ==============================
// Fichier:			TDCLDockCmdSingleLong.cp
// Projet:			Desktop Connection Library
//
// Créé le:			02/02/2001
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
// The Original Code is TDCLDockCmdSingleLong.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdSingleLong.cp,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdSingleLong( void )
// --------------------------------------------------------------------------------	//
TDCLDockCmdSingleLong::TDCLDockCmdSingleLong( void )
	:
		TDCLDockCommand(),
		mLong( 0 )
{
}

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdSingleLong( KUInt32, KUInt32 )
// --------------------------------------------------------------------------------	//
TDCLDockCmdSingleLong::TDCLDockCmdSingleLong( KUInt32 inCommand, KUInt32 inLong )
	:
		TDCLDockCommand( inCommand ),
		mLong( UByteSex_ToBigEndian( inLong ) )
{
}

// --------------------------------------------------------------------------------	//
//  * GetData( void )
// --------------------------------------------------------------------------------	//
const void*
TDCLDockCmdSingleLong::GetData( void )
{
	return (void*) &mLong;
}

// --------------------------------------------------------------------------------	//
//  * GetLength( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLDockCmdSingleLong::GetLength( void ) const
{
	return sizeof( mLong );
}

// ========================================================================================= //
// ... C++ offers even more flexible control over the visibility of member                   //
// objects and member functions.  Specifically, members may be placed in the                 //
// public, private, or protected parts of a class.  Members declared in the                  //
// public parts are visible to all clients; members declared in the private                  //
// parts are fully encapsulated; and members declared in the protected parts                 //
// are visible only to the class itself and its subclasses.  C++ also supports               //
// the notion of *_______friends*: cooperative classes that are permitted to see each //
// other's private parts.                                                                    //
//                 -- Grady Booch, "Object Oriented Design with Applications"                //
// ========================================================================================= //
