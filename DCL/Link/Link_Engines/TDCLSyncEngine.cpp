// ==============================
// Fichier:			TDCLSyncEngine.cp
// Projet:			Desktop Connection Library
//
// Créé le:			7/4/2003
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
// The Original Code is TDCLSyncEngine.cp.
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
// $Id: TDCLSyncEngine.cp,v 1.11 2004/11/24 18:11:34 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLSyncEngine.h>

// ANSI C
#include <stdlib.h>
#include <stdio.h>

// K
#include <K/Unicode/UUTF16Conv.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdArray.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdCString.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPackageList.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Link_Engines/TDCLSyncSourceEngine.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>

// ------------------------------------------------------------------------- //
//  * Déverminage
// ------------------------------------------------------------------------- //

#undef KERROR_ENABLED
#if defined(error_TDCLSyncEngine) && error_TDCLSyncEngine
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLSyncEngine) && trace_TDCLSyncEngine
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLSyncEngine) && debug_TDCLSyncEngine
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#define KDEBUG_ENABLED 1
#undef KTRACE_ENABLED
#define KTRACE_ENABLED 1
 
// -------------------------------------------------------------------------- //
//  * TDCLSyncEngine( TDCLFullDockLink* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::TDCLSyncEngine( TDCLFullDockLink* inLink )
	:
		TDCLLinkEngine( inLink ),
		mSyncOptions(),
		mDoPackages( false ),
		mSyncAll( false ),
		mStores( nil ),
		mStoresCount( 0 ),
        mState( kUninitialized ),
        mNewtonTime( 0 ),
        mStoreIndex( 0 ),
        mSoups(),
		mSoupsArray( nil ),
		mSoupsCount( 0 ),
		mSoupIndex( 0 ),
		mSoupInfo(),
		mEntries(),
		mEntriesArray( nil ),
		mEntriesCount( 0 ),
		mEntryIndex( 0 ),
		mCurrentSourceEngine( nil ),
		mAddingEntries( false ),
		mCompleteSync( true ),
		mInterrupted( false )
{
	KTRACE( "TDCLSyncEngine" );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLSyncEngine( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::~TDCLSyncEngine( void )
{
	KTRACE( "~TDCLSyncEngine" );
}

// -------------------------------------------------------------------------- //
//  * StartSynchronizing( void )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::StartSynchronizing( void )
{
	KTRACE( "StartSynchronizing" );

	TDCLDockCmdNoData theGetSyncOptionCmd( TDCLDockCommand::kDGetSyncOptions );
	theGetSyncOptionCmd.SendCommand( GetLink()->GetPipe() );
	
	// Début.
	mInterrupted = false;
	
	// Première source.
	ResetSourceCursor();

	// On attend les options.
	mState = kWFOptions;
	KTRACE( " -> kWFOptions" );
}

// -------------------------------------------------------------------------- //
//  * StopSynchronizing( void )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::StopSynchronizing( void )
{
	KTRACE( "StopSynchronizing" );
	mInterrupted = true;
}

// -------------------------------------------------------------------------- //
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLSyncEngine::ProcessDockCommand(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE1( "ProcessDockCommand (%p)", inCommand );

	switch (mState)
	{
		case kUninitialized:
			mState = TransitionFromUninitialized(
									inCommand, outProcessed );
			break;
		
		case kWFOptions:
			mState = TransitionFromWFOptions(
									inCommand, outProcessed );
			break;

		case kWFStoreNames:
			mState = TransitionFromWFStoreNames(
									inCommand, outProcessed );
			break;
			
		case kWFPreliminaryTime:
			mState = TransitionFromWFPreliminaryTime(
									inCommand, outProcessed );
			break;
			
		case kWFSetStoreResult:
			mState = TransitionFromWFSetStoreResult(
									inCommand, outProcessed );
			break;
		
		case kWFPackageIDs:
			mState = TransitionFromWFPackageIDs(
									inCommand, outProcessed );
			break;
		
		case kWFSetStoreResult2:
			mState = TransitionFromWFSetStoreResult2(
									inCommand, outProcessed );
			break;
		
		case kWFStoreSyncTime:
			mState = TransitionFromWFStoreSyncTime(
									inCommand, outProcessed );
			break;
		
		case kWFSoupInfo:
			mState = TransitionFromWFSoupInfo(
									inCommand, outProcessed );
			break;

		case kWFSoupIDs:
			mState = TransitionFromWFSoupIDs(
									inCommand, outProcessed );
			break;

		case kWFChangedIDs:
			mState = TransitionFromWFChangedIDs(
									inCommand, outProcessed );
			break;
			
		case kWFEntry:
			mState = TransitionFromWFEntry(
									inCommand, outProcessed );
			break;
			
		case kWFSetStoreToDefaultResult:
			mState = TransitionFromWFSetStoreToDefaultResult(
									inCommand, outProcessed );
			break;
		
		case kWFModifiedResult:
			mState = TransitionFromWFModifiedResult(
									inCommand, outProcessed );
			break;

		case kWFAddedID:
			mState = TransitionFromWFAddedID(
									inCommand, outProcessed );
			break;
		
		case kEnd:
			break;	// Pour gcc.

		case kInterrupted:
			mState = TransitionFromInterrupted(
									inCommand, outProcessed );
			break;
	}
	
	if (mInterrupted)
	{
		mState = kInterrupted;
	}
	
	return (mState != kEnd);
}

// -------------------------------------------------------------------------- //
//  * TransitionFromUninitialized( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromUninitialized(
							TDCLDockCommand* /* inCommand */,
							Boolean* /* outProcessed */ )
{
	KTRACE( "Transition: kUninitialized ->" );
	throw DCLBadStateError;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFOptions( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFOptions(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFOptions ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDSyncOptions )
	{
		// Ce sont les options pour la synchronisation.
		TDCLDockCmdNSObject* theSyncOptionsCmd =
			(TDCLDockCmdNSObject*) inCommand;
		mSyncOptions = theSyncOptionsCmd->GetObject();
		
		// On analyse un peu ces options.
		if (!mSyncOptions.IsNIL())
		{
			TDCLNSFrame& theOptionsAsFrame = mSyncOptions.ToFrame();
			mDoPackages = !theOptionsAsFrame.Get( "packages" ).IsNIL();
			mSyncAll = !theOptionsAsFrame.Get( "syncAll" ).IsNIL();
			mStoresRef = theOptionsAsFrame.Get( "stores" );
			mStores = &mStoresRef.ToArray();
			mStoresCount = mStores->GetLength();

			// On demande le temps pour connaître l'heure du Newton.
			TDCLDockCmdSingleLong theGetSyncTimeCmd(
										TDCLDockCommand::kDLastSyncTime, 0 );
			theGetSyncTimeCmd.SendCommand( GetLink()->GetPipe() );
			
			theResult = kWFPreliminaryTime;
			KTRACE( " -> kWFPreliminaryTime" );
		} else {
			// Les options sont NIL.
			// Je prends les options par défaut et je demande la liste
			// des magasins.
			TDCLNSFrame* theOptionsAsFrame = new TDCLNSFrame();

			mSyncOptions = theOptionsAsFrame;
			theOptionsAsFrame->Set( "packages", TDCLNSRef::kTRUEREF );
			mDoPackages = true;
			theOptionsAsFrame->Set( "syncAll", TDCLNSRef::kTRUEREF );
			mSyncAll = true;
			
			// Demande de la liste des magasins.
			TDCLDockCmdNoData theGetStoreNamesCmd(
					TDCLDockCommand::kDGetStoreNames );
			theGetStoreNamesCmd.SendCommand( GetLink()->GetPipe() );

			theResult = kWFStoreNames;
			KTRACE( " -> kWFStoreNames" );
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFStoreNames( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFStoreNames(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFStoreNames ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDStoreNames )
	{
		// Ce sont les noms des magasins.
		TDCLDockCmdNSObject* theStoreNamesCmd = (TDCLDockCmdNSObject*) inCommand;
		mStoresRef = theStoreNamesCmd->GetObject();
		mStores = &mStoresRef.ToArray();
		mStoresCount = mStores->GetLength();

		TDCLNSFrame& theOptionsAsFrame = mSyncOptions.ToFrame();
		theOptionsAsFrame.Set( "stores", mStoresRef );
		
		// On demande le temps pour connaître l'heure du Newton.
		TDCLDockCmdSingleLong theGetSyncTimeCmd(
									TDCLDockCommand::kDLastSyncTime, 0 );
		theGetSyncTimeCmd.SendCommand( GetLink()->GetPipe() );
		
		theResult = kWFPreliminaryTime;
		KTRACE( " -> kWFPreliminaryTime" );

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFPreliminaryTime( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFPreliminaryTime(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFPreliminaryTime ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDCurrentTime )
	{
		// On stocke l'heure actuelle du Newton.
		TDCLDockCmdSingleLong* theTimeCmd = (TDCLDockCmdSingleLong*) inCommand;
		mNewtonTime = (KSInt32) theTimeCmd->GetLong();
		
		if (DoSynchronizePatches() || (mDoPackages && DoSynchronizePackages()))
		{
			// Première itération sur les magasins pour les rustines &
			// les paquets 1.x.

			// On commence par le premier magasin.
			mStoreIndex = 0;
			
			// Récupération dudit magasin et on demande au Newton de changer.
			TDCLDockCmdNSObject theSetStoreCmd(
									TDCLDockCommand::kDSetCurrentStore,
									mStores->Get( mStoreIndex ));
			theSetStoreCmd.SendCommand( GetLink()->GetPipe() );
			
			theResult = kWFSetStoreResult;
			KTRACE( " -> kWFSetStoreResult" );
		} else {
			// Sources.
			theResult = SyncNextSource();
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFSetStoreResult( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFSetStoreResult(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFSetStoreResult ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDResult )
	{
		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResultValue = (KSInt32) theResultCmd->GetLong();
		if (theResultValue)
		{
			// On relance l'erreur reçue du Newton.
			throw DCLNewton( theResultValue );
		}

		if (DoSynchronizePatches())
		{
			// Synchronisation des rustines.
			throw DCLNotImplementedError;
#warning handle synchronization of system patches
		} else {
			// Synchronisation des paquets seulement.
			
			// Récupération des informations sur les paquets.
			TDCLDockCmdNoData theGetPacakageIDsCmd( TDCLDockCommand::kDGetPackageIDs );
			theGetPacakageIDsCmd.SendCommand( GetLink()->GetPipe() );
			theResult = kWFPackageIDs;
			KTRACE( " -> kWFPackageIDs" );
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFPackageIDs( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFPackageIDs(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFPackageIDs ->" );
	if ( inCommand->GetCommand() == TDCLDockCommand::kDPackageIDList )
	{
		*outProcessed = true;
//				TDCLDockCmdPackageList* theList = (TDCLDockCmdPackageList*) inCommand;
#warning handle synchronization of packages
		throw DCLNotImplementedError;
	}
	
	return mState;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFSetStoreResult2( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFSetStoreResult2(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFSetStoreResult2 ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDResult )
	{
		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResultValue = (KSInt32) theResultCmd->GetLong();
		if (theResultValue)
		{
			// On relance l'erreur reçue du Newton.
			throw DCLNewton( theResultValue );
		}

		// Optimise-t-on la synchronisation avec l'heure de la dernière
		// synchronisation?
		mCompleteSync = true;

		KUInt32 theLastSyncDate = 0;

		if (mCurrentSourceEngine->HasModificationDate())
		{
			// Recherche de ce magasin dans les données de synchronisation.
			(void) GetCurrentStoreFrame( &theLastSyncDate );

			if (theLastSyncDate > 0)
			{
				mCompleteSync = false;
			}
		}
		
		if (mCompleteSync)
		{
			// Première soupe.
			mSoupIndex = -1;
			theResult = SyncNextSoup();
		} else {
			// On récupère l'heure de la dernière synchronisation dans
			// les données de synchronisation.
			TDCLDockCmdSingleLong theGetSyncTimeCmd(
					TDCLDockCommand::kDLastSyncTime,
					theLastSyncDate
					 );
			theGetSyncTimeCmd.SendCommand( GetLink()->GetPipe() );
			
			theResult = kWFStoreSyncTime;
			KTRACE( " -> kWFStoreSyncTime" );
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFStoreSyncTime( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFStoreSyncTime(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFStoreSyncTime ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDCurrentTime )
	{
		// Première soupe.
		mSoupIndex = -1;
		theResult = SyncNextSoup();

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFSoupInfo( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFSoupInfo(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFSoupInfo ->" );
	EState theResult = mState;

	if ( inCommand->GetCommand() == TDCLDockCommand::kDSoupInfo )
	{
		TDCLDockCmdNSObject* theSoupInfoCmd = (TDCLDockCmdNSObject*) inCommand;
		mSoupInfo = theSoupInfoCmd->GetObject();
		mSoupName = &mSoupInfo.ToFrame().Get( "name" ).ToString();
		
		if (mAddingEntries)
		{
			TDCLNSRef theNewEntry = mCurrentSourceEngine->GetNextNewEntry();
			if (theNewEntry.IsNIL())
			{
				theResult = SyncNextSoup();
			} else {
				// On ajoute cette entrée.
				TDCLDockCmdNSObject theAddEntryCmd(
							TDCLDockCommand::kDAddEntry,
							theNewEntry );
				theAddEntryCmd.SendCommand( GetLink()->GetPipe() );
				theResult = kWFAddedID;
				KTRACE( " -> kWFAddedID" );
			}
		} else {
			// Récupération de la liste des entrées des données de synchronisation.
			mSyncDataEntriesArray = nil;
			
			KUInt32 nbSoups = mSyncDataSoupsArray->GetLength();
			KUInt32 indexSoups;
			for (indexSoups = 0; indexSoups < nbSoups; indexSoups++)
			{
				TDCLNSFrame& theSyncDataSoup =
					mSyncDataSoupsArray->Get( indexSoups ).ToFrame();
				TDCLNSString& theSyncDataSoupName =
					theSyncDataSoup.Get( "name" ).ToString();
				if (mSoupName->StrEqual( theSyncDataSoupName ))
				{
					mSyncDataEntriesArray =
						&theSyncDataSoup.Get( "entries" ).ToArray();
					break;
				}
			}
			
			if (mSyncDataEntriesArray == nil)
			{
				// Soupe non trouvée.
				// Création de la structure.
				TDCLNSRef theSyncDataSoupFrameRef = TDCLNSRef::MakeFrame();
				TDCLNSFrame& theSyncDataSoupFrame =
						theSyncDataSoupFrameRef.ToFrame();
				theSyncDataSoupFrame.Set( "name", mSoupName );
				TDCLNSRef theSyncDataSoupEntries = TDCLNSRef::MakeArray();
				theSyncDataSoupFrame.Set( "entries", theSyncDataSoupEntries );
				
				mSyncDataEntriesArray = &theSyncDataSoupEntries.ToArray();
			}
			
			// On demande la liste des entrées au Newton.
			TDCLDockCmdNoData theGetSoupsCmd( TDCLDockCommand::kDGetSoupIDs );
			theGetSoupsCmd.SendCommand( GetLink()->GetPipe() );

			theResult = kWFSoupIDs;
			KTRACE( " -> kWFSoupIDs" );
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFSoupIDs( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFSoupIDs(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFSoupIDs ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDSoupIDs )
	{
		TDCLDockCmdArray* theSoupIDsCmd = (TDCLDockCmdArray*) inCommand;
		// On copie le tableau avec les entrées.
		mEntriesCount = theSoupIDsCmd->GetCount();
		mEntries = TDCLNSRef::MakeArray( mEntriesCount );
		mEntriesArray = &mEntries.ToArray();
		KUInt32 indexEntry;
		for (indexEntry = 0; indexEntry < mEntriesCount; indexEntry++)
		{
			TDCLNSRef theEntryFrameRef = TDCLNSRef::MakeFrame();
			TDCLNSFrame& theEntryFrame = theEntryFrameRef.ToFrame();
			TDCLNSRef theEntryID = TDCLNSRef::MakeInt(
									(KSInt32)
										theSoupIDsCmd->Get( indexEntry ) );
			theEntryFrame.Set( "id", theEntryID );
			theEntryFrame.Set( "changed", TDCLNSRef::kNILREF );
			theEntryFrame.Set( "key", GetEntryKeyFromSyncData( theEntryID ) );
			mEntriesArray->Set( indexEntry, theEntryFrameRef );
		}
		
		if (!mCompleteSync)
		{
			// On demande la liste des entrées qui ont changé.
			TDCLDockCmdNoData theGetSoupsCmd( TDCLDockCommand::kDGetChangedIDs );
			theGetSoupsCmd.SendCommand( GetLink()->GetPipe() );

			theResult = kWFChangedIDs;
			KTRACE( " -> kWFChangedIDs" );
		} else {
			// Entrée suivante.
			mEntryIndex = -1;
			theResult = SyncNextEntry();
		}

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFChangedIDs( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFChangedIDs(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFChangedIDs ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDChangedIDs )
	{
		TDCLDockCmdArray* theChangedIDsCmd = (TDCLDockCmdArray*) inCommand;
		// Ajout de l'information de modification dans le tableau des entrées.
		KUInt32 indexEntry;
		KUInt32 nbChanged = theChangedIDsCmd->GetCount();
		for (indexEntry = 0; indexEntry < mEntriesCount; indexEntry++)
		{
			TDCLNSFrame& theEntryFrame =
				mEntriesArray->Get( indexEntry ).ToFrame();

			KSInt32 theID = theEntryFrame.Get( "id" ).ToInt();

			KUInt32 indexChanged;
			for (indexChanged = 0; indexChanged < nbChanged; indexChanged++)
			{
				if ((KSInt32) theChangedIDsCmd->Get( indexChanged ) == theID)
				{
					theEntryFrame.Set( "changed", TDCLNSRef::kTRUEREF );
					break;
				}
			}
		}

		// Entrée suivante.
		mEntryIndex = -1;
		theResult = FetchNextEntry();

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFEntry( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFEntry(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFEntry ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDEntry )
	{
		// Récupération de l'entrée.
		TDCLDockCmdNSObject* theEntryCmd = (TDCLDockCmdNSObject*) inCommand;
		TDCLNSRef theEntry = theEntryCmd->GetObject();
		
		// Filtrage.
		if (mCurrentSourceEngine->FilterEntry(
									*mSoupName, theEntry ))
		{
			// Récupération/mise à jour de la clé.
			TDCLNSRef theKey =
					mCurrentSourceEngine->GetEntryKey( *mSoupName, theEntry );
			
			if (!mCurrentEntry->Get( "key" ).IsNIL())
			{
				if (mCurrentSourceEngine->CompareKeys(
							theKey,
							mCurrentEntry->Get( "key" ) ) != 0)
				{
					// La clé a changé.
					// Il faudra supprimer l'ancien élément.
					mCurrentEntry->Set( "newKey", theKey );
					mCurrentEntry->Set( "new", TDCLNSRef::kTRUEREF );
				}
			} else {
				mCurrentEntry->Set( "key", theKey );
			}
			
			// Enregistrement de l'entrée.
			mCurrentEntry->Set( "data", theEntry );
		} else {
			// La source n'est pas intéressée par cette entrée.
			// Suppression du tableau.
			mCurrentEntry = nil;
			mEntriesArray->Set( (KUInt32) mEntryIndex, TDCLNSRef::kNILREF );
		}
		
		// Entrée suivante.
		theResult = FetchNextEntry();

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFSetStoreToDefaultResult( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFSetStoreToDefaultResult(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFSetStoreToDefaultResult ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDResult )
	{
		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResultValue = (KSInt32) theResultCmd->GetLong();
		if (theResultValue)
		{
			// On relance l'erreur reçue du Newton.
			throw DCLNewton( theResultValue );
		}

		// On commence par la première soupe.
		mSoupIndex = -1;

		theResult = SyncNextSoup();

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFModifiedResult( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFModifiedResult(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFModifiedResult ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDResult )
	{
		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResultValue = (KSInt32) theResultCmd->GetLong();
		if (theResultValue)
		{
			// On relance l'erreur reçue du Newton.
			throw DCLNewton( theResultValue );
		}

		// Entrée suivante.
		theResult = SyncNextEntry();

		*outProcessed = true;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromWFAddedID( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromWFAddedID(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kWFAddedID ->" );
	EState theResult = mState;
	
	if ( inCommand->GetCommand() == TDCLDockCommand::kDAddedID )
	{
		TDCLNSRef theNewEntry = mCurrentSourceEngine->GetNextNewEntry();
		if (theNewEntry.IsNIL())
		{
			theResult = SyncNextSoup();
		} else {
			// On ajoute cette entrée.
			TDCLDockCmdNSObject theAddEntryCmd(
						TDCLDockCommand::kDAddEntry,
						theNewEntry );
			theAddEntryCmd.SendCommand( GetLink()->GetPipe() );
			theResult = kWFAddedID;
			KTRACE( " -> kWFAddedID" );
		}

		*outProcessed = true;
	} else if ( inCommand->GetCommand() == TDCLDockCommand::kDResult ) {
		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResultValue = (KSInt32) theResultCmd->GetLong();
		// On relance l'erreur reçue du Newton.
		throw DCLNewton( theResultValue );
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * TransitionFromInterrupted( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::TransitionFromInterrupted(
							TDCLDockCommand* /* inCommand */,
							Boolean* outProcessed )
{
	KTRACE( "Transition: kInterrupted ->" );
	
	TDCLDockCmdNoData theOpCompleteCmd( TDCLDockCommand::kDOperationDone );
	theOpCompleteCmd.SendCommand( GetLink()->GetPipe() );
	KTRACE( " -> kEnd" );

	return kEnd;
}

// -------------------------------------------------------------------------- //
//  * DoSynchronizePatches( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLSyncEngine::DoSynchronizePatches( void )
{
	KTRACE( "DoSynchronizePatches" );
	return false;
}

// -------------------------------------------------------------------------- //
//  * DoSynchronizePackages( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLSyncEngine::DoSynchronizePackages( void )
{
	KTRACE( "DoSynchronizePackages" );
	return false;
}

// -------------------------------------------------------------------------- //
//  * ResetSourceCursor( void )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::ResetSourceCursor( void )
{
	KTRACE( "ResetSourceCursor" );
}

// -------------------------------------------------------------------------- //
//  * GetNextSourceEngine( void )
// -------------------------------------------------------------------------- //
TDCLSyncSourceEngine*
TDCLSyncEngine::GetNextSourceEngine( void )
{
	KTRACE( "GetNextSourceEngine" );
	return nil;
}

// -------------------------------------------------------------------------- //
//  * SyncProgress( EProgressState, TDCLSyncSourceEngine*, KUInt32, ... )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::SyncProgress(
					EProgressState /* inCurrentState */,
					TDCLSyncSourceEngine* /* inCurrentSource */,
					KUInt32 /* inStoreIndex */,
					KUInt32 /* inStoreCount */,
					KUInt32 /* inEntryIndex */,
					KUInt32 /* inEntryCount */ )
{
	KTRACE( "SyncProgress" );
}

// -------------------------------------------------------------------------- //
//  * SyncNextSource( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::SyncNextSource( void )
{
	KTRACE( "SyncNextSource" );
	EState theResult = mState;
	
	// Récupération de la source suivante.
	do {
		mCurrentSourceEngine = GetNextSourceEngine();
		
		if (mCurrentSourceEngine == nil)
		{
			// On a fini avec toutes les sources.
			theResult = kEnd;
			
			// Contrairement à la documentation, le Newton ne comprend pas
			// la commande kDSyncResults, donc on envoie kDOperationDone.
			TDCLDockCmdNoData theOpCompleteCmd(
									TDCLDockCommand::kDOperationDone );
			theOpCompleteCmd.SendCommand( GetLink()->GetPipe() );
			break;
		}
		
		// Initialisation de la source.
		TDCLNSRef theSourceVersion =
			TDCLNSRef::MakeInt(
					(KSInt32) mCurrentSourceEngine->Init( mNewtonTime ) );

		// Récupération de la liste des soupes gérées par la source.
		mSoups = mCurrentSourceEngine->GetSoupNames();
		mSoupsArray = &mSoups.ToArray();
		mSoupsCount = (KSInt32) mSoupsArray->GetLength();
		
		if (mSoupsCount > 0)
		{
			Boolean canProceedWithSource = true;

			// Vérification qu'elle colle avec les desiderata de l'utilisateur.
			if (!mSyncAll)
			{
#warning handle synchronization preferences.
				// continue
			}
			
			if (canProceedWithSource)
			{
				// Récupération des données de synchronisation pour
				// cette soupe.
				mSyncData = mCurrentSourceEngine->GetSyncEngineData();
				
				if (!mSyncData.IsNIL())
				{
					TDCLNSFrame& theSyncDataFrame = mSyncData.ToFrame();
					
					// Vérification de la version.
					if (theSyncDataFrame.Get( "formatMajor" )
							!= TDCLNSRef::MakeInt( 1 ))
					{
						throw DCLNotImplementedError;
					}

					// Ainsi que celle de la source.
					if (theSyncDataFrame.Get( "sourceVersion" )
							!= theSourceVersion)
					{
						mSyncData = TDCLNSRef::kNILREF;
					}
				}
				
				if (mSyncData.IsNIL())
				{
					// Création des données de synchronisation.
					CreateSyncData( theSourceVersion );
				} else {
					LookForCurrentNewtonInSyncData();
				}
				
				// On commence par la première soupe.
				mSoupIndex = -1;
				
				// On n'ajoute pas encore d'entrées.
				mAddingEntries = false;
				
				// On commence par le premier magasin.
				mStoreIndex = 0;
				
				// Récupération dudit magasin et on demande au Newton de
				// changer.
				TDCLDockCmdNSObject theSetStoreCmd(
										TDCLDockCommand::kDSetCurrentStore,
										mStores->Get( mStoreIndex ));
				theSetStoreCmd.SendCommand( GetLink()->GetPipe() );
				
				theResult = kWFSetStoreResult2;
				KTRACE( " -> kWFSetStoreResult2" );

				// Sortie.
				break;
			}
		}
	} while (true);
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SyncNextSoup( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::SyncNextSoup( void )
{
	KTRACE( "SyncNextSoup" );
	EState theResult = mState;

	// Récupération de la prochaine soupe.
	while (++mSoupIndex < mSoupsCount)
	{
		TDCLNSString& theSoupName =
				mSoupsArray->Get((KUInt32) mSoupIndex).ToString();

		// On synchronise cette soupe.
		TDCLDockCmdCString theSetSoupGetInfoCmd(
				TDCLDockCommand::kDSetSoupGetInfo, theSoupName );
		theSetSoupGetInfoCmd.SendCommand( GetLink()->GetPipe() );
	
		theResult = kWFSoupInfo;
		KTRACE( " -> kWFSoupInfo" );
		break;
	}

	if (mSoupIndex == mSoupsCount)
	{
		if (mAddingEntries)
		{
			// La synchronisation est terminée.
			// Passage à la source suivante.
			theResult = SyncNextSource();
		} else {
			// On passe au magasin suivant.
			mStoreIndex++;
			
			if (mStoreIndex < mStoresCount)
			{
				TDCLDockCmdNSObject theSetStoreCmd(
									TDCLDockCommand::kDSetCurrentStore,
									mStores->Get( mStoreIndex ));
				theSetStoreCmd.SendCommand( GetLink()->GetPipe() );
			
				theResult = kWFSetStoreResult2;
				KTRACE( " -> kWFSetStoreResult2" );
			} else {
				// On passe à l'ajout des entrées sur le magasin par défaut.
				mAddingEntries = true;
				TDCLDockCmdNoData theSetStoreCmd(
									TDCLDockCommand::kDSetStoreToDefault );
				theSetStoreCmd.SendCommand( GetLink()->GetPipe() );
			
				theResult = kWFSetStoreToDefaultResult;
				KTRACE( " -> kWFSetStoreToDefaultResult" );
			}
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * FetchNextEntry( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::FetchNextEntry( void )
{
	KTRACE( "FetchNextEntry" );
	EState theResult = mState;
	
	while (((KUInt32) ++mEntryIndex) < mEntriesCount)
	{
		mCurrentEntry = &mEntriesArray->Get( (KUInt32) mEntryIndex ).ToFrame();

		// On récupère l'entrée (les données), uniquement si l'entrée a été
		// modifiée ou si elle n'était pas présente ou si on fait une
		// synchronisation complète.
		Boolean isNewEntry = mCurrentEntry->Get( "key" ).IsNIL();
		mCurrentEntry->Set( "new", TDCLNSRef::MakeBoolean( isNewEntry ));

		if (mCompleteSync
			|| (isNewEntry)
			|| (!mCurrentEntry->Get( "changed" ).IsNIL()))
		{
			KSInt32 theID = mCurrentEntry->Get( "id" ).ToInt();
			
			TDCLDockCmdSingleLong theReturnEntryCmd(
							TDCLDockCommand::kDReturnEntry,
							(KUInt32) theID );
			theReturnEntryCmd.SendCommand( GetLink()->GetPipe() );
			theResult = kWFEntry;
			KTRACE( " -> kWFEntry" );
			break;
		} else {
			mCurrentEntry->Set( "data", TDCLNSRef::kNILREF );
		}
	}
	
	if (((KUInt32) mEntryIndex) == mEntriesCount)
	{
		// On a récupéré toutes les entrées.
		// On passe à la modification des entrées
		theResult = SyncEntries();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SyncEntries( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::SyncEntries( void )
{
	KTRACE( "SyncNextEntry" );

#warning handle multiple keys

#warning optimize by sorting.
	KUInt32 indexEntries = 0;
	for (indexEntries = 0; indexEntries < mEntriesCount; indexEntries++)
	{
	}

	mEntryIndex = -1;
	return SyncNextEntry();
}

// -------------------------------------------------------------------------- //
//  * SyncNextEntry( void )
// -------------------------------------------------------------------------- //
TDCLSyncEngine::EState
TDCLSyncEngine::SyncNextEntry( void )
{
	KTRACE( "SyncNextEntry" );
	EState theResult = mState;
	
	while (((KUInt32) ++mEntryIndex) < mEntriesCount)
	{
		TDCLNSRef theCurrentEntryRef =
			mEntriesArray->Get( (KUInt32) mEntryIndex );

		if (!theCurrentEntryRef.IsNIL())
		{
			// L'entrée est toujours présente (i.e. a été retenue par la
			// source).

			mCurrentEntry = &theCurrentEntryRef.ToFrame();
			TDCLNSRef theEntry = mCurrentEntry->Get( "data" );
			if (!theEntry.IsNIL())
			{
				TDCLNSFrame& theEntryFrame = theEntry.ToFrame();
				
				// Synchronisation.
				TDCLSyncSourceEngine::ESyncModification theModif;
				TDCLNSRef theKey = mCurrentEntry->Get( "key" );
				
				if (mCompleteSync)
				{	
					theModif = TDCLSyncSourceEngine::kCompleteSync;
				} else if (mCurrentEntry->Get( "new" ).IsNIL()) {
					theModif = TDCLSyncSourceEngine::kModifiedEntry;
				} else {
					theModif = TDCLSyncSourceEngine::kNewEntry;
					
					if (mCurrentEntry->HasSlot( "newKey" ))
					{
						theKey = mCurrentEntry->Get( "newKey" );
					}
				}
				
				TDCLSyncSourceEngine::ESyncResult theSyncResult;
				
				// à corriger
				theSyncResult = mCurrentSourceEngine->SyncNewtonEntry(
									theModif,
									0,
									*mSoupName,
									theKey,
									theEntry );
				
				Boolean stateChanged = false;
				switch( theSyncResult )
				{
					case TDCLSyncSourceEngine::kModified:
						{
							TDCLDockCmdNSObject theChangedCmd(
										TDCLDockCommand::kDChangedEntry,
										theEntry );
							theChangedCmd.SendCommand( GetLink()->GetPipe() );
							theResult = kWFModifiedResult;
							KTRACE( " -> kWFModifiedResult" );
							stateChanged = true;
						}
						break;
					
					case TDCLSyncSourceEngine::kUnchanged:
						// Passage à l'entrée suivante.
						break;

					case TDCLSyncSourceEngine::kDeleted:
						{
							KUInt32 theID =
								(KUInt32) theEntryFrame.Get( "id" ).ToInt();
							TDCLDockCmdArray theDeleteCommand(
								TDCLDockCommand::kDDeleteEntries, &theID, 1 );
							theDeleteCommand.SendCommand(
								GetLink()->GetPipe() );
							theResult = kWFModifiedResult;
							KTRACE( " -> kWFModifiedResult" );
							stateChanged = true;
						}
						break;
				}
				
				if (stateChanged)
				{
					break;
				}
			}
		}
	}
	
	if (((KUInt32) mEntryIndex) == mEntriesCount)
	{
		// On a synchronisé toutes les entrées sur le Newton.
		// On passe à l'ajout des nouvelles entrées.		
#warning TODO: add entries
		// Soupe suivante.
		theResult = SyncNextSoup();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * NormalizeAddress( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLSyncEngine::NormalizeAddress( const TDCLNSRef& inOriginalEmail )
{
	TDCLNSRef theResult = inOriginalEmail;
	if (inOriginalEmail.IsString())
	{
		TDCLNSRef theClass = inOriginalEmail.ClassOf();
		if ((theClass != KDCLSYM::kSYMstring_email)
			&& (theClass.IsSymbol())
			&& (theClass.ToSymbol().IsSubClass(KDCLSYM::kSYMstring_email)))
		{
			// Traduction à la mimine pour l'instant.
			if (theClass == TDCLNSRef::MakeSymbol("string.email.netcourrier.com"))
			{
				theResult = inOriginalEmail & "@netcourrier.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.free.fr")) {
				theResult = inOriginalEmail & "@free.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.yahoo.fr")) {
				theResult = inOriginalEmail & "@yahoo.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.teaser.fr")) {
				theResult = inOriginalEmail & "@teaser.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.ifrance.com")) {
				theResult = inOriginalEmail & "@ifrance.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.infonie.fr")) {
				theResult = inOriginalEmail & "@infonie.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.bigfoot.com")) {
				theResult = inOriginalEmail & "@bigfoot.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.cybercable.fr")) {
				theResult = inOriginalEmail & "@cybercable.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.dotcom.fr")) {
				theResult = inOriginalEmail & "@dotcom.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.freesbee.fr")) {
				theResult = inOriginalEmail & "@freesbee.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.club-internet.fr")) {
				theResult = inOriginalEmail & "@club-internet.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.libertysurf.fr")) {
				theResult = inOriginalEmail & "@libertysurf.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.skynet.be")) {
				theResult = inOriginalEmail & "@skynet.be";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.hotmail.com")) {
				theResult = inOriginalEmail & "@hotmail.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.online.fr")) {
				theResult = inOriginalEmail & "@online.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.wanadoo.fr")) {
				theResult = inOriginalEmail & "@wanadoo.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.freesurf.fr")) {
				theResult = inOriginalEmail & "@freesurf.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.mac.com")) {
				theResult = inOriginalEmail & "@mac.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.chez.com")) {
				theResult = inOriginalEmail & "@chez.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.noos.fr")) {
				theResult = inOriginalEmail & "@noos.fr";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.caramail.com")) {
				theResult = inOriginalEmail & "@caramail.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.eworld")) {
				theResult = inOriginalEmail & "@eworld.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.mcimail")) {
				theResult = inOriginalEmail & "@mcimail.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.attmail")) {
				theResult = inOriginalEmail & "@attmail.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.aol")) {
				theResult = inOriginalEmail & "@aol.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.compuserve")) {
				// incorrect, il faudrait remplacer les , par des points.
				theResult = inOriginalEmail & "@compuserve.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.easylink")) {
				theResult = inOriginalEmail & "@eln.attmail.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.prodigy")) {
				theResult = inOriginalEmail & "@prodigy.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.genie")) {
				theResult = inOriginalEmail & "@genie.geis.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.delphi")) {
				theResult = inOriginalEmail & "@delphi.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.msn")) {
				theResult = inOriginalEmail & "@msn.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.interchange")) {
				theResult = inOriginalEmail & "@ichange.com";
			} else if (theClass == TDCLNSRef::MakeSymbol("string.email.radiomail")) {
				theResult = inOriginalEmail & "@radiomail.net";
			}
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetEmailSystemName( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLSyncEngine::GetEmailSystemName( const TDCLNSRef& inEmailAddress )
{
	TDCLNSRef theResult = TDCLNSRef::kNILREF;
	TDCLNSRef theClass = inEmailAddress.ClassOf();
	if ((theClass.IsSymbol())
		&& (theClass.ToSymbol().IsSubClass(KDCLSYM::kSYMstring_email)))
	{
		// Traduction à la mimine pour l'instant.
		if (theClass == TDCLNSRef::MakeSymbol("string.email.internet"))
		{
			theResult = TDCLNSRef::MakeString("Internet");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.netcourrier.com"))
		{
			theResult = TDCLNSRef::MakeString("NetCourrier");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.free.fr")) {
			theResult = TDCLNSRef::MakeString("Free");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.yahoo.fr")) {
			theResult = TDCLNSRef::MakeString("Yahoo Fr");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.teaser.fr")) {
			theResult = TDCLNSRef::MakeString("Teaser");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.ifrance.com")) {
			theResult = TDCLNSRef::MakeString("iFrance");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.infonie.fr")) {
			theResult = TDCLNSRef::MakeString("Infonie");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.bigfoot.com")) {
			theResult = TDCLNSRef::MakeString("BigFoot");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.cybercable.fr")) {
			theResult = TDCLNSRef::MakeString("Cybercable");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.dotcom.fr")) {
			theResult = TDCLNSRef::MakeString("DotCom");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.freesbee.fr")) {
			theResult = TDCLNSRef::MakeString("Freesbee");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.club-internet.fr")) {
			theResult = TDCLNSRef::MakeString("Club-Internet");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.libertysurf.fr")) {
			theResult = TDCLNSRef::MakeString("LibertySurf");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.skynet.be")) {
			theResult = TDCLNSRef::MakeString("SkyNet Be");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.hotmail.com")) {
			theResult = TDCLNSRef::MakeString("Hotmail");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.online.fr")) {
			theResult = TDCLNSRef::MakeString("Online");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.wanadoo.fr")) {
			theResult = TDCLNSRef::MakeString("Wanadoo");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.freesurf.fr")) {
			theResult = TDCLNSRef::MakeString("Freesurf");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.mac.com")) {
			theResult = TDCLNSRef::MakeString("Mac");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.chez.com")) {
			theResult = TDCLNSRef::MakeString("Chez");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.noos.fr")) {
			theResult = TDCLNSRef::MakeString("Noos");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.caramail.com")) {
			theResult = TDCLNSRef::MakeString("Caramail");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.eworld")) {
			theResult = TDCLNSRef::MakeString("eWorld");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.mcimail")) {
			theResult = TDCLNSRef::MakeString("MCI Mail");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.attmail")) {
			theResult = TDCLNSRef::MakeString("AT&T Mail");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.aol")) {
			theResult = TDCLNSRef::MakeString("America Online");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.compuserve")) {
			// incorrect, il faudrait remplacer les , par des points.
			theResult = TDCLNSRef::MakeString("CompuServe");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.easylink")) {
			theResult = TDCLNSRef::MakeString("EasyLink");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.prodigy")) {
			theResult = TDCLNSRef::MakeString("Prodigy");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.genie")) {
			theResult = TDCLNSRef::MakeString("GEnie");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.delphi")) {
			theResult = TDCLNSRef::MakeString("Delphi");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.msn")) {
			theResult = TDCLNSRef::MakeString("Network");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.interchange")) {
			theResult = TDCLNSRef::MakeString("Interchange");
		} else if (theClass == TDCLNSRef::MakeSymbol("string.email.radiomail")) {
			theResult = TDCLNSRef::MakeString("RadioMail");
		} else {
			theResult = TDCLNSRef::MakeString("E-Mail");
		}
	}
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateCurrentNewtonFrame( void )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLSyncEngine::CreateCurrentNewtonFrame( void )
{
	KTRACE( "CreateCurrentNewtonFrame" );
	TDCLNSRef theResult = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theNewtonFrame = theResult.ToFrame();
	TDCLFullDockLink* theLink = GetLink();
	
	KUInt32 theNewtonID = theLink->GetNewtonID();
	theNewtonFrame.Set( "newtonID",
			TDCLNSRef::MakeBinary(
				&theNewtonID,
				sizeof( theNewtonID ) ) );

	theNewtonFrame.Set( "internalStoreSig",
			TDCLNSRef::MakeInt(
				(KSInt32) theLink->GetInternalStoreSig() ) );

	TDCLNSRef theStoresArray = TDCLNSRef::MakeArray();
	theNewtonFrame.Set( "stores", theStoresArray );

	// On enregister le pointeur sur la liste des magasins.
	mSyncDataStoresArray = &theStoresArray.ToArray();
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * CreateSyncData( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::CreateSyncData( const TDCLNSRef& inSourceVersion )
{
	KTRACE( "CreateSyncData" );
	mSyncData = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theSyncDataFrame = mSyncData.ToFrame();

	// Ajout de la version de TDCLSyncEngine.
	theSyncDataFrame.Set( "formatMajor", TDCLNSRef::MakeInt( 1 ) );
	theSyncDataFrame.Set( "formatMinor", TDCLNSRef::MakeInt( 0 ) );

	// Version de la source.
	theSyncDataFrame.Set( "sourceVersion", inSourceVersion );
	
	// Tableau avec les Newtons.
	TDCLNSRef theNewtonsArrayRef = TDCLNSRef::MakeArray();
	theSyncDataFrame.Set( "newtons", theNewtonsArrayRef );
	
	// Ajout des informations sur le Newton connecté.
	theNewtonsArrayRef.ToArray().Add( CreateCurrentNewtonFrame() );
}

// -------------------------------------------------------------------------- //
//  * LookForCurrentNewtonInSyncData( void )
// -------------------------------------------------------------------------- //
void
TDCLSyncEngine::LookForCurrentNewtonInSyncData( void )
{
	KTRACE( "LookForCurrentNewtonInSyncData" );

	// Recherche du Newton courant.
	TDCLFullDockLink* theLink = GetLink();
	KUInt32 theNewtonID = theLink->GetNewtonID();
	KUInt32 theStoreSignature = theLink->GetInternalStoreSig();
	
	TDCLNSArray& theNewtonsArray =
		mSyncData.ToFrame().Get( "newtons" ).ToArray();
	
	Boolean foundNewton = false;
	
	KUInt32 nbNewtons = theNewtonsArray.GetLength();
	KUInt32 indexNewtons;
	for (indexNewtons = 0; indexNewtons < nbNewtons; indexNewtons++)
	{
		TDCLNSFrame& theNewtonFrame = 
			theNewtonsArray.Get( indexNewtons ).ToFrame();
		
		if (
			((KUInt32) theNewtonFrame.Get( "internalStoreSig" ).ToInt())
				== theStoreSignature)
		{
			TDCLNSBinary& theNewtonIDBinary =
				theNewtonFrame.Get( "newtonID" ).ToBinary();
			if ((theNewtonIDBinary.GetLength() == sizeof( theNewtonID ))
				&& (*((KUInt32*) theNewtonIDBinary.GetPointer())
					== theNewtonID ))
			{
				// Bingo.
				foundNewton = true;
				mSyncDataStoresArray =
					&theNewtonFrame.Get( "stores" ).ToArray();
				break;
			}
		}
	} // for (indexNewtons = 0; ...
	
	if (!foundNewton)
	{
		// Ceci modifie mSyncDataStoresArray.
		theNewtonsArray.Add( CreateCurrentNewtonFrame() );
	}
}

// -------------------------------------------------------------------------- //
//  * GetCurrentStoreFrame( KUInt32* )
// -------------------------------------------------------------------------- //
TDCLNSFrame&
TDCLSyncEngine::GetCurrentStoreFrame( KUInt32* outLastSyncDate /* = nil */ )
{
	KTRACE( "GetCurrentStoreFrame" );
	TDCLNSRef theSignature =
		mStores->Get( mStoreIndex ).ToFrame().Get( "signature" );

	KUInt32 nbStores = mSyncDataStoresArray->GetLength();
	KUInt32 indexStores;
	for (indexStores = 0; indexStores < nbStores; indexStores++)
	{
		TDCLNSFrame& theStoreFrame =
			mSyncDataStoresArray->Get(indexStores).ToFrame();
		if (theStoreFrame.Get( "signature" ) == theSignature)
		{
			// Trouvé.
			mSyncDataSoupsArray = &theStoreFrame.Get( "soups" ).ToArray();
			if (outLastSyncDate)
			{
				TDCLNSBinary& theLastSyncDateBinary =
					theStoreFrame.Get( "lastSyncDate" ).ToBinary();
				if (theLastSyncDateBinary.GetLength() == sizeof( *outLastSyncDate ))
				{
					*outLastSyncDate =
						*((KUInt32*) theLastSyncDateBinary.GetPointer());
				}
			}
			
			return theStoreFrame;
		}
	}
	
	if (outLastSyncDate)
	{
		*outLastSyncDate = 0;
	}
	
	// Création d'une nouvelle structure.
	TDCLNSRef theStoreFrameRef = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theStoreFrame = theStoreFrameRef.ToFrame();

	theStoreFrame.Set( "lastSyncDate", TDCLNSRef::MakeInt( 0 ) );
	theStoreFrame.Set( "signature", theSignature );
	
	TDCLNSRef theSoupsArrayRef = TDCLNSRef::MakeArray();
	theStoreFrame.Set( "soups", theSoupsArrayRef );
	mSyncDataSoupsArray = &theSoupsArrayRef.ToArray();
	
	return theStoreFrame;
}

// -------------------------------------------------------------------------- //
//  * GetEntryKeyFromSyncData( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLSyncEngine::GetEntryKeyFromSyncData( const TDCLNSRef& inID )
{
	TDCLNSRef theResult;
	
	// Itération sur les entrées des données de synchronisation (qui restent).
	KUInt32 nbEntries = mSyncDataEntriesArray->GetLength();
	KUInt32 indexEntries;
	for (indexEntries = 0; indexEntries < nbEntries; indexEntries++)
	{
		TDCLNSRef theEntry = mSyncDataEntriesArray->Get( indexEntries );
		
		// L'entrée est-elle toujours là?
		if (!theEntry.IsNIL())
		{
			// Est-ce notre entrée?
			TDCLNSFrame& theEntryFrame = theEntry.ToFrame();
			if (theEntryFrame.Get( "id" ) == inID)
			{
				// Vi.
				
				// Copie de la clé.
				theResult = theEntryFrame.Get( "key" );

				// Suppression de la liste.
				mSyncDataEntriesArray->Set( indexEntries, TDCLNSRef::kNILREF );
				
				// Fini.
				break;				
			}
		}
	}
	
	return theResult;
}

// ========================================================================= //
// It is practically impossible to teach good programming style to students  //
// that have had prior exposure to BASIC: as potential programmers they are  //
// mentally mutilated beyond hope of regeneration.                           //
//               -- Edsger W. Dijkstra, SIGPLAN Notices, Volume 17, Number 5 //
// ========================================================================= //
