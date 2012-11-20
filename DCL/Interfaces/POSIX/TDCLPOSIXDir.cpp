// ==============================
// Fichier:			TDCLPOSIXDir.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			22/6/2003
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
// The Original Code is TDCLPOSIXDir.cp.
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
// $Id: TDCLPOSIXDir.cp,v 1.6 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXDir.h>

// SUSv2
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// -------------------------------------------------------------------------- //
//  * TDCLPOSIXDir( TDCLPOSIXFiles*, const char* )
// -------------------------------------------------------------------------- //
TDCLPOSIXDir::TDCLPOSIXDir(
				TDCLPOSIXFiles* inFilesIntf,
				TDCLFSItemRef inFolder,
				const char* inPath )
	:
		TDCLFolder( inFilesIntf, inFolder ),
		TDCLPOSIXDirBase( inFilesIntf, inFolder, inPath )
{
}

// -------------------------------------------------------------------------- //
//  * MakeParentFolder( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXDir::MakeParentFolder( void ) const
{
	// Copie du chemin (pour POSIX, dirname ne prend pas forcément un
	// const char*).
	const char* thisPath = GetPOSIXPath();
	size_t theLength = ::strlen( thisPath ) + 1;
	char* thePath = (char*) ::malloc( theLength );
	(void) ::memcpy( thePath, thisPath, theLength );
	
	TDCLFSItemRef theResult = TDCLFSItemRef(
				new TDCLPOSIXDir(
						(TDCLPOSIXFiles*) GetFilesIntf(),
						TDCLFSItemRef(),
						::dirname( thePath ) ) );

	::free( thePath );
	
	return theResult;	
}

// ============================================================================= //
// The Buddha, the Godhead, resides quite as comfortably in the circuits of a    //
// digital computer or the gears of a cycle transmission as he does at the top   //
// of a mountain or in the petals of a flower.  To think otherwise is to demean  //
// the Buddha -- which is to demean oneself.                                     //
//                 -- Robert Pirsig, "Zen and the Art of Motorcycle Maintenance" //
// ============================================================================= //
