// ==============================
// Fichier:			TDCLLoadPkgEngine.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			10/5/2003
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
// The Original Code is TDCLLoadPkgEngine.cp.
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
// $Id: TDCLLoadPkgEngine.cp,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLLoadPkgEngine.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Package/TDCLPackage.h>

// -------------------------------------------------------------------------- //
//  * TDCLLoadPkgEngine( TDCLFullDockLink* )
// -------------------------------------------------------------------------- //
TDCLLoadPkgEngine::TDCLLoadPkgEngine( TDCLFullDockLink* inLink )
	:
		TDCLLinkEngine( inLink )
{
}

// -------------------------------------------------------------------------- //
//  * InstallPackage( TDCLFSItemRef )
// -------------------------------------------------------------------------- //
void
TDCLLoadPkgEngine::InstallPackage(
					TDCLFSItemRef inPackage )
{
	// RŽcupŽration du nom du paquet.
	TDCLPackage thePackage( inPackage );
	StartInstallingPackage( thePackage.GetPackageName() );
	
	// Envoi de la commande TDCLDockCmdLoadPkg
	{
		TDCLDockCmdLoadPkg theLoadPackageMessage( inPackage );

		theLoadPackageMessage.SendCommand(
									GetLink()->GetPipe(), ProgressFunc, this );
	}
}

// -------------------------------------------------------------------------- //
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLLoadPkgEngine::ProcessDockCommand(
							TDCLDockCommand* inCommand,
							Boolean* outProcessed )
{
	if ( inCommand->GetCommand() == TDCLDockCommand::kDResult )
	{
		// On gre la commande.		
		*outProcessed = true;

		TDCLDockCmdSingleLong* theResultCmd = (TDCLDockCmdSingleLong*) inCommand;
		KSInt32 theResult = (KSInt32) theResultCmd->GetLong();
		if (theResult)
		{
			// On relance l'erreur reue du Newton.
			throw DCLNewton( theResult );
		}
	}
	
	return false;
}

// -------------------------------------------------------------------------- //
//  * StartInstallingPackage( const KUInt16* )
// -------------------------------------------------------------------------- //
void
TDCLLoadPkgEngine::StartInstallingPackage( const KUInt16* /* inPackageName */ )
{
}

// -------------------------------------------------------------------------- //
//  * InstallationProgress( double )
// -------------------------------------------------------------------------- //
void
TDCLLoadPkgEngine::InstallationProgress( double /* inProgress */ )
{
}

// -------------------------------------------------------------------------- //
//  * ProgressFunc( void*, double )
// -------------------------------------------------------------------------- //
void
TDCLLoadPkgEngine::ProgressFunc( void* inRefCon, double inProgress )
{
	((TDCLLoadPkgEngine*) inRefCon)->InstallationProgress( inProgress );
}

// ======================================================================= //
// Real computer scientists like having a computer on their desk, else how //
// could they read their mail?                                             //
// ======================================================================= //
