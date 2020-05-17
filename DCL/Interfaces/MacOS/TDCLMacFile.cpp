// ==============================
// Fichier:			TDCLMacFile.cp
// Projet:			Desktop Connection Library
//
// Créé le:			09/02/2003
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
// The Original Code is TDCLMacFile.cp.
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
// $Id: TDCLMacFile.cp,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacFile.h>

// MacOS
#if TARGET_OS_MACOS
	#include <Files.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// ------------------------------------------------------------------------- //
//  * TDCLMacFile( TDCLMacFiles*, TDCLFSItemRef )
// ------------------------------------------------------------------------- //
TDCLMacFile::TDCLMacFile( TDCLMacFiles* inFilesIntf, TDCLFSItemRef inFolder )
	:
		TDCLFile( inFilesIntf, inFolder ),
		mReadOnly( false ),
		mIsOpen( false )
{
	// Affichage administratif.
}

// ------------------------------------------------------------------------- //
//  * IsOpen( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacFile::IsOpen( void ) const
{
	return mIsOpen;
}

// ------------------------------------------------------------------------- //
//  * IsReadOnly( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacFile::IsReadOnly( void ) const
{
	return mReadOnly;
}

// ------------------------------------------------------------------------- //
//  * DoOpen( Boolean )
// ------------------------------------------------------------------------- //
void
TDCLMacFile::DoOpen( Boolean inReadOnly )
{
	mReadOnly = inReadOnly;
	mIsOpen = true;
}

// ------------------------------------------------------------------------- //
//  * DoClose( void )
// ------------------------------------------------------------------------- //
void
TDCLMacFile::DoClose( void )
{
	mIsOpen = false;
}

// ------------------------------------------------------------------------- //
//  * DoSetMetaData( const FSSpec*, OSType, OSType )
// ------------------------------------------------------------------------- //
void
TDCLMacFile::DoSetMetaData(
						const FSSpec* inFSSpec,
						OSType inCreator,
						OSType inFileType )
{
	// Changement du type/créateur.
	FInfo theInfo;

	OSErr theErr = ::FSpGetFInfo( inFSSpec, &theInfo );
	if(theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	theInfo.fdType = inFileType;
	theInfo.fdCreator = inCreator;

	theErr = ::FSpSetFInfo( inFSSpec, &theInfo );
	if(theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * ToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacFile::ToFrame( void )
{
	TDCLNSRef theSegment = TDCLFile::ToFrame();
	
	if (IsOnDesktop())
	{
		// Nous sommes sur le bureau.
		TDCLNSFrame& theSegmentAsFrame = theSegment.ToFrame();

		// Ajout du volume.
		theSegmentAsFrame.Set(
						"whichVol", TDCLNSRef::MakeInt( GetVRefNum() ) );
	}
			
	return theSegment;
}

// =================================================================== //
// Adding manpower to a late software project makes it later.          //
//                 -- F. Brooks, "The Mythical Man-Month"              //
//                                                                     //
// Whenever one person is found adequate to the discharge of a duty by //
// close application thereto, it is worse execute by two persons and   //
// scarcely done at all if three or more are employed therein.         //
//                 -- George Washington, 1732-1799                     //
// =================================================================== //