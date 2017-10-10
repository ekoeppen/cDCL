// ==============================
// Fichier:			UDCLPaperback.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			9/5/2004
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
// The Original Code is UDCLPaperback.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: UDCLPaperback.cp,v 1.3 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/UDCLPaperback.h>

// ANSI C & POSIX
#include <stdlib.h>
#include <string.h>

// ISO C++
#include <stdexcept>

// K
#include <K/Defines/UByteSex.h>
#include <K/Unicode/UUTF16Conv.h>

// DCL
#include <DCL/Data/Text/TDCLFontSpecification.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/Streams/TDCLStream.h>

using namespace std;

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * BuildBook( TDCLStream*, EEncoding, const char*, const char*, ... )
// -------------------------------------------------------------------------- //
TDCLNSRef
UDCLPaperback::BuildBook(
			TDCLStream* inTextStream,
			EEncoding inEncoding,
			const char* inBookTitle,
			const char* inTOCPrefix /* = nil */,
			Boolean inDeleteTOCLines /* = true */,
			const char* inFontName /* = nil */,
			int inFontSize /* = 9 */ )
{
	TDCLNSRef theResult;

	KUInt16* theText = nil;
	KUInt16** theTOCEntries = nil;
	KUInt32* theTOCIndexes = nil;
	KUInt32 theTOCSize = 0;
	
	try {
		// Extraction de la chaîne.
		KUInt32 theTextSize;
		ExtractString( inTextStream, inEncoding, &theText, &theTextSize );
		
		// Conversion des fins de ligne.
		ConvertEndLines( theText, &theTextSize );
		
		// Construction de la TOC
		if (inTOCPrefix)
		{
			BuildTOC(
				theText,
				&theTextSize,
				inTOCPrefix,
				inDeleteTOCLines,
				&theTOCEntries,
				&theTOCIndexes,
				&theTOCSize );
		}
		
		// Construction de la police.
		TDCLNSRef theFont = BuildFont( inFontName, inFontSize );

		// Construction du livre.
		TDCLNSRef theString = TDCLNSRef::MakeString( theText, theTextSize );
		::free(theText);
		theText = nil;

		theResult = BuildBook(
					theString,
					theFont,
					inBookTitle,
					theTOCEntries,
					theTOCIndexes,
					theTOCSize );

		if (theTOCEntries)
		{
			KUInt32 indexTOCEntries;
			for (indexTOCEntries = 0; indexTOCEntries < theTOCSize; indexTOCEntries++)
			{
				::free( theTOCEntries[indexTOCEntries] );
			}
			::free( theTOCEntries );
		}
		if (theTOCIndexes)
		{
			::free( theTOCIndexes );
		}
	} catch (...) {
		if (theText)
		{
			::free( theText );
		}
		if (theTOCEntries)
		{
			KUInt32 indexTOCEntries;
			for (indexTOCEntries = 0; indexTOCEntries < theTOCSize; indexTOCEntries++)
			{
				::free( theTOCEntries[indexTOCEntries] );
			}
			::free( theTOCEntries );
		}
		if (theTOCIndexes)
		{
			::free( theTOCIndexes );
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * ExtractString( TDCLStream*, EEncoding, KUInt16**, KUInt32* )
// -------------------------------------------------------------------------- //
void
UDCLPaperback::ExtractString(
		TDCLStream* inTextStream,
		EEncoding inEncoding,
		KUInt16** outUTF16Text,
		KUInt32* outSize )
{
	// Lecture du fichier en entier par paquets de 64 Ko.
	KUInt32 theBufferSize = 0;
	KUInt32 theChunkSize = 65536;
	KUInt8* theBuffer = (KUInt8*) ::malloc( 0 );
	KUInt16* theUTF16Text = nil;
	
	try {
		do {
			theBuffer =
				(KUInt8*) ::realloc( theBuffer, theBufferSize + theChunkSize );
			inTextStream->Read( &theBuffer[theBufferSize], &theChunkSize );
			theBufferSize += theChunkSize;
		} while (theChunkSize == 65536);
		
		// Conversion en UTF16.
		switch (inEncoding)
		{
			case kISO88591:
			{
				theUTF16Text = (KUInt16*)
					::malloc( sizeof(KUInt16) * theBufferSize );
				
				UUTF16Conv::FromISO88591(
								theBuffer,
								theUTF16Text,
								theBufferSize );
			}
			break;
			
			case kISO88592:
			{
				theUTF16Text = (KUInt16*)
					::malloc( sizeof(KUInt16) * theBufferSize );
				
				UUTF16Conv::FromISO88592(
								theBuffer,
								theUTF16Text,
								theBufferSize );
			}
			break;
			
			case kMacRoman:
			{
				theUTF16Text = (KUInt16*)
					::malloc( sizeof(KUInt16) * theBufferSize );
				
				UUTF16Conv::FromMacRoman(
								theBuffer,
								theUTF16Text,
								theBufferSize );
			}
			break;
			
			case kUTF8:
			{
				theUTF16Text = (KUInt16*)
					::malloc( sizeof(KUInt16) * theBufferSize );
				
				size_t theInputSize = theBufferSize;
				size_t theOutputSize = theBufferSize;
				if (UUTF16Conv::FromUTF8(
								theBuffer,
								&theInputSize,
								theUTF16Text,
								&theOutputSize ) != UUTF16Conv::kInputExhausted)
				{
					throw runtime_error( "Error while decoding UTF-8 input" );
				}
				
				theBufferSize = (KUInt32) theOutputSize;
			}
			break;
			
			case kUTF16:			
			{
				theUTF16Text = (KUInt16*)
					::realloc( theBuffer, sizeof(KUInt16) * theBufferSize  );
				theBuffer = nil;
			}
			break;
		}
	} catch (...) {
		// Ménage
		if (theUTF16Text)
		{
			::free( theUTF16Text );
		}
		if (theBuffer)
		{
			::free( theBuffer );
		}
		
		throw;
	}

	// Ménage
	if (theBuffer)
	{
		::free( theBuffer );
	}
	
	*outUTF16Text = theUTF16Text;
	*outSize = theBufferSize;
}

// -------------------------------------------------------------------------- //
//  * ConvertEndLines( KUInt16*, KUInt32* )
// -------------------------------------------------------------------------- //
void
UDCLPaperback::ConvertEndLines(
		KUInt16* inUTF16Text,
		KUInt32* ioSize )
{
	// Conversion des fins de ligne au format Macintosh (si nécessaire).
	KUInt32 theBufferSize = *ioSize;
	KUInt32 indexString;
	for (indexString = 0; indexString < theBufferSize; indexString++)
	{
		if (inUTF16Text[indexString] == UByteSex_ToBigEndian( (KUInt16) 10 ))
		{
			// Unix.
			inUTF16Text[indexString] = UByteSex_ToBigEndian( (KUInt16) 13 );
		} else if ((inUTF16Text[indexString] == UByteSex_ToBigEndian( (KUInt16) 13 ))
				&& (inUTF16Text[indexString+1] == UByteSex_ToBigEndian( (KUInt16) 10 ))) {
			// DOS.
			// Suppression du caractère suivant.
			(void) ::memmove(
				&inUTF16Text[indexString+1],
				(const void*) &inUTF16Text[indexString+2],
				(theBufferSize - indexString - 1) * sizeof(KUInt16) );
			
			theBufferSize--;
		}
	}
	
	*ioSize = theBufferSize;
}

// -------------------------------------------------------------------------- //
//  * BuildTOC( KUInt16*, KUInt32*, const char*, Boolean, KUInt16***, ... )
// -------------------------------------------------------------------------- //
void
UDCLPaperback::BuildTOC(
		KUInt16* inText,
		KUInt32* ioTextSize,
		const char* inTOCLinePrefix,
		Boolean inDeleteTOCLines,
		KUInt16*** outTOCEntries,
		KUInt32** outTOCIndexes,
		KUInt32* outTOCSize )
{
	KUInt32 theTextSize = *ioTextSize;

	*outTOCEntries = (KUInt16**) ::malloc( 0 );
	*outTOCIndexes = (KUInt32*) ::malloc( 0 );
	*outTOCSize = 0;

	KUInt32 prefixLength = (KUInt32) ::strlen( inTOCLinePrefix );
	KUInt32 nbTOCEntries = 0;

	KUInt32 indexString;
	for (indexString = 0; indexString < theTextSize; indexString++)
	{
		// Une nouvelle ligne.
		KUInt32 lineStart = indexString;
		
		// Commence-t-elle par notre préfixe?
		Boolean matches = true;
		KUInt32 indexPrefix;
		for (
				indexPrefix = 0;
				(indexPrefix < prefixLength)
					&& (indexString < theTextSize);
				indexPrefix++, indexString++)
		{
			if (UByteSex_ToBigEndian(inText[indexString])
						!= ((KUInt16) inTOCLinePrefix[indexPrefix]))
			{
				matches = false;
				break;
			}
		}
		
		KUInt32 tocEntryStart = 0;	// Avoid gcc warning.
		KUInt32 lineEnd = indexString;
		
		if ((matches) && (indexString < theTextSize))
		{
			tocEntryStart = indexString;
		}
		
		// On court jusqu'à la fin de la ligne.
		for ( ; indexString < theTextSize; indexString++)
		{
			if (inText[indexString]
					== UByteSex_ToBigEndian( (KUInt16) 13 ))
			{
				// Fin de ligne.
				lineEnd = indexString;
				break;
			}
		}
		
		if (indexString == theTextSize)
		{
			// Fin du texte.
			lineEnd = indexString;
		}

		// Ici:
		// matches -> si on a un préfixe.
		// lineStart -> indice du premier caractère de la ligne.
		// lineEnd -> indice juste après le dernier caractère
		//				(saut de ligne ou fin du texte)
		// tocEntryStart -> indice du caractère juste après le préfixe.

		if ((matches) && (lineEnd != tocEntryStart))
		{
			// Ajout de l'entrée. On évite d'ajouter une entrée vide.
			nbTOCEntries++;

			*outTOCEntries =
				(KUInt16**) ::realloc(
					*outTOCEntries, sizeof(KUInt16*) * nbTOCEntries );
			*outTOCIndexes =
				(KUInt32*) ::realloc(
					*outTOCIndexes, sizeof(KUInt32) * nbTOCEntries );

			// Taille avec terminateur.
			KUInt32 theTOCEntrySize = lineEnd - tocEntryStart + 1;
			KUInt16* theTOCEntry =
				(KUInt16*) ::malloc(
					theTOCEntrySize * sizeof(KUInt16) );
			(void) ::memcpy(
							theTOCEntry,
							&inText[tocEntryStart],
							(theTOCEntrySize - 1) * sizeof( KUInt16) );

			theTOCEntry[theTOCEntrySize - 1] = 0;
			(*outTOCEntries)[nbTOCEntries - 1] = theTOCEntry;
			(*outTOCIndexes)[nbTOCEntries - 1] = lineStart;

			if (inDeleteTOCLines)
			{
				// Suppression de la ligne.
				if (theTextSize > lineEnd)
				{
					(void) ::memmove(
						&inText[lineStart],
						(const void*) &inText[lineEnd + 1],
						(theTextSize - lineEnd) * sizeof(KUInt16));
				}				
				theTextSize -= (lineEnd - lineStart);
				indexString -= (lineEnd - lineStart + 1);
			}
		} // if ((matches) && (lineEnd != tocEntryStart))
	}

	*outTOCSize = nbTOCEntries;
	*ioTextSize = theTextSize;
}

// -------------------------------------------------------------------------- //
//  * BuildFont( const char*, int )
// -------------------------------------------------------------------------- //
TDCLNSRef
UDCLPaperback::BuildFont(
		const char* inFontName,
		int inFontSize )
{
	TDCLNSRef theFontFamily = KDCLSYM::kSYMgeneva;
	
	if (inFontName)
	{
		theFontFamily = TDCLNSRef::MakeSymbol( inFontName );
	}
	
	return TDCLFontSpecification( theFontFamily, (KUInt32) inFontSize ).GetNSRef();
}

// -------------------------------------------------------------------------- //
//  * BuildBook( const TDCLNSRef&, const TDCLNSRef&, const char*, ... )
// -------------------------------------------------------------------------- //
TDCLNSRef
UDCLPaperback::BuildBook(
			const TDCLNSRef& inText,
			const TDCLNSRef& inDefaultFont,
			const char* inBookTitle,
			const KUInt16* const * inTOCEntries,
			const KUInt32* inTOCIndexes,
			KUInt32 inTOCSize )
{
	TDCLNSRef theResult = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theBookFrame = theResult.ToFrame();

	KSInt32 theLength = (KSInt32) inText.ToString().GetStrLen();
	
	// Un peu de pub.
	theBookFrame.Set( "abouttext",
		TDCLNSRef::MakeString(
			"Built with PBBookMaker\r(Desktop Connection Library)" ) );
	theBookFrame.Set( "cc", TDCLNSRef::MakeInt( 1 ) );
	theBookFrame.Set( "cp", TDCLNSRef::MakeInt( 0 ) );
	TDCLNSRef theCSArrayRef = TDCLNSRef::MakeArray();
	TDCLNSArray& theCSArray = theCSArrayRef.ToArray();
	theCSArray.Add( TDCLNSRef::MakeInt( -536870911 ) );
	// On découpe par tranche de 2K.
#warning we should fix that once David Fedor remembers about this chunkify magic
	KSInt32 chunkOffset = 2000;
	while (chunkOffset < theLength)
	{
		theCSArray.Add( TDCLNSRef::MakeInt( chunkOffset ) );
		chunkOffset += 2000;
	}	
	theCSArray.Add( TDCLNSRef::MakeInt( theLength ) );
	theBookFrame.Set( "cs", theCSArrayRef );
	theBookFrame.Set( "hs", TDCLNSRef::MakeArray() );
	theBookFrame.Set( "minVers", TDCLNSRef::MakeInt( 2000 ) );
	// places: a binary with a long per TOC entry.
	KUInt32 indexTOC;
	KUInt32 theTOCIndexesBinarySize = inTOCSize * sizeof( KUInt32 );
	KUInt32* theTOCIndexesBinary =
		(KUInt32*) ::malloc( theTOCIndexesBinarySize );
	for (indexTOC = 0; indexTOC < inTOCSize; indexTOC++)
	{
		theTOCIndexesBinary[indexTOC] =
				UByteSex_ToBigEndian( inTOCIndexes[indexTOC] );
	}	
	
	theBookFrame.Set( "places", TDCLNSRef::MakeBinary( theTOCIndexesBinary, theTOCIndexesBinarySize ) );
	::free( theTOCIndexesBinary );

	theBookFrame.Set( "text", inText );
	theBookFrame.Set( "title", TDCLNSRef::MakeString( inBookTitle ) );
	// Idem.
	theBookFrame.Set( "tl", TDCLNSRef::MakeInt( theLength - 1 ) );
	// A string with a TOC entry per line.
	KUInt16* theTOCStringBuffer = (KUInt16*) ::malloc( sizeof( KUInt16 ) );
	KUInt32 theTOCBufferSize = 0;
	theTOCStringBuffer[0] = 0;
	
	for (indexTOC = 0; indexTOC < inTOCSize; indexTOC++)
	{
		const KUInt16* theTOCEntry = inTOCEntries[indexTOC];
		KUInt32 theEntrySize = (KUInt32) UUTF16CStr::StrLen( theTOCEntry );
		KUInt32 theNewBufferSize =
			(theTOCBufferSize + theEntrySize + 2) * sizeof( KUInt16 );
		theTOCStringBuffer =
			(KUInt16*) ::realloc( theTOCStringBuffer, theNewBufferSize );
		
		// Copie des caractères.
		(void) ::memcpy(
			&theTOCStringBuffer[theTOCBufferSize],
			(const void*) theTOCEntry,
			theEntrySize * sizeof( KUInt16 ) );

		// Fin de ligne.
		theTOCBufferSize += theEntrySize;
		theTOCStringBuffer[theTOCBufferSize++] =
					UByteSex_ToBigEndian( (KUInt16) 13 );
	}
		
	// Terminateur.
	theTOCStringBuffer[theTOCBufferSize] = 0;

	theBookFrame.Set(
			"toc", 
			TDCLNSRef::MakeString( (KUInt16*) theTOCStringBuffer ) );
	::free( theTOCStringBuffer );

	theBookFrame.Set( KDCLSYM::kSYMviewFont, inDefaultFont );
	
	return theResult;
}

// ============================================================ //
// Your computer account is overdrawn.  Please see Big Brother. //
// ============================================================ //
