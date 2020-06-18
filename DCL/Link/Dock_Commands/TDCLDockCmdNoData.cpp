// ==============================
// Fichier:			TDCLDockCmdNoData.cp
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
// The Original Code is TDCLDockCmdNoData.cp.
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
// $Id: TDCLDockCmdNoData.cp,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>


// =========================================================== //
//   ••••• Implementation for class TDCLDockCmdNoData •••••    //
// =========================================================== //

// --------------------------------------------------------------------------------	//
//	• TDCLDockCmdNoData( void )
// --------------------------------------------------------------------------------	//
// Default Constructor

TDCLDockCmdNoData::TDCLDockCmdNoData( void )
	:
		TDCLDockCommand ()
{
}

// --------------------------------------------------------------------------------	//
//	• TDCLDockCmdNoData( KUInt32 )
// --------------------------------------------------------------------------------	//
// Preferred Constructor

TDCLDockCmdNoData::TDCLDockCmdNoData( KUInt32 inCommand )
	:
		TDCLDockCommand ( inCommand )
{
}

// --------------------------------------------------------------------------------	//
//	• GetLength( void )
// --------------------------------------------------------------------------------	//
// Required function for the implementation. Returns 0.

KUInt32
TDCLDockCmdNoData::GetLength( void ) const
{
	return 0;
}

// ====================================================================== //
// My sister opened a computer store in Hawaii.  She sells C shells down  //
// by the seashore.                                                       //
// ====================================================================== //
