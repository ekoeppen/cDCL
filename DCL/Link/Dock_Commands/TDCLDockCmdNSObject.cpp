// ==============================
// Fichier:			TDCLDockCmdNSObject.cp
// Projet:			Desktop Connection Library
//
// Créé le:			17/01/2002
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
// The Original Code is TDCLDockCmdNSObject.cp.
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
// $Id: TDCLDockCmdNSObject.cp,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/Streams/TDCLMemStream.h>

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdNSObject( void )
// -------------------------------------------------------------------------- //
TDCLDockCmdNSObject::TDCLDockCmdNSObject( void )
	:
		TDCLDockCommand(),
		mBuffer( nil ),
		mBufferSize( 0 )
{
	// This space for rent.
}

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdNSObject( KUInt32, const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLDockCmdNSObject::TDCLDockCmdNSObject(
			KUInt32 inCommand,
			const TDCLNSRef& inObject )
	:
		TDCLDockCommand( inCommand ),
		mBuffer( nil ),
		mBufferSize( 0 )
{
	// Conversion de la référence en données NSOF.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( inObject );
	
	mBufferSize = theMemStream.GetBufferSize();
	mBuffer = ::malloc( mBufferSize );
	
	(void) ::memcpy( mBuffer, theMemStream.GetBuffer(), mBufferSize );
}

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdNSObject( KUInt32, KUInt32, const void* )
// -------------------------------------------------------------------------- //
TDCLDockCmdNSObject::TDCLDockCmdNSObject(
					KUInt32 inCommand,
					KUInt32 inLength,
					const void* inData
				)
	:
		TDCLDockCommand( inCommand ),
		mBuffer( nil ),
		mBufferSize( inLength )
{
	mBuffer = (KUInt8*) ::malloc( inLength );
	if (inLength > 0)
	{
		if (mBuffer == nil)
		{
			// Pas assez de mémoire.
			throw DCLMemError;
		}
	
		// Copie des données.
		(void) ::memcpy( mBuffer, inData, inLength );
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLDockCmdNSObject( void )
// -------------------------------------------------------------------------- //
TDCLDockCmdNSObject::~TDCLDockCmdNSObject( void )
{
	if (mBuffer)
	{
		::free( mBuffer );
	}
}

// -------------------------------------------------------------------------- //
//  * GetObject( void ) const
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLDockCmdNSObject::GetObject( void ) const
{
	TDCLNSRef theResult;
	if (mBuffer)
	{
		TDCLMemStream theStream( mBuffer, mBufferSize );
		TDCLNSOFDecoder theDecoder( &theStream );
	
		theResult = theDecoder.GetNextObject();
	}

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SetObject( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
void
TDCLDockCmdNSObject::SetObject( const TDCLNSRef& inObject )
{
	// Libération de la mémoire tampon si elle n'est pas déjà libre.
	if (mBuffer)
	{
		::free( mBuffer );
		mBuffer = nil;
		mBufferSize = 0;
	}

	// Conversion de la référence en données NSOF.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( inObject );
	
	KUInt32 theBufferSize = mBufferSize = theMemStream.GetBufferSize();
	void* theBuffer = mBuffer = ::malloc( theBufferSize );
	
	(void) ::memcpy( theBuffer, theMemStream.GetBuffer(), theBufferSize );
}

// -------------------------------------------------------------------------- //
//  * GetData( void )
// -------------------------------------------------------------------------- //
const void*
TDCLDockCmdNSObject::GetData( void )
{
	return mBuffer;
}

// -------------------------------------------------------------------------- //
//  * GetLength( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLDockCmdNSObject::GetLength( void ) const
{
	return mBufferSize;
}

// ================================================================================ //
// Thus spake the master programmer:                                                //
//         "When a program is being tested, it is too late to make design changes." //
//                 -- Geoffrey James, "The Tao of Programming"                      //
// ================================================================================ //
