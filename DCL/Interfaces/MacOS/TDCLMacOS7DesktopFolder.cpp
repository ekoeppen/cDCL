// ==============================
// Fichier:			TDCLMacOS7DesktopFolder.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			10/1/2003
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
// The Original Code is TDCLMacOS7DesktopFolder.cp.
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
// $Id: TDCLMacOS7DesktopFolder.cp,v 1.5 2004/11/24 18:11:19 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7DesktopFolder.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>

// -------------------------------------------------------------------------- //
// TDCLMacOS7DesktopFolder( TDCLMacFiles* )
// -------------------------------------------------------------------------- //
TDCLMacOS7DesktopFolder::TDCLMacOS7DesktopFolder( TDCLMacFiles* inFilesIntf )
	:
		TDCLMacDesktopFolder( inFilesIntf ),
		TDCLFolder( inFilesIntf )
{
}

// -------------------------------------------------------------------------- //
//  * GetItemByName( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7DesktopFolder::GetItemByName(
							const KUInt16* inName,
							KSInt32 inVolRefNum /* = 0 */ )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * CreateFile( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7DesktopFolder::CreateFile( const KUInt16* inName )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * CreateFolder( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7DesktopFolder::CreateFolder( const KUInt16* inName )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * CountElements( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLMacOS7DesktopFolder::CountElements( void )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * GetItems( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLMacOS7DesktopFolder::GetItems( void )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// ================================================= //
// Help!  I'm trapped in a Chinese computer factory! //
// ================================================= //