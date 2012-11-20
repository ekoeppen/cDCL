// ==============================
// Fichier:			TDCLLogApplication.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			31/3/2003
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
// The Original Code is TDCLLogApplication.cp.
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
// $Id: TDCLLogApplication.cp,v 1.4 2004/11/24 14:09:00 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>

// ------------------------------------------------------------------------- //
//  * TDCLLogApplication( FILE* )
// ------------------------------------------------------------------------- //
TDCLLogApplication::TDCLLogApplication( FILE* inLogFile )
	:
		mLogFile( inLogFile )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//  * WaitConnection( TDCLServer*, TDCLCommLayer* inLayer )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::WaitConnection(
							TDCLServer* inServer,
							TDCLCommLayer* inLayer )
{
	(void) ::fprintf( mLogFile, "Attente d'une connexion (%p-%p).\n",
			inServer, inLayer );
}

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Connected( TDCLLink* inLink, TDCLCommLayer* inLayer )
{
	(void) ::fprintf( mLogFile, "[%p] Connexion Žtablie (%p)\n",
			inLink, inLayer );
}

// ------------------------------------------------------------------------- //
//  * ConnectedToNewtonDevice( TDCLLink*, const KUInt16* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::ConnectedToNewtonDevice(
									TDCLLink* inLink, const KUInt16* inName )
{
	// Conversion du nom en MacRoman
	char* theNameStr = new char [510];	// Le nom en MacRoman
	UUTF16CStr::ToISO88591( inName, (KUInt8*) theNameStr, 510 );

	(void) ::fprintf( mLogFile, "[%p] ConnectŽ sur le Newton de %s\n",
				inLink, theNameStr );
}

// ------------------------------------------------------------------------- //
//  * Disconnecting( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Disconnecting( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] DŽconnexion\n",
				inLink );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Disconnected( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] DŽconnectŽ\n", inLink );
}

// ------------------------------------------------------------------------- //
//  * ServerIsDown( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::ServerIsDown( TDCLServer* inServer )
{
	(void) ::fprintf(
			mLogFile, "Fin du serveur (%p)\n", inServer );
}

// ------------------------------------------------------------------------- //
//  * Cancel( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Cancel( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] Annulation\n", inLink );
}

// ------------------------------------------------------------------------- //
//  * Timeout( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Timeout( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] DŽpassement de temporisation\n",
				inLink );
}

// ------------------------------------------------------------------------- //
//  * AppCommandBeingProcessed( TDCLLink*, TDCLAppCommand* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::AppCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand )
{
	(void) ::fprintf(
			mLogFile,
			"[%p] Commande Application [%p] en cours d'exŽcution\n",
			inLink,
			inCommand );
}

// ------------------------------------------------------------------------- //
//  * AppCommandProcessed( TDCLLink*, TDCLAppCommand*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::AppCommandProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand,
							TDCLException* inException )
{
	if (inException)
	{
		(void) ::fprintf(
			mLogFile,
			"[%p] Problme avec la commande [%p]\n",
			inLink,
			inCommand );
	} else {
		(void) ::fprintf(
			mLogFile,
			"[%p] Commande [%p] exŽcutŽe avec succs\n",
			inLink,
			inCommand );
	}
}

// ------------------------------------------------------------------------- //
//  * DockCommandBeingProcessed( TDCLLink*, TDCLDockCommand*, TDCLLinkEngine* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::DockCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLDockCommand* inCommand,
							TDCLLinkEngine* inEngine )
{
	(void) ::fprintf(
		mLogFile,
		"[%p] Commande Dock [%p] (moteur: [%p]) en cours d'exŽcution\n",
		inLink,
		inCommand,
		inEngine );
}

// ------------------------------------------------------------------------- //
//  * DockCommandProcessed( TDCLLink*, TDCLLinkEngine*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::DockCommandProcessed(
							TDCLLink* inLink,
							TDCLLinkEngine* inEngine,
							TDCLException* inException )
{
	if (inException)
	{
		(void) ::fprintf(
			mLogFile,
			"[%p] Problme avec une commande dock (moteur [%p])\n",
			inLink,
			inEngine );
	} else {
		(void) ::fprintf(
			mLogFile,
			"[%p] Commande dock (moteur [%p]) exŽcutŽe avec succs\n",
			inLink,
			inEngine );
	}
}

// ------------------------------------------------------------------------- //
//  * HandleServerException( TDCLServer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::HandleServerException(
							TDCLServer* inServer,
							TDCLException* inException )
{
	(void) ::fprintf( mLogFile,
				"Exception au niveau du serveur [%p]!\n",
				inServer );

	LogException( inException );
}

// ------------------------------------------------------------------------- //
//  * HandleCommLayerException( TDCLCommLayer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException )
{
	(void) ::fprintf( mLogFile,
				"Exception au niveau de la couche de communication [%p]!\n",
				inCommLayer );

	LogException( inException );
}

// ------------------------------------------------------------------------- //
//  * HandleLinkException( TDCLLink*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::HandleLinkException(
							TDCLLink* inLink,
							TDCLException* inException )
{
	(void) ::fprintf( mLogFile,
				"Exception au niveau du lien [%p]!\n",
				inLink );

	LogException( inException );
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::HandleException( TDCLException* inException )
{
	(void) ::fprintf( mLogFile, "Exception non interceptŽe!\n" );

	LogException( inException );
}

// ------------------------------------------------------------------------- //
//  * LogException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::LogException( TDCLException* inException )
{
	if (inException)
	{
#if kDCLDebugFILEandLINE
		(void) ::fprintf( mLogFile, ">> %s (code: %i, erreur: %i) [%s:%u]\n",
						inException->GetExceptionName(),
						(int) inException->GetExCode(),
						(int) inException->GetError(),
						inException->GetFileName(),
						(unsigned int) inException->GetLine() );
#else
		(void) ::fprintf( mLogFile, ">> %s (code: %i, erreur: %i)\n",
						inException->GetExceptionName(),
						(int) inException->GetExCode(),
						(int) inException->GetError() );
#endif
	} else {
		(void) ::fprintf( mLogFile, ">> Exception inconnue\n" );
	}
}

// ======================================================================== //
// Do you suffer painful elimination?                                       //
//                 -- Don Knuth, "Structured Programming with Gotos"        //
//                                                                          //
// Do you suffer painful recrimination?                                     //
//                 -- Nancy Boxer, "Structured Programming with Come-froms" //
//                                                                          //
// Do you suffer painful illumination?                                      //
//                 -- Isaac Newton, "Optics"                                //
//                                                                          //
// Do you suffer painful hallucination?                                     //
//                 -- Don Juan, cited by Carlos Casteneda                   //
// ======================================================================== //
