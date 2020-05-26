// ==============================
// Fichier:			TDCLPackage.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			29/10/2003
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
// The Original Code is TDCLPackage.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 1999-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPackage.cp,v 1.19 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPackage.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// K
#include <K/Tests/KDebug.h>
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLPackageException.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Interfaces/UDCLTime.h>
#include <DCL/Package/TDCLPkgNOSPart.h>
#include <DCL/Package/TDCLPkgPart.h>
#include <DCL/Streams/TDCLStream.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>
#include <DCL/Streams/TDCLMemStream.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //
const KUInt8 TDCLPackage::kNOS1CompatibleSignature[8]
								= { 'p', 'a', 'c', 'k', 'a', 'g', 'e', '0' };
const KUInt8 TDCLPackage::kNOS1IncompatibleSignature[8]
								= { 'p', 'a', 'c', 'k', 'a', 'g', 'e', '1' };
const char* const TDCLPackage::kDefaultHandlerInfoData =
		"Desktop Connection Library::DCL/Package/TDCLPackage::$Revision: 1.19 $";

// -------------------------------------------------------------------------- //
//  * TDCLPackage( void )
// -------------------------------------------------------------------------- //
TDCLPackage::TDCLPackage( void )
	:
		mNOS1Compatible( true ),
		mPackageID( kDefaultID ),
		mPackageFlags( 0 ),
		mVersion( 0 ),
		mCreationDate( UDCLTime::TimeInSecondsSince1904() ),
		mCopyrightStr( nil ),
		mNameStr( nil ),
		mNumParts( 0 ),
		mParts( nil )
{
	mParts = (SPartData*) ::malloc( 0 );
	
	// Allocationd du nom.
	mNameStr = (KUInt16*) ::malloc( sizeof( KUInt16 ) );
	*mNameStr = 0;

	// Allocationd de la chaîne de copyright.
	mCopyrightStr = (KUInt16*) ::malloc( sizeof( KUInt16 ) );
	*mCopyrightStr = 0;
}

// -------------------------------------------------------------------------- //
//  * TDCLPackage( TDCLFile* )
// -------------------------------------------------------------------------- //
TDCLPackage::TDCLPackage( TDCLFile* inFile )
	:
		mNOS1Compatible( true ),
		mPackageID( kDefaultID ),
		mPackageFlags( 0 ),
		mVersion( 0 ),
		mCreationDate( 0 ),
		mCopyrightStr( nil ),
		mNameStr( nil ),
		mNumParts( 0 ),
		mParts( nil )
{
	Boolean wasOpen = false;
	KSInt64 theOldPosition = 0;
	if (inFile->IsOpen())
	{
		theOldPosition = inFile->GetCursor();
		inFile->SetCursor( 0, TDCLFile::kFromStart );
		wasOpen = true;
	} else {
		inFile->Open( true /* read only */ );
	}

	try {
		// Lecture.
		ReadPackage( inFile );
	} catch (...) {
		// Fermeture ou restauration.
		if (wasOpen)
		{
			inFile->SetCursor( theOldPosition, TDCLFile::kFromStart );
		} else {
			inFile->Close();
		}

		throw;	// On relance.
	}

	// Fermeture ou restauration.
	if (wasOpen)
	{
		inFile->SetCursor( theOldPosition, TDCLFile::kFromStart );
	} else {
		inFile->Close();
	}
}

// -------------------------------------------------------------------------- //
//  * TDCLPackage( TDCLStream* )
// -------------------------------------------------------------------------- //
TDCLPackage::TDCLPackage( TDCLStream* inStream )
	:
		mNOS1Compatible( true ),
		mPackageID( kDefaultID ),
		mPackageFlags( 0 ),
		mVersion( 0 ),
		mCreationDate( 0 ),
		mCopyrightStr( nil ),
		mNameStr( nil ),
		mNumParts( 0 ),
		mParts( nil )
{
	// Lecture.
	ReadPackage( inStream );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPackage( void )
// -------------------------------------------------------------------------- //
TDCLPackage::~TDCLPackage( void )
{
	if (mCopyrightStr)
	{
		::free( mCopyrightStr );
	}
	if (mNameStr)
	{
		::free( mNameStr );
	}

	if (mParts)
	{
		KUInt32 indexParts;

		// Suppression de toutes les parties.
		for (indexParts = 0; indexParts < mNumParts; indexParts++)
		{
			if (mParts[indexParts].fInfoData)
			{
				::free( mParts[indexParts].fInfoData );
			}
			if (mParts[indexParts].fPart)
			{
				delete mParts[indexParts].fPart;
			}
		}
		::free( mParts );
	}
}

// -------------------------------------------------------------------------- //
//  * WriteToStream( TDCLRandomAccessStream* ) const
// -------------------------------------------------------------------------- //
void
TDCLPackage::WriteToStream( TDCLRandomAccessStream* inOutStream ) const
{
	// On écrit les informations suivantes:
	// - le catalogue (partie fixe) (SPackageDirectory)
	// - la chaîne Copyright.
	// - la chaîne du nom.
	// - les différentes parties.
	// - les données pour le gestionnaire pour chaque partie.
	// - les parties.
	
	// Calcul de la taille.
	KUInt32 theCopyrightStrLen =
				(UUTF16CStr::StrLen( mCopyrightStr ) + 1) * sizeof(KUInt16);
	KUInt32 theNameStrLen =
				(UUTF16CStr::StrLen( mNameStr ) + 1) * sizeof(KUInt16);
	KUInt32 theDirectorySize =
				sizeof(SPackageDirectory)
				+ theCopyrightStrLen
				+ theNameStrLen
				+ (sizeof(SPartEntry) * mNumParts);
	// Ajout des tailles des informations et des parties.
	KUInt32 indexParts;
	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		// Informations.
		theDirectorySize += mParts[indexParts].fInfoSize;
	}
	
	KUInt32 directoryAlignment = 4 - (theDirectorySize % 4);
	if (directoryAlignment != 4)
	{
		theDirectorySize += directoryAlignment;
	} else {
		directoryAlignment = 0;
	}

	KUInt32 theGlobalSize = theDirectorySize;
    KUInt32 relocationPageCount = 0;
	KUInt32 allRelocationCount = 0;
	KUInt32 theRelocationSize = 0;

	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		// Données.
		TDCLPkgPart* thePart = mParts[indexParts].fPart;
		KASSERT( thePart != nil );
		relocationPageCount += thePart->GetRelocationPageCountEstimate();
		allRelocationCount += thePart->GetRelocationCount();
	    KUInt32 relocationSetMaxSize = sizeof(KUInt16) * 2;
	    if (allRelocationCount < 256) {
	        relocationSetMaxSize += allRelocationCount;
	        if (relocationSetMaxSize & 0x3)
	        {
	            relocationSetMaxSize += 4 - (relocationSetMaxSize & 0x3);
	        }
	    } else {
	        relocationSetMaxSize += 256;
	    }
	    KUInt32 relocationHeaderSize = 5 * sizeof(KUInt32);
	    theRelocationSize = relocationHeaderSize + relocationPageCount * relocationSetMaxSize;
    }

    theGlobalSize += theRelocationSize;

	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		// Données.
		TDCLPkgPart* thePart = mParts[indexParts].fPart;
		KASSERT( thePart != nil );
		theGlobalSize += thePart->GetSize( theGlobalSize );
		KUInt32 remain = 4 - (theGlobalSize % 4);
		if (remain != 4)
		{
			theGlobalSize += remain;
		}
	}

    KUInt32 packageFlags = mPackageFlags;
    bool nos1Compatible = mNOS1Compatible;

    if (relocationPageCount > 0 && !(mPackageFlags & kDirRelocationFlag))
    {
        packageFlags |= kDirRelocationFlag;
        nos1Compatible = false;
    }

	// On écrit désormais le paquet.
	KUInt32 writeCount = 8;
	if (nos1Compatible)
	{
		inOutStream->Write( kNOS1CompatibleSignature, &writeCount );
	} else {
		inOutStream->Write( kNOS1IncompatibleSignature, &writeCount );
	}

	inOutStream->PutLong( mPackageID );
	inOutStream->PutLong( packageFlags );
	inOutStream->PutLong( mVersion );
	inOutStream->PutShort( 0 );
	inOutStream->PutShort( (KUInt16) theCopyrightStrLen );
	inOutStream->PutShort( (KUInt16) theCopyrightStrLen );
	inOutStream->PutShort( (KUInt16) theNameStrLen );
	inOutStream->PutLong( theGlobalSize );
	inOutStream->PutLong( mCreationDate );
	inOutStream->PutLong( 0 );
	inOutStream->PutLong( 0 );
	inOutStream->PutLong( theDirectorySize );
	inOutStream->PutLong( mNumParts );
	
	// Ensuite, les parties.
	KUInt32 theInfoOffset = theCopyrightStrLen + theNameStrLen;
	
	KUInt32 theOffset = 0;
	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		inOutStream->PutLong( theOffset );

		// Données.
		TDCLPkgPart* thePart = mParts[indexParts].fPart;
		KUInt32 theSize = thePart->GetSize( theOffset + theDirectorySize + theRelocationSize );
		theOffset += theSize;
		KUInt32 remain = 4 - (theOffset % 4);
		if (remain != 4)
		{
			theOffset += remain;
		}
		inOutStream->PutLong( theSize );
		inOutStream->PutLong( theSize );
		inOutStream->PutLong( mParts[indexParts].fType );
		inOutStream->PutLong( 0 );
		inOutStream->PutLong( mParts[indexParts].fFlags );
		inOutStream->PutShort( (KUInt16) theInfoOffset );
		KUInt32 theInfoSize = mParts[indexParts].fInfoSize;
		inOutStream->PutShort( (KUInt16) theInfoSize );
		theInfoOffset += theInfoSize;
		inOutStream->PutLong( 0 );
	}
	
	// Ensuite, les données du catalogue.
	if (theCopyrightStrLen)
	{
		writeCount = theCopyrightStrLen;
		inOutStream->Write( mCopyrightStr, &writeCount );
	}
	writeCount = theNameStrLen;
	inOutStream->Write( mNameStr, &writeCount );
	
	// Données des parties.
	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		writeCount = mParts[indexParts].fInfoSize;
		inOutStream->Write( mParts[indexParts].fInfoData, &writeCount );
	}
	
	// Alignement.
	for ( ; directoryAlignment > 0; directoryAlignment-- )
	{
		inOutStream->PutByte( 0 );
	}

	// Write package relocation.
	KSInt64 numEntriesCursor = 0;
	KUInt32* allRelocations = new KUInt32 [ allRelocationCount ];
	KUInt32 allRelocationIx = 0;
	
	if (packageFlags & kDirRelocationFlag)
	{
	    KUInt32 relocationSetMaxSize = sizeof(KUInt16) * 2;
	    if (allRelocationCount < 256) {
	        relocationSetMaxSize += allRelocationCount;
	        if (relocationSetMaxSize & 0x3)
	        {
	            relocationSetMaxSize += 4 - (relocationSetMaxSize & 0x3);
	        }
	    } else {
	        relocationSetMaxSize += 256;
	    }
	    KUInt32 relocationHeaderSize = 5 * sizeof(KUInt32);
	    inOutStream->PutLong( 0 );  // reserved
	    inOutStream->PutLong( theRelocationSize );
	    inOutStream->PutLong( 1024 );   // pageSize
	    numEntriesCursor = inOutStream->GetCursor();
	    inOutStream->PutLong( 0 );      // numEntries, not set yet
	    inOutStream->PutLong( 0 );      // baseAddress
	    for (int i = 0; i < relocationPageCount; i++)
	    {
	        for (int j = 0; j < relocationSetMaxSize; j += sizeof(KUInt32))
	        {
	            inOutStream->PutLong( 0 );
	        }
	    }
	}

	// Enfin, les parties.
	theOffset = theDirectorySize + theRelocationSize;
	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		// Données.
		TDCLPkgPart* thePart = mParts[indexParts].fPart;
		writeCount = thePart->GetSize( theOffset );
		const void* theData = thePart->GetBuffer( theOffset );
		KUInt32 thePartRelocationCount = thePart->GetRelocationCount();
		if (thePartRelocationCount > 0)
		{
		    const KUInt32* thePartRelocations = thePart->GetRelocations();
		    for (int i = 0; i < thePartRelocationCount; i++)
		    {
		        allRelocations[allRelocationIx] = thePartRelocations[i] + theOffset;
		        allRelocationIx++;
		    }
		}
		theOffset += writeCount;
		
		inOutStream->Write( theData, &writeCount );
		
		KUInt32 remain = 4 - (theOffset % 4);
		if (remain != 4)
		{
			// Alignement.
			theOffset += remain;
			for ( ; remain > 0; remain-- )
			{
				inOutStream->PutByte( 0 );
			}
		}
	}

	// Retour sur les relocations.
	if (packageFlags & kDirRelocationFlag && allRelocationCount > 0)
	{
	    KSInt64 endCursor = inOutStream->GetCursor();
	    inOutStream->SetCursor(numEntriesCursor + 8, TDCLRandomAccessStream::kFromStart);
	    KUInt32 numSets = 1;
	    KUInt32 allRelocationIx = 0;
	    KUInt32 relocation = allRelocations[0];
	    KUInt16 currentPage = (KUInt16) (relocation >> 10);
	    inOutStream->PutShort(currentPage);
	    KUInt32 currentPageOffsetCount = 0;
	    KSInt64 currentPageOffsetCountCursor = inOutStream->GetCursor();
	    inOutStream->PutShort(0);
	    while (allRelocationIx < allRelocationCount)
	    {
	        allRelocationIx++;

	        KUInt8 offset = (KUInt8) ((relocation & 0x3FF) >> 2);
	        inOutStream->PutByte(offset);
	        currentPageOffsetCount++;

	        if (allRelocationIx < allRelocationCount)
	        {
                relocation = allRelocations[allRelocationIx];
                KUInt16 nextRelocPage = (KUInt16) (relocation >> 10);
                if (nextRelocPage != currentPage)
                {
                    // Pad page and write the number of entries
                    if (currentPageOffsetCount & 0x3)
                    {
                        for (int i = 0; i < 4 - (currentPageOffsetCount & 0x3); i++)
                        {
                            inOutStream->PutByte(0);
                        }
                    }
                    inOutStream->PutShort(nextRelocPage);
                    KSInt64 nextPageOffsetCountCursor = inOutStream->GetCursor();
                    inOutStream->SetCursor(currentPageOffsetCountCursor, TDCLRandomAccessStream::kFromStart);
                    inOutStream->PutShort(currentPageOffsetCount);
                    inOutStream->SetCursor(nextPageOffsetCountCursor, TDCLRandomAccessStream::kFromStart);
                    inOutStream->PutShort(0);
                    currentPageOffsetCountCursor = nextPageOffsetCountCursor;
                    currentPageOffsetCount = 0;
                    currentPage = nextRelocPage;
                    numSets++;
                }
            } else {
                inOutStream->SetCursor(currentPageOffsetCountCursor, TDCLRandomAccessStream::kFromStart);
                inOutStream->PutShort(currentPageOffsetCount);
	        }
	    }

	    inOutStream->SetCursor(numEntriesCursor, TDCLRandomAccessStream::kFromStart);
	    inOutStream->PutLong(numSets);
	    inOutStream->SetCursor(endCursor, TDCLRandomAccessStream::kFromStart);
	}

	delete [] allRelocations;
}

// -------------------------------------------------------------------------- //
//  * WriteToStream( TDCLStream* ) const
// -------------------------------------------------------------------------- //
void
TDCLPackage::WriteToStream( TDCLStream* inOutStream ) const
{
    TDCLMemStream memStream;
    WriteToStream(&memStream);
    KUInt32 size = (KUInt32) memStream.GetCursor();
    inOutStream->Write(memStream.GetBuffer(), &size);
}

// -------------------------------------------------------------------------- //
//  * IsPackage( TDCLFile* )
// -------------------------------------------------------------------------- //
Boolean
TDCLPackage::IsPackage( TDCLFile* inFile )
{
	Boolean theResult = false;
	
	Boolean wasOpen = false;
	KSInt64 theOldPosition = 0;
	if (inFile->IsOpen())
	{
		theOldPosition = inFile->GetCursor();
		inFile->SetCursor( 0, TDCLFile::kFromStart );
		wasOpen = true;
	} else {
		inFile->Open( true /* read only */ );
	}
	
	try {
		// Lecture des 8 premiers octets (si possible).
		KUInt8 thePreamble[8];
		KUInt32 nbRead = 8;
		inFile->Read( thePreamble, &nbRead );
		
		if (nbRead == 8)
		{
			// Test que ce sont bien package0 ou package1.
			theResult =
					(::memcmp(
						thePreamble,
						kNOS1CompatibleSignature,
						8 ) == 0)
				||	(::memcmp(
						thePreamble,
						kNOS1IncompatibleSignature,
						8 ) == 0);
		}
	} catch ( ... ) {
		// Fermeture ou restauration.
		if (wasOpen)
		{
			inFile->SetCursor( theOldPosition, TDCLFile::kFromStart );
		} else {
			inFile->Close();
		}

		throw;
	}

	// Fermeture ou restauration.
	if (wasOpen)
	{
		inFile->SetCursor( theOldPosition, TDCLFile::kFromStart );
	} else {
		inFile->Close();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * ReadPackage( TDCLStream* )
// -------------------------------------------------------------------------- //
void
TDCLPackage::ReadPackage( TDCLStream* inStream )
{
	KASSERT(mParts == nil);
	KASSERT(mCopyrightStr == nil);
	KASSERT(mNameStr == nil);
	
	// Lecture de la signature.
	KUInt8 theSignature[8];
	KUInt32 nbRead = 8;
	
	inStream->Read( theSignature, &nbRead );
	
	if (nbRead != 8)
	{
		throw DCLEOF;
	}
	
	if (::memcmp( theSignature, kNOS1CompatibleSignature, 8 ) == 0)
	{
		mNOS1Compatible = true;
	} else if (::memcmp( theSignature, kNOS1IncompatibleSignature, 8 ) == 0) {
		mNOS1Compatible = false;
	} else {
		throw DCLPackage( kPkgErrUnknownSignature );
	}
	
	// Lecture de la suite.
	mPackageID = inStream->GetLong();
	mPackageFlags = inStream->GetLong();
	mVersion = inStream->GetLong();
	KUInt16 copyrightOffset = inStream->GetShort();
	KUInt16 copyrightLength = inStream->GetShort();
	KUInt16 nameOffset = inStream->GetShort();
	KUInt16 nameLength = inStream->GetShort();
	
	// Les chaînes sont des chaînes de mots de 16 bits.
	if (copyrightLength & 0x1)
	{
		throw DCLPackage( kPkgStringsDontSeemUniCStr );
	}
	if (nameLength & 0x1)
	{
		throw DCLPackage( kPkgStringsDontSeemUniCStr );
	}

	KUInt32 thePackageSize = inStream->GetLong();
	mCreationDate = inStream->GetLong();
	(void) inStream->GetLong();	// fReserved1
	(void) inStream->GetLong();	// fReserved2
	KUInt32 theDirectorySize = inStream->GetLong();
	mNumParts = inStream->GetLong();
	
	// Lecture des entrées sur les parties.
	mParts = (SPartData*) ::malloc( mNumParts * sizeof( SPartData ) );
	SPartLocation* thePartsLocations =
		(SPartLocation*) ::malloc( mNumParts * sizeof( SPartLocation ) );

	// Protection contre les exceptions lors de la lecture.
	KUInt32 indexParts;
	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
		mParts[indexParts].fInfoData = nil;
		mParts[indexParts].fPart = nil;
	}
	
	KUInt32 theDirectoryInfoSize = sizeof( SPackageDirectory )
				+ (sizeof(SPartEntry) * mNumParts);
	if (theDirectorySize <= theDirectoryInfoSize)
	{
		throw DCLPackage( kPkgErrWeirdDirectorySize );
	}

	for (indexParts = 0; indexParts < mNumParts; indexParts++)
	{
	    // fDataOffset is the offset from the beginning of part data
	    // (after directory and relocation)
		thePartsLocations[indexParts].fDataOffset = inStream->GetLong();
		thePartsLocations[indexParts].fDataSize = inStream->GetLong();
		(void) inStream->GetLong(); // fSize2
		mParts[indexParts].fType = inStream->GetLong();
		(void) inStream->GetLong(); // fReserved1
		mParts[indexParts].fFlags = inStream->GetLong();
		thePartsLocations[indexParts].fInfoOffset = inStream->GetShort() + theDirectoryInfoSize;
		mParts[indexParts].fInfoSize = inStream->GetShort();
		(void) inStream->GetLong(); // fReserved2
	}

	// Ensuite, les données du catalogue.
	KUInt32 theDirectoryDataSize = theDirectorySize - theDirectoryInfoSize;
	
	// Création d'une zone de mémoire.
	KUInt8* theDirectoryData =
					(KUInt8*) ::malloc( (size_t) theDirectoryDataSize );
	
	// Lecture.
	try {
		nbRead = theDirectoryDataSize;
		inStream->Read( theDirectoryData, &nbRead );
	
		if (nbRead != theDirectoryDataSize)
		{
			throw DCLEOF;
		}
		
		// Copie des différentes données.
		if (copyrightLength > 0)
		{
			if (((KUInt32) (copyrightLength + copyrightOffset))
											> theDirectoryDataSize)
			{
				throw DCLPackage( kPkgErrWeirdDirectorySize );
			}
			
			mCopyrightStr = (KUInt16*) ::malloc( copyrightLength );
			(void) ::memcpy(
						mCopyrightStr,
						(const void*) &theDirectoryData[copyrightOffset],
						copyrightLength );
			
			if (mCopyrightStr[(copyrightLength / 2) - 1] != 0)
			{
				throw DCLPackage( kPkgStringsDontSeemUniCStr );
			}
		} else {
			mCopyrightStr = (KUInt16*) ::malloc( sizeof(KUInt16) );
			*mCopyrightStr = 0;
		}
		
		if (nameLength == 0)
		{
			throw DCLPackage( kPkgErrNoName );
		}

		if (((KUInt32) (nameLength + nameOffset)) > theDirectoryDataSize)
		{
			throw DCLPackage( kPkgErrWeirdDirectorySize );
		}
		
		mNameStr = (KUInt16*) ::malloc( nameLength );
		(void) ::memcpy(
					mNameStr,
					(const void*) &theDirectoryData[nameOffset],
					nameLength );

		if (mNameStr[(nameLength / 2) - 1] != 0)
		{
			throw DCLPackage( kPkgStringsDontSeemUniCStr );
		}
		
		// Puis les informations pour les gestionnaires de parties.
		for (indexParts = 0; indexParts < mNumParts; indexParts++)
		{
			KUInt32 theOffset =
				thePartsLocations[indexParts].fInfoOffset
					- theDirectoryInfoSize;
			KUInt32 theSize = mParts[indexParts].fInfoSize;
			
			if (theOffset + theSize > theDirectoryDataSize)
			{
				throw DCLPackage( kPkgErrWeirdDirectorySize );
			}
			
			// Allocation.
			KUInt8* thePointer = (KUInt8*) ::malloc( theSize );
			mParts[indexParts].fInfoData = thePointer;
			
			// Copie.
			(void) ::memcpy(
					thePointer,
					(const void*) &theDirectoryData[theOffset],
					theSize );
		}
	} catch (...) {
		// Ménage.
		::free( thePartsLocations );
		::free( theDirectoryData );
		
		// Relance.
		throw;
	}
	
	::free( theDirectoryData );

	KUInt32 theRelocationSize = 0;
	if (mPackageFlags & kDirRelocationFlag)
	{
        // Read the whole thing with GetLong & co to make sure we convert from
        // bigendian.
        KUInt32 readRelocationSize = 0;
		KUInt32 reservedWord = inStream->GetLong();
		readRelocationSize += 4;
		theRelocationSize = inStream->GetLong();
		readRelocationSize += 4;
        KUInt32 pageSize = inStream->GetLong();
		readRelocationSize += 4;
        KUInt32 numEntries = inStream->GetLong();
		readRelocationSize += 4;
        KUInt32 baseAddress = inStream->GetLong();
		readRelocationSize += 4;

		KUInt32 hostRelocationSize = sizeof(SPackageRelocationData) + sizeof(SPackageRelocationSet) * numEntries;
		mRelocationData = (SPackageRelocationData*) ::malloc( hostRelocationSize );
        mRelocationData->fReserved = reservedWord;
        mRelocationData->fRelocationSize = theRelocationSize;
        mRelocationData->fPageSize = pageSize;
        mRelocationData->fNumEntries = numEntries;
        mRelocationData->fBaseAddress = baseAddress;

        for (KUInt32 entriesIx = 0; entriesIx < numEntries; entriesIx++) {
            mRelocationData->fRelocationSets[entriesIx].fPageNumber = inStream->GetShort();
            readRelocationSize += 2;
            KUInt32 offsetCount = inStream->GetShort();
            readRelocationSize += 2;
            mRelocationData->fRelocationSets[entriesIx].fOffsetCount = offsetCount;
            inStream->Read( mRelocationData->fRelocationSets[entriesIx].fOffsets, &offsetCount );
            if (offsetCount != mRelocationData->fRelocationSets[entriesIx].fOffsetCount)
            {
                throw DCLEOF;
            }
            readRelocationSize += offsetCount;
            int paddingCount = 4 - (offsetCount % 4);
            if (paddingCount < 4) {
                for (int i = 0; i < paddingCount; i++)
                {
                    readRelocationSize++;
                    (void) inStream->GetByte();
                }
            }
        }
        for (; readRelocationSize < theRelocationSize; readRelocationSize++)
        {
            (void) inStream->GetByte();
        }
	}
	
	// Finalement, lecture des parties.
	KUInt32 thePartDataSize = thePackageSize - theDirectorySize - theRelocationSize;
	KUInt32 thePartDataOffset = theDirectorySize + theRelocationSize;
	KUInt8* thePartData = (KUInt8*) ::malloc( thePartDataSize );
	
	// Lecture.
	try {
		nbRead = thePartDataSize;
		inStream->Read( thePartData, &nbRead );
	
		if (nbRead != thePartDataSize)
		{
			throw DCLEOF;
		}

		// Copie des parties.
		for (indexParts = 0; indexParts < mNumParts; indexParts++)
		{
			KUInt32 theOffset = thePartsLocations[indexParts].fDataOffset;
			KUInt32 theSize = thePartsLocations[indexParts].fDataSize;
			
			if (theOffset + theSize > thePartDataSize)
			{
				throw DCLPackage( kPkgErrWeirdPartSize );
			}
			
			// Création des données.
			if ((mParts[indexParts].fFlags & kPartTypeMask)
					== kPartNOSPart)
			{
				mParts[indexParts].fPart =
					new TDCLPkgNOSPart(
						thePartDataOffset + theOffset,
						(const void*) &thePartData[theOffset],
						theSize );
			} else {
				mParts[indexParts].fPart =
					new TDCLPkgPart(
						thePartDataOffset + theOffset,
						(const void*) &thePartData[theOffset],
						theSize );
			}
		}
	} catch (...) {
		// Ménage.
		::free( thePartsLocations );
		::free( thePartData );
		
		// Relance.
		throw;
	}
	
	::free( thePartsLocations );
	::free( thePartData );
}

// -------------------------------------------------------------------------- //
//  * AddPart( KUInt32, KUInt32, TDCLPkgPart* )
// -------------------------------------------------------------------------- //
KUInt32
TDCLPackage::AddPart(
				KUInt32 inType,
				KUInt32 inFlags,
				TDCLPkgPart* inPart )
{
	size_t theDataLength = ::strlen( kDefaultHandlerInfoData );
	return AddPart(
				inType,
				inFlags,
				theDataLength,
				(const KUInt8*) kDefaultHandlerInfoData,
				inPart );
}

// -------------------------------------------------------------------------- //
//  * AddPart( KUInt32, KUInt32, KUInt32, const KUInt8*, TDCLPkgPart* )
// -------------------------------------------------------------------------- //
KUInt32
TDCLPackage::AddPart(
				KUInt32 inType,
				KUInt32 inFlags,
				KUInt32 inInfoSize,
				const KUInt8* inInfoData,
				TDCLPkgPart* inPart )
{
	// Agrandissement du tableau.
	KUInt32 indexPart = mNumParts;
	mNumParts++;
	mParts = (SPartData*) ::realloc( mParts, sizeof(SPartData) * mNumParts );
	
	// Ajout des données.
	mParts[indexPart].fType = inType;
	mParts[indexPart].fFlags = inFlags;
	mParts[indexPart].fInfoSize = inInfoSize;
	mParts[indexPart].fPart = inPart;
	
	// Copie.
	KUInt8* theInfoData = (KUInt8*) ::malloc( inInfoSize );
	(void) ::memcpy( theInfoData, inInfoData, inInfoSize );
	mParts[indexPart].fInfoData = theInfoData;
	
	return indexPart;
}

// -------------------------------------------------------------------------- //
//  * RemovePart( KUInt32 )
// -------------------------------------------------------------------------- //
void
TDCLPackage::RemovePart( KUInt32 inPartIndex )
{
	KASSERT( inPartIndex < mNumParts );

	// Suppression des données.
	
	// Ajout des données.
	::free( mParts[inPartIndex].fInfoData );
	delete mParts[inPartIndex].fPart;

	// Rétrécissement du tableau.
	(void) ::memmove(
				(void*) &mParts[inPartIndex],
				(const void*) &mParts[inPartIndex + 1],
				(mNumParts - inPartIndex - 1) * sizeof(SPartData) );
	
	mNumParts--;
	mParts = (SPartData*) ::realloc( mParts, sizeof(SPartData) * mNumParts );
}

// -------------------------------------------------------------------------- //
//  * SetCopyrightString( const KUInt16* )
// -------------------------------------------------------------------------- //
void
TDCLPackage::SetCopyrightString( const KUInt16* inCopyrightStr )
{
	size_t theCopyrightLength = UUTF16CStr::StrLen( inCopyrightStr );
	
	if (theCopyrightLength > kCopyrightMaxLength)
	{
		throw DCLBadParamError;
	}
	
	KUInt32 theSize = (theCopyrightLength + 1) * sizeof( KUInt16 );
		
	// Modification de la taille.
	mCopyrightStr = (KUInt16*) ::realloc( mCopyrightStr, theSize );
		
	// Copie.
	(void) ::memcpy( mCopyrightStr, inCopyrightStr, theSize );
}

// -------------------------------------------------------------------------- //
//  * SetPackageName( const KUInt16* )
// -------------------------------------------------------------------------- //
void
TDCLPackage::SetPackageName( const KUInt16* inNameStr )
{
	size_t theNameLength = UUTF16CStr::StrLen( inNameStr );
	
	if (theNameLength > kPackageNameMaxLength)
	{
		throw DCLBadParamError;
	}
	
	KUInt32 theSize = (theNameLength + 1) * sizeof( KUInt16 );
	
	// Modification de la taille.
	mNameStr = (KUInt16*) ::realloc( mNameStr, theSize );
	
	// Copie.
	(void) ::memcpy( mNameStr, inNameStr, theSize );
}

// -------------------------------------------------------------------------- //
//  * SetCopyrightString( const KUInt16* )
// -------------------------------------------------------------------------- //
void
TDCLPackage::SetCopyrightString( const char* inCopyrightStr )
{
	size_t theCopyrightLength = ::strlen( inCopyrightStr );
	
	if (theCopyrightLength > kCopyrightMaxLength)
	{
		throw DCLBadParamError;
	}
	
	KUInt32 theSize = (theCopyrightLength + 1) * sizeof( KUInt16 );

	// Modification de la taille.
	mCopyrightStr = (KUInt16*) ::realloc( mCopyrightStr, theSize );

	// Conversion
	UUTF16CStr::FromISO88591( (const KUInt8*) inCopyrightStr, mCopyrightStr );
}

// -------------------------------------------------------------------------- //
//  * SetPackageName( const char* )
// -------------------------------------------------------------------------- //
void
TDCLPackage::SetPackageName( const char* inNameStr )
{
	size_t theNameLength = ::strlen( inNameStr );
	
	if (theNameLength > kPackageNameMaxLength)
	{
		throw DCLBadParamError;
	}
	
	KUInt32 theSize = (theNameLength + 1) * sizeof( KUInt16 );

	// Modification de la taille.
	mNameStr = (KUInt16*) ::realloc( mNameStr, theSize );
	
	// Conversion
	UUTF16CStr::FromISO88591( (const KUInt8*) inNameStr, mNameStr );
}

// ========================================================= //
// Computers are not intelligent.  They only think they are. //
// ========================================================= //
