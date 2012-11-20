// ==============================
// Fichier:			TDCLMacFolder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			10/1/2003
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
// The Original Code is TDCLMacFolder.cp.
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
// $Id: TDCLMacFolder.cp,v 1.3 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacFolder.h>

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// ------------------------------------------------------------------------- //
//  * TDCLMacFolder( TDCLMacFiles*, TDCLFSItemRef )
// ------------------------------------------------------------------------- //
TDCLMacFolder::TDCLMacFolder(
				TDCLMacFiles* inFilesIntf,
				TDCLFSItemRef inParentFolder /* = TDCLFSItemRef() */ )
	:
		TDCLFolder( inFilesIntf, inParentFolder )
{
	// Défense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * ToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacFolder::ToFrame( void )
{
	TDCLNSRef theSegment = TDCLFolder::ToFrame();
	TDCLNSFrame& theSegmentAsFrame = theSegment.ToFrame();
	
	Boolean needVolRef = false;	// S'il faut ajouter la référence du volume
	
	if (IsVolume())
	{
		// Nous sommes un volume.
		theSegmentAsFrame.Set( "type",
			TDCLNSRef::MakeInt( TDCLDockCommand::kDisk_FSType ) );
		theSegmentAsFrame.Set( "disktype", TDCLNSRef::MakeSymbol( "harddisk" ) );
		
		needVolRef = true;
	} else if (IsOnDesktop()) {
		needVolRef = true;
	}
	
	// Remarque: ce n'est pas tout à fait ce que dit le protocole.
	// Normalement, on a un nombre négatif pour les éléments sur le bureau
	// et 0 pour les disques, sauf qu'il nous faut bien distinguer les volumes
	// homonymes sur le bureau.
	if (needVolRef)
	{
		// Ajout du volume.
		theSegmentAsFrame.Set( "whichVol", TDCLNSRef::MakeInt( GetVRefNum() ) );
	}
	
	return theSegment;
}

// =============================================================================== //
//         A manager was about to be fired, but a programmer who worked for him    //
// invented a new program that became popular and sold well.  As a result, the     //
// manager retained his job.                                                       //
//         The manager tried to give the programmer a bonus, but the programmer    //
// refused it, saying, "I wrote the program because I though it was an interesting //
// concept, and thus I expect no reward."                                          //
//         The manager, upon hearing this, remarked, "This programmer, though he   //
// holds a position of small esteem, understands well the proper duty of an        //
// employee.  Lets promote him to the exalted position of management consultant!"  //
//         But when told this, the programmer once more refused, saying, "I exist  //
// so that I can program.  If I were promoted, I would do nothing but waste        //
// everyone's time.  Can I go now?  I have a program that I'm working on."         //
//                 -- Geoffrey James, "The Tao of Programming"                     //
// =============================================================================== //