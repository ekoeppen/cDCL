// ==============================
// Fichier:			TDCLFullDockLink.cp
// Projet:			Desktop Connection Library
//
// Créé le:			22/06/2002
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
// The Original Code is TDCLFullDockLink.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLFullDockLink.cp,v 1.6 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLFullDockLink.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// K
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Link/App_Commands/TDCLAppCmdFile.h>
#include <DCL/Link/App_Commands/TDCLAppCmdKbdChar.h>
#include <DCL/Link/App_Commands/TDCLAppCmdString.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Link/Link_Engines/TDCLBrowseEngine.h>
#include <DCL/Link/Link_Engines/TDCLKeyboardEngine.h>
#include <DCL/Link/Link_Engines/TDCLLoadPkgEngine.h>
#include <DCL/Link/Link_Engines/TDCLSyncEngine.h>

// ------------------------------------------------------------------------- //
//  * Déverminage
// ------------------------------------------------------------------------- //

#undef KERROR_ENABLED
#if defined(error_TDCLFullDockLink) && error_TDCLFullDockLink
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLFullDockLink) && trace_TDCLFullDockLink
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLFullDockLink) && debug_TDCLFullDockLink
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

// --------------------------------------------------------------------------------	//
//  * TDCLFullDockLink( TDCLApplication*, KUInt32, KUInt32 )
// --------------------------------------------------------------------------------	//
// New link using the pipe. (now unique constructor)

TDCLFullDockLink::TDCLFullDockLink(
	TDCLApplication* inApplication,
	KUInt32 inWhatToProcess,
	KUInt32 inIcons )
	:
		TDCLDockLink( inApplication, inIcons ),
		mHandledCommandSets( inWhatToProcess ),
		mCurrentEngine( nil ),
		mDefaultBrowseEngine( nil ),
		mDefaultLoadPkgEngine( nil )
{
	// Création des moteurs par défaut.
	mDefaultBrowseEngine = new TDCLBrowseEngine( this );
	mDefaultLoadPkgEngine = new TDCLLoadPkgEngine( this );
}

// --------------------------------------------------------------------------------	//
//  * ~TDCLFullDockLink( void )
// --------------------------------------------------------------------------------	//
// Destructor.

TDCLFullDockLink::~TDCLFullDockLink( void )
{
	if (mDefaultBrowseEngine)
	{
		delete mDefaultBrowseEngine;
	}
	if (mDefaultLoadPkgEngine)
	{
		delete mDefaultLoadPkgEngine;
	}
}

// --------------------------------------------------------------------------------	//
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// --------------------------------------------------------------------------------	//
TDCLLink::EState
TDCLFullDockLink::ProcessDockCommand(
				TDCLDockCommand* inCommand,
				Boolean* outProcessed )
{
	TDCLLink::EState theResult = kRunning;	// Par défaut on reste connecté.
	Boolean processed = false;
	
	// Y a-t-il un moteur courant?
	// Si oui, on lui demande de faire le boulot.
	if (mCurrentEngine)
	{
		try {
			Boolean workIsNotOver =
					mCurrentEngine->ProcessDockCommand( inCommand, &processed );
			if (processed)
			{
				// La commande a été gérée, on le dit à l'application.
				GetApplication()->DockCommandProcessed(
						this,
						mCurrentEngine,
						nil );
			}
			if (!workIsNotOver)
			{
				// Le moteur en a terminé.
				mCurrentEngine->WorkIsOver();
				mCurrentEngine = nil;
			}
		} catch ( TDCLException& inException ) {
			// On va dire que la commande a été gérée.
			processed = true;
			GetApplication()->DockCommandProcessed(
					this,
					mCurrentEngine,
					&inException );
		}
	}
		
	if (!processed)
	{
		theResult = TDCLDockLink::ProcessDockCommand( inCommand, &processed );
	}

	// Si ni le moteur ni le lien n'ont exécuté la commande, alors on
	// n'utilise plus le moteur.
	if (!processed)
	{
		if (mCurrentEngine)
		{
			GetApplication()->DockCommandProcessed(
					this,
					mCurrentEngine,
					nil );
			mCurrentEngine->WorkIsOver();
			mCurrentEngine = nil;
		}

		switch( inCommand->GetCommand() )
		{
			case TDCLDockCommand::kDRequestToBrowse:
				if (mHandledCommandSets & kFileBrowsingSet)
				{
					// Le Newton veut butiner
					TDCLDockCmdNSObject* theRequestToBrowseCommand =
							(TDCLDockCmdNSObject*) inCommand;
					DoBrowse( theRequestToBrowseCommand->GetObject() );
					processed = true;
				}
				break;			

			case TDCLDockCommand::kDStartKeyboardPassthrough:
				if (mHandledCommandSets & kKeyboardSet)
				{
					// Le Newton veut utiliser le clavier.
					DoUseKeyboard();
					processed = true;
				}
				break;

			case TDCLDockCommand::kDSynchronize:
				if (mHandledCommandSets & kSyncSet)
				{
					// Le Newton veut synchroniser
					DoSynchronize();
					processed = true;
				}
				break;			

			case TDCLDockCommand::kDOperationCanceled:
			case TDCLDockCommand::kDOperationCanceled2:
			{
				// Envoi de la commande pour acquiescer
				TDCLDockCmdNoData theAckCmd( TDCLDockCommand::kDOpCanceledAck );
				theAckCmd.SendCommand( GetPipe() );
				processed = true;
			}
			break;

			case TDCLDockCommand::kDOpCanceledAck:
			{
				processed = true;
			}
			break;
		}
		
		// Un nouveau moteur est-il en route?
		if (mCurrentEngine)
		{
			GetApplication()->DockCommandBeingProcessed(
					this,
					inCommand,
					mCurrentEngine );
		}
	}

	*outProcessed = processed;
	
	// On appelle Idle de toute façon.
	(void) Idle();
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * ProcessAppCommand( TDCLAppCommand*, Boolean* )
// --------------------------------------------------------------------------------	//
TDCLLink::EState
TDCLFullDockLink::ProcessAppCommand(
				TDCLAppCommand* inCommand,
				Boolean* outProcessed )
{
	TDCLLink::EState theResult = kRunning;	// Par défaut, on reste connecté.
	Boolean processed = false;

	// Si on a un moteur de lien, on lui demande.
	if (mCurrentEngine)
	{
		if (!mCurrentEngine->ProcessAppCommand( inCommand, &processed ))
		{
			GetApplication()->DockCommandProcessed(
					this,
					mCurrentEngine,
					nil );
			mCurrentEngine->WorkIsOver();
			mCurrentEngine = nil;
		}
	} else {
		// Commandes exécutées lorsqu'il n'y a pas de moteur.
		switch( inCommand->GetCommandID() )
		{
			case TDCLAppCommand::kInstallPackage:
			{
				// Récupération du moteur pour l'installation
				TDCLLoadPkgEngine* theEngine = GetLoadPkgEngine();
				
				// Vérifions qu'il y a un moteur pour l'installation
				if (theEngine)
				{
					// On note le moteur.
					mCurrentEngine = theEngine;
					
					// On dit au Newton qu'on va installer un paquet.
					SendRequest( TDCLDockCommand::kDRequestToInstall );
					
					// Puis on l'installe.
					DoInstallPackage( (TDCLFile*) inCommand->GetEventData() );
					processed = true;
				}
			}
			break;
			
			case TDCLAppCommand::kStartUsingKeyboard:
			{
				// Récupération du moteur pour le clavier.
				TDCLKeyboardEngine* theEngine = GetKeyboardEngine();
				
				if (theEngine)
				{
					// On note le moteur.
					mCurrentEngine = theEngine;
					
					// On dit au Newton qu'on va utiliser le clavier.
					{
						TDCLDockCmdNoData theRequestCmd(
								TDCLDockCommand::kDStartKeyboardPassthrough );

						theRequestCmd.SendCommand( GetPipe() );
					}
					
					// Il doit répondre par kDStartKeyboardPassthrough
					TDCLDockCommand* theNewtMessage =
							TDCLDockCommand::ReceiveCommand(
										GetPipe() );
					if (theNewtMessage->GetCommand()
							== TDCLDockCommand::kDStartKeyboardPassthrough)
					{
						delete theNewtMessage;	// Plus besoin.
					} else {
						// Je ne m'attendais pas à cette commande. On dégage.
						delete theNewtMessage;
						throw DCLUnexpDockCmd;
					}
					
					// On dit au moteur qu'on commence à utiliser le clavier.
					theEngine->StartUsingKeyboard();
					
					processed = true;
				}
				// Sinon la commande est ignorée et l'application
				// recevra une erreur.
			}
			break;

			
			case TDCLAppCommand::kSynchronize:
			{
				// Vérifions qu'il y a un moteur pour la synchronisation.
				if (GetSyncEngine())
				{
					// On synchronise directement (on ne dit pas au Newton
					// qu'on va synchroniser, le MP120 n'aime pas).
					DoSynchronize();
					processed = true;
				} // Idem.
			}
			break;
		}
	}
	
	if (processed)
	{
		*outProcessed = true;
	} else {
		theResult = TDCLDockLink::ProcessAppCommand( inCommand, outProcessed );
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * IntallPackage( TDCLFSItemRef )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::InstallPackage( TDCLFSItemRef inPackage )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCmdFile(
				TDCLAppCommand::kIdleAppCommand,
				TDCLAppCommand::kInstallPackage,
				inPackage );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * Synchronize( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::Synchronize( void )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCommand(
				TDCLAppCommand::kIdleAppCommand,
				TDCLAppCommand::kSynchronize );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * StartUsingKeyboard( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::StartUsingKeyboard( void )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCommand(
				TDCLAppCommand::kIdleAppCommand,
				TDCLAppCommand::kStartUsingKeyboard );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}
	
// --------------------------------------------------------------------------------	//
//  * SendKeyboardString( const KUInt16* )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::SendKeyboardString( const KUInt16* inString )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCmdString(
				TDCLAppCommand::kKbdAppCommand,
				TDCLAppCommand::kSendString,
				inString );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}
	
// --------------------------------------------------------------------------------	//
//  * SendKeyboardChar( KUInt16, KUInt16 )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::SendKeyboardChar( KUInt16 inChar, KUInt16 inFlags )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCmdKbdChar( inChar, inFlags );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}
	
// --------------------------------------------------------------------------------	//
//  * StopUsingKeyboard( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLFullDockLink::StopUsingKeyboard( void )
{
	// Création de la commande à envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCommand(
				TDCLAppCommand::kKbdAppCommand,
				TDCLAppCommand::kEndUsingKeyboard );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connecté, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}
	
// --------------------------------------------------------------------------------	//
//  * DoInstallPackage( TDCLFSItemRef )
// --------------------------------------------------------------------------------	//
void
TDCLFullDockLink::DoInstallPackage( TDCLFSItemRef inPackage )
{
	// Récupération de l'objet pour faire la synchro.
	TDCLLoadPkgEngine* theEngine = GetLoadPkgEngine();
	
	if (theEngine == nil)
	{
		TDCLDockCmdSingleLong theResultCmd(
										TDCLDockCommand::kDResult, (KUInt32) -28031 );
		theResultCmd.SendCommand( GetPipe() );
	} else {
		mCurrentEngine = theEngine;
		theEngine->InstallPackage( inPackage );
	}

}
	
// --------------------------------------------------------------------------------	//
//  * DoSynchronize( void )
// --------------------------------------------------------------------------------	//
void
TDCLFullDockLink::DoSynchronize( void )
{
	// Récupération de l'objet pour faire la synchro.
	TDCLSyncEngine* theEngine = GetSyncEngine();
	
	if (theEngine == nil)
	{
		TDCLDockCmdSingleLong theResultCmd( TDCLDockCommand::kDResult, (KUInt32) -28031 );
		theResultCmd.SendCommand( GetPipe() );
	} else {
		mCurrentEngine = theEngine;
		theEngine->StartSynchronizing();
	}
}

// --------------------------------------------------------------------------------	//
//  * DoUseKeyboard( void )
// --------------------------------------------------------------------------------	//
void
TDCLFullDockLink::DoUseKeyboard( void )
{
	// Récupération de l'objet pour utiliser le clavier.
	TDCLKeyboardEngine* theEngine = GetKeyboardEngine();
	
	if (theEngine == nil)
	{
		TDCLDockCmdSingleLong theResultCmd( TDCLDockCommand::kDResult, (KUInt32) -28031 );
		theResultCmd.SendCommand( GetPipe() );
	} else {
		mCurrentEngine = theEngine;
		theEngine->StartUsingKeyboard();
	}
}

// --------------------------------------------------------------------------------	//
//  * SendRequest( KUInt32 inRequest )
// --------------------------------------------------------------------------------	//
void
TDCLFullDockLink::SendRequest( KUInt32 inRequest )
{
	// On dit au Newton qu'on va effectuer une opération.
	{
		TDCLDockCmdNoData theRequestCmd( inRequest );

		theRequestCmd.SendCommand( GetPipe() );
	}
		
	// Récupération du kDResult (je filtre les commandes "hello").
	KSInt32 theResult = TDCLDockCommand::GetResultCmd( GetPipe() );
	if (theResult)
	{
		// On relance l'erreur reçue du Newton.
		throw DCLNewton( theResult );
	}
}

// --------------------------------------------------------------------------------	//
//  * DoBrowse( const TDCLNSRef& )
// --------------------------------------------------------------------------------	//
void
TDCLFullDockLink::DoBrowse( const TDCLNSRef& inFileTypes )
{
	// Récupération de l'objet pour faire le butinage.
	TDCLBrowseEngine* theEngine = GetBrowseEngine();
	
	if (theEngine == nil)
	{
		TDCLDockCmdSingleLong theResultCmd( TDCLDockCommand::kDResult, (KUInt32) -28031 );
		theResultCmd.SendCommand( GetPipe() );
	} else {
		mCurrentEngine = theEngine;
		theEngine->StartBrowsing( inFileTypes );
	}
}

// --------------------------------------------------------------------------------	//
//  * GetBrowseEngine( void )
// --------------------------------------------------------------------------------	//
TDCLBrowseEngine*
TDCLFullDockLink::GetBrowseEngine( void )
{
	return mDefaultBrowseEngine;
}

// --------------------------------------------------------------------------------	//
//  * GetLoadPkgEngine( void )
// --------------------------------------------------------------------------------	//
TDCLLoadPkgEngine*
TDCLFullDockLink::GetLoadPkgEngine( void )
{
	return mDefaultLoadPkgEngine;
}

// --------------------------------------------------------------------------------	//
//  * GetKeyboardEngine( void )
// --------------------------------------------------------------------------------	//
TDCLKeyboardEngine*
TDCLFullDockLink::GetKeyboardEngine( void )
{
	return nil;
}

// --------------------------------------------------------------------------------	//
//  * GetSyncEngine( void )
// --------------------------------------------------------------------------------	//
TDCLSyncEngine*
TDCLFullDockLink::GetSyncEngine( void )
{
	return nil;
}

// --------------------------------------------------------------------------------	//
//  * GetAppCommandMask( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLFullDockLink::GetAppCommandMask( void )
{
	KUInt32 theResult;
	
	// Si on a un moteur, on lui demande.
	if (mCurrentEngine)
	{
		theResult = mCurrentEngine->GetAppCommandMask();
	} else {
		// On demande à la classe mère.
		theResult = TDCLDockLink::GetAppCommandMask();
	}
	
	return theResult;
}

// ======================================================================= //
// Real programmers don't draw flowcharts.  Flowcharts are, after all, the //
// illiterate's form of documentation.  Cavemen drew flowcharts; look how  //
// much good it did them.                                                  //
// ======================================================================= //
