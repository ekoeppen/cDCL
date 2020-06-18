// ==============================
// Fichier:			TDCLPOSIXFile.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			22/6/2003
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
// The Original Code is TDCLPOSIXFile.cp.
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
// $Id: TDCLPOSIXFile.cp,v 1.10 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>

// C89, POSIX & X/Open Portability Guide Issue 4.2
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXDir.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>

// -------------------------------------------------------------------------- //
//  * TDCLPOSIXFile( TDCLPOSIXFiles*, const char* )
// -------------------------------------------------------------------------- //
TDCLPOSIXFile::TDCLPOSIXFile(
					TDCLPOSIXFiles* inFilesIntf,
					const char* inPath )
	:
		TDCLFile( inFilesIntf, DoMakeParentFolder( inFilesIntf, inPath ) ),
		mFile( nil ),
		mPath( nil ),
		mReadOnly( false )
{
	// Copie du chemin.
	size_t thePathLength = ::strlen( inPath ) + 1;
	mPath = (const char*) ::malloc( thePathLength );
	if (mPath == nil)
	{
		throw DCLMemError;
	}
	(void) ::memcpy( (char*) mPath, inPath, thePathLength );
}

// -------------------------------------------------------------------------- //
//  * TDCLPOSIXFile( TDCLPOSIXFiles*, TDCLFSItemRef, const char* )
// -------------------------------------------------------------------------- //
TDCLPOSIXFile::TDCLPOSIXFile(
					TDCLPOSIXFiles* inFilesIntf,
					TDCLFSItemRef inParentFolder,
					const char* inPath )
	:
		TDCLFile( inFilesIntf, inParentFolder ),
		mFile( nil ),
		mPath( nil ),
		mReadOnly( false )
{
	// Copie du chemin.
	size_t thePathLength = ::strlen( inPath ) + 1;
	mPath = (const char*) ::malloc( thePathLength );
	if (mPath == nil)
	{
		throw DCLMemError;
	}
	(void) ::memcpy( (char*) mPath, inPath, thePathLength );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPOSIXFile( void )
// -------------------------------------------------------------------------- //
TDCLPOSIXFile::~TDCLPOSIXFile( void )
{
	// On ferme le fichier.
	Close();

	if (mPath)
	{
		::free( (void*) mPath );
	}
}

// -------------------------------------------------------------------------- //
//  * Open( Boolean )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Open( Boolean inReadOnly )
{
	// On s'arrête si le fichier est déjà ouvert.
	if (mFile)
	{
		throw DCLBadStateError;
	}

	mFile = ::fopen( mPath, inReadOnly ? "r" : "r+" );
	mReadOnly = inReadOnly;
	
	if (mFile == nil)
	{
		// Une erreur est survenue.
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
				
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Create( void )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Create( void )
{
	// Vérifions que le fichier n'est pas ouvert.
	if (mFile)
	{
		throw DCLBadStateError;
	}

	mFile = ::fopen( mPath, "r+" );
	mReadOnly = false;
	
	if (mFile != nil)
	{
		throw DCLPlatformExistsAlready( errno );
	}
	
	// Création du fichier.
	mFile = ::fopen( mPath, "w" );

	if (mFile == nil)
	{
		// Une erreur est survenue.
		int theErr = errno;
		
		switch (theErr)
		{
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	
	Close();
}

// -------------------------------------------------------------------------- //
//  * Delete( void )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Delete( void )
{
	// Vérifions que le fichier n'est pas ouvert.
	if (mFile)
	{
		throw DCLBadStateError;
	}
	
	if (::unlink( mPath ))
	{
		// Une erreur est survenue.
		int theErr = errno;
		
		switch (theErr)
		{
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * IsOpen( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPOSIXFile::IsOpen( void ) const
{
	return mFile != nil;
}

// -------------------------------------------------------------------------- //
//  * IsReadOnly( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPOSIXFile::IsReadOnly( void ) const
{
	return (mFile != nil) && mReadOnly;
}

// -------------------------------------------------------------------------- //
//  * SetCursor( KSInt64, ECursorMode )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::SetCursor( KSInt64 inPos, ECursorMode inMode )
{
	// Vérifions que le fichier est ouvert.
	if (!mFile)
	{
		throw DCLBadStateError;
	}

	long whence = SEEK_CUR;	// Avoid warning by gcc.
	switch (inMode)
	{
		case kFromStart:
			whence = SEEK_SET;
			break;

		case kFromLEOF:
			whence = SEEK_END;
			break;
		
		case kFromCursor:
			whence = SEEK_CUR;
	}
	
	if (::fseeko( mFile, inPos, whence ))
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case EBADF:
			case EINVAL:
			case EOVERFLOW:
				throw DCLPlatformPosition( theErr );
			
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * GetCursor( void )
// ------------------------------------------------------------------------- //
KSInt64
TDCLPOSIXFile::GetCursor( void )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	off_t thePosition = ::ftello( mFile );

	if (thePosition == -1)
	{
		int theErr = errno;
		
		throw DCLPlatformUnknownError( theErr );
	}

	return thePosition;
} 

// -------------------------------------------------------------------------- //
//  * GetLength( void )
// -------------------------------------------------------------------------- //
KUInt64
TDCLPOSIXFile::GetLength( void )
{
	struct stat theFileInfos;
	if (::stat( mPath, &theFileInfos ))
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
			
			case EACCES:
				throw DCLPlatformPermission( theErr );
			
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	
	return (KUInt64) theFileInfos.st_size;
}

// -------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Read( void* outBuffer, KUInt32* ioCount )
{
	// Vérifions que le fichier est ouvert.
	if (!mFile)
	{
		throw DCLBadStateError;
	}
	
	size_t toRead = *ioCount;
	size_t read = ::fread( outBuffer, 1, toRead, mFile );
	if (read != toRead)
	{
		*ioCount = read;
		if (!(feof(mFile)))
		{
			// Une erreur est survenue.
			int theErr = ferror(mFile);
			clearerr( mFile );
			throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Write( const void* outBuffer, KUInt32* ioCount )
{
	// Vérifions que le fichier est ouvert.
	if (!mFile)
	{
		throw DCLBadStateError;
	}
	
	// Vérifions que le fichier est ouvert en lecture/écriture.
	if (mReadOnly)
	{
		throw DCLReadOnly;
	}

	size_t toWrite = *ioCount;
	size_t written = ::fwrite( outBuffer, 1, toWrite, mFile );
	if (written != toWrite)
	{
		*ioCount = written;

		// Une erreur est survenue.
		int theErr = ferror(mFile);
		clearerr( mFile );
		throw DCLPlatformUnknownError( theErr );
	}
}

// -------------------------------------------------------------------------- //
//  * FlushOutput( void )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::FlushOutput( void )
{
	if (!mFile)
	{
		throw DCLBadStateError;
	}
	
	// Vérifions que le fichier est ouvert en lecture/écriture.
	if (mReadOnly)
	{
		throw DCLReadOnly;
	}
	
	if (::fflush( mFile ))
	{
		int theErr = errno;
		throw DCLPlatformUnknownError( theErr );
	}
}

// -------------------------------------------------------------------------- //
//  * Close( void )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXFile::Close( void )
{
	if (mFile)
	{
		(void) ::fclose( mFile );
		mFile = nil;
	}
}

// ------------------------------------------------------------------------- //
//  * GetKind( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLPOSIXFile::GetKind( void ) const
{
	return TDCLNSRef::MakeString( "Some POSIX file" );
}

// ------------------------------------------------------------------------- //
//  * GetCreationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLPOSIXFile::GetCreationDate( void ) const
{
	struct stat theFileInfos;
	if (::stat( mPath, &theFileInfos ))
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
			
			case EACCES:
				throw DCLPlatformPermission( theErr );
			
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}

	// Traduction au format de date Newton.
	return (theFileInfos.st_ctime / 60)
		+ TDCLPOSIXFiles::kMinutesBetween1904and1970;
}

// ------------------------------------------------------------------------- //
//  * GetModificationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLPOSIXFile::GetModificationDate( void ) const
{
	struct stat theFileInfos;
	if (::stat( mPath, &theFileInfos ))
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
			
			case EACCES:
				throw DCLPlatformPermission( theErr );
			
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}

	// Traduction au format de date Newton.
	return (theFileInfos.st_mtime / 60)
		+ TDCLPOSIXFiles::kMinutesBetween1904and1970;
}

// ------------------------------------------------------------------------- //
//  * GetStringPath( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLPOSIXFile::GetStringPath( void ) const
{
	// Conversion en chaîne NewtonScript.
	return TDCLNSRef::MakeString( mPath );
}

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLPOSIXFile::MakeName( void ) const
{
	// Copie du chemin (pour POSIX, basename ne prend pas forcément un
	// const char*).
	size_t theLength = ::strlen( mPath ) + 1;
	char* thePath = (char*) ::malloc( theLength );
	(void) ::memcpy( thePath, mPath, theLength );

	char* theBaseName = ::basename( thePath );
	
	// Création de la chaîne UTF-16.
	theLength = ::strlen( theBaseName ) + 1;
	KUInt16* theResult =
		(KUInt16*) ::malloc( theLength * sizeof( KUInt16 ) );
	// En fait, je n'ai aucune idée de l'encodage du nom.
	UUTF16CStr::FromISO88591( (const KUInt8*) theBaseName, theResult, theLength );
		
	::free( thePath );
	
	return theResult;	
}

// ------------------------------------------------------------------------- //
//  * DoMakeParentFolder( TDCLPOSIXFiles*, const char* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXFile::DoMakeParentFolder(
					TDCLPOSIXFiles* inFilesIntf,
					const char* inPath )
{
	// Copie du chemin (pour POSIX, dirname ne prend pas forcément un
	// const char*).
	size_t theLength = ::strlen( inPath ) + 1;
	char* thePath = (char*) ::malloc( theLength );
	(void) ::memcpy( thePath, inPath, theLength );
	
	TDCLFSItemRef theResult = TDCLFSItemRef(
				new TDCLPOSIXDir(
						inFilesIntf,
						TDCLFSItemRef(),
						::dirname( thePath ) ) );

	::free( thePath );
	
	return theResult;	
}

// ------------------------------------------------------------------------- //
//  * MakeParentFolder( void ) const
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXFile::MakeParentFolder( void ) const
{
	return DoMakeParentFolder( (TDCLPOSIXFiles*) GetFilesIntf(), mPath );
}

// ====================================================================== //
// Almost anything derogatory you could say about today's software design //
// would be accurate.                                                     //
//                 -- K.E. Iverson                                        //
// ====================================================================== //
