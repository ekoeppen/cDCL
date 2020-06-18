// ==============================
// Fichier:			TDCLMemStream.cp
// Projet:			Desktop Connection Library
//
// Créé le:			19/10/2002
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
// The Original Code is TDCLMemStream.cp.
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
// $Id: TDCLMemStream.cp,v 1.6 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLMemStream.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Math/UTInt64.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>

// ------------------------------------------------------------------------- //
//  * TDCLMemStream( void )
// ------------------------------------------------------------------------- //
TDCLMemStream::TDCLMemStream( void )
	:
		mBuffer( nil ),
		mBufferSize( 0 ),
		mAllocatedSize( 0 ),
		mCursor( 0 )
{
	mBuffer = ::malloc( kSizeIncrement );
	if (mBuffer == nil)
	{
		throw DCLMemError;
	}
	
	mAllocatedSize = kSizeIncrement;
}

// ------------------------------------------------------------------------- //
//  * TDCLMemStream( const void*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLMemStream::TDCLMemStream( const void* inBuffer, KUInt32 inLength )
	:
		mBuffer( nil ),
		mBufferSize( inLength ),
		mAllocatedSize( inLength ),
		mCursor( 0 )
{
	mBuffer = ::malloc( inLength );
	if (inLength > 0)
	{
		if (mBuffer == nil)
		{
			throw DCLMemError;
		}
		
		// Copie des données.
		(void) ::memcpy( mBuffer, inBuffer, inLength );
	}
	
	mAllocatedSize = inLength;
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMemStream( void )
// ------------------------------------------------------------------------- //
TDCLMemStream::~TDCLMemStream( void )
{
	if (mBuffer)
	{
		::free( mBuffer );
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMemStream::Read( void* outBuffer, KUInt32* ioCount )
{
	KUInt32 theCount = *ioCount;
	// Vérification qu'on ne dépasse pas.
	if (theCount > (mBufferSize - mCursor))
	{
		theCount = mBufferSize - mCursor;
	}
	
	// Copie des données.
	(void) ::memcpy( outBuffer, &((const KUInt8*) mBuffer)[mCursor], theCount );
	
	// Incrémentation du curseur.
	mCursor += theCount;
	
	// Nombre d'octets lus.
	*ioCount = theCount;
}

// ------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMemStream::Write( const void* inBuffer, KUInt32* ioCount )
{
	// Redimmensionnement de la mémoire tampon si nécessaire.
	KUInt32 count = *ioCount;
	if ((mCursor + count) > mAllocatedSize)
	{
		if ((mCursor + count) <= (mAllocatedSize + kSizeIncrement))
		{
			mAllocatedSize += kSizeIncrement;
		} else {
			mAllocatedSize = mCursor + count;
		}
		
		mBuffer = ::realloc( mBuffer, mAllocatedSize );
		if (mBuffer == nil)
		{
			throw DCLMemError;
		}
	}
	
	(void) ::memcpy( &((KUInt8*) mBuffer)[mCursor], inBuffer, count );
	mCursor += count;
	if (mCursor > mBufferSize)
	{
		mBufferSize = mCursor;
	}
}

// ------------------------------------------------------------------------- //
//  * FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMemStream::FlushOutput( void )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//  * PeekByte( void )
// ------------------------------------------------------------------------- //
KUInt8
TDCLMemStream::PeekByte( void )
{
	// Vérification qu'on ne dépasse pas.
	if ((mCursor + 1) >= mBufferSize)
	{
		throw DCLEOF;
	}
	
	return ((KUInt8*) mBuffer)[mCursor];
}

// -------------------------------------------------------------------------- //
//  * SetCursor( KSInt64, ECursorMode )
// -------------------------------------------------------------------------- //
void
TDCLMemStream::SetCursor( KSInt64 inPos, ECursorMode inMode )
{
	switch (inMode)
	{
		case kFromStart:
			{
				if (inPos < 0)
				{
					throw DCLPosition;
				}
				if (inPos > mBufferSize)
				{
					throw DCLPosition;
				}

				mCursor = UTInt64::ToKUInt32( inPos );
			}
			break;

		case kFromLEOF:
			{
				KSInt64 theCursor = mBufferSize - inPos;
				if (theCursor < 0)
				{
					throw DCLPosition;
				}
				if (theCursor > mBufferSize)
				{
					throw DCLPosition;
				}

				mCursor = UTInt64::ToKUInt32( theCursor );
			}
			break;
		
		case kFromCursor:
			{
				KSInt64 theCursor = mCursor + inPos;
				if (theCursor < 0)
				{
					throw DCLPosition;
				}
				if (theCursor > mBufferSize)
				{
					throw DCLPosition;
				}

				mCursor = UTInt64::ToKUInt32( theCursor );
			}
	}
}

// ------------------------------------------------------------------------- //
//  * GetCursor( void )
// ------------------------------------------------------------------------- //
KSInt64
TDCLMemStream::GetCursor( void )
{
	return mCursor;
} 

// ========================================================================= //
// There are two ways of constructing a software design.  One way is to make //
// it so simple that there are obviously no deficiencies and the other is to //
// make it so complicated that there are no obvious deficiencies.            //
//                 -- C.A.R. Hoare                                           //
// ========================================================================= //
