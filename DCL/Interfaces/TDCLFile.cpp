// ==============================
// Fichier:			TDCLFile.cp
// Projet:			Desktop Connection Library
//
// Créé le:			04/02/2003
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
// The Original Code is TDCLFile.cp.
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
// $Id: TDCLFile.cp,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFile.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/Package/TDCLPackage.h>

// ------------------------------------------------------------------------- //
//  * TDCLFile( IDCLFiles*, TDCLFSItemRef )
// ------------------------------------------------------------------------- //
TDCLFile::TDCLFile( IDCLFiles* inFilesIntf, TDCLFSItemRef inParentFolder )
	:
		TDCLFSItem( inFilesIntf, kFile, inParentFolder )
{
	// Défense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * PeekByte( void )
// ------------------------------------------------------------------------- //
KUInt8
TDCLFile::PeekByte( void )
{
	// Lecture de l'octet.
	KUInt8 theResult = GetByte();
	
	// Retour en arrière.
	SetCursor( -1, kFromCursor );
	
	return theResult;
}

// -------------------------------------------------------------------------- //
// MakeParentFolder( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLFile::MakeParentFolder( void ) const
{
	// Cette méthode ne doit jamais être appelée.
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * ToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLFile::ToFrame( void )
{
	TDCLNSRef theSegment = TDCLNSRef::MakeFrame();

	// Suivant le type, on met 'folder ou 'file
	TDCLNSFrame& theSegmentAsFrame = theSegment.ToFrame();
	
	// Ajout du nom
	theSegmentAsFrame.Set( "name", TDCLNSRef::MakeString( GetName() ) );
	
	// Puis le type.
	theSegmentAsFrame.Set( "type",
		TDCLNSRef::MakeInt( TDCLDockCommand::kFile_FSType ) );

	return theSegment;
}

// ------------------------------------------------------------------------- //
//  * GetIcon( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLFile::GetIcon( void ) const
{
	return TDCLNSRef::kNILREF;
}

// ------------------------------------------------------------------------- //
//  * IsPackage( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLFile::IsPackage( void )
{
	Boolean theResult = false;
	
	// On protège l'appel à TDCLPackage::IsPackage.
	try {
		theResult = TDCLPackage::IsPackage( this );
	} catch ( ... ) {
		theResult = false;
	}
	
	return theResult;
}

// ====================================================================== //
// C makes it easy for you to shoot yourself in the foot.  C++ makes that //
// harder, but when you do, it blows away your whole leg.                 //
//                 -- Bjarne Stroustrup                                   //
// ====================================================================== //
