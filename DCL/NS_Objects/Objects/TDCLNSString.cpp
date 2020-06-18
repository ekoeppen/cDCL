// ==============================
// Fichier:			TDCLNSString.cp
// Projet:			Desktop Connection Library
//
// Créé le:			21/10/2002
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
// The Original Code is TDCLNSString.cp.
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
// $Id: TDCLNSString.cp,v 1.5 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  Constantes
// ------------------------------------------------------------------------- //

const KUInt16 TDCLNSString::kSpaceChar = UByteSex_ToBigEndian( (KUInt16) ' ' );

// ------------------------------------------------------------------------- //
//  * TDCLNSString( const char* )
// ------------------------------------------------------------------------- //
TDCLNSString::TDCLNSString( const char* inString )
	:
		TDCLNSBinary( KDCLSYM::kSYMstring, 0 )
{
	size_t theStrLen = ::strlen( inString );
	SetLength( 2 * (theStrLen + 1) );
	
	// Conversion de la chaîne.
	UUTF16CStr::FromISO88591(
						(const KUInt8*) inString, (KUInt16*) GetPointer() );
}

// ------------------------------------------------------------------------- //
//  * TDCLNSString( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLNSString::TDCLNSString( const KUInt16* inString )
	:
		TDCLNSBinary( KDCLSYM::kSYMstring, 0 )
{
	size_t theTotalLen = sizeof(KUInt16)
									* (UUTF16CStr::StrLen( inString ) + 1);
	SetLength( theTotalLen );
	
	// Copie de la chaîne.
	(void) ::memcpy( (KUInt16*) GetPointer(), inString, theTotalLen );
}

// ------------------------------------------------------------------------- //
//  * TDCLNSString( const KUInt16*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSString::TDCLNSString( const KUInt16* inString, KUInt32 inSize )
	:
		TDCLNSBinary( KDCLSYM::kSYMstring, 0 )
{
	size_t theTotalLen = sizeof(KUInt16) * (inSize + 1);
	SetLength( theTotalLen );
	
	// Copie de la chaîne.
	(void) ::memcpy(
				(KUInt16*) GetPointer(),
				inString,
				theTotalLen - sizeof(KUInt16) );
	
	// Ajout du terminateur.
	((KUInt16*) GetPointer())[inSize] = 0;
}

// ------------------------------------------------------------------------- //
//  * TDCLNSString( const TDCLNSString& )
// ------------------------------------------------------------------------- //
TDCLNSString::TDCLNSString( TDCLStream* inStream, KUInt32 inLength )
	:
		TDCLNSBinary( inStream, inLength, KDCLSYM::kSYMstring )
{
	// Loi du 29 juillet 1881.
}

// ------------------------------------------------------------------------- //
//  * TDCLNSString( KUInt16*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSString::TDCLNSString( KUInt16* inString, KUInt32 inLength )
	:
		TDCLNSBinary( (void*) inString, inLength, KDCLSYM::kSYMstring )
{
	// Les données ne sont pas copiées.
}

// ------------------------------------------------------------------------- //
//  * FromNSOF( TDCLNSOFDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSString*
TDCLNSString::FromNSOF( TDCLNSOFDecoder* inDecoder )
{
	TDCLStream* theStream = inDecoder->GetInputStream();
	
	KUInt32 nbBytes = theStream->GetXLong();
	
	// Création de l'objet.
	TDCLNSString* theResult = new TDCLNSString( theStream, nbBytes );

	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ) );

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * StrEqual( const char* ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSString::StrEqual( const char* inAlter ) const
{
	// Conversion de la chaîne ISO 8859-1 en unicode.
	size_t theStrLen = ::strlen( inAlter );
	KUInt16* theStringInUCS2 =
			(KUInt16*) ::malloc( sizeof(KUInt16) * (theStrLen + 1) );
	
	// Conversion de la chaîne.
	UUTF16CStr::FromISO88591(
			(const KUInt8*) inAlter, theStringInUCS2 );
	
	Boolean theResult = StrEqual( theStringInUCS2 );
	
	// Libération.
	::free( theStringInUCS2 );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * StrEqual( const KUInt16* ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSString::StrEqual( const KUInt16* inAlter ) const
{
	Boolean theResult = false;
	
	// Comparaison (puisque les deux chaînes sont terminées par des nuls).
	KUInt32 indexStr;
	KUInt32 theMax = GetLength() / 2;
	const KUInt16* thisString = GetString();
	
	for (indexStr = 0; indexStr < theMax; indexStr++)
	{
		KUInt16 theAlterChar = inAlter[indexStr];
		if (theAlterChar != thisString[indexStr])
		{
			break;
		}
		
		if (theAlterChar == 0)
		{
			theResult = true;
			break;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * StrEqual( const TDCLNSString& ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSString::StrEqual( const TDCLNSString& inAlter ) const
{
	return StrEqual( inAlter.GetString() );
}

// ------------------------------------------------------------------------- //
//  * Catenate( const char* ) const
// ------------------------------------------------------------------------- //
TDCLNSString*
TDCLNSString::Catenate( const char* inAlter ) const
{
	size_t thisStrLen = GetStrLen();
	size_t alterStrLen = UUTF16CStr::StrLen( (const KUInt8*) inAlter );
	size_t theNewStrLen = thisStrLen + alterStrLen;
	size_t theNewSize = (theNewStrLen + 1) * sizeof(KUInt16);
	
	KUInt16* theNewString = (KUInt16*) ::malloc( theNewSize );
	if (theNewString == nil)
	{
		throw DCLMemError;
	}
	
	// Copie des données pour la première chaîne.
	(void) ::memcpy( theNewString, GetString(), thisStrLen * sizeof(KUInt16) );
	
	// Conversion de la seconde chaîne.
	UUTF16CStr::FromISO88591(
			(const KUInt8*) inAlter, &theNewString[thisStrLen] );
	
	return new TDCLNSString( (KUInt16*) theNewString, theNewSize );
}

// ------------------------------------------------------------------------- //
//  * Catenate( const KUInt16* ) const
// ------------------------------------------------------------------------- //
TDCLNSString*
TDCLNSString::Catenate( const KUInt16* inAlter ) const
{
	size_t thisStrLen = GetStrLen();
	// Y compris le terminateur.
	size_t alterStrLen = UUTF16CStr::StrLen( inAlter );
	size_t alterSize = (alterStrLen + 1) * sizeof(KUInt16);
	size_t theNewStrLen = thisStrLen + alterStrLen;
	size_t theNewSize = (theNewStrLen + 1) * sizeof(KUInt16);
	
	KUInt16* theNewString = (KUInt16*) ::malloc( theNewSize );
	if (theNewString == nil)
	{
		throw DCLMemError;
	}
	
	// Copie des données pour la première chaîne.
	(void) ::memcpy( theNewString, GetString(), thisStrLen * sizeof(KUInt16) );
	
	// Copie des données pour la deuxième chaîne avec le terminateur.
	(void) ::memcpy( &theNewString[thisStrLen], inAlter, alterSize );
	
	return new TDCLNSString( (KUInt16*) theNewString, theNewSize );
}

// ------------------------------------------------------------------------- //
//  * CatenateWithSpace( const char* ) const
// ------------------------------------------------------------------------- //
TDCLNSString*
TDCLNSString::CatenateWithSpace( const char* inAlter ) const
{
	size_t thisStrLen = GetStrLen();
	size_t alterStrLen = UUTF16CStr::StrLen( (const KUInt8*) inAlter );
	size_t theNewStrLen = thisStrLen + alterStrLen + 1;
	size_t theNewSize = (theNewStrLen + 1) * sizeof(KUInt16);
	
	KUInt16* theNewString = (KUInt16*) ::malloc( theNewSize );
	if (theNewString == nil)
	{
		throw DCLMemError;
	}
	
	// Copie des données pour la première chaîne.
	(void) ::memcpy( theNewString, GetString(), thisStrLen * sizeof(KUInt16) );
	
	// Espace.
	theNewString[thisStrLen] = kSpaceChar;
	
	// Conversion de la seconde chaîne.
	UUTF16CStr::FromISO88591(
			(const KUInt8*) inAlter, &theNewString[thisStrLen + 1] );
	
	return new TDCLNSString( (KUInt16*) theNewString, theNewSize );
}

// ------------------------------------------------------------------------- //
//  * Catenate( const KUInt16* ) const
// ------------------------------------------------------------------------- //
TDCLNSString*
TDCLNSString::CatenateWithSpace( const KUInt16* inAlter ) const
{
	size_t thisStrLen = GetStrLen();
	size_t alterStrLen = UUTF16CStr::StrLen( inAlter );
	size_t alterSize = (alterStrLen + 1) * sizeof(KUInt16);
	size_t theNewStrLen = thisStrLen + alterStrLen + 1;
	size_t theNewSize = (theNewStrLen + 1) * sizeof(KUInt16);
	
	KUInt16* theNewString = (KUInt16*) ::malloc( theNewSize );
	if (theNewString == nil)
	{
		throw DCLMemError;
	}
	
	// Copie des données pour la première chaîne.
	(void) ::memcpy( theNewString, GetString(), thisStrLen * sizeof(KUInt16) );
	
	// Espace.
	theNewString[thisStrLen] = kSpaceChar;
	
	// Copie des données pour la deuxième chaîne avec le terminateur.
	(void) ::memcpy( &theNewString[thisStrLen + 1], inAlter, alterSize );
	
	return new TDCLNSString( (KUInt16*) theNewString, theNewSize );
}

// ========================================================================= //
// The primary purpose of the DATA statement is to give names to constants;  //
// instead of referring to pi as 3.141592653589793 at every appearance, the  //
// variable PI can be given that value with a DATA statement and used        //
// instead of the longer form of the constant.  This also simplifies         //
// modifying the program, should the value of pi change.                     //
//                 -- FORTRAN manual for Xerox Computers                     //
// ========================================================================= //
