// ==============================
// Fichier:			TDCLBrowseEngine.cp
// Projet:			Desktop Connection Library
//
// Créé le:			8/4/2003
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
// The Original Code is TDCLBrowseEngine.cp.
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
// $Id: TDCLBrowseEngine.cp,v 1.9 2004/11/24 18:11:34 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLBrowseEngine.h>

// ANSI C
#include <stdlib.h>

// ISO C++
#include <new>

// K
#include <K/Math/UTInt64.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/TDCLFolder.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>

// -------------------------------------------------------------------------- //
//  * TDCLBrowseEngine( TDCLFullDockLink*, TDCLFSItemRef )
// -------------------------------------------------------------------------- //
TDCLBrowseEngine::TDCLBrowseEngine(
				TDCLFullDockLink* inLink,
				TDCLFSItemRef inDefaultFolder /* = TDCLFSItemRef() */ )
	:
		TDCLLinkEngine( inLink ),
		mFilesIntf( nil ),
		mCurrentFolderRef( nil ),
		mCurrentFolder( nil ),
		mIsMac( false ),
		mFileTypes()
{
	mFilesIntf = inLink->GetApplication()->GetFilesIntf();
	if (inDefaultFolder.GetFSItem() == nil)
	{
		mCurrentFolderRef = mFilesIntf->GetRootFolder();
	} else {
		mCurrentFolderRef = inDefaultFolder;
	}
	mCurrentFolder = (TDCLFolder*) mCurrentFolderRef;
	mIsMac = (mFilesIntf->GetKind() == TDCLDockCommand::kMac_DType);
}

// -------------------------------------------------------------------------- //
//  * StartBrowsing( const TDCLNSRef& inFileTypes )
// -------------------------------------------------------------------------- //
void
TDCLBrowseEngine::StartBrowsing( const TDCLNSRef& inFileTypes )
{
	mFileTypes = inFileTypes;
	
	// On envoie une commande kDResult au Newton.
	TDCLDockCmdSingleLong theResultCmd( TDCLDockCommand::kDResult, 0 );
	theResultCmd.SendCommand( GetLink()->GetPipe() );
}

// -------------------------------------------------------------------------- //
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLBrowseEngine::ProcessDockCommand(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	Boolean theResult = true;
	
	TDCLPipe* thePipe = GetLink()->GetPipe();

	switch ( inCommand->GetCommand() )
	{
		case TDCLDockCommand::kDGetDefaultPath:
			{
				// Envoi du chemin courant.
				TDCLDockCmdNSObject thePathCmd(
								TDCLDockCommand::kDPath,
								mCurrentFolder->GetPath() );
				thePathCmd.SendCommand( thePipe );
				*outProcessed = true;
			}
			break;

		case TDCLDockCommand::kDGetFilesAndFolders:
			{
				// Envoi des éléments dans le dossier.
				TDCLDockCmdNSObject theContentCmd(
								TDCLDockCommand::kDFilesAndFolders,
								GetFilesAndFolders() );
					
				theContentCmd.SendCommand( thePipe );
				*outProcessed = true;
			}	
		break;

		case TDCLDockCommand::kDSetPath:
			{
				TDCLNSRef thePathRef =
					((TDCLDockCmdNSObject*) inCommand)->GetObject();

				SetPath( thePathRef );
				
				// Envoi du chemin.
				TDCLDockCmdNSObject thePathCmd(
					TDCLDockCommand::kDPath, mCurrentFolder->GetPath() );
				thePathCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}	
		break;

		case TDCLDockCommand::kDGetFileInfo:
			{
				// Récupération de l'élément.
				TDCLNSRef theFileName =
					((TDCLDockCmdNSObject*) inCommand)->GetObject();
				
				TDCLNSRef theInfo = GetFileInfo( theFileName );
				
				// Envoi de la commande.
				TDCLDockCmdNSObject theFileInfoCmd(
					TDCLDockCommand::kDFileInfo, theInfo );
				theFileInfoCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}
		break;

		case TDCLDockCommand::kDLoadPackageFile:
			{
				TDCLNSRef theFileName =
					((TDCLDockCmdNSObject*) inCommand)->GetObject();
				KUInt16* theString;
				if (theFileName.IsString())
				{
					theString = theFileName.ToString().GetString();
				} else {
					theString = theFileName.ToFrame().Get("name").ToString().GetString();
				}
				TDCLFSItemRef thePackage = mCurrentFolder->GetItemByName( theString );
				
				DoInstallPackage( thePackage );
				*outProcessed = true;
				theResult = false;
			}
		break;

		case TDCLDockCommand::kDGetDevices:
			{
				TDCLNSArray theArray;

				// On compte les éléments.
				KUInt32 nbRoots = mFilesIntf->CountRootFolders();
				
				// Itération
				KUInt32 indexRoots;
				for (indexRoots = 0; indexRoots < nbRoots; indexRoots++)
				{
					// Structure pour cette racine.
					TDCLNSRef theFrameRef = TDCLNSRef::MakeFrame();
					TDCLNSFrame& theFrame = theFrameRef.ToFrame();
					theFrame.Set( "name",
						TDCLNSRef::MakeString(
							mFilesIntf->GetRootFolderName( indexRoots ) ) );
					theFrame.Set( "disktype", TDCLNSRef::MakeInt( 1 /* hard drive */ ) );
					theArray.Add( theFrameRef );
				}

				TDCLDockCmdNSObject theDevicesCmd(
					TDCLDockCommand::kDDevices, TDCLNSRef( theArray ) );
				theDevicesCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}
		break;

		case TDCLDockCommand::kDSetDrive:
			{
				// Récupération du nom & on change de racine.
				TDCLNSRef theDriveName =
					((TDCLDockCmdNSObject*) inCommand)->GetObject();
				if (!theDriveName.IsString())
				{
					throw DCLBadDockCmd;
				}
				mCurrentFolderRef =
					mFilesIntf->GetRootFolder(
						theDriveName.ToString().GetString() );
				mCurrentFolder = (TDCLFolder*) mCurrentFolderRef;
				
				// On envoie le chemin.
				TDCLDockCmdNSObject thePathCmd(
								TDCLDockCommand::kDPath,
								mCurrentFolder->GetPath() );
				thePathCmd.SendCommand( thePipe );
				*outProcessed = true;
			}
		break;

		case TDCLDockCommand::kDGetFilters:
			{
#warning move the windows filters at the file level
				TDCLDockCmdNSObject theFiltersCmd(
								TDCLDockCommand::kDFilters,
								TDCLNSRef::kNILREF );
				theFiltersCmd.SendCommand( thePipe );
				*outProcessed = true;
			}
		break;

		// Pas encore gérées.
		case TDCLDockCommand::kDSetFilter:
		case TDCLDockCommand::kDResolveAlias:
		case TDCLDockCommand::kDImportFile:
			throw DCLNotImplementedError;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetFilesAndFolders( void )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLBrowseEngine::GetFilesAndFolders( void )
{
	Boolean onlyImports = false;
	Boolean onlyPackages = false;
	Boolean onlySyncFiles = false;
	if (mFileTypes.IsSymbol())
	{
		if (mFileTypes == TDCLNSRef::MakeSymbol( "import" ))
		{
			onlyImports = true;
		} else if (mFileTypes == TDCLNSRef::MakeSymbol( "packages" )) {
			onlyPackages = true;
		} else if (mFileTypes == TDCLNSRef::MakeSymbol( "syncFiles" )) {
			onlySyncFiles = true;
		}
	}

	TDCLNSRef theResult = TDCLNSRef::MakeArray();
	TDCLNSArray& theResultAsArray = theResult.ToArray();
	
	// Récupération des éléments.
	TDCLFSItemRef* theItems = nil;
	try {
		theItems = mCurrentFolder->GetItems();
	} catch ( TDCLException& inException ) {
		// En cas d'exception, on considère qu'il n'y a pas d'élément.
	}

	if (theItems)
	{
		// Itération sur les éléments: on les ajoute et on libère la mémoire.
		TDCLFSItemRef* theCursor = theItems;
		while ( theCursor->GetFSItem() != nil )
		{
			TDCLFSItemRef theElement = *theCursor++;
			TDCLFSItem* theItem = theElement.GetFSItem();
			if (theItem->GetClass() == TDCLFSItem::kFile)
			{
				// Devons-nous ajouter le fichier?
				if (onlyPackages)
				{
					if (((TDCLFile*) theItem)->IsPackage())
					{
						theResultAsArray.Add( theElement->ToFrame() );
					}
				} else {
					// Par défaut, j'ajoute le fichier.
					theResultAsArray.Add( theElement->ToFrame() );
				}
			} else {
				// On ajoute toujours les dossiers.
				theResultAsArray.Add( theElement->ToFrame() );
			}
		
			// Libération.
			theElement.~TDCLFSItemRef();
		}
	
		// Libération du tableau.
		::free( theItems );
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetFileInfo( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLBrowseEngine::GetFileInfo( const TDCLNSRef& inFileName )
{
	// Récupération du fichier.
	TDCLFSItemRef theElement = GetFile( inFileName );
	
	TDCLFSItem* theItem = theElement.GetFSItem();
	if ((theItem == nil) || (theItem->GetClass() != TDCLFSItem::kFile))
	{
		// On ne s'attend pas à ça. On dégage.
		throw DCLBadDockCmd;
	}
	
	TDCLFile* theItemAsFile = (TDCLFile*) theItem;
	
	// Création de la structure pour les informations sur le fichier.
	TDCLNSRef theInfo = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theInfoFrame = theInfo.ToFrame();
	theInfoFrame.Set( "kind", theItemAsFile->GetKind() );
	KSInt32 theSize = -1;
	if (theItemAsFile->GetLength() < 0x60000000)
	{
		theSize = (KSInt32) UTInt64::GetLo( theItemAsFile->GetLength() );
	}
	theInfoFrame.Set( "size",
		TDCLNSRef::MakeInt( (KSInt32) theSize ) );
	theInfoFrame.Set( "created",
		TDCLNSRef::MakeInt( (KSInt32) theItemAsFile->GetCreationDate() ) );
	theInfoFrame.Set( "modified",
		TDCLNSRef::MakeInt( (KSInt32) theItemAsFile->GetModificationDate() ) );
	theInfoFrame.Set( "path", theItemAsFile->GetStringPath() );
	
	TDCLNSRef theIcon = theItemAsFile->GetIcon();
	if (!theIcon.IsNIL())
	{
		theInfoFrame.Set( "icon", theIcon );
	}
	
	return theInfo;
}

// -------------------------------------------------------------------------- //
//  * SetPath( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
void
TDCLBrowseEngine::SetPath( const TDCLNSRef& inPath )
{
	TDCLNSArray& thePath = inPath.ToArray();
	
	// Interprétation du chemin.
	const KUInt16* theRootName = nil;
	TDCLNSRef theElement;
	TDCLNSRef theElementName;
	// Si on est sur Mac, le premier élément doit être Desktop
	if (mIsMac)
	{
		// Vérifions qu'on a bien Desktop.
		theElement = thePath.Get(0);
		Boolean isDesktop = false;
		if (theElement.IsString())
		{
			isDesktop = theElement.ToString().StrEqual( "Desktop" );
		} else {
			isDesktop = theElement.ToFrame().Get("name").ToString().StrEqual( "Desktop" );
		}
		if (!isDesktop)
		{
			throw DCLBadDockCmd;
		}
	} else {
		// On a normalement X:\ avec X un nom de disque.
		// Vérifions.
		theElement = thePath.Get(0);
		if (theElement.IsString())
		{
			theElementName = theElement;
		} else {
			theElementName = theElement.ToFrame().Get("name");
		}
		
		TDCLNSString& theElementAsString = theElementName.ToString();
		theRootName = theElementAsString.GetString();
	}
	
	mCurrentFolderRef = mFilesIntf->GetRootFolder( theRootName );
	mCurrentFolder = (TDCLFolder*) mCurrentFolderRef;

	KUInt32 thePathLength = thePath.GetLength();
	KUInt32 thePathIndex = 1;
	while (thePathIndex < thePathLength)
	{
		theElement = thePath.Get(thePathIndex++);
		
		// Récupération de l'élément.
		mCurrentFolderRef = GetFile( theElement );
		mCurrentFolder = (TDCLFolder*) mCurrentFolderRef;
	}
}

// -------------------------------------------------------------------------- //
//  * GetFile( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLBrowseEngine::GetFile( const TDCLNSRef& inFileName )
{
	KUInt16* theName;
	KSInt32 theVolRefNum = 0;
	if (inFileName.IsString())
	{
		theName = inFileName.ToString().GetString();
	} else {
		TDCLNSFrame& theFileNameAsFrame = inFileName.ToFrame();
		theName = theFileNameAsFrame.Get("name").ToString().GetString();
		if (theFileNameAsFrame.HasSlot("whichVol"))
		{
			theVolRefNum = theFileNameAsFrame.Get("whichVol").ToInt();
		}
	}
	return mCurrentFolder->GetItemByName( theName, theVolRefNum );	
}

// ===================================================================== //
// This process can check if this value is zero, and if it is, it does   //
// something child-like.                                                 //
//                 -- Forbes Burkowski, CS 454, University of Washington //
// ===================================================================== //
