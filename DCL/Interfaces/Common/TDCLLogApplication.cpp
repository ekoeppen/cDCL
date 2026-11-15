// ==============================
// Fichier:			TDCLLogApplication.cp
// Projet:			Desktop Connection Library
//
// Créé le:			31/3/2003
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
	(void) ::fprintf( mLogFile, "Waiting for connection (%p-%p).\n",
			inServer, inLayer );
}

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Connected( TDCLLink* inLink, TDCLCommLayer* inLayer )
{
	(void) ::fprintf( mLogFile, "[%p] Connection established (%p)\n",
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

	(void) ::fprintf( mLogFile, "[%p] Connected to %s's Newton\n",
				inLink, theNameStr );
}

// ------------------------------------------------------------------------- //
//  * Disconnecting( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Disconnecting( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] Disconnecting\n",
				inLink );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Disconnected( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] Disconnected\n", inLink );
}

// ------------------------------------------------------------------------- //
//  * ServerIsDown( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::ServerIsDown( TDCLServer* inServer )
{
	(void) ::fprintf(
			mLogFile, "End of server (%p)\n", inServer );
}

// ------------------------------------------------------------------------- //
//  * Cancel( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Cancel( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] Canceled\n", inLink );
}

// ------------------------------------------------------------------------- //
//  * Timeout( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::Timeout( TDCLLink* inLink )
{
	(void) ::fprintf( mLogFile, "[%p] Timeout\n", inLink );
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
			"[%p] Application command [%p] is being processed\n",
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
			"[%p] Exception with command [%p]\n",
			inLink,
			inCommand );
	} else {
		(void) ::fprintf(
			mLogFile,
			"[%p] Command [%p] succeeded\n",
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
		"[%p] Dock command [%p] (engine: [%p]) is being processed\n",
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
			"[%p] Exception with dock command (engine [%p])\n",
			inLink,
			inEngine );
	} else {
		(void) ::fprintf(
			mLogFile,
			"[%p] Dock command (engine [%p]) succeeded\n",
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
				"Exception at server level [%p]!\n",
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
				"Exception at communication layer level [%p]!\n",
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
				"Exception at link level [%p]!\n",
				inLink );

	LogException( inException );
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLLogApplication::HandleException( TDCLException* inException )
{
	(void) ::fprintf( mLogFile, "Unhandled exception!\n" );

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
		(void) ::fprintf( mLogFile, ">> %s (code: %i, error: %i) [%s:%u]\n",
						inException->GetExceptionName(),
						(int) inException->GetExCode(),
						(int) inException->GetError(),
						inException->GetFileName(),
						(unsigned int) inException->GetLine() );
#else
		(void) ::fprintf( mLogFile, ">> %s (code: %i, error: %i)\n",
						inException->GetExceptionName(),
						(int) inException->GetExCode(),
						(int) inException->GetError() );
#endif
	} else {
		(void) ::fprintf( mLogFile, ">> Unknown exception\n" );
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
