// ==============================
// Fichier:			TDCLFSItemRef.cp
// Projet:			Desktop Connection Library
//
// Créé le:			9/4/2003
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
// The Original Code is TDCLFSItemRef.cp.
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
// $Id: TDCLFSItemRef.cp,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

// DCL
#include <DCL/Interfaces/TDCLFSItem.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Interfaces/TDCLFolder.h>

// -------------------------------------------------------------------------- //
//  * TDCLFSItemRef( TDCLFSItem* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef::TDCLFSItemRef( TDCLFSItem* inItem )
	:
		mFSItem( inItem )
{
	if (mFSItem)
	{
		mFSItem->IncrementRef();	
	}
}

// ------------------------------------------------------------------------- //
// 	* TDCLFSItemRef( const TDCLNSObject& )
// ------------------------------------------------------------------------- //
TDCLFSItemRef::TDCLFSItemRef( const TDCLFSItemRef& inCopy )
	:
		mFSItem( inCopy.mFSItem )
{
	if (mFSItem)
	{
		mFSItem->IncrementRef();	
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLFSItemRef( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef::~TDCLFSItemRef( void )
{
	if (mFSItem && mFSItem->DecrementRef())
	{
		delete mFSItem;
	}
}

// ------------------------------------------------------------------------- //
// 	* operator = ( const TDCLFSItemRef& )
// ------------------------------------------------------------------------- //
TDCLFSItemRef&
TDCLFSItemRef::operator = ( const TDCLFSItemRef& inCopy )
{
	if (mFSItem != inCopy.mFSItem)
	{
		if (mFSItem && mFSItem->DecrementRef())
		{
			delete mFSItem;
		}

		mFSItem = inCopy.mFSItem;
		if (mFSItem)
		{
			mFSItem->IncrementRef();
		}
	}
	
	return *this;
}

// -------------------------------------------------------------------------- //
//  * operator TDCLFile* ( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef::operator TDCLFile* ( void ) const
{
	if (mFSItem->GetClass() != TDCLFSItem::kFile)
	{
		throw DCLClassCast;
	}
	
	return (TDCLFile*) mFSItem;
}

// -------------------------------------------------------------------------- //
//  * operator TDCLFolder* ( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef::operator TDCLFolder* ( void ) const
{
	if (mFSItem->GetClass() != TDCLFSItem::kFolder)
	{
		throw DCLClassCast;
	}
	
	return (TDCLFolder*) mFSItem;
}

// ========================================== //
// Writing software is more fun than working. //
// ========================================== //
