// ==============================
// Fichier:			TDCLMacFiles.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLMacFiles.cp.
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
// $Id: TDCLMacFiles.cp,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>

// MacOS
#if TARGET_OS_MACOS
#include <Files.h>
#include <Processes.h>
#include <Script.h>
#include <MacErrors.h>
#else
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

// ANSI C
#include <string.h>

// K
#include <K/Misc/UPStrings.h>
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7DesktopFolder.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonDesktopFolder.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

// ------------------------------------------------------------------------- //
//  * TDCLMacFiles( void )
// ------------------------------------------------------------------------- //
TDCLMacFiles::TDCLMacFiles( void )
	:
#if TARGET_OS_OPENSTEP
		mRootFolder( new TDCLMacCarbonDesktopFolder( this ) )
#else
		mRootFolder( new TDCLMacOS7DesktopFolder( this ) )
#endif
{
	// Recherche annonceurs.
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMacFiles( void )
// ------------------------------------------------------------------------- //
TDCLMacFiles::~TDCLMacFiles( void )
{
}

// ------------------------------------------------------------------------- //
//	* GetKind( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacFiles::GetKind( void )
{
	return TDCLDockCommand::kMac_DType;
}

// ------------------------------------------------------------------------- //
//	* GetRootFolder( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacFiles::GetRootFolder( const KUInt16* /* inRootName = nil */ )
{
	return mRootFolder;
}

// ------------------------------------------------------------------------- //
//  * CountRootFolders( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacFiles::CountRootFolders( void ) const
{
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetRootFolderName( KUInt32 ) const
// ------------------------------------------------------------------------- //
const KUInt16*
TDCLMacFiles::GetRootFolderName( KUInt32 /* inFolderIndex */ ) const
{
	throw DCLNotImplementedError;
}

// ============================================== //
// Nobody said computers were going to be polite. //
// ============================================== //