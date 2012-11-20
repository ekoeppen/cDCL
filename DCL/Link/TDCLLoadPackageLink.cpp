// ==============================
// Fichier:			TDCLLoadPackageLink.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			13/08/2001
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
// The Original Code is TDCLLoadPackageLink.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLLoadPackageLink.cp,v 1.4 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLoadPackageLink.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>

// K
#include <K/Misc/TSmartPtr.h>

// DCL
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Link/App_Commands/TDCLAppCmdFile.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// ------------------------------------------------------------------------- //
//  *ÊTDCLLoadPackageLink( TDCLApplication* )
// ------------------------------------------------------------------------- //
TDCLLoadPackageLink::TDCLLoadPackageLink(
		TDCLApplication* inApplication )
	:
		TDCLLink( inApplication, TDCLDockCommand::kLoadPackage_SType )
{
	// Gaz ˆ tous les Žtages.
}

// ------------------------------------------------------------------------- //
//  *ÊDoConnect( void )
// ------------------------------------------------------------------------- //
void
TDCLLoadPackageLink::DoConnect( void )
{
	// The TDCLLink does the first part of the job.
	
	TDCLLink::DoConnect();

	// Envoi de kDSetTimeout.
	{	
		long theTimeout = GetPipe()->GetTimeout();
		if (theTimeout < 0)
		{
			// Valeur par dŽfaut.
			theTimeout = kDefaultTimeout;
		}
		
		// CrŽation.
		TDCLDockCmdSingleLong theSetTimeoutMessage(
			TDCLDockCommand::kDSetTimeout, (KUInt32) theTimeout );

		// Envoi
		theSetTimeoutMessage.SendCommand( GetPipe() );
	}
	
	// RŽception de kDResult.
	{
		TSmartPtr<TDCLDockCommand> theNewtMessage;
		
		theNewtMessage = TDCLDockCommand::ReceiveCommand( GetPipe() );

		// VŽrification que c'est bien une commande kDResult.
		if (theNewtMessage->GetCommand() != TDCLDockCommand::kDResult)
		{
			// Duh!
			throw DCLUnexpDockCmd;
		}

		// C'est bien une structure	TDCLDockCmdSingleLong
		TDCLDockCmdSingleLong* theResult =
			(TDCLDockCmdSingleLong*) theNewtMessage.Get();
		
		// On lve une exception si le code n'est pas 0.
		KSInt32 theResultInt = (KSInt32) theResult->GetLong();
		if (theResultInt)
		{
			throw DCLNewton( theResultInt );
		}
		
		// LibŽration.
		theNewtMessage.Delete();
	}
}

// ------------------------------------------------------------------------- //
//  * ProcessAppCommand( TDCLAppCommand*, Boolean* )
// ------------------------------------------------------------------------- //
TDCLLink::EState
TDCLLoadPackageLink::ProcessAppCommand(
				TDCLAppCommand* inCommand,
				Boolean* outProcessed )
{
	TDCLLink::EState theResult = kRunning;	// Par dŽfaut, on reste connectŽ.

	if ( inCommand->GetCommandID() == TDCLAppCommand::kInstallPackage)
	{
		DoInstallPackage( (TDCLFile*) inCommand->GetEventData() );
		*outProcessed = true;
	} else {
		theResult = TDCLLink::ProcessAppCommand( inCommand, outProcessed );
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * IntallPackage( TDCLFSItemRef )
// ------------------------------------------------------------------------- //
Boolean
TDCLLoadPackageLink::InstallPackage( TDCLFSItemRef inPackage )
{
	// CrŽation de la commande ˆ envoyer.
	TDCLAppCommand* theCommand =
		new TDCLAppCmdFile(
				TDCLAppCommand::kIdleAppCommand,
				TDCLAppCommand::kInstallPackage,
				inPackage );
	
	// Envoi (si possible).
	Boolean theResult = PostAppCommand( theCommand );
	
	if (!theResult)
	{
		// On n'est pas connectŽ, donc on nettoie.
		delete theCommand;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * DoIntallPackage( TDCLFile* )
// ------------------------------------------------------------------------- //
void
TDCLLoadPackageLink::DoInstallPackage( TDCLFSItemRef inPackage )
{
	// J'envoie la commande TDCLDockCmdLoadPkg
	{
		TDCLDockCmdLoadPkg theLoadPackageMessage( inPackage );

		theLoadPackageMessage.SendCommand( GetPipe(), ProgressFunc, this );
	}

	// Je rŽcupre le rŽsultat.
	{
		TDCLDockCommand* theNewtMessage =
				TDCLDockCommand::ReceiveCommand( GetPipe() );
		if (theNewtMessage->GetCommand() == TDCLDockCommand::kDResult)
		{
			// It's a kDResult, process it.
			TDCLDockCmdSingleLong* theResultCmd =
					(TDCLDockCmdSingleLong*) theNewtMessage;
			KSInt32 theResult = (KSInt32) theResultCmd->GetLong();
			delete theNewtMessage;	// I no longer need it.
			if (theResult)
			{
				// Throw the error that the Newton sent.
				throw DCLNewton( theResult );
			}
		} else {
			// It's not what I expected. Get out.
			delete theNewtMessage;
			throw DCLUnexpDockCmd;
		}
	}
}

// ------------------------------------------------------------------------- //
//  * StartInstallingPackage( void )
// ------------------------------------------------------------------------- //
void
TDCLLoadPackageLink::StartInstallingPackage( void )
{
}

// ------------------------------------------------------------------------- //
//  * InstallationProgress( double inProgress )
// ------------------------------------------------------------------------- //
void
TDCLLoadPackageLink::InstallationProgress( double /* inProgress */ )
{
}

// ------------------------------------------------------------------------- //
//  * ProgressFunc( void*, double )
// ------------------------------------------------------------------------- //
void
TDCLLoadPackageLink::ProgressFunc( void* inRefCon, double inProgress )
{
	((TDCLLoadPackageLink*) inRefCon)->InstallationProgress( inProgress );
}

// ========================================================================= //
// At the source of every error which is blamed on the computer you will     //
// find at least two human errors, including the error of blaming it on the  //
// computer.                                                                 //
// ========================================================================= //
