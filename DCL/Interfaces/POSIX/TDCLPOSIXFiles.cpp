// ==============================
// Fichier:			TDCLPOSIXFiles.cp
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
// The Original Code is TDCLPOSIXFiles.cp.
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
// $Id: TDCLPOSIXFiles.cp,v 1.8 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXRootDir.h>

// ------------------------------------------------------------------------- //
//  * Constantes
// ------------------------------------------------------------------------- //
const KUInt32 TDCLPOSIXFiles::kMinutesBetween1904and1970 = 34714080;
const KUInt16 TDCLPOSIXFiles::kRootFolderName[3] = { '/', ':', '\0' };

// ------------------------------------------------------------------------- //
//  * TDCLPOSIXFiles( Boolean )
// ------------------------------------------------------------------------- //
TDCLPOSIXFiles::TDCLPOSIXFiles( Boolean inFollowSymLinks )
	:
		mRootFolder( new TDCLPOSIXRootDir( this ) ),
		mFollowSymLinks( inFollowSymLinks )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLPOSIXFiles( void )
// ------------------------------------------------------------------------- //
TDCLPOSIXFiles::~TDCLPOSIXFiles( void )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//	* GetKind( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLPOSIXFiles::GetKind( void )
{
	return TDCLDockCommand::kWin_DType;
}

// ------------------------------------------------------------------------- //
//	* GetRootFolder( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXFiles::GetRootFolder( const KUInt16* /* inRootName = nil */ )
{
	return mRootFolder;
}

// ------------------------------------------------------------------------- //
//	* CountRootFolders( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLPOSIXFiles::CountRootFolders( void ) const
{
	// On n'a qu'une seule racine. Cette classe doit être dérivée pour
	// en avoir plus d'une.
	return 1;
}

// ------------------------------------------------------------------------- //
//	* GetRootFolderName( KUInt32 ) const
// ------------------------------------------------------------------------- //
const KUInt16*
TDCLPOSIXFiles::GetRootFolderName( KUInt32 inFolderIndex ) const
{
	if (inFolderIndex != 0)
	{
		throw DCLBadParamError;
	}

	return kRootFolderName;
}

// ========================================================================= //
// Such efforts are almost always slow, laborious, political, petty, boring, //
// ponderous, thankless, and of the utmost criticality.                      //
//         -- Leonard Kleinrock, on standards efforts                        //
// ========================================================================= //
