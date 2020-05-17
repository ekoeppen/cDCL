// ==============================
// Fichier:			TDCLMacCarbonFile.cp
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
// The Original Code is TDCLMacCarbonFile.cp.
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
// $Id: TDCLMacCarbonFile.cp,v 1.13 2004/11/24 18:11:23 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFile.h>

// MacOS
#if TARGET_OS_MACOS
	#include <Folders.h>
	#include <CFString.h>
	#include <Icons.h>
	#include <LaunchServices.h>
#else
	#include <CoreServices/CoreServices.h>
	#include <CoreFoundation/CFString.h>
	#include <ApplicationServices/ApplicationServices.h>
#endif

// K
#include <K/Math/UTInt64.h>
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>
#include <DCL/Interfaces/TDCLRootFolder.h>
#include <DCL/Interfaces/MacOS/TDCLMacFolder.h>
#include <DCL/Interfaces/MacOS_X/UDCLMacCarbonUtils.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonDesktopFolder.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

#warning improve error handling

// ------------------------------------------------------------------------- //
//  * TDCLMacCarbonFile( TDCLMacFiles*, TDCLFSItemRef, const FSRef* )
// ------------------------------------------------------------------------- //
TDCLMacCarbonFile::TDCLMacCarbonFile(
				TDCLMacFiles* inFilesIntf,
				TDCLFSItemRef inFolder,
				const FSRef* inFSRef )
	:
		TDCLMacFile( inFilesIntf, inFolder ),
		mRef( *inFSRef ),
		mRefIsValid( true ),
		mOpenedFork( 0 ),
		mVRefNum( 0 ),
		mIsOnDesktop( false )
{
	// On récupère le nom du fichier (utile si on se fait supprimer et qu'il
	// faut nous recréer).
	SetName( MakeName() );
	
	// Récupération de la référence sur le volume et du dossier parent.
	FSCatalogInfo theCatalogInfo;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoVolume,
						&theCatalogInfo, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						&mParentRef /* parentRef */);

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	mVRefNum = theCatalogInfo.volume;
}

// ------------------------------------------------------------------------- //
//  * TDCLMacCarbonFile( TDCLMacFiles*, TDCLFSItemRef, const FSRef* ... )
// ------------------------------------------------------------------------- //
TDCLMacCarbonFile::TDCLMacCarbonFile(
				TDCLMacFiles* inFilesIntf,
				TDCLFSItemRef inFolder,
				const FSRef* inFSRef,
				const KUInt16* inName )
	:
		TDCLMacFile( inFilesIntf, inFolder ),
		mRefIsValid( false ),
		mOpenedFork( 0 ),
		mVRefNum( 0 ),
		mIsOnDesktop( false )
{
	// Copie du nom du fichier.
	size_t theNameSize = (UUTF16CStr::StrLen( inName ) + 1) * sizeof( KUInt16);
	KUInt16* theName = (KUInt16*) ::malloc( theNameSize );
	if (theName == nil)
	{
		throw DCLMemError;
	}
	
	(void) ::memcpy( theName, inName, theNameSize );
	
	// On enregistre le nom du fichier.
	SetName( theName );
	
	// Et le FSRef s'il est correct.
	if (inFSRef)
	{
		mRef = *inFSRef;
		mRefIsValid = true;
	}
	
	mParentRef = *((TDCLMacCarbonFolder&) *inFolder).GetRef();
	mVRefNum = ((TDCLMacCarbonFolder&) *inFolder).GetVRefNum();
}

// -------------------------------------------------------------------------- //
//  * TDCLMacCarbonFile( TDCLMacFiles*, const FSRef* )
// -------------------------------------------------------------------------- //
TDCLMacCarbonFile::TDCLMacCarbonFile(
				TDCLMacFiles* inFilesIntf,
				const FSRef* inRef )
	:
		TDCLMacFile( inFilesIntf,
					DoMakeParentFolder( inFilesIntf, &mIsOnDesktop, inRef ) ),
		mRef( *inRef ),
		mRefIsValid( true ),
		mOpenedFork( 0 ),
		mVRefNum( 0 )
{
	// On récupère le nom du fichier.
	SetName( MakeName() );

	// Récupération de la référence sur le volume et du dossier parent.
	FSCatalogInfo theCatalogInfo;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoVolume,
						&theCatalogInfo, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						&mParentRef /* parentRef */);

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	mVRefNum = theCatalogInfo.volume;
}

// ------------------------------------------------------------------------- //
//  * TDCLMacCarbonFile( void )
// ------------------------------------------------------------------------- //
TDCLMacCarbonFile::~TDCLMacCarbonFile()
{
	Close();
}

// ------------------------------------------------------------------------- //
//  * Open( Boolean )
// ------------------------------------------------------------------------- //
void 
TDCLMacCarbonFile::Open( Boolean inReadOnly ) 
{
	// On s'arrête si le fichier est déjà ouvert.
	if (IsOpen())
	{
		throw DCLBadStateError;
	}

	// Création du FSRef si nécessaire.
	if (!mRefIsValid)
	{
		CreateFSRef();
	}
	
	HFSUniStr255 dataForkName;
	OSErr theErr = ::FSGetDataForkName( &dataForkName );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	KSInt8 perms;

	if(inReadOnly) 
	{
		perms = fsRdPerm;
	} else {
		perms = fsRdWrPerm;
	}
	theErr = ::FSOpenFork(
					&mRef,
					dataForkName.length,
					dataForkName.unicode,
					perms,
					&mOpenedFork );
	
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	DoOpen( inReadOnly );
}

// ------------------------------------------------------------------------- //
//  * Create( OSType, OSType )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::Create(
						OSType inCreator,
						OSType inFileType )
{
	// Création du fichier.
	FSSpec theFSSpec;	// FSSpec pour changer le type/créateur.
	const KUInt16* theName = GetName();
	OSErr theErr = ::FSCreateFileUnicode(
						&mParentRef,
						UUTF16CStr::StrLen( theName ),
						theName,
						kFSCatInfoNone,
						NULL /* catalogInfo */,
						&mRef,
						&theFSSpec );

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	mRefIsValid = true;

	// Changement du type/créateur.
	DoSetMetaData( &theFSSpec, inCreator, inFileType );
}

// ------------------------------------------------------------------------- //
//  * SetMetaData( OSType, OSType )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::SetMetaData(
						OSType inCreator,
						OSType inFileType )
{
	// Création du FSRef si nécessaire.
	if (!mRefIsValid)
	{
		CreateFSRef();
	}

	// Récupération du FSSpec sur le fichier.
	FSSpec theFSSpec;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoNone,
						NULL, /* catalogInfo */
						NULL, /* outName */
						&theFSSpec, /* fsSpec */
						NULL /* parentRef */);

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Changement du type/créateur.
	DoSetMetaData( &theFSSpec, inCreator, inFileType );
}

// ------------------------------------------------------------------------- //
//  * Delete( void )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::Delete( void ) 
{
	// Vérifions que le fichier n'est pas ouvert.
	if (IsOpen())
	{
		throw DCLBadStateError;
	}

	// Création du FSRef si nécessaire.
	if (!mRefIsValid)
	{
		CreateFSRef();
	}

	OSErr theErr = ::FSDeleteObject( &mRef );

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Le FSRef n'est plus valide.
	mRefIsValid = false;
} 

// ------------------------------------------------------------------------- //
//  * SetCursor( KSInt64, ECursorMode )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::SetCursor( KSInt64 inPos, ECursorMode inMode )
{
	KUInt16 positionMode;
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	switch(inMode) {
		case kFromStart:
			positionMode = fsFromStart;
			break;
		case kFromLEOF:
			positionMode = fsFromLEOF;
			break;
		default:
			positionMode = fsFromMark;
			break;
	}

	OSErr theErr = ::FSSetForkPosition( mOpenedFork, positionMode, inPos );
	if (theErr != noErr)
	{
		throw DCLPlatformPosition( theErr );
	}
} 

// ------------------------------------------------------------------------- //
//  * GetCursor( void )
// ------------------------------------------------------------------------- //
KSInt64
TDCLMacCarbonFile::GetCursor( void )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	SInt64 thePosition;
	OSErr theErr = ::FSGetForkPosition( mOpenedFork, &thePosition );
	if (theErr != noErr)
	{
		throw DCLPlatformPosition( theErr );
	}
	
	return thePosition;
} 

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt64
TDCLMacCarbonFile::GetLength( void )
{
	FSCatalogInfo catalogInfo;
	 
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoDataSizes,
						&catalogInfo,
						NULL, /* outName */
						NULL, /* fsSpec */
						NULL /* parentRef */);

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

#if TYPE_LONGLONG
	return catalogInfo.dataLogicalSize;
	// Remarque: ça risque de merder ici si:
	//	defined(_MSC_VER) && !defined(__MWERKS__) && defined(_M_IX86)
	// Mais normalement, on n'est pas dans ce cas.
	// Et si ça l'est, il suffit d'utiliser
	// leur super macro dans Math64.h qui risque d'être foireuse
	// comme x86 n'a pas le même indien.
#else
	return UTInt64::CreateKUInt64(
						catalogInfo.dataLogicalSize.hi,
						catalogInfo.dataLogicalSize.lo );
#endif
} 

// ------------------------------------------------------------------------- //
//  * Read( void*, long* )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::Read (void *outBuffer, KUInt32 *ioCount)
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	
	ByteCount count;
	OSErr theErr = ::FSReadFork(
							mOpenedFork, 
							fsAtMark, 
							0,	// right here, right now
								// (euh, il n'est pas ignoré ce paramètre?)
							*ioCount, 
							outBuffer, 
							&count ); 

	*ioCount = count;

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
} 

// ------------------------------------------------------------------------- //
//  * Write( const void*, long* )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::Write (const void *inBuffer, KUInt32 *ioCount)
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	
	// Vérifions que le fichier est ouvert en lecture/écriture.
	if (IsReadOnly())
	{
		throw DCLReadOnly;
	}

	ByteCount count;
	OSErr theErr = ::FSWriteFork(
							mOpenedFork, 
							fsAtMark, 
							0,	// right here, right now
								// (euh, il n'est pas ignoré ce paramètre?)
							*ioCount, 
							inBuffer, 
							&count ); 

	*ioCount = count;

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
} 

// ------------------------------------------------------------------------- //
//  * FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::FlushOutput( void )
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	OSErr theErr = ::FSFlushFork( mOpenedFork );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * Close( void )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::Close (void)
{
	if (IsOpen())
	{
		OSErr theErr = ::FSCloseFork( mOpenedFork );
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		DoClose();
	}
}

// ------------------------------------------------------------------------- //
//  * GetKind( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacCarbonFile::GetKind( void ) const
{
	// Récupération du type grâce au LaunchServices.
	CFStringRef theKindString;
	OSStatus theErr = ::LSCopyKindStringForRef( &mRef, &theKindString );
	
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	CFIndex theSize = ::CFStringGetLength( theKindString );
	
	// Allocation.
	KUInt16* theString =
			(KUInt16*) ::malloc( (theSize + 1) * sizeof( KUInt16 ) );

	// Copie de la chaîne.
	::CFStringGetCharacters(
				theKindString, CFRangeMake( 0, theSize ), theString );

	// Terminateur.
	theString[theSize] = 0;
	
	// Libération (1)
	::CFRelease( theKindString );

	TDCLNSRef theResult = TDCLNSRef::MakeString( theString );

	// Libération (2)
	::free( theString );	
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetCreationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacCarbonFile::GetCreationDate( void ) const
{
	FSCatalogInfo theCatalogInfo;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoCreateDate,
						&theCatalogInfo, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						NULL );
	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Traduction au format de date Newton.
	KUInt64 theResultInSeconds =
				UTInt64::CreateKUInt64(
						theCatalogInfo.createDate.highSeconds,
						theCatalogInfo.createDate.lowSeconds );
	return UTInt64::GetLo( theResultInSeconds / 60 );
}

// ------------------------------------------------------------------------- //
//  * GetModificationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacCarbonFile::GetModificationDate( void ) const
{
	FSCatalogInfo theCatalogInfo;
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoContentMod,
						&theCatalogInfo, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						NULL );
	if (theErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Traduction au format de date Newton.
	// J'utilise les entiers sur 64 bits pour faire la division.
	KUInt64 theResultInSeconds =
				UTInt64::CreateKUInt64(
						theCatalogInfo.contentModDate.highSeconds,
						theCatalogInfo.contentModDate.lowSeconds );
	return UTInt64::GetLo( theResultInSeconds / 60 );
}

// ------------------------------------------------------------------------- //
//  * GetStringPath( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacCarbonFile::GetStringPath( void ) const
{
	// Allocation de la chaîne.
	KUInt8* thePathString = (KUInt8*) ::malloc( 4096 );
	OSStatus theErr = ::FSRefMakePath( &mRef, thePathString, 4096 );
	if (theErr)
	{
		::free( thePathString );
		throw DCLPlatformUnknownError( theErr );
	}
	
	// La même en UTF-16.
	KUInt16* thePathStringAsUTF16 = (KUInt16*) ::malloc( 8192 );
	UUTF16CStr::FromUTF8( thePathString, thePathStringAsUTF16, 4096 );

	// Libération.
	::free( thePathString );

	// Conversion en chaîne NewtonScript.
	TDCLNSRef theResult = TDCLNSRef::MakeString( thePathStringAsUTF16 );
	
	// Libération.
	::free( thePathStringAsUTF16 );

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetIcon( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacCarbonFile::GetIcon( void ) const
{
	// Récupération d'informations sur le fichier.
	HFSUniStr255 theFileName;
	FSCatalogInfo theCatalogInfo;
	OSStatus theErr = ::FSGetCatalogInfo(
						&mRef,
						kIconServicesCatalogInfoMask,
						&theCatalogInfo, /* catalogInfo */
						&theFileName, /* outName */
						NULL, /* fsSpec */
						NULL /* parentRef */);
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	// Récupération de l'icône.
	IconRef theIcon;
	theErr = ::GetIconRefFromFileInfo (
						&mRef,
						theFileName.length,
						theFileName.unicode,
						kIconServicesCatalogInfoMask,
						&theCatalogInfo,
						kIconServicesNormalUsageFlag /* inUsageFlags */,
						&theIcon,
						NULL);
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Conversion en noir et blanc, 32x32.
	IconFamilyHandle theIconFamilyHandle;
	theErr = ::IconRefToIconFamily(
			theIcon,
			kSelectorLarge1Bit,
			&theIconFamilyHandle );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	if (theIconFamilyHandle == nil)
	{
		throw DCLUnknownError;
	}
	
	// Extraction.
	Handle theIconHandle = ::NewHandle( 128 );	// 32x32/8
	theErr = ::GetIconFamilyData(
			theIconFamilyHandle,
			'ICN#',
			theIconHandle );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Création du binaire.
	::HLock( theIconHandle );
	TDCLNSRef theResult =
				TDCLNSRef::MakeBinary( *theIconHandle, 128 /* 32*32/8 */ );

	// Libération des données.
	::DisposeHandle( theIconHandle );
	::DisposeHandle( (Handle) theIconFamilyHandle );
	
	// Libération de la référence.
	theErr = ReleaseIconRef( theIcon );
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * MakeName( void ) const
// ------------------------------------------------------------------------- //
KUInt16*
TDCLMacCarbonFile::MakeName( void ) const
{
	// MakeName ne peut être appelé si mRef n'est pas valide.
	if (!mRefIsValid)
	{
		throw DCLDoesntExist;
	}

	HFSUniStr255 theFileName;
	 
	OSErr theErr = ::FSGetCatalogInfo(
						&mRef,
						kFSCatInfoNone,
						NULL, /* catalogInfo */
						&theFileName,
						NULL, /* fsSpec */
						NULL /* parentRef */);
	
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	return UDCLMacCarbonUtils::HFSPlusToNewtonUnicode( &theFileName );
} 

// ------------------------------------------------------------------------- //
//  * DoMakeParentFolder( TDCLMacFiles*, Boolean*, const FSRef* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFile::DoMakeParentFolder(
					TDCLMacFiles* inFilesIntf,
					Boolean* outIsOnDesktop,
					const FSRef* inFSRef )
{
	// Référence sur le dossier parent et sur volume.
	FSRef theParentRef;
	OSErr theErr = ::FSGetCatalogInfo(
						inFSRef,
						kFSCatInfoNone,
						NULL, /* catalogInfo */
						NULL, /* outName */
						NULL, /* fsSpec */
						&theParentRef );
	
	if (theErr != noErr)
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

	TDCLFSItemRef theResult;
	if ((theErr == noErr)
		&& (::FSCompareFSRefs( &theParentRef, &theDesktopFolderRef ) == noErr))
	{
		*outIsOnDesktop = true;
		theResult = inFilesIntf->GetRootFolder( nil /* ignoré */ );
	} else {
		*outIsOnDesktop = false;
		theResult = TDCLFSItemRef(
					new TDCLMacCarbonFolder( inFilesIntf, &theParentRef ) );
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * CreateFSRef( void )
// ------------------------------------------------------------------------- //
void
TDCLMacCarbonFile::CreateFSRef( void )
{
	const KUInt16* theName = GetName();
	OSErr theErr = ::FSMakeFSRefUnicode(
					&mRef,
					UUTF16CStr::StrLen(theName),
					theName,
					kTextEncodingUnknown,
					&mRef );

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * IsOnDesktop( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacCarbonFile::IsOnDesktop( void ) const
{
	return mIsOnDesktop;
}

// ------------------------------------------------------------------------- //
//  * GetVRefNum( void ) const
// ------------------------------------------------------------------------- //
short
TDCLMacCarbonFile::GetVRefNum( void ) const
{
	return mVRefNum;
}

// ------------------------------------------------------------------------- //
//  * MakeParentFolder( void ) const
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonFile::MakeParentFolder( void ) const
{
	return DoMakeParentFolder(
					(TDCLMacFiles*) GetFilesIntf(), &mIsOnDesktop, &mRef );
}

// ===================================================================== //
// A budget is just a method of worrying before you spend money, as well //
// as afterward.                                                         //
// ===================================================================== //