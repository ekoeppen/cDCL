// ==============================
// Fichier:			TDCLMacGetPutFiles.cp
// Projet:			Desktop Connection Library
//
// Créé le:			30/01/2003
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
// The Original Code is TDCLMacGetPutFiles.cp.
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
// $Id: TDCLMacGetPutFiles.cp,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacGetPutFiles.h>

// MacOS
#if TARGET_OS_MACOS
#include <Files.h>
#include <MacErrors.h>
#include <Navigation.h>
#include <Script.h>
#include <StandardFile.h>
#include <AEDataModel.h>
#else
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#endif

// K
#include <K/Misc/UPStrings.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>

#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFile.h>

// ------------------------------------------------------------------------- //
//	* TDCLMacGetPutFiles( TDCLMacFiles* )
// ------------------------------------------------------------------------- //
TDCLMacGetPutFiles::TDCLMacGetPutFiles( TDCLMacFiles* inFilesIntf )
	:
		mFilesIntf( inFilesIntf ),
		mNavServicesAvailable( true )	// Défaut sous OS X.
{
}

// ------------------------------------------------------------------------- //
//	* ~TDCLMacGetPutFiles( void )
// ------------------------------------------------------------------------- //
TDCLMacGetPutFiles::~TDCLMacGetPutFiles( void )
{
}

// ------------------------------------------------------------------------- //
//  * DoGetFile( const char*, Boolean )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacGetPutFiles::DoGetFile(
					const char* inPrompt,
					Boolean inOnlyPackages )
{
	TDCLFSItemRef theResult;
	
	if (mNavServicesAvailable)
	{
		// Adjust the options to fit our needs.
		NavDialogOptions	myDialogOptions;

		// Grab default options for dialog box
		// Get the error and throw an exception if it's not noErr
		OSErr theErr = ::NavGetDefaultDialogOptions( &myDialogOptions );
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
		
		NavObjectFilterUPP	theFilterUPP = NULL;
		if (inOnlyPackages)
		{
			theFilterUPP =
					NewNavObjectFilterUPP(&PackageNavServicesFilterProc);
		}
		
		// Set the prompt.
		// (I translate it from CString)
		UPStrings::C2PStrCopy( myDialogOptions.message, inPrompt );
	
		// Pas d'aperçu.
		myDialogOptions.dialogOptionFlags &= ~kNavAllowPreviews;
		// Ni de menu pour le type.
		myDialogOptions.dialogOptionFlags |= kNavNoTypePopup;
        
		NavReplyRecord myReply;	// the Reply
		
		theErr = ::NavChooseFile(
			NULL,	/* Let the NavServices find where to start */
			&myReply,
			&myDialogOptions,
			NULL, 	/* eventProc */
			NULL, 	/* previewProc */
			theFilterUPP, 	/* filterProc */
			nil, 	/* typeList */
			nil		/* callBackUD */
			);
		if (theErr)
		{
			if (inOnlyPackages)
			{
				DisposeNavObjectFilterUPP(theFilterUPP);
			}
			if (theErr == userCanceledErr)
			{
				throw DCLPlatformUserCancelled( theErr );
			} else {
				throw DCLPlatformUnknownError( theErr );
			}
		}

		try {
			// Let's look into the reply.
			if(!myReply.validRecord)
			{
				if (inOnlyPackages)
				{
					DisposeNavObjectFilterUPP(theFilterUPP);
				}

				// User cancelled.
				throw DCLUserCancelled;
			}
		
			// Extract the file.
			AEKeyword   theKeyword;	// Unused
			DescType    theActualType;	// Unused
			Size        theActualSize;	// Unused
			FSRef      myFSRef;
                        
			// Extraction du FSRef
			theErr = ::AEGetNthPtr(
							// Il n'y a qu'un seul élément de toute façon.
							&( myReply.selection ), 1,
							typeFSRef,
							&theKeyword,
							&theActualType,
							&myFSRef,
							sizeof( myFSRef ),
							&theActualSize
						);
			if (theErr)
			{
				if (inOnlyPackages)
				{
					DisposeNavObjectFilterUPP(theFilterUPP);
				}
				throw DCLPlatformUnknownError( theErr );
			}
			
			theResult = TDCLFSItemRef(
						new TDCLMacCarbonFile( mFilesIntf, &myFSRef ) );
		} catch( ... ) {
			// I must dispose the reply.
			(void) ::NavDisposeReply( &myReply );
			
			throw;	// Rethrow
		}
		
		theErr = ::NavDisposeReply( &myReply );	// Dispose the reply.
		if (theErr)
		{
			if (inOnlyPackages)
			{
				DisposeNavObjectFilterUPP(theFilterUPP);
			}
			throw DCLPlatformUnknownError( theErr );
		}
		
		if (inOnlyPackages)
		{
			DisposeNavObjectFilterUPP(theFilterUPP);
		}
	} else {
		// With Carbon/MacOS X, we don't have anything so throw an exception.
		throw DCLNotAvailable;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PutFile( const char*, const char* )
// ------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLMacGetPutFiles::PutFile(
					const char* inPrompt,
					const char* inDefaultName )
{
	TDCLFSItemRef theResult;
	
	if (mNavServicesAvailable)
	{
		// Use the NavServices.
		// Adjust the options to fit our needs.
		NavDialogOptions	myDialogOptions;

		// Grab default options for dialog box
		// Get the error and throw an exception if it's not noErr
		OSErr theErr = ::NavGetDefaultDialogOptions( &myDialogOptions );
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
		
		// Set the prompt.
		// (I translate it from CString)
		UPStrings::C2PStrCopy( myDialogOptions.message, inPrompt );
	
		// Set the default name.
		// (idem)
		UPStrings::C2PStrCopy( myDialogOptions.savedFileName, inDefaultName );

		// Drapeaux.
		// Pas de modèle.
		myDialogOptions.dialogOptionFlags &= ~kNavAllowStationery;
		// Ni de menu pour le type
		myDialogOptions.dialogOptionFlags &= ~kNavNoTypePopup;

		NavReplyRecord myReply;	// the Reply
		
		theErr = ::NavPutFile(
			NULL,	/* Let the NavServices find where to start */
			&myReply,
			&myDialogOptions,
			NULL, 	/* eventProc */
			kNavGenericSignature,
			kNavGenericSignature,
			nil		/* callBackUD */
			);
		if (theErr)
		{
			if (theErr == userCanceledErr)
			{
				throw DCLPlatformUserCancelled( theErr );
			} else {
				throw DCLPlatformUnknownError( theErr );
			}
		}

		try {
			// Let's look into the reply.
			if(!myReply.validRecord)
			{
				// User cancelled.
				throw DCLUserCancelled;
			}
		
			// Extract the file.
			AEKeyword   theKeyword;	// Unused
			DescType    theActualType;	// Unused
			Size        theActualSize;	// Unused
			FSRef      myFSRef;
                        
			// Extraction du FSRef
			theErr = ::AEGetNthPtr(
							// Il n'y a qu'un seul élément de toute façon.
							&( myReply.selection ), 1,
							typeFSRef,
							&theKeyword,
							&theActualType,
							&myFSRef,
							sizeof( myFSRef ),
							&theActualSize
						);
			if (theErr)
			{
				throw DCLPlatformUnknownError( theErr );
			}
			
			theResult = TDCLFSItemRef(
							new TDCLMacCarbonFile( mFilesIntf, &myFSRef ) );
			
			if (myReply.replacing)
			{
				((TDCLFile*) theResult)->Delete();
			}
		} catch( ... ) {
			// Libération de la réponse.
			(void) ::NavDisposeReply( &myReply );
			
			throw;	// Rethrow
		}
		
		theErr = ::NavDisposeReply( &myReply );	// Dispose the reply.
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
		
	} else {
		// With Carbon/MacOS X, we don't have anything so throw an exception.
		throw DCLNotAvailable;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PackageNavServicesFilterProc( AEDesc*, void*, NavCallBackUserData, ... )
// ------------------------------------------------------------------------- //
pascal Boolean
TDCLMacGetPutFiles::PackageNavServicesFilterProc(
					AEDesc* inItem,
					void* inInfo, 
					NavCallBackUserData /* inCallBackUD */,
					NavFilterModes /* inFilterMode */ )
{
	Boolean theResult = true;
    
	do {
    	if (inItem->descriptorType != typeFSS)
    	{
    		break;
    	}

        if (((NavFileOrFolderInfo*)inInfo)->isFolder)
        {
        	break;
        }
        
        theResult = false;
        
		FSSpec thePackageFSSpec;

		// On récupère un FSRef directement.
		OSErr theErr;
#if CALL_NOT_IN_CARBON
		thePackageFSSpec = *((FSSpec*) *inItem->dataHandle);
#else
		theErr = ::AEGetDescData(
							inItem,
							&thePackageFSSpec,
							sizeof( thePackageFSSpec ));
		if (theErr != noErr)
		{
			break;
		}
#endif

		short theRefNum;
		theErr = ::FSpOpenDF( &thePackageFSSpec, fsRdPerm, &theRefNum );
		if (theErr != noErr)
		{
			break;
		}
		
		// Lecture des 8 premiers octets.
		KUInt32 theFirstBytes[2];
		long readCount = sizeof(theFirstBytes);
		theErr = ::FSRead( theRefNum, &readCount, theFirstBytes ); 

		if (theErr != noErr)
		{
			(void) ::FSClose( theRefNum );
			break;
		}
		
		if ((theFirstBytes[0] == 'pack')
			&& ((theFirstBytes[1] == 'age0')
				|| (theFirstBytes[1] == 'age1')))
		{
			theResult = true;
		}

		(void) ::FSClose( theRefNum );
	} while ( false );

    return theResult;
}
// =================================================================== //
// You know you've been sitting in front of your Lisp machine too long //
// when you go out to the junk food machine and start wondering how to //
// make it give you the CADR of Item H so you can get that yummie      //
// chocolate cupcake that's stuck behind the disgusting vanilla one.   //
// =================================================================== //
