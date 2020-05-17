// ==============================
// Fichier:			TDCLStdStream.cp
// Projet:			Desktop Connection Library
//
// Créé le:			16/10/2002
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
// The Original Code is TDCLStdStream.cp.
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
// $Id: TDCLStdStream.cp,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStdStream.h>

// ANSI C
#include <stdlib.h>
#include <stdio.h>

#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// ------------------------------------------------------------------------- //
//  * TDCLStdStream( void )
// ------------------------------------------------------------------------- //
TDCLStdStream::TDCLStdStream( void )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLStdStream( void )
// ------------------------------------------------------------------------- //
TDCLStdStream::~TDCLStdStream( void )
{
}

// ------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLStdStream::Read( void* outBuffer, KUInt32* ioCount )
{
	size_t theCount = ::fread( outBuffer, 1, *ioCount, stdin );
	
	if (*ioCount != theCount)
	{
		*ioCount = theCount;

		if (0 == feof( stdin ))
		{
			throw DCLUnknownError;
		}
	}
}

// ------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLStdStream::Write( const void* inBuffer, KUInt32* ioCount )
{
	size_t theCount = ::fwrite( inBuffer, 1, *ioCount, stdout );
	
	if (*ioCount != theCount)
	{
		*ioCount = theCount;

		throw DCLUnknownError;
	}
}

// ------------------------------------------------------------------------- //
//  * FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLStdStream::FlushOutput( void )
{
	(void) ::fflush( stdout );
}

// ------------------------------------------------------------------------- //
//  * PeekByte( void )
// ------------------------------------------------------------------------- //
KUInt8
TDCLStdStream::PeekByte( void )
{
	int theNextChar = getc( stdin );
	if (theNextChar == EOF)
	{
		throw DCLEOF;
	}
	
	if (::ungetc( theNextChar, stdin ) != theNextChar)
	{
		throw DCLUnknownError;
	}
	
	return (KUInt8) theNextChar;
}

// ========================================================================= //
// Seems a computer engineer, a systems analyst, and a programmer were       //
// driving down a mountain when the brakes gave out.  They screamed down the //
// mountain, gaining speed, but finally managed to grind to a halt, more by  //
// luck than anything else, just inches from a thousand foot drop to jagged  //
// rocks.  They all got out of the car:                                      //
//         The computer engineer said, "I think I can fix it."               //
//         The systems analyst said, "No, no, I think we should take it      //
// into town and have a specialist look at it."                              //
//         The programmer said, "OK, but first I think we should get back    //
// in and see if it does it again."                                          //
// ========================================================================= //
