// ==============================
// Fichier:			TDCLDockCmdNewtonInfo.cp
// Projet:			Desktop Connection Library
//
// Créé le:			06/02/2001
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
// The Original Code is TDCLDockCmdNewtonInfo.cp.
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
// $Id: TDCLDockCmdNewtonInfo.cp,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// DCL
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdNewtonInfo( KUInt32 inLength, const void* inData )
// --------------------------------------------------------------------------------	//
TDCLDockCmdNewtonInfo::TDCLDockCmdNewtonInfo( KUInt32 inLength, const void* inData )
	:
		TDCLDockCommand( kDNewtonInfo )
{
	// Let's perform some checks about the data
	if ( inLength != sizeof( SNewtonInfo ) )
	{
		throw DCLBadDockCmd;
	}
	
	// Let's copy the data.
	(void) ::memcpy( &mNewtonInfo, inData, sizeof (SNewtonInfo) );

	// That's all folks.
}

// ======================================================================== //
// Unix is a lot more complicated (than CP/M) of course -- the typical Unix //
// hacker can never remember what the PRINT command is called this week --  //
// but when it gets right down to it, Unix is a glorified video game.       //
// People don't do serious work on Unix systems; they send jokes around the //
// world on USENET or write adventure games and research papers.            //
//                 -- E. Post                                               //
//                 "Real Programmers Don't Use Pascal", Datamation, 7/83    //
// ======================================================================== //
