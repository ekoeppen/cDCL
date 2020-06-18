// ==============================
// Fichier:			TDCLPOSIXRootDir.cp
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
// The Original Code is TDCLPOSIXRootDir.cp.
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
// $Id: TDCLPOSIXRootDir.cp,v 1.6 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXRootDir.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// -------------------------------------------------------------------------- //
//  * TDCLPOSIXRootDir( TDCLPOSIXFiles* )
// -------------------------------------------------------------------------- //
TDCLPOSIXRootDir::TDCLPOSIXRootDir( TDCLPOSIXFiles* inFilesIntf )
	:
		TDCLFolder( inFilesIntf ),
		TDCLPOSIXDirBase( inFilesIntf, TDCLFSItemRef(), "/" ),
		TDCLRootFolder( inFilesIntf )
{
}

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLPOSIXRootDir::MakeName( void ) const
{
	// Le répertoire racine est c:\ parce qu'on fait croire au Newton qu'on est
	// un système sous Windows.

	const char* theName = "c:\\";
	// Création de la chaîne UTF-16.
	size_t theLength = ::strlen( theName ) + 1;
	KUInt16* theResult =
		(KUInt16*) ::malloc( theLength * sizeof( KUInt16 ) );
	UUTF16CStr::FromASCII( (const KUInt8*) theName, theResult, theLength );
	
	return theResult;	
}

// ============================= //
// I have not yet begun to byte! //
// ============================= //
