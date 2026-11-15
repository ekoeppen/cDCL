// ==============================
// Fichier:			UDCLMacCarbonUtils.cp
// Projet:			Desktop Connection Library
//
// Créé le:			9/4/2003
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
// The Original Code is UDCLMacCarbonUtils.cp.
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
// $Id: UDCLMacCarbonUtils.cp,v 1.5 2004/11/24 14:09:01 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/UDCLMacCarbonUtils.h>

// ANSI C
#include <stdlib.h>

// MacOS
#if TARGET_OS_MACOS
	#include <TextEncodingConverter.h>
	#include <TextCommon.h>
	#include <UnicodeConverter.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// ------------------------------------------------------------------------- //
//  * HFSPlusToNewtonUnicode( const HFSUniStr255* )
// ------------------------------------------------------------------------- //
KUInt16*
UDCLMacCarbonUtils::HFSPlusToNewtonUnicode( const HFSUniStr255* inHFSPlusName )
{
	// Création du convertisseur
	TextEncoding HFSPlusUnicode =
			::CreateTextEncoding(
				kTextEncodingUnicodeV3_2,
				kUnicodeNoSubset,
				kUnicode16BitFormat);
	TextEncoding NewtonUnicode =
			::CreateTextEncoding(
				kTextEncodingUnicodeV3_2,
				kUnicodeCanonicalCompVariant,
				kUnicode16BitFormat);
	UnicodeMapping theMapping;
	theMapping.unicodeEncoding = HFSPlusUnicode;
	theMapping.otherEncoding = NewtonUnicode;
	theMapping.mappingVersion = kUnicodeUseLatestMapping;
	UnicodeToTextInfo theConverter;
	// Remarque: je n'ai plus de cheveux. CreateTextToUnicodeInfo et
	// TECCreateConverter ne marchent pas (j'ai kTextUnsupportedEncodingErr)
	OSStatus theErr = ::CreateUnicodeToTextInfo(
				&theMapping,
				&theConverter );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}	

	// Allocation.
	ByteCount theLength = ((ByteCount) inHFSPlusName->length) * 2;
		// Majorant de la taille en UTF-16 composé
	KUInt16* theResult = (KUInt16*) ::malloc( theLength + 2 );
	if (theResult == nil)
	{
		throw DCLMemError;
	}
	
	// Conversion.
	ByteCount actualInputLength;
	ByteCount actualOutputLength;
	theErr = ::ConvertFromUnicodeToText(
					theConverter,
					theLength,
					inHFSPlusName->unicode,
					kUnicodeUseFallbacksMask | kUnicodeLooseMappingsMask,
					0,
					NULL,
					NULL, //  oOffsetCount	(les entêtes disent can be NULL,
						  //                 mais pas la doc)
					NULL, //  oOffsetArray	(idem)
					theLength,
					&actualInputLength,
					&actualOutputLength,
					(LogicalAddress) theResult );

	if (theErr != noErr)
	{
		(void) ::DisposeUnicodeToTextInfo( &theConverter );
		throw DCLPlatformUnknownError( theErr );
	}

	// Ajout du terminateur.
	theResult[actualOutputLength / 2] = 0;
	
	// Libération du convertisseur.
	theErr = ::DisposeUnicodeToTextInfo( &theConverter );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * CFToNSString( CFStringRef )
// ------------------------------------------------------------------------- //
TDCLNSRef
UDCLMacCarbonUtils::CFToNSString(
							CFStringRef inCFStringRef )
{
	// Détermination de la taille.
	CFIndex theLength = ::CFStringGetLength( inCFStringRef );
	
	// Allocation de la chaîne avec malloc.
	KUInt16* theBuffer = (KUInt16*)
					::malloc( (size_t) (theLength + 1) * sizeof( KUInt16 ));
	if (theBuffer == nil)
	{
		throw DCLMemError;
	}
	
	// Copie des données
	(void) ::CFStringGetCharacters(
					inCFStringRef,
					CFRangeMake( 0, theLength ),
					theBuffer );

	// Terminateur.
	theBuffer[ theLength ] = 0;
	
	TDCLNSRef theResult = TDCLNSRef::MakeString( theBuffer );
	
	// Libération de la mémoire tampon.
	::free( theBuffer );

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * NSToCFString( const TDCLNSString& )
// ------------------------------------------------------------------------- //
CFStringRef
UDCLMacCarbonUtils::NSToCFString( const TDCLNSString& inNSString )
{
	return ::CFStringCreateWithCharacters(
				kCFAllocatorDefault,
				inNSString.GetString(),
				(CFIndex) inNSString.GetStrLen());
}

// ------------------------------------------------------------------------- //
//  * CFToCString( CFStringRef )
// ------------------------------------------------------------------------- //
const char*
UDCLMacCarbonUtils::CFToCString( CFStringRef inCFStringRef )
{
	// Détermination de la taille.
	CFIndex theLength = ::CFStringGetLength( inCFStringRef );
                            
	// Allocation de la chaîne avec malloc.
	char* theResult = (char*) ::malloc( (size_t) theLength + 1 );
	if (theResult == nil)
	{
		throw DCLMemError;
	}
                            
	// Copie des données avec le terminateur.
	(void) ::CFStringGetCString(
			inCFStringRef,
			theResult,
			theLength + 1,
			kCFStringEncodingMacRoman );

    return theResult;
}

// ====================================================================== //
// A computer without COBOL and Fortran is like a piece of chocolate cake //
// without ketchup and mustard.                                           //
// ====================================================================== //