// ==============================
// Fichier:			TDCLDockCmdArray.cp
// Projet:			Desktop Connection Library
//
// Créé le:			14/4/2003
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
// The Original Code is TDCLDockCmdArray.cp.
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
// $Id: TDCLDockCmdArray.cp,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdArray.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdArray( KUInt32, KUInt32*, KUInt32 )
// --------------------------------------------------------------------------------	//
TDCLDockCmdArray::TDCLDockCmdArray(
								KUInt32 inCommand,
								const KUInt32* inArray,
								KUInt32 inSize )
	:
		TDCLDockCommand( inCommand ),
		mBufferData( nil )
{
	size_t theLength = (inSize + 1) * sizeof( KUInt32 );
	
	mBufferData = (KUInt32*) ::malloc( theLength );
	(void) ::memcpy( &mBufferData[1], inArray, theLength );
	mBufferData[0] = inSize;
}

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdArray( KUInt32, KUInt32 )
// --------------------------------------------------------------------------------	//
TDCLDockCmdArray::TDCLDockCmdArray( KUInt32 inCommand, KUInt32 inSize )
	:
		TDCLDockCommand( inCommand ),
		mBufferData( nil )
{
	size_t theLength = (inSize + 1) * sizeof( KUInt32 );
	
	mBufferData = (KUInt32*) ::malloc( theLength );
	mBufferData[0] = inSize;
}

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdArray( KUInt32, KUInt32, void* )
// --------------------------------------------------------------------------------	//
TDCLDockCmdArray::TDCLDockCmdArray(
					KUInt32 inCommand,
					KUInt32 inBufferSize,
					void* inBuffer )
	:
		TDCLDockCommand( inCommand ),
		mBufferData( nil )
{
	mBufferData = (KUInt32*) ::malloc( inBufferSize );
	
	if (inBufferSize > 0)
	{
		if (mBufferData == nil)
		{
			// Not enough memory.
			throw DCLMemError;
		}

		// Copie des données.
		(void) ::memcpy( mBufferData, inBuffer, inBufferSize );
	}	
}

// --------------------------------------------------------------------------------	//
//  * ~TDCLDockCmdArray( void )
// --------------------------------------------------------------------------------	//
TDCLDockCmdArray::~TDCLDockCmdArray( void )
{
	if (mBufferData)
	{
		::free( mBufferData );
	}
}

// --------------------------------------------------------------------------------	//
//  * GetData( void )
// --------------------------------------------------------------------------------	//
const void*
TDCLDockCmdArray::GetData( void )
{
	return (void*) mBufferData;
}

// --------------------------------------------------------------------------------	//
//  * GetLength( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLDockCmdArray::GetLength( void ) const
{
	return (mBufferData[0] + 1) * sizeof( KUInt32 );
}

// ===================== //
// You're at Witt's End. //
// ===================== //
