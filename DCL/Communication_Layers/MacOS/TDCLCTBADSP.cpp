// ==============================
// Fichier:			TDCLCTBADSP.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			21/01/2002
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
// The Original Code is TDCLCTBADSP.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLCTBADSP.cp,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Communication_Layers/MacOS/TDCLCTBADSP.h>

// ANSI headers.
#include <stdio.h>
#include <stdlib.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/MacOS/UDCLMacMachineName.h>

#define kADSPToolStr		"AppleTalk ADSP Tool"
#define kADSPConfigToolStr	\
				"LocalADSPType \"Docker\" RegisterName 1 LocalADSPName \"%s\""

// ------------------------------------------------------------------------- //
//  * TDCLCTBADSP( IDCLThreads*, const char*, KUInt32, long )
// ------------------------------------------------------------------------- //
TDCLCTBADSP::TDCLCTBADSP(
		IDCLThreads* inThreadsIntf,
		const char* inMachineName /* = nil */,
		KUInt32	inMaxPipeCount /* = kDefaultPipeCount */,
		long	inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLCommToolBox(
				inThreadsIntf,
				inMaxPipeCount,
				inTimeout )
{
	// Name for this machine.
	const char* theMachineName = inMachineName;
	
	// Do I have the machine name?
	if (theMachineName == nil)
	{
		theMachineName = UDCLMacMachineName::GetMachineName();
	}
	
	// String for the full configuration string (including the machine name)
	char theConfigString[510] = "";
	
	(void) ::sprintf( theConfigString, kADSPConfigToolStr, theMachineName );

	// I no longer need theMachineName copy string at this point.
	if (inMachineName == nil)
	{
		::free( (void*) theMachineName );
	}

	// Call CommLayer init method. May throw.
	Init( kADSPToolStr, theConfigString );
}

// ------------------------------------------------------------------------- //
//  * IsAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCTBADSP::IsAvailable( void )
{
	return TDCLCommToolBox::IsAvailable( kADSPToolStr );
}

// ========================================================== //
// What this country needs is a good five cent microcomputer. //
// ========================================================== //