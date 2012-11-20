// ==============================
// Fichier:			TDCLPOSIXDirBase.cp
// Projet:			Desktop Connection Library
// 
// CrŽŽ le:			20/9/2003
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
// The Original Code is TDCLPOSIXDirBase.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPOSIXDirBase.cp,v 1.10 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXDirBase.h>

// SUSv2
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// ISO C++
#include <new>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXDir.h>

// -------------------------------------------------------------------------- //
//  * TDCLPOSIXDirBase( TDCLPOSIXFiles*, const char* )
// -------------------------------------------------------------------------- //
TDCLPOSIXDirBase::TDCLPOSIXDirBase(
				TDCLPOSIXFiles* inFilesIntf,
				TDCLFSItemRef inFolder,
				const char* inPath )
	:
		TDCLFolder( inFilesIntf, inFolder ),
		mDirRef( nil ),
		mPath( nil )
{
	// Copie du chemin.
	size_t thePathLength = ::strlen( inPath ) + 1;
	mPath = (const char*) ::malloc( thePathLength );
	if (mPath == nil)
	{
		throw DCLMemError;
	}
	(void) ::memcpy( (char*) mPath, inPath, thePathLength );
	
	// Ouverture du dossier.
	OpenDir();
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPOSIXDirBase( void )
// -------------------------------------------------------------------------- //
TDCLPOSIXDirBase::~TDCLPOSIXDirBase( void )
{
	if (mDirRef)
	{
		(void) ::closedir( mDirRef );
	}
	if (mPath)
	{
		::free( (void*) mPath );
	}
}

// -------------------------------------------------------------------------- //
//  * GetItemByName( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXDirBase::GetItemByName(
						const KUInt16* inName,
						KSInt32 /* inVolRefNum *//* = 0 */ )
{
	TDCLFSItem* theItem = nil;
	
	// Recherche de l'ŽlŽment.
	if (mDirRef)
	{
		// Conversion du nom en ISO-8859-1.
		size_t theNameLength = UUTF16CStr::StrLen( inName ) + 1;
		char* theName = (char*) ::malloc( theNameLength );
		UUTF16CStr::ToISO88591( inName, (KUInt8*) theName, theNameLength );
		theNameLength--;

		::rewinddir( mDirRef );
		do {
			dirent* theEntry = ::readdir( mDirRef );
			if (theEntry == nil)
			{
				break;
			}
			
			if ((::strcmp( theEntry->d_name, "." ) != 0)
				&& (::strcmp( theEntry->d_name, ".." ) != 0)
				&& (::strcmp( theEntry->d_name, theName ) == 0))
			{
				// Construction du chemin.
				size_t dirPathLength = ::strlen( mPath );
				char* thePath = (char*) ::malloc( dirPathLength + theNameLength + 2 );
				(void) ::memcpy( thePath, mPath, dirPathLength );
				thePath[dirPathLength] = '/';
				(void) ::memcpy( &thePath[dirPathLength + 1], theName, theNameLength + 1 );

				// Determinons le type de cet ŽlŽment, avec stat
				// (pour rŽsoudre le lien Žventuel, surtout d_type est une extension BSD).
				struct stat theFileInfos;
				int statResult;
				if (((TDCLPOSIXFiles*) GetFilesIntf())->GetFollowSymLinks())
				{
					statResult = ::stat( thePath, &theFileInfos );
				} else {
					statResult = ::lstat( thePath, &theFileInfos );
				}

				if (statResult)
				{
					int theErr = errno;
					
					switch (theErr)
					{
						case ENOENT:
							throw DCLPlatformDoesntExist( theErr );
						
						case EACCES:
							// En cas de problme de droits, on va dire que l'ŽlŽment
							// n'existe pas.
							break;
						
						default:
							throw DCLPlatformUnknownError( theErr );
					}
				} else {
					if (S_ISDIR(theFileInfos.st_mode))
					{
						theItem = new TDCLPOSIXDir(
										(TDCLPOSIXFiles*) GetFilesIntf(),
										TDCLFSItemRef( this ),
										thePath );
					} else if (S_ISREG(theFileInfos.st_mode)) {
						theItem = new TDCLPOSIXFile(
										(TDCLPOSIXFiles*) GetFilesIntf(),
										TDCLFSItemRef( this ),
										thePath );
					}
				}
				
				// LibŽration du chemin.
				::free( thePath );
				
				break;
			}
		} while ( true );

		// LibŽration du nom.
		::free( theName );
	}
		
	return TDCLFSItemRef( theItem );
}

// -------------------------------------------------------------------------- //
//  * CreateFile( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXDirBase::CreateFile( const KUInt16* inName )
{
	// Conversion du nom en ISO-8859-1.
	size_t theNameLength = UUTF16CStr::StrLen( inName ) + 1;
	char* theName = (char*) ::malloc( theNameLength );
	UUTF16CStr::ToISO88591( inName, (KUInt8*) theName, theNameLength );
	theNameLength--;
	
	// VŽrification qu'on ne crŽe pas de fichier .. ou .
	if ((::strcmp( theName, ".." ) == 0)
		|| (::strcmp( theName, "." ) == 0))
	{
		throw DCLExistsAlready;
	}

	// Construction du chemin.
	size_t dirPathLength = ::strlen( mPath );
	char* thePath = (char*) ::malloc( dirPathLength + theNameLength + 2 );
	(void) ::memcpy( thePath, mPath, dirPathLength );
	thePath[dirPathLength] = '/';
	(void) ::memcpy( &thePath[dirPathLength + 1], theName, theNameLength + 1 );

	// LibŽration du nom.
	::free( theName );

	TDCLFSItemRef theResult =
				TDCLFSItemRef( new TDCLPOSIXFile(
							(TDCLPOSIXFiles*) GetFilesIntf(),
							TDCLFSItemRef( this ),
							thePath ));
	
	// LibŽration du chemin.
	::free( thePath );

	((TDCLPOSIXFile*) ((TDCLFile*) theResult))->Create();
		
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateFolder( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLPOSIXDirBase::CreateFolder( const KUInt16 *inName )
{
	// Conversion du nom en ISO-8859-1.
	size_t theNameLength = UUTF16CStr::StrLen( inName ) + 1;
	char* theName = (char*) ::malloc( theNameLength );
	UUTF16CStr::ToISO88591( inName, (KUInt8*) theName, theNameLength );
	theNameLength--;

	// VŽrification qu'on ne crŽe pas de dossier .. ou .
	if ((::strcmp( theName, ".." ) == 0)
		|| (::strcmp( theName, "." ) == 0))
	{
		throw DCLExistsAlready;
	}
	
	// Construction du chemin.
	size_t dirPathLength = ::strlen( mPath );
	char* thePath = (char*) ::malloc( dirPathLength + theNameLength + 2 );
	(void) ::memcpy( thePath, mPath, dirPathLength );
	thePath[dirPathLength] = '/';
	(void) ::memcpy( &thePath[dirPathLength + 1], theName, theNameLength + 1 );

	// LibŽration du nom.
	::free( theName );

	// CrŽation du rŽpertoire.
	if (::mkdir( thePath, 0777 ))
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
	
	TDCLFSItemRef theResult =
				TDCLFSItemRef( new TDCLPOSIXDir(
							(TDCLPOSIXFiles*) GetFilesIntf(),
							TDCLFSItemRef( this ),
							thePath ));
	
	// LibŽration du chemin.
	::free( thePath );
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * OpenDir( void )
// -------------------------------------------------------------------------- //
void
TDCLPOSIXDirBase::OpenDir( void )
{
	// Ouverture du dossier.
	mDirRef = ::opendir( mPath );
	if (mDirRef == nil)
	{
		// Une erreur est survenue.
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
				
			case EACCES:
				// En cas de problme de droits, on va dire que le rŽpertoire
				// est vide.
				break;
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * CountElements( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLPOSIXDirBase::CountElements( void )
{
	KUInt32 nbElements = 0;

	// Si le rŽpertoire est protŽgŽ, on dit qu'il est vide.
	if (mDirRef)
	{
		::rewinddir( mDirRef );

		do
		{
			dirent* theEntry = ::readdir( mDirRef );
			if (theEntry == nil)
			{
				break;
			}
			
			if ((::strcmp( theEntry->d_name, "." ) != 0)
				&& (::strcmp( theEntry->d_name, ".." ) != 0))
			{
				nbElements++;
			}
		} while ( true );
	}
		
	return nbElements;
} 

// -------------------------------------------------------------------------- //
//  * MakeName( void ) const
// -------------------------------------------------------------------------- //
KUInt16*
TDCLPOSIXDirBase::MakeName( void ) const
{
	// Copie du chemin (pour POSIX, basename ne prend pas forcŽment un
	// const char*).
	size_t theLength = ::strlen( mPath ) + 1;
	char* thePath = (char*) ::malloc( theLength );
	(void) ::memcpy( thePath, mPath, theLength );

	char* theBaseName = ::basename( thePath );
	
	// CrŽation de la cha”ne UTF-16.
	theLength = ::strlen( theBaseName ) + 1;
	KUInt16* theResult =
		(KUInt16*) ::malloc( theLength * sizeof( KUInt16 ) );
	// En fait, je n'ai aucune idŽe de l'encodage du nom.
	UUTF16CStr::FromISO88591( (const KUInt8*) theBaseName, theResult, theLength );
		
	// LibŽration du chemin.
	::free( thePath );
	
	return theResult;	
}

// -------------------------------------------------------------------------- //
//  * GetItems( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLPOSIXDirBase::GetItems( void ) 
{
	KUInt32 nbItems = CountElements();
	
	TDCLFSItemRef* theResult =
		(TDCLFSItemRef*) ::malloc( sizeof( TDCLFSItemRef ) * (nbItems + 1)  );

	if (mDirRef)
	{
		KUInt32 indexItems = 0;
	
		::rewinddir( mDirRef );

		while (true)
		{
			dirent* theEntry = ::readdir( mDirRef );
			if ((theEntry == nil) || (indexItems == nbItems))
			{
				// C'est la fin.
				// Ajout du terminateur.
				new (&theResult[indexItems]) TDCLFSItemRef();
				break;
			}
			
			if ((::strcmp( theEntry->d_name, "." ) != 0)
				&& (::strcmp( theEntry->d_name, ".." ) != 0))
			{
				size_t theNameLength = ::strlen( theEntry->d_name );

				// Construction du chemin.
				size_t dirPathLength = ::strlen( mPath );
				char* thePath = (char*) ::malloc( dirPathLength + theNameLength + 2 );
				(void) ::memcpy( thePath, mPath, dirPathLength );
				thePath[dirPathLength] = '/';
				(void) ::memcpy( &thePath[dirPathLength + 1], theEntry->d_name, theNameLength + 1 );

				// Determinons le type de cet ŽlŽment, avec stat
				// (pour rŽsoudre le lien Žventuel, surtout d_type est une extension BSD).
				struct stat theFileInfos;
				int statResult;
				if (((TDCLPOSIXFiles*) GetFilesIntf())->GetFollowSymLinks())
				{
					statResult = ::stat( thePath, &theFileInfos );
				} else {
					statResult = ::lstat( thePath, &theFileInfos );
				}
				
				if (statResult != 0)
				{
					int theErr = errno;
					
					switch (theErr)
					{
						case ENOENT:
							throw DCLPlatformDoesntExist( theErr );
						
						case EACCES:
							// En cas de problme de droits, on n'ajoute pas l'ŽlŽment ˆ la liste.
							break;
						
						default:
							throw DCLPlatformUnknownError( theErr );
					}
				} else {
					TDCLFSItem* theItem = nil;

					if (S_ISDIR(theFileInfos.st_mode))
					{
						theItem = new TDCLPOSIXDir(
										(TDCLPOSIXFiles*) GetFilesIntf(),
										TDCLFSItemRef( this ),
										thePath );
					} else if (S_ISREG(theFileInfos.st_mode)) {
						theItem = new TDCLPOSIXFile(
										(TDCLPOSIXFiles*) GetFilesIntf(),
										TDCLFSItemRef( this ),
										thePath );
					}
					
					if (theItem)
					{
						// Ajout de l'ŽlŽment
						new (&theResult[indexItems]) TDCLFSItemRef( theItem );
					
						// ƒlŽment suivant.
						indexItems++;
					}
				} // if (statResult != 0)
				
				// LibŽration du chemin.
				::free( thePath );
			} // if ((::strcmp( theEntry->d_name ...
		} // while (true)
	} else {
		// Ajout du terminateur.
		new (&theResult[0]) TDCLFSItemRef();
	}

	return theResult;
}

// =================================================================== //
// Heuristics are bug ridden by definition.  If they didn't have bugs, //
// then they'd be algorithms.                                          //
// =================================================================== //
