// ==============================
// Fichier:			TDCLMacCarbonDesktopFolder.cp
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
// The Original Code is TDCLMacCarbonDesktopFolder.cp.
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
// $Id: TDCLMacCarbonDesktopFolder.cp,v 1.8 2004/11/24 18:11:23 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonDesktopFolder.h>

// ANSI C
#include <stdlib.h>

// ISO C++
#include <new>
#include <stdlib.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFile.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h>

// -------------------------------------------------------------------------- //
// TDCLMacCarbonDesktopFolder( TDCLMacFiles* )
// -------------------------------------------------------------------------- //
TDCLMacCarbonDesktopFolder::TDCLMacCarbonDesktopFolder(
												TDCLMacFiles* inFilesIntf )
	:
		TDCLFolder( inFilesIntf ),
		TDCLMacDesktopFolder( inFilesIntf )
{
	// Sommes-nous sur le bureau?
	OSErr theErr = ::FSFindFolder(
						kOnAppropriateDisk,
						kDesktopFolderType,
						false,
						&mDesktopFolderRef );

	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
}

// -------------------------------------------------------------------------- //
//  * GetItemByName( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonDesktopFolder::GetItemByName(
				const KUInt16* inName,
				KSInt32 inVolRefNum /* = 0 */ )
{
	// Sous OS X, inVolRefNum n'est utilisé que pour les volumes.
	
  	TDCLFSItem* theItem = nil;
	FSRef theItemRef;
	OSErr theErr;
	
	// On commence par tenter un volume.
	ItemCount theVolumeIndex = 1;
	do {
		FSRef theVolumeRef;
		HFSUniStr255 theVolumeName;
		FSVolumeRefNum theVolumeRefNum;
		theErr = ::FSGetVolumeInfo(
						0,
						theVolumeIndex,
						&theVolumeRefNum,
						kFSVolInfoNone,
						NULL,
						&theVolumeName,
						&theVolumeRef );
	
		if (theErr)
		{
			if (theErr == nsvErr)
			{
				break;
			} else {
				throw DCLPlatformUnknownError( theErr );
			}
		}
		
		// Est-ce mon volume?
		if (inVolRefNum == theVolumeRefNum)
		{
			KUInt32 theSize = theVolumeName.length;
			KUInt32 indexStr;
			Boolean strEqual = true;
			for (indexStr = 0; indexStr < theSize; indexStr++)
			{
				if (inName[indexStr] != theVolumeName.unicode[indexStr])
				{
					strEqual = false;
					break;
				}
				
				if (inName[indexStr] == 0)
				{
					strEqual = false;	// On ne sait jamais, la chaîne Pascal
										// peut avoir un caractère nul.
					break;
				}
			}
			
			if (strEqual && (inName[indexStr] == 0))
			{
				theItem =
					new TDCLMacCarbonFolder(
							(TDCLMacFiles*) GetFilesIntf(), &theVolumeRef );
				break;
			}
		}
				
		theVolumeIndex++;
	} while ( true );

	// Si c'est bon, on sort, sinon on tente sur le bureau.
	if (theItem == nil)
	{
		theErr = ::FSMakeFSRefUnicode(
								&mDesktopFolderRef,
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
	}
		
	return TDCLFSItemRef( theItem );
}

// -------------------------------------------------------------------------- //
//  * CreateFile( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonDesktopFolder::CreateFile( const KUInt16* /* inName */ )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * CreateFolder( const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacCarbonDesktopFolder::CreateFolder( const KUInt16* /* inName */ )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * CountElements( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLMacCarbonDesktopFolder::CountElements( void )
{
#warning TODO.
	throw DCLNotImplementedError;
}

// -------------------------------------------------------------------------- //
//  * GetItems( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef*
TDCLMacCarbonDesktopFolder::GetItems( void )
{
	TDCLMacFiles* theFilesIntf = (TDCLMacFiles*) GetFilesIntf();
	KUInt32 theElementsCount;
	TDCLFSItemRef* theResult =
			TDCLMacCarbonFolder::GetFolderItems(
							theFilesIntf,
							&mDesktopFolderRef,
							&theElementsCount );

	// Ensuite on ajoute les volumes.
	ItemCount theVolumeIndex = 1;
	do {
		FSRef theVolumeRef;
		OSErr theErr =
				::FSGetVolumeInfo(
						0,
						theVolumeIndex,
						NULL,
						kFSVolInfoNone,
						NULL,
						NULL,
						&theVolumeRef );
	
		if (theErr)
		{
			if (theErr == nsvErr)
			{
				break;
			} else {
				// Libération.
				::free( theResult );
				
				throw DCLPlatformUnknownError( theErr );
			}
		}
		
		// Ajout au tableau des résultats.
		theResult = (TDCLFSItemRef*)
				::realloc(
						theResult,
						(theElementsCount + 2) * sizeof( TDCLFSItemRef ) );
		
		new (&theResult[theElementsCount++]) TDCLFSItemRef( 
					new TDCLMacCarbonFolder(
							theFilesIntf,
							&theVolumeRef ));
		
		theVolumeIndex++;
	} while ( true );
	
	// Ajout du terminateur.
	new (&theResult[theElementsCount]) TDCLFSItemRef();
	
	return theResult;
}

// ============================================================== //
// All I ask is a chance to prove that money can't make me happy. //
// ============================================================== //