//==============================
// Fichier:			TTestEmptyFilesIntf.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			17/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: TTestEmptyFilesIntf.cp,v 1.6 2003/12/28 07:04:50 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "TTestEmptyFilesIntf.h"

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// ------------------------------------------------------------------------- //
//  * GetKind( void )
// ------------------------------------------------------------------------- //
KUInt32
TTestEmptyFilesIntf::GetKind( void )
{
	return TDCLDockCommand::kMac_DType;
}

// ------------------------------------------------------------------------- //
//  * GetRootFolder( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TTestEmptyFilesIntf::GetRootFolder( const KUInt16* /* inDriveName */ )
{
	(void) ::printf( "GetRootFolder -> crash!\n" );
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * CountRootFolders( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TTestEmptyFilesIntf::CountRootFolders( void ) const
{
	(void) ::printf( "CountRootFolders -> crash!\n" );
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetRootFolderName( KUInt32 ) const
// ------------------------------------------------------------------------- //
const KUInt16*
TTestEmptyFilesIntf::GetRootFolderName( KUInt32 /* inFolderIndex */ ) const
{
	(void) ::printf( "GetRootFolderName -> crash!\n" );
	throw DCLNotImplementedError;
}

// ========================================================================== //
// Windows NT Beer: Comes in 32-oz. cans, but you can only buy it by the      //
// truckload. This causes most people to have to go out and buy bigger        //
// refrigerators. The can looks just like Windows 3.1 Beer's, but the         //
// company promises to change the can to look just like Windows 95 Beer's --  //
// after Windows 95 beer starts shipping. Touted as an "industrial strength"  //
// beer, and suggested only for use in bars.                                  //
// ========================================================================== //
