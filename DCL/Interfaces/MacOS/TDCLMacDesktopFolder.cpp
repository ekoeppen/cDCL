// ==============================
// Fichier:			TDCLMacDesktopFolder.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			11/1/2003
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
// The Original Code is TDCLMacDesktopFolder.cp.
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
// $Id: TDCLMacDesktopFolder.cp,v 1.6 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacDesktopFolder.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //
const char* const TDCLMacDesktopFolder::kDesktopStr = "Desktop";

// -------------------------------------------------------------------------- //
//  * TDCLMacDesktopFolder( TDCLMacFiles* )
// -------------------------------------------------------------------------- //
TDCLMacDesktopFolder::TDCLMacDesktopFolder( TDCLMacFiles* inFilesIntf )
	:
		TDCLFolder( inFilesIntf ),
		TDCLRootFolder( inFilesIntf )
{
}

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLMacDesktopFolder::MakeName( void ) const
{
	// On retourne "Desktop"
	size_t theSize = ::strlen( kDesktopStr ) + 1;
	
	KUInt16* theResult = (KUInt16*) ::malloc( theSize * sizeof( KUInt16 ) );
	
	UUTF16CStr::FromMacRoman(
					(const KUInt8*) kDesktopStr, theResult, theSize );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * ToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacDesktopFolder::ToFrame( void )
{
	TDCLNSRef theSegment = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theSegmentAsFrame = theSegment.ToFrame();
	
	// Ajout du nom
	theSegmentAsFrame.Set( "name", TDCLNSRef::MakeString( GetName() ) );
	
	// Puis le type.
	theSegmentAsFrame.Set( "type",
		TDCLNSRef::MakeInt( TDCLDockCommand::kDesktop_FSType ) );

	return theSegment;
}

// ======================================= //
// Can't open /usr/games/lib/fortunes.dat. //
// ======================================= //
