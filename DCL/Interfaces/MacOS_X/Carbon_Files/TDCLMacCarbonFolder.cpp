// ==============================
// Fichier:			TDCLMacCarbonFolder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			13/1/2003
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
// The Original Code is TDCLMacCarbonFolder.cp.
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
// $Id: TDCLMacCarbonFolder.cp,v 1.7 2004/11/24 18:11:23 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/MacOS_X/UDCLMacCarbonUtils.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFile.h>

// ANSI C
#include <stdlib.h>

// ISO C++
#include <new>

#warning improve error handling

// -------------------------------------------------------------------------- //
//  * TDCLMacCarbonFolder( TDCLMacFiles*, const FSRef* )
// -------------------------------------------------------------------------- //
TDCLMacCarbonFolder::TDCLMacCarbonFolder(
					TDCLMacFiles* inFilesIntf,
					const FSRef* inRef )
	:
		TDCLMacFolder( inFilesIntf ),
		mRef( *inRef ),
		mVRefNum( 0 ),
		mIsVolume( false ),
		mIsOnDesktop( false )
{
	Init();
}

// -------------------------------------------------------------------------- //
//  * TDCLMacCarbonFolder( TDCLMacFiles*, TDCLMacFolder*, const FSRef* )
// -------------------------------------------------------------------------- //
TDCLMacCarbonFolder::TDCLMacCarbonFolder(
					TDCLMacFiles* inFilesIntf,
					TDCLFSItemRef inParentFolder,
					const FSRef* inRef )
	:
		TDCLMacFolder( inFilesIntf, inParentFolder ),
		mRef( *inRef ),
		mVRefNum( 0 ),
		mIsVolume( false ),
		mIsOnDesktop( false )
{
	Init();
}

// -------------------------------------------------------------------------- //
//  * Init( void )
// -------------------------------------------------------------------------- //
void
TDCLMacCarbonFolder::Init( void )
{
	// Récupération de la référence sur le volume.
	FSCatalogInfo theCatalogInfo;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoVolume,
						&theCatalogInfo, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						NULL /* parentRef */);

	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	mVRefNum = theCatalogInfo.volume;

	// Sommes-nous un volume?
	FSRef theVolumeRootFolderRef;
	theErr = ::FSGetVolumeInfo(
					mVRefNum,
					0,				// volumeIndex
					NULL,			// actualVolume
					kFSVolInfoNone,	// whichInfo
					NULL,			// info
					NULL,			// volumeName
					&theVolumeRootFolderRef ); // rootDirectory
	if ((theErr == noErr)
		&& (::FSCompareFSRefs(
					&mRef, &theVolumeRootFolderRef ) == noErr))
	{
		mIsVolume = true;
	} else {
		mIsVolume = false;

		// Récupération de la référence sur le parent.
		theErr = ::FSGetCatalogInfo(
					&mRef,
					kFSCatInfoNone,
					NULL, /* catalogInfo */
					NULL, /* outName */
					NULL, /* fsSpec */
					&mParentRef /* parentRef */);

		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		// Sommes-nous sur le bureau?
		FSRef theDesktopFolderRef;
		theErr = ::FSFindFolder(
							kOnAppropriateDisk,
							kDesktopFolderType,
							false,
							&theDesktopFolderRef );

		if ((theErr == noErr)
			&& (::FSCompareFSRefs(
					&mParentRef,
					&theDesktopFolderRef ) == noErr))
		{
			mIsOnDesktop = true;
		} else {
			mIsOnDesktop = false;
		}
	}
}

// -------------------------------------------------------------------------- //
//  * GetItemByName( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFolder::GetItemByName(
						const KUInt16 *inName,
						KSInt32 /* inVolRefNum *//* = 0 */ )
{
	TDCLFSItem* theItem = nil;
	FSRef theItemRef;
	 
	OSErr theErr = ::FSMakeFSRefUnicode(
								&mRef,
								UUTF16CStr::StrLen(inName),
								inName,
								kUnicode16BitFormat,
								&theItemRef);
	if (theErr != fnfErr)
	{
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		FSCatalogInfo theCatalogInfo;
		theErr = ::FSGetCatalogInfo(
									&theItemRef,
									kFSCatInfoNodeFlags,
									&theCatalogInfo,
									NULL,
									NULL,
									NULL );

		if (theCatalogInfo.nodeFlags & kFSNodeIsDirectoryMask)
		{
			// bummer another folder...
			theItem = new TDCLMacCarbonFolder(
							(TDCLMacFiles*) GetFilesIntf(),
							TDCLFSItemRef( this ),
							&theItemRef );
		} else {
			// congratulations, it's a file!
			theItem = new TDCLMacCarbonFile(
							(TDCLMacFiles*) GetFilesIntf(),
							TDCLFSItemRef( this ),
							&theItemRef );
		}
	}
		
	return TDCLFSItemRef( theItem );
}

// -------------------------------------------------------------------------- //
//  * CreateFile( const KUInt16*, OSType, OSType )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFolder::CreateFile(
					const KUInt16* inName,
					OSType inCreator /* = TDCLMacFiles::kCreator */,
					OSType inFileType /* = TDCLMacFiles::kFileType */ )
{
	TDCLFSItemRef theResult =
				TDCLFSItemRef( new TDCLMacCarbonFile(
							(TDCLMacFiles*) GetFilesIntf(),
							this,
							nil,
							inName ));
	
	((TDCLMacCarbonFile*) ((TDCLFile*) theResult))
											->Create( inCreator, inFileType );
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateFolder( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFolder::CreateFolder( const KUInt16 *inName )
{
	FSRef newRef;
	OSErr theErr = FSCreateDirectoryUnicode(
							&mRef,
							UUTF16CStr::StrLen(inName),
							inName,
							0L,
							NULL,
							&newRef,
							NULL,
							NULL); 

	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	return TDCLFSItemRef(
				new TDCLMacCarbonFolder(
					(TDCLMacFiles*) GetFilesIntf(), &newRef ) );
}

// -------------------------------------------------------------------------- //
//  * CountElements( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLMacCarbonFolder::CountElements( void )
{
	FSCatalogInfo catalogInfo;
	 
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoValence ,
						&catalogInfo,
						NULL,
						NULL,
						NULL); 
	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	return catalogInfo.valence;
} 

// -------------------------------------------------------------------------- //
//  * GetItems( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLMacCarbonFolder::GetItems( void ) 
{
	return GetFolderItems( (TDCLMacFiles*) GetFilesIntf(), &mRef );
}

// -------------------------------------------------------------------------- //
//  * GetFolderItems( TDCLMacFiles*, const FSRef*, KUInt32* ) ...
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLMacCarbonFolder::GetFolderItems(
				TDCLMacFiles* inFilesIntf,
				const FSRef* inRef,
				KUInt32* outCount ) 
{
	const ItemCount	kRequestCount = 4096; 
	TDCLFSItemRef* theResult =
						(TDCLFSItemRef*) ::malloc( sizeof( TDCLFSItemRef ) );
		// Terminateur
	KUInt32 theResultIndex = 0;

	// Création d'un itérateur.
	FSIterator theIterator;
	OSStatus myStatus =
			::FSOpenIterator( inRef, kFSIterateFlat, &theIterator );

	if( myStatus != noErr )
	{
		throw DCLPlatformUnknownError( myStatus );
	}

	// Allocate storage for the returned information
	FSCatalogInfo* catalogInfoArray =
				(FSCatalogInfo *) ::malloc(
									sizeof(FSCatalogInfo) * kRequestCount );
	FSRef* fsRefArray =
				(FSRef *) ::malloc( sizeof(FSRef) * kRequestCount );

	ItemCount actualCount;
	Boolean gotThemAll = false;
	while ( !gotThemAll ) {
		myStatus = FSGetCatalogInfoBulk(
						theIterator,
						kRequestCount,
						&actualCount,
						NULL,
						kFSCatInfoNodeFlags | kFSCatInfoFinderInfo,
						catalogInfoArray,
						fsRefArray,
						NULL,
						NULL );

		if (myStatus == noErr)
		{
			gotThemAll = false;
		} else if (myStatus == errFSNoMoreItems) {
			gotThemAll = true;
		} else {
			// Nettoyage.
			::free( catalogInfoArray );
			::free( fsRefArray );
			KUInt32 indexResult;
			for (
					indexResult = 0;
					indexResult < theResultIndex;
					indexResult++)
			{
				theResult[indexResult].~TDCLFSItemRef();
			}
			::free( theResult );
			throw DCLPlatformUnknownError( myStatus );
		}
		
		// On redimensionne le résultat.
		theResult = (TDCLFSItemRef*) ::realloc(
							theResult, 
							sizeof( TDCLFSItemRef )
								* (theResultIndex + 1 + actualCount) );

		// Ajout des éléments.		
		KUInt32 indexElements;
		for( indexElements = 0; indexElements < actualCount; indexElements++ )
		{
			if ( catalogInfoArray[indexElements].nodeFlags
												& kFSNodeIsDirectoryMask )
			{
				// bummer another folder...
				new (&theResult[theResultIndex++]) TDCLFSItemRef(
							new TDCLMacCarbonFolder(
										inFilesIntf,
										&fsRefArray[indexElements] ));
			} else {
				// congratulations, it's a file!
				new (&theResult[theResultIndex++]) TDCLFSItemRef(
							new TDCLMacCarbonFile(
										inFilesIntf,
										&fsRefArray[indexElements] ));
			}
		}
	}
	
	// Libération des tableaux.
	::free( (void *) catalogInfoArray );
	::free( (void *) fsRefArray );

	// Ajout du terminateur.
	new (&theResult[theResultIndex]) TDCLFSItemRef();
	
	// On retourne le nombre.
	if (outCount)
	{
		*outCount = theResultIndex;
	}

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLMacCarbonFolder::MakeName( void ) const
{
	HFSUniStr255 nameStructure;
	 
	OSErr theErr = ::FSGetCatalogInfo(
							&mRef, 0L, NULL, &nameStructure, NULL, NULL );
	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	 
	return UDCLMacCarbonUtils::HFSPlusToNewtonUnicode( &nameStructure );
} 

// -------------------------------------------------------------------------- //
//  * MakeParentFolder( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFolder::MakeParentFolder( void ) const 
{
	TDCLFSItemRef theResult;
	
	if ((mIsVolume) || (mIsOnDesktop))
	{
		theResult = GetFilesIntf()->GetRootFolder();
	} else {
		theResult = TDCLFSItemRef(new TDCLMacCarbonFolder(
								(TDCLMacFiles*) GetFilesIntf(), &mParentRef ));
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * IsVolume( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacCarbonFolder::IsVolume( void ) const
{
	return mIsVolume;
}

// ------------------------------------------------------------------------- //
//  * IsOnDesktop( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacCarbonFolder::IsOnDesktop( void ) const
{
	return mIsOnDesktop;
}

// ------------------------------------------------------------------------- //
//  * GetVRefNum( void ) const
// ------------------------------------------------------------------------- //
short
TDCLMacCarbonFolder::GetVRefNum( void ) const
{
	return mVRefNum;
}

// ============================================ //
//  INVENTORY                                   //
// Four be the things I am wiser to know:       //
// Idleness, sorrow, a friend, and a foe.       //
//                                              //
// Four be the things I'd been better without:  //
// Love, curiosity, freckles, and doubt.        //
//                                              //
// Three be the things I shall never attain:    //
// Envy, content, and sufficient champagne.     //
//                                              //
// Three be the things I shall have till I die: //
// Laughter and hope and a sock in the eye.     //
// ============================================ //