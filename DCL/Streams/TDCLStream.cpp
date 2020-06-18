// ==============================
// Fichier:			TDCLStream.cp
// Projet:			Desktop Connection Library
//
// Créé le:			19/01/2002
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
// The Original Code is TDCLStream.cp.
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
// $Id: TDCLStream.cp,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStream.h>

// ANSI C
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>

// ------------------------------------------------------------------------- //
//  * GetCString( KUInt32 )
// ------------------------------------------------------------------------- //
KUInt8*
TDCLStream::GetCString( KUInt32 inNbChars )
{
	KUInt8* theResult = (KUInt8*) ::malloc( inNbChars + 1 );

	if (theResult == nil)
	{
		throw DCLMemError;
	}

	// Un petit try/catch pour nettoyer en sortant.
	try {
		KUInt32 theCount = inNbChars;
		Read( theResult, &theCount );

		if (theCount != inNbChars)
		{
			throw DCLEOF;
		}

	} catch ( ... ) {
		if (theResult)
		{
			::free( theResult );
		}

		throw;	// Rethrow
	}

	// Terminateur.
	theResult[inNbChars] = '\0';

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetCString( void )
// ------------------------------------------------------------------------- //
KUInt8*
TDCLStream::GetCString( void )
{
	size_t bufferLength = 10; 	// Taille de la mémoire tampon
	size_t strLength = 0;		// Taille de la chaîne
	KUInt8* theResult = (KUInt8*) ::malloc( bufferLength );

	if (theResult == nil)
	{
		throw DCLMemError;
	}

	try {
		KUInt8 theChar;
		KUInt32 count = 1;

		do {
			Read( &theChar, &count );

			if (count == 0)	// EOF.
			{
				throw DCLEOF;
			}

			// Add this byte.
			if (strLength == bufferLength)
			{
				// Increase the buffer size with realloc.
				bufferLength += 10;
				theResult = (KUInt8*)  ::realloc( theResult, bufferLength );
			}

			theResult[ strLength ] = theChar;
            strLength++;
		} while (theChar != '\0');

	} catch ( ... ) {
		if (theResult)
		{
			::free( theResult );
		}

		throw;	// Rethrow
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetLine( void )
// ------------------------------------------------------------------------- //
KUInt8*
TDCLStream::GetLine( void )
{
	size_t bufferLength = 10; 	// Taille de la mémoire tampon
	size_t strLength = 0;		// Taille de la chaîne
	KUInt8* theResult = (KUInt8*) ::malloc( bufferLength );

	if (theResult == nil)
	{
		throw DCLMemError;
	}

	try {
		KUInt8 theChar = 0;
		KUInt32 count = 1;

		do {
			Read( &theChar, &count );

			if (count == 0)	// EOF.
			{
			    if (strLength == 0)
			    {
                    ::free(theResult);
                    theResult = nil;
			    }
			    break;
			}

			// Add this byte.
			if (strLength == bufferLength)
			{
				// Increase the buffer size with realloc.
				bufferLength += 10;
				theResult = (KUInt8*)  ::realloc( theResult, bufferLength );
			}

			theResult[ strLength ] = theChar;
			strLength++;
		} while (theChar != '\n' && theChar != '\r');
		if (theResult)
		{
            size_t targetLength = strLength + 1;
            if (targetLength < bufferLength)
            {
                // Increase the buffer size with realloc.
                bufferLength = targetLength;
                theResult = (KUInt8*)  ::realloc( theResult, targetLength );
            }
            if (theChar == '\r')
            {
                theResult[ strLength - 1 ] = '\n';
            }
            theResult[ strLength ] = 0;
            if (theChar == '\r')
            {
                try {
                    theChar = PeekByte();
                    if (theChar == '\n')
                    {
                        (void) GetByte();
                    }
                } catch (TDCLEOFException& ex) {
                    // OK.
                }
            }
		}
	} catch ( ... ) {
		if (theResult)
		{
			::free( theResult );
		}

		throw;	// Rethrow
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PutCString( const KUInt8* )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutCString( const KUInt8* inString )
{
	// Taille de la chaîne plus le caractère nul.
	KUInt32 strLength = ::strlen( (char*) inString ) + 1;

	Write( inString, &strLength );
}

// ------------------------------------------------------------------------- //
//  * PutString( const char* )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutString( const char* inString )
{
	// Taille de la chaîne sans le caractère nul.
	KUInt32 strLength = ::strlen( (char*) inString );

	Write( inString, &strLength );
}

// ------------------------------------------------------------------------- //
//  * GetUniString( void )
// ------------------------------------------------------------------------- //
KUInt16*
TDCLStream::GetUniString( void )
{
	int bufferLength = 10;
	int strLength = 0;
	KUInt16* theResult =
					(KUInt16*) ::malloc( bufferLength * sizeof( KUInt16 ) );

	if (theResult == nil)
	{
		throw DCLMemError;
	}

	try {
		KUInt16 theChar;
		KUInt32 count = 2;

		do {
			Read( &theChar, &count );

			if (count != 2)	// EOF.
			{
				throw DCLEOF;
			}

			// Add this character.
			if (strLength == bufferLength)
			{
				// Increase the buffer size with realloc.
				bufferLength += 10;
				theResult = (KUInt16*)
					::realloc( theResult, bufferLength * sizeof( KUInt16 ) );
			}

			theResult[ strLength ] = theChar;

		} while (theChar != '\0');

	} catch ( ... ) {
		if (theResult)
			::free( theResult );

		throw;	// Rethrow
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PutUniString( const KUInt16* )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutUniString( const KUInt16* inString )
{
	KUInt32 strLength = UUTF16CStr::StrLen( inString ) + 1;

	Write( inString, &strLength );
}

// ------------------------------------------------------------------------- //
//  * GetLong( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLStream::GetLong( void )
{
	KUInt32 theResult;						// Mémoire tampon
	KUInt32 length = sizeof( theResult );	// Taille de la mémoire tampon.

	Read( &theResult, &length );

	if (length < sizeof( theResult ))
	{
		throw DCLEOF;
	}

	return UByteSex_FromBigEndian( theResult );
}

// ------------------------------------------------------------------------- //
//  * PutLong( const KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutLong( const KUInt32 inLong )
{
	KUInt32 theLong = UByteSex_ToBigEndian( inLong );	// Mémoire tampon
	KUInt32 length = sizeof( theLong );		// Taille de la mémoire tampon.

	Write( &theLong, &length );
}

// ------------------------------------------------------------------------- //
//  * GetXLong( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLStream::GetXLong( void )
{
	KUInt32 theResult;

	// Lecture du premier octet.
	KUInt8 theFirstByte;	// Premier octet.
	KUInt32 length = sizeof( theFirstByte );	// Taille de l'octet.
	Read( &theFirstByte, &length );

	if (length < sizeof( theFirstByte ))
	{
		throw DCLEOF;
	}

	if (theFirstByte == 0xFF)
	{
		KUInt32 theBuffer;					// Mémoire tampon
		length = sizeof( theBuffer );		// Taille de la mémoire tampon.
		Read( &theBuffer, &length );

		if (length < sizeof( theBuffer ))
		{
			throw DCLEOF;
		}

		theResult = UByteSex_FromBigEndian( theBuffer );
	} else {
		theResult = theFirstByte;
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PutXLong( const KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutXLong( const KUInt32 inLong )
{
	// Le paramètre tient-il sur un octet?
	if (inLong <= 254)
	{
		PutByte( (KUInt8) inLong );
	} else {
		// Ecriture de 0xFF
		PutByte( 0xFF );

		// Ecriture du long
		PutLong( inLong );
	}
}

// ------------------------------------------------------------------------- //
//  * GetShort( void )
// ------------------------------------------------------------------------- //
KUInt16
TDCLStream::GetShort( void )
{
	KUInt16 theResult;						// Mémoire tampon
	KUInt32 length = sizeof( theResult );	// Taille de la mémoire tampon.

	Read( &theResult, &length );

	if (length < sizeof( theResult ))
	{
		throw DCLEOF;
	}

	return UByteSex_FromBigEndian( theResult );
}

// ------------------------------------------------------------------------- //
//  * PutShort( const KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutShort( const KUInt16 inShort )
{
	KUInt16 theShort = UByteSex_ToBigEndian( inShort );	// Mémoire tampon.
	KUInt32 length = sizeof( theShort );		// Taille de la mémoire tampon.

	Write( &theShort, &length );
}

// ------------------------------------------------------------------------- //
//  * GetByte( void )
// ------------------------------------------------------------------------- //
KUInt8
TDCLStream::GetByte( void )
{
	KUInt8 theResult;						// Mémoire tampon
	KUInt32 length = sizeof( theResult );	// Taille de la mémoire tampon.

	Read( &theResult, &length );

	if (length < sizeof( theResult ))
	{
		throw DCLEOF;
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PutByte( const KUInt8 )
// ------------------------------------------------------------------------- //
void
TDCLStream::PutByte( const KUInt8 inByte )
{
	KUInt32 length = sizeof( inByte );	// Taille de la mémoire tampon

	Write( &inByte, &length );
}

// ============================================ //
// One person's error is another person's data. //
// ============================================ //
