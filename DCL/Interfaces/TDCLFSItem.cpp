// ==============================
// Fichier:			TDCLFSItem.cp
// Projet:			Desktop Connection Library
//
// Créé le:			09/02/2003
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
// The Original Code is TDCLFSItem.cp.
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
// $Id: TDCLFSItem.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFSItem.h>

// ANSI C
#include <stdlib.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/TDCLFolder.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>

// ------------------------------------------------------------------------- //
//  * TDCLFSItem( IDCLFiles*, EFSItemClass, TDCLFSItemRef )
// ------------------------------------------------------------------------- //
TDCLFSItem::TDCLFSItem(
				IDCLFiles* inFilesIntf,
				EFSItemClass inItemClass,
				TDCLFSItemRef inParentFolder )
	:
		mFilesIntf( inFilesIntf ),
		mClass( inItemClass ),
		mParentFolder( inParentFolder ),
		mName( nil )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLFSItem( void )
// ------------------------------------------------------------------------- //
TDCLFSItem::~TDCLFSItem( void )
{
	// Si le nom existe, le supprimer.
	if (mName != nil)
	{
		::free( mName );
	}
}

// ------------------------------------------------------------------------- //
//  * GetParentFolder( void )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLFSItem::GetParentFolder( void )
{
	// Si le parent n'existe pas, le créer, puis incrémenter son compteur de référence.
	if (mParentFolder.GetFSItem() == nil)
	{
		mParentFolder = MakeParentFolder();
	}
	
	return mParentFolder;
}

// ------------------------------------------------------------------------- //
//  * GetName( void )
// ------------------------------------------------------------------------- //
const KUInt16*
TDCLFSItem::GetName( void )
{
	KUInt16* theResult = mName;

	// Si le nom n'existe pas, le créer.
	if (theResult == nil)
	{
		theResult = MakeName();
		mName = theResult;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetPath( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLFSItem::GetPath( void )
{
	TDCLNSRef thePath = GetParentFolder()->GetPath();

	// Ajout de cet élément.
	thePath.ToArray().Add( ToFrame() );

	return thePath;
}

// ------------------------------------------------------------------------- //
//  * SetName( void )
// ------------------------------------------------------------------------- //
void
TDCLFSItem::SetName( KUInt16* inName )
{
	// Le nom ne doit pas déjà exister.
	if (mName!= nil)
	{
		throw DCLUnknownError;
	}
	
	mName = inName;
}

// ======================================================================== //
// I've finally learned what "upward compatible" means.  It means we get to //
// keep all our old mistakes.                                               //
//                 -- Dennie van Tassel                                     //
// ======================================================================== //
