// ==============================
// Fichier:			TDCLFolder.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			09/02/2003
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
// The Original Code is TDCLFolder.cp.
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
// $Id: TDCLFolder.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFolder.h>

// DCL
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// ------------------------------------------------------------------------- //
//  * TDCLFolder( IDCLFiles*, TDCLFSItemRef )
// ------------------------------------------------------------------------- //
TDCLFolder::TDCLFolder(
			IDCLFiles* inFilesIntf,
			TDCLFSItemRef inParentFolder /* = TDCLFSItemRef() */ )
	:
		TDCLFSItem( inFilesIntf, kFolder, inParentFolder )
{
	// DŽfense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * ToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLFolder::ToFrame( void )
{
	TDCLNSRef theSegment = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theSegmentAsFrame = theSegment.ToFrame();
	
	// Ajout du nom
	theSegmentAsFrame.Set( "name", TDCLNSRef::MakeString( GetName() ) );
	
	// Puis le type.
	theSegmentAsFrame.Set( "type",
		TDCLNSRef::MakeInt( TDCLDockCommand::kFolder_FSType ) );

	return theSegment;
}

// ========================================================================== //
//         A master was explaining the nature of Tao to one of his novices.   //
// "The Tao is embodied in all software -- regardless of how insignificant,"  //
// said the master.                                                           //
//         "Is Tao in a hand-held calculator?" asked the novice.              //
//         "It is," came the reply.                                           //
//         "Is the Tao in a video game?" continued the novice.                //
//         "It is even in a video game," said the master.                     //
//         "And is the Tao in the DOS for a personal computer?"               //
//         The master coughed and shifted his position slightly.  "The lesson //
// is over for today," he said.                                               //
//                 -- Geoffrey James, "The Tao of Programming"                //
// ========================================================================== //
