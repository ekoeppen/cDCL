// ==============================
// Fichier:			TDCLDockCmdTwoNSObjects.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			1/11/2003
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
// The Original Code is TDCLDockCmdTwoNSObjects.cp.
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
// $Id: TDCLDockCmdTwoNSObjects.cp,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/Streams/TDCLMemStream.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdTwoNSObjects( void )
// -------------------------------------------------------------------------- //
TDCLDockCmdTwoNSObjects::TDCLDockCmdTwoNSObjects( void )
	:
		TDCLDockCommand(),
		mBuffer( nil ),
		mBufferSize( 0 )
{
	// This space for rent.
}

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdTwoNSObjects( KUInt32, const TDCLNSRef&, const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLDockCmdTwoNSObjects::TDCLDockCmdTwoNSObjects(
			KUInt32 inCommand,
			const TDCLNSRef& inFirstObject,
			const TDCLNSRef& inSecondObject )
	:
		TDCLDockCommand( inCommand ),
		mBuffer( nil ),
		mBufferSize( 0 )
{
	// Conversion des références en données NSOF.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( inFirstObject );
	theEncoder.AddObject( inSecondObject );
	
	mBufferSize = theMemStream.GetBufferSize();
	mBuffer = ::malloc( mBufferSize );
	
	(void) ::memcpy( mBuffer, theMemStream.GetBuffer(), mBufferSize );
}

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdTwoNSObjects( KUInt32, KUInt32, const void* )
// -------------------------------------------------------------------------- //
TDCLDockCmdTwoNSObjects::TDCLDockCmdTwoNSObjects(
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
//  * ~TDCLDockCmdTwoNSObjects( void )
// -------------------------------------------------------------------------- //
TDCLDockCmdTwoNSObjects::~TDCLDockCmdTwoNSObjects( void )
{
	if (mBuffer)
	{
		::free( mBuffer );
	}
}

// -------------------------------------------------------------------------- //
//  * GetObjects( void ) const
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLDockCmdTwoNSObjects::GetObjects( void ) const
{
	TDCLNSRef theResult;
	if (mBuffer)
	{
		TDCLMemStream theStream( mBuffer, mBufferSize );
		TDCLNSOFDecoder theDecoder( &theStream );

		theResult = TDCLNSRef::MakeArray();
		TDCLNSArray& theResultAsArray = theResult.ToArray();
		theResultAsArray.Add( theDecoder.GetNextObject() );
		theResultAsArray.Add( theDecoder.GetNextObject() );
	}

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SetObjects( const TDCLNSRef&, const TDCLNSRef& )
// -------------------------------------------------------------------------- //
void
TDCLDockCmdTwoNSObjects::SetObjects(
			const TDCLNSRef& inFirstObject,
			const TDCLNSRef& inSecondObject )
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
	theEncoder.AddObject( inFirstObject );
	theEncoder.AddObject( inSecondObject );
	
	KUInt32 theBufferSize = mBufferSize = theMemStream.GetBufferSize();
	void* theBuffer = mBuffer = ::malloc( theBufferSize );
	
	(void) ::memcpy( theBuffer, theMemStream.GetBuffer(), theBufferSize );
}

// -------------------------------------------------------------------------- //
//  * GetData( void )
// -------------------------------------------------------------------------- //
const void*
TDCLDockCmdTwoNSObjects::GetData( void )
{
	return mBuffer;
}

// -------------------------------------------------------------------------- //
//  * GetLength( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLDockCmdTwoNSObjects::GetLength( void ) const
{
	return mBufferSize;
}

// ========= //
// try again //
// ========= //
