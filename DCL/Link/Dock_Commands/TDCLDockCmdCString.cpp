// ==============================
// Fichier:			TDCLDockCmdCString.cp
// Projet:			Desktop Connection Library
//
// Créé le:			13/4/2003
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
// The Original Code is TDCLDockCmdCString.cp.
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
// $Id: TDCLDockCmdCString.cp,v 1.4 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdCString.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdCString( KUInt32, const KUInt16* )
// --------------------------------------------------------------------------------	//
TDCLDockCmdCString::TDCLDockCmdCString( KUInt32 inCommand, const KUInt16* inString )
	:
		TDCLDockCommand( inCommand ),
		mString( nil ),
		mLength( (UUTF16CStr::StrLen( inString ) + 1) * sizeof( KUInt16 ) )
{
	mString = (KUInt16*) ::malloc( mLength );
	(void) ::memcpy( mString, inString, mLength );
}

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdCString( KUInt32, TDCLNSString& )
// --------------------------------------------------------------------------------	//
TDCLDockCmdCString::TDCLDockCmdCString(
									KUInt32 inCommand, const TDCLNSString& inString )
	:
		TDCLDockCommand( inCommand ),
		mString( nil ),
		mLength( inString.GetLength() )
{
	mString = (KUInt16*) ::malloc( mLength );
	(void) ::memcpy( mString, inString.GetString(), mLength );
}

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdCString( KUInt32, KUInt32, void* )
// --------------------------------------------------------------------------------	//
TDCLDockCmdCString::TDCLDockCmdCString(
					KUInt32 inCommand,
					KUInt32 inBufferSize,
					void* inBuffer )
	:
		TDCLDockCommand( inCommand ),
		mString( nil ),
		mLength( inBufferSize )
{
	mString = (KUInt16*) ::malloc( inBufferSize );
	
	if (inBufferSize > 0)
	{
		if (mString == nil)
		{
			// Not enough memory.
			throw DCLMemError;
		}

		// Copie des données.
		(void) ::memcpy( mString, inBuffer, inBufferSize );
	}	
}

// --------------------------------------------------------------------------------	//
//  * ~TDCLDockCmdCString( void )
// --------------------------------------------------------------------------------	//
TDCLDockCmdCString::~TDCLDockCmdCString( void )
{
	if (mString)
	{
		::free( mString );
	}
}

// --------------------------------------------------------------------------------	//
//  * GetData( void )
// --------------------------------------------------------------------------------	//
const void*
TDCLDockCmdCString::GetData( void )
{
	return (void*) mString;
}

// --------------------------------------------------------------------------------	//
//  * GetLength( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLDockCmdCString::GetLength( void ) const
{
	return mLength;
}

// ======================================================= //
// Systems programmers are the high priests of a low cult. //
//                 -- R.S. Barton                          //
// ======================================================= //
