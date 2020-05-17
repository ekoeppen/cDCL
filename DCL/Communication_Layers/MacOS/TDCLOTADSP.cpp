// ==============================
// Fichier:			TDCLOTADSP.cp
// Projet:			Desktop Connection Library
//
// Créé le:			25/01/2002
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
// The Original Code is TDCLOTADSP.cp.
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
// $Id: TDCLOTADSP.cp,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLOTADSP.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// MacOS
#include <Resources.h>
#include <TextUtils.h>
#include <Gestalt.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Interfaces/MacOS/UDCLMacMachineName.h>

// ------------------------------------------------------------------------------------	//
//	* Constantes
// ------------------------------------------------------------------------------------	//
#define kOTADSPConfigStr	"tilisten,adsp"

// ------------------------------------------------------------------------------------	//
//	* TDCLOTADSP( IDCLThreads*, const char*, KUInt32, long )
// ------------------------------------------------------------------------------------	//
TDCLOTADSP::TDCLOTADSP(
		IDCLThreads*	inThreadsIntf,
		const char* 	inMachineName /* = nil */,
		KUInt32			inMaxPipeCount /* = kDefaultPipeCount */,
		long			inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLOpenTransport(
				inThreadsIntf,
				inMaxPipeCount,
				inTimeout )
{
	// Create the configuration.
	OTConfigurationRef theConfiguration
		= ::OTCreateConfiguration( kOTADSPConfigStr );
	
	if (theConfiguration == nil)
	{
		throw DCLMemError;
	}
	
	// Final address string
	// Format: escaped machine name + ":" + "Docker" + NULL
	char* theAddress = nil;

	try {
		// Make the name.
		// Type is Docker
		// Name is machine name.
		// I should escape \, @ & : in the name (there is none in "Docker" :)
		
		const char* theMachineName = inMachineName;
		if (inMachineName == nil)
		{
			theMachineName = UDCLMacMachineName::GetMachineName();
		}
		
		// Let's suppose that the machine name is only made of chars I should escape.
		unsigned long theMachineNameLen = ::strlen( theMachineName );	// Length of the machine name
		// Final address string
		// Format: escaped machine name + ":" + "Docker" + NULL
		theAddress = (char*) ::malloc( (unsigned long) ((2 * theMachineNameLen) + 8) );
		
		if (theAddress == nil)
		{
			throw DCLMemError;
		}
		
		// Loop to escape the characters.
		int indexMachineName;	// Index on the machine name string.
		char* cursorAddress = theAddress;	// Cursor to escape characters.
		for (indexMachineName = 0; indexMachineName < theMachineNameLen; indexMachineName++)
		{
			char theCurrentChar = theMachineName[ indexMachineName ];	// Current character
			if ((theCurrentChar == '@') || (theCurrentChar == '\\') || (theCurrentChar == ':'))
			{
				*cursorAddress = '\\';
				cursorAddress++;
			}
			
			*cursorAddress = theCurrentChar;
			cursorAddress++;
		}
		
		// Dispose the machine name if we allocated it.
		if (inMachineName == nil)
		{
			::free( (void*) theMachineName );
		}
		
		// Add :Docker & null terminator.
		{
			char suffix[8] = ":Docker";
			(void) ::memcpy( cursorAddress, suffix, 8 );
		}

		// Create the local address info structure.
		DDPNBPAddress theDDPNBPAddress;

		theDDPNBPAddress.Init( 0 /* net */, 0 /* node */, 0 /* socket */ );

		TBind theLocalAddressInfo;
		theLocalAddressInfo.addr.buf = (KUInt8*) &theDDPNBPAddress;
		theLocalAddressInfo.addr.len = theDDPNBPAddress.SetNBPEntity( theAddress );
		theLocalAddressInfo.qlen = inMaxPipeCount;

		// Call Init.
		Init( theConfiguration, &theLocalAddressInfo );
	} catch (...) {
		// Dispose the configuration anyway.
		::OTDestroyConfiguration( theConfiguration );
		
		// And the address string.
		if (theAddress)
		{
			::free( theAddress );
		}
		
		throw;	// Rethrow.
	}

	// Dispose the configuration.
	::OTDestroyConfiguration( theConfiguration );

	// And the address string.
	if (theAddress)
	{
		::free( theAddress );
	}	
}

// ------------------------------------------------------------------------------------	//
//	* IsAvailable( void )
// ------------------------------------------------------------------------------------	//
Boolean
TDCLOTADSP::IsAvailable( void )
{
	Boolean theResult = false;	// Default is ADSP with OT isn't available.
	if (TDCLOpenTransport::IsAvailable())
	{
		// Ask via Gestalt.
		KUInt32 theOTInfos;		// OT attributes
		OSErr theErr = ::Gestalt( gestaltOpenTpt, (long*) &theOTInfos );

		if (theErr == noErr)
		{
			// Check that I have ADSP
			if (theOTInfos & gestaltOpenTptAppleTalkPresentMask)
			{
				theResult = true;
			}
		}
	}

	return theResult;
}

// ===================================================================== //
// A language that doesn't affect the way you think about programming is //
// not worth knowing.                                                    //
// ===================================================================== //