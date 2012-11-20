// ==============================
// Fichier:			UDCLCTBConfig.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is UDCLCTBConfig.cp.
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
// $Id: UDCLCTBConfig.cp,v 1.5 2004/11/24 14:09:01 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/UDCLCTBConfig.h>

// MacOS
#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
#endif

#include <CommResources.h>
#include <CTBUtilities.h>
#include <Connections.h>
#include <Traps.h>

// ANSI C
#include <string.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
#endif

// K
#include <K/Misc/UPStrings.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// ------------------------------------------------------------------------- //
//  * GetCommToolConfig( char*, char* )
// ------------------------------------------------------------------------- //
// Dérivé de l'exemple GetToolConfig (et bien sûr modifié)
/*
	File:			GetToolConfig.c
	
	Description:	This q&d allows one to accumulate into a TEXT file the config strings 
				of Connection Tools that have been configured with CMChoose.

	Author:		GDG

	Copyright: 	Copyright: © 1999 by Apple Computer, Inc.
				all rights reserved.
	
	Disclaimer:	You may incorporate this sample code into your applications without
				restriction, though the sample code has been provided "AS IS" and the
				responsibility for its operation is 100% yours.  However, what you are
				not permitted to do is to redistribute the source as "DSC Sample Code"
				after having made changes. If you're going to re-distribute the source,
				we require that you make it clear in the source that the code was
				descended from Apple Sample Code, but that you've made changes.
	
	Change History (most recent first):
			6/22/99	Updated for Metrowerks Codewarrior Pro 2.1(KG)
			2/97	recompiled PPC and 68K projects in CodeWarrior 11(MW)
			8/30/91 fixed it up to be useful(GDG)
			8/10/91	created this bugger from some existing source lying about(GDG)
			

*/
void
UDCLCTBConfig::GetCommToolConfig( char* outToolName, char* outConfigString )
{
	short           procID;
	ConnHandle      connection;
	CMBufferSizes   bufSizes;
	Str255          toolName;
	Point           where;
	short           result;
	Ptr				configStream;
	
	// initialize CTB
	if (::NGetTrapAddress(_CommToolboxDispatch, OSTrap) ==
		::NGetTrapAddress(_Unimplemented, OSTrap))
	{
		throw DCLNotAvailable;
	}
	
	OSErr theErr = ::InitCRM();
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	theErr = ::InitCTBUtilities();
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	if (cmNoTools == ::InitCM())	// Connection Manager
	{
		throw DCLNotAvailable;
	}

	// get a Connection Tool name
	theErr = ::CRMGetIndToolName('cbnd',1,toolName);
	if (theErr != noErr) {
		throw DCLPlatformUnknownError( theErr );
	}

	// get a resource ID for it
	procID = ::CMGetProcID(toolName);
	if (-1 == procID) {
		throw DCLNotAvailable;
	}

	// init the CMBufferSizes variable so that Tool will init with defaults
	bufSizes[cmDataIn] = 0;
	bufSizes[cmDataOut] = 0;
	bufSizes[cmCntlIn] = 0;
	bufSizes[cmCntlOut] = 0;
	bufSizes[cmAttnIn] = 0;
	bufSizes[cmAttnOut] = 0;
	
	// now get a conn record set up 
	connection = ::CMNew(procID, cmData|cmNoMenus|cmQuiet, bufSizes, 0, 0);
	if (connection == nil) {
		throw DCLNotAvailable;
	}
	
	// CMChoose Dialog has to hang off this point (global coordinates)
	::SetPt(&where,20,40);

	// now do CMChoose et al:
	result = ::CMChoose(&connection,where,NULL);
	if ((result == chooseOKMajor) || (result == chooseOKMinor)) {
		configStream = ::CMGetConfig(connection);
		if (configStream == NULL) {
			throw DCLNotAvailable;
		} else {
			::CMGetToolName((**connection).procID,toolName);

			UPStrings::P2CStrCopy( outToolName, toolName );	
			
			unsigned long theLength = ::strlen ( configStream );
			::BlockMove( configStream, outConfigString, (long) theLength + 1 );
			::DisposePtr(configStream);
		}
	} else {
		::CMDispose(connection);
		throw DCLUserCancelled;
	}
	::CMDispose(connection);
}

// ================================================ //
// An elephant is a mouse with an operating system. //
// ================================================ //