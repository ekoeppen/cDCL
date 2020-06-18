// ==============================
// Fichier:			TDCLMacOS7Folder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			10/1/2003
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
// The Original Code is TDCLMacOS7Folder.cp.
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
// $Id: TDCLMacOS7Folder.cp,v 1.5 2004/11/24 18:11:19 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7Folder.h>

// ANSI C
#include <stdlib.h>

// K
#include <K/Misc/UPStrings.h>
#include <K/Unicode/UUTF16CStr.h>
#include <K/Unicode/UUTF16Conv.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7File.h>

// MacOS
#include <Files.h>
#include <Script.h>

// -------------------------------------------------------------------------- //
//  * TDCLMacOS7Folder( TDCLMacFiles*, short, long )
// -------------------------------------------------------------------------- //
TDCLMacOS7Folder::TDCLMacOS7Folder(
					TDCLMacFiles* inFilesIntf,
					short inVRefNum,
					long inDirID )
	:
		TDCLMacFolder( inFilesIntf ),
		mVRefNum( inVRefNum ),
		mDirID( inDirID )
{
	// Peinture fraîche.
}

// -------------------------------------------------------------------------- //
//  * TDCLMacOS7Folder( TDCLMacFiles*, TDCLFSItemRef, long )
// -------------------------------------------------------------------------- //
TDCLMacOS7Folder::TDCLMacOS7Folder(
					TDCLMacFiles* inFilesIntf,
					TDCLFSItemRef inParentFolder,
					long inDirID )
	:
		TDCLMacFolder( inFilesIntf, inParentFolder ),
		mDirID( inDirID )
{
	// Récupération de la référence sur le volume.
	mVRefNum = ((TDCLMacOS7Folder*) ((TDCLFolder*) inParentFolder))->mVRefNum;
}

// -------------------------------------------------------------------------- //
//  * GetItemByName( const KUInt16*, KSInt32 )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7Folder::GetItemByName(
									const KUInt16* inName,
									KSInt32 /* inVolRefNum */ /* = 0 */ )
{
	TDCLFSItemRef theResult;
#warning incomplete
	// Création du FSSpec pour le fichier.
	FSSpec theFSSpec;
	if (MakeFSSpecForItem( inName, &theFSSpec ))
	{
		// Est-ce un dossier ou un fichier?

		throw DCLNotImplementedError;
	} // else l'élément n'existe pas.

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateFile( const KUInt16*, OSType, OSType )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7Folder::CreateFile(
					const KUInt16* inName,
					OSType inCreator /* = TDCLMacFiles::kCreator */,
					OSType inFileType /* = TDCLMacFiles::kFileType */ )
{
	// Création du FSSpec pour le fichier.
	FSSpec theFSSpec;
	if (MakeFSSpecForItem( inName, &theFSSpec ))
	{
		throw DCLExistsAlready;
	}

	TDCLMacOS7File* theFileResult =
		new TDCLMacOS7File(
				(TDCLMacFiles*) GetFilesIntf(),
				TDCLFSItemRef( this ),
				&theFSSpec );
	TDCLFSItemRef theResult( theFileResult );

	theFileResult->Create( inCreator, inFileType );

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateFolder( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7Folder::CreateFolder( const KUInt16* inName )
{
	// Création du FSSpec pour le dossier.
	FSSpec theFSSpec;
	if (MakeFSSpecForItem( inName, &theFSSpec ))
	{
		throw DCLExistsAlready;
	}

	KSInt32 theDirID;
	OSErr theErr = FSpDirCreate( &theFSSpec, smSystemScript, &theDirID );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	return TDCLFSItemRef(
		new TDCLMacOS7Folder(
				(TDCLMacFiles*) GetFilesIntf(),
				TDCLFSItemRef( this ),
				theDirID ));
}

// -------------------------------------------------------------------------- //
//  * CountElements( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLMacOS7Folder::CountElements( void )
{
#warning TODO
	return 0;
}

// -------------------------------------------------------------------------- //
//  * GetItems( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLMacOS7Folder::GetItems( void )
{
#warning TODO
	return nil;
}

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLMacOS7Folder::MakeName( void ) const
{
	// Récupération du nom.
	Str255 theFolderName;		// Mémoire tampon pour le nom.
	CInfoPBRec theParamBlock;	// Structure pour PBGetCatInfo

	/*	http://developer.apple.com/techpubs/mac/Files/Files-238.html
		If the value of ioFDirIndex is negative, PBGetCatInfo ignores
		ioNamePtr and returns information about the directory specified
		by ioDrDirID. */

	theParamBlock.dirInfo.ioNamePtr = (StringPtr) &theFolderName;
	theParamBlock.dirInfo.ioVRefNum = mVRefNum;
	theParamBlock.dirInfo.ioFDirIndex = -1;
	theParamBlock.dirInfo.ioDrDirID = mDirID;

	OSErr theErr = PBGetCatInfoSync( &theParamBlock );

	/*
		noErr				0		No error
		nsvErr				-35		No such volume
		ioErr				-36		I/O error
		bdNamErr			-37		Bad filename
		fnfErr				-43		File not found
		paramErr			-50		No default volume
		dirNFErr			-120	Directory not found or incomplete pathname
		afpAccessDenied		-5000	User does not have the correct access
		afpObjectTypeErr	-5025	Directory not found or incomplete pathname
	*/

	switch ( theErr )
	{
		case noErr:
			break;

		case fnfErr:
		case dirNFErr:
		case afpObjectTypeErr:
			throw DCLPlatformDoesntExist( theErr );
			break;

		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */

	// Allocation de l'espace nécessaire (avec malloc)
	unsigned int theSize = (unsigned int) theFolderName[0];
	KUInt16* theResult = (KUInt16*)
		::malloc( theSize * sizeof( KUInt16 ) );

	// Conversion depuis MacRoman
	UUTF16Conv::FromMacRoman(
		(KUInt8*) &theFolderName[1], theResult, theSize );

	// Ajout du terminateur.
	theResult[theSize] = 0;

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * MakeParentFolder( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacOS7Folder::MakeParentFolder( void ) const
{
	// Création d'un nouveau TDCLMacOS7Folder
	return TDCLFSItemRef(
			new TDCLMacOS7Folder(
					(TDCLMacFiles*) GetFilesIntf(),
					mVRefNum,
					GetParentID() ));
}

// -------------------------------------------------------------------------- //
//  * GetParentID( void ) const
// -------------------------------------------------------------------------- //
long
TDCLMacOS7Folder::GetParentID( void ) const
{
	// Récupération de l'ID du dossier parent.
	CInfoPBRec theParamBlock;

	/*	http://developer.apple.com/techpubs/mac/Files/Files-238.html
		If the value of ioFDirIndex is negative, PBGetCatInfo ignores
		ioNamePtr and returns information about the directory specified
		by ioDrDirID. */

	theParamBlock.dirInfo.ioNamePtr = nil;
	theParamBlock.dirInfo.ioVRefNum = mVRefNum;
	theParamBlock.dirInfo.ioFDirIndex = -1;
	theParamBlock.dirInfo.ioDrDirID = mDirID;

	OSErr theErr = PBGetCatInfoSync( &theParamBlock );

	/*
		noErr				0		No error
		nsvErr				-35		No such volume
		ioErr				-36		I/O error
		bdNamErr			-37		Bad filename
		fnfErr				-43		File not found
		paramErr			-50		No default volume
		dirNFErr			-120	Directory not found or incomplete pathname
		afpAccessDenied		-5000	User does not have the correct access
		afpObjectTypeErr	-5025	Directory not found or incomplete pathname
	*/

	switch ( theErr )
	{
		case noErr:
			break;

		case fnfErr:
		case dirNFErr:
		case afpObjectTypeErr:
			throw DCLPlatformDoesntExist( theErr );
			break;

		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */

	return theParamBlock.dirInfo.ioDrParID;
}

// -------------------------------------------------------------------------- //
//  * MakeFSSpecForItem( const KUInt16*, KSInt32, FSSpec* ) const throw ...
// -------------------------------------------------------------------------- //
Boolean
TDCLMacOS7Folder::MakeFSSpecForItem(
					const KUInt16* inName,
					FSSpec* outFSSpec ) const
{
	Boolean theResult = true;	// Si l'élément existe.

	// Traduction du nom en StrFileName.
	size_t theNameLen = UUTF16CStr::StrLen( inName );

	StrFileName theFileName;
	size_t theOutputLen = sizeof(theFileName) - 1;
	if (UUTF16Conv::ToMacRoman(
				inName,
				&theNameLen,
				(KUInt8*) &theFileName[1],
				&theOutputLen,
				UUTF16Conv::kStopOnUnrepChar) != UUTF16Conv::kInputExhausted)
	{
		throw DCLBadParamError;
	}

	theFileName[0] = (unsigned char) theOutputLen;

	OSErr theErr = FSMakeFSSpec( mVRefNum, mDirID, theFileName, outFSSpec );
	if (theErr)
	{
		if (theErr == fnfErr)
		{
			theResult = false;	// L'élément n'existe pas.
		} else {
			throw DCLPlatformUnknownError( theErr );
		}
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * IsVolume( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacOS7Folder::IsVolume( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * IsOnDesktop( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacOS7Folder::IsOnDesktop( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetVRefNum( void ) const
// ------------------------------------------------------------------------- //
short
TDCLMacOS7Folder::GetVRefNum( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ============================================================================ //
// [From the operation manual for the CI-300 Dot Matrix Line Printer, made      //
// in Japan]:                                                                   //
//                                                                              //
// The excellent output machine of MODEL CI-300 as extraordinary DOT MATRIX     //
// LINE PRINTER, built in two MICRO-PROCESSORs as well as EAROM, is featured by //
// permitting wonderful co-existence such as; "high quality against low cost,"  //
// "diversified functions with compact design," "flexibility in accessibleness  //
// and durability of approx. 2000,000,00 Dot/Head," "being sophisticated in     //
// mechanism but possibly agile operating under noises being extremely          //
// suppressed" etc.                                                             //
//                                                                              //
// And as a matter of course, the final goal is just simply to help achieve     //
// "super shuttle diplomacy" between cool data, perhaps earned by HOST          //
// COMPUTER, and warm heart of human being.                                     //
// ============================================================================ //