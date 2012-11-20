// ==============================
// Fichier:			TDCLDockCmdGeneric.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			02/02/2001
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
// The Original Code is TDCLDockCmdGeneric.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdGeneric.cp,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// ------------------------------------------------------------------------- //
//  * TDCLDockCommand( void )
// ------------------------------------------------------------------------- //
TDCLDockCmdGeneric::TDCLDockCmdGeneric( void )
	:
		TDCLDockCommand (),
		mLength ( 0 ),
		mData( nil )
{
	// Allocation d'un pointeur vide.
	mData = ::malloc( 0 );
}

// ------------------------------------------------------------------------- //
//  * TDCLDockCmdGeneric( KUInt32, KUInt32, const void* )
// ------------------------------------------------------------------------- //
TDCLDockCmdGeneric::TDCLDockCmdGeneric(
					KUInt32 inCommand,
					KUInt32 inLength,
					const void* inData )
	:
		TDCLDockCommand ( inCommand ),
		mLength ( inLength ),
		mData( nil )
{
	// I first allocate a pointer of the size inLength.
	mData = ::malloc( inLength );	// I use malloc because this code should be portable.
	
	if (inLength > 0)
	{
		if (mData == nil)
		{
			// Not enough memory.
			throw DCLMemError;
		}

		// Copie des donnŽes.
		(void) ::memcpy( mData, inData, inLength );	// Idem for memmove.
	}	
}

// ------------------------------------------------------------------------- //
//  * ~TDCLDockCmdGeneric( void )
// ------------------------------------------------------------------------- //
TDCLDockCmdGeneric::~TDCLDockCmdGeneric( void )
{
	if (mData)
	{
		::free( mData );
	}
}

// ------------------------------------------------------------------------- //
//  * SetLength( KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLDockCmdGeneric::SetLength( KUInt32 inLength )
{
	// Le pointeur n'est pas nul ici (ou alors, ::malloc( 0 ) retourne NULL)
	mLength = inLength;
	
	mData = ::realloc( mData, inLength );

	if ((inLength > 0) && (mData == nil))
	{
		// Not enough memory.
		throw DCLMemError;
	}
}

// ------------------------------------------------------------------------- //
//  * CopyData( const void* )
// ------------------------------------------------------------------------- //
void
TDCLDockCmdGeneric::CopyData( const void* inData )
{
	// The data pointer cannot be null here.
	(void) ::memcpy( mData, inData, mLength );
}

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLDockCmdGeneric::GetLength( void ) const
{
	return mLength;
}

// ------------------------------------------------------------------------- //
//  * GetData( void )
// ------------------------------------------------------------------------- //
const void*
TDCLDockCmdGeneric::GetData( void )
{
	return (const void*) mData;
}

// ============================================================ //
// Assembly language experience is [important] for the maturity //
// and understanding of how computers work that it provides.    //
//                 -- D. Gries                                  //
// ============================================================ //
