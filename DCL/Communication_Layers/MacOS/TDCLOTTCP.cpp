// ==============================
// Fichier:			TDCLOTTCP.cp
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
// The Original Code is TDCLOTTCP.cp.
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
// $Id: TDCLOTTCP.cp,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Communication_Layers/MacOS/TDCLOTTCP.h>

// MacOS Headers
#if TARGET_OS_MACOS
	#include <Gestalt.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

#include <DCL/Exceptions/Errors/TDCLMemError.h>

// =================================================== //
//   ••••• Implementation for class TDCLOTTCP •••••    //
// =================================================== //

#define kOTTCPConfigStr		"tilisten,tcp"

// ------------------------------------------------------------------------------------	//
//	• TDCLOTTCP( TDCLInterface*, long, long )
// ------------------------------------------------------------------------------------	//
// Public constructor.

TDCLOTTCP::TDCLOTTCP(
		long	inPort,
		IDCLThreads* inThreadsIntf,
		KUInt32	inMaxPipeCount /* = kDefaultPipeCount */,
		long	inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLOpenTransport(
				inThreadsIntf,
				inMaxPipeCount,
				inTimeout )
{
	// Create the configuration.
	OTConfigurationRef theConfiguration
		= ::OTCreateConfiguration( kOTTCPConfigStr );
	
	if (theConfiguration == nil)
	{
		throw DCLMemError;
	}
	
	try {
	
		// Create the local address info.
		TBind theLocalAddressInfo;
		InetAddress theLocalIPAddress;
	
		::OTInitInetAddress( &theLocalIPAddress, (InetPort) inPort, kOTAnyInetAddress);
		theLocalAddressInfo.addr.buf = (KUInt8 *) &theLocalIPAddress;
		theLocalAddressInfo.addr.len = sizeof(theLocalIPAddress);
		theLocalAddressInfo.qlen = inMaxPipeCount;
	
		// Call Init.
		Init( theConfiguration, &theLocalAddressInfo );
	} catch (...) {
		// Dispose the configuration anyway.
		::OTDestroyConfiguration( theConfiguration );
	}

	::OTDestroyConfiguration( theConfiguration );
}

// ------------------------------------------------------------------------------------	//
//	• IsAvailable( void )
// ------------------------------------------------------------------------------------	//
// Checks if the service is available.

Boolean
TDCLOTTCP::IsAvailable( void )
{
	Boolean theResult = false;	// Default is TCP/IP with OT isn't available.
	if (TDCLOpenTransport::IsAvailable())
	{
		// Ask via Gestalt.
		KUInt32 theOTInfos;		// OT attributes
		OSErr theErr = ::Gestalt( gestaltOpenTpt, (long*) &theOTInfos );

		if (theErr == noErr)
		{
			// Check that I have TCP/IP
			if (theOTInfos & gestaltOpenTptTCPPresentMask)
			{
				theResult = true;
			}
		}
	}

	return theResult;
}

// ======================================================= //
// Scotty: Captain, we din' can reference it!              //
// Kirk:   Analysis, Mr. Spock?                            //
// Spock:  Captain, it doesn't appear in the symbol table. //
// Kirk:   Then it's of external origin?                   //
// Spock:  Affirmative.                                    //
// Kirk:   Mr. Sulu, go to pass two.                       //
// Sulu:   Aye aye, sir, going to pass two.                //
// ======================================================= //