// ==============================
// Fichier:			TDCLOTSerial.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/01/2002
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
// The Original Code is TDCLOTSerial.cp.
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
// $Id: TDCLOTSerial.cp,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLOTSerial.h>

#if !TARGET_OS_MACOS
	#error OTSerial ne marche que sur MacOS
#endif

// ANSI C
#include <string.h>
#include <stdlib.h>

// MacOS
#include <Gestalt.h>
#include <OpenTransportProtocol.h>

// DCL
#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// ------------------------------------------------------------------------------------	//
//	* TDCLOTSerial( IDCLThreads*, const char*, KUInt32, long )
// ------------------------------------------------------------------------------------	//
TDCLOTSerial::TDCLOTSerial(
		IDCLThreads*	inThreadsIntf,
		unsigned long	inPort,
		long			inBaud		/* = 38400 */,
		long			inTimeout	/* = kDefaultTimeout */
	)
		:
			TDCLOpenTransport(
				inThreadsIntf,
				1,				// 1 seule connexion à la fois.
				inTimeout ),
			mBaud( inBaud )
{
	// Création de la configuration.
	// Récupération de la chaîne.
	char theConfigStr[37];
	theConfigStr[36] = 0;
	if (!GetSerialPortInfos( inPort, theConfigStr, nil ))
	{
		throw DCLNotAvailable;
	}
	
	OTConfigurationRef theConfiguration
		= ::OTCreateConfiguration( theConfigStr );
	if (theConfiguration == nil)
	{
		throw DCLMemError;
	}

	try {
		TBind theLocalAddressInfo;
		theLocalAddressInfo.addr.buf = nil;
		theLocalAddressInfo.addr.len = 0;
		theLocalAddressInfo.qlen = 0;

		// Appelons Init
		Init( theConfiguration, &theLocalAddressInfo );
	} catch (...) {
		// Libération de la configuration.
		::OTDestroyConfiguration( theConfiguration );
		
		throw;	// Rethrow.
	}

	// Libération de la configuration.
	::OTDestroyConfiguration( theConfiguration );
}

// ------------------------------------------------------------------------------------	//
//	* IsAvailable( void )
// ------------------------------------------------------------------------------------	//
Boolean
TDCLOTSerial::IsAvailable( void )
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

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortInfos( unsigned long )
// ------------------------------------------------------------------------------------	//
char*
TDCLOTSerial::GetSerialPortName( unsigned long inIndex )
{
	char* theResult = nil;
	// theResult n'est pas modifié si le port n'existe pas.
	(void) GetSerialPortInfos( inIndex, nil, &theResult );
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortInfos( unsigned long, char outConfigStr[36] )
// ------------------------------------------------------------------------------------	//
Boolean
TDCLOTSerial::GetSerialPortInfos(
					unsigned long inIndex,
					char outConfigStr[36],
					char** outNameStr )
{
	Boolean theResult = false;
	
	// Initialisation d'OT.
	InitOT();
	
	OTPortRecord theRecord;	// Enregistrement.

	unsigned long indexRealPorts = 0;	// Index sur les ports réels.
	unsigned long indexPorts;
	for (indexPorts = 0; indexPorts <= inIndex; indexPorts++)
	{
		Boolean portAvailable = ::OTGetIndexedPort( &theRecord, indexPorts );
		
		if (portAvailable)
		{
			// Est-ce un alias?
			if (!(theRecord.fInfoFlags & kOTPortIsAlias))
			{
				// Est-ce bien un port série?
				OTPortRef thePortRef = theRecord.fRef;
				KUInt16 theDeviceType = ::OTGetDeviceTypeFromPortRef( thePortRef );
						
				if (theDeviceType == kOTSerialDevice)
				{
					// L'index correspond-il?
					if (indexRealPorts == inIndex)
					{
						// Copie de la chaîne de configuration.
						if (outConfigStr)
						{
							(void) ::memcpy(
											outConfigStr,
											theRecord.fPortName,
											sizeof( outConfigStr ));
						}
						
						if (outNameStr)
						{
							Str255 theName;
							::OTGetUserPortNameFromPortRef( thePortRef, theName );

							// Taille de la chaîne avec le terminateur.
							long theLen = theName[0] + 1;
							char* theResult = (char*) ::malloc( (unsigned long) theLen );
							if (theResult == nil)
							{
								throw DCLMemError;
							}
						
							// Copie du nom.
							(void) ::memcpy( theResult, &theName[1], (unsigned long) theLen - 1 );
							theResult[theLen - 1] = '\0';
							
							*outNameStr = theResult;
						}
						
						theResult = true;
						
						// Inutile de continuer.
						break;
					} else {
						indexRealPorts++;
					}
				}
			} // if (!(theRecord.fInfoFlags & kOTPortIsAlias))
		} else {
			// Plus de port série.
			// theResult vaut déjà false
			break;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* CountSerialPorts( void )
// ------------------------------------------------------------------------------------	//
unsigned long
TDCLOTSerial::CountSerialPorts( void )
{
	unsigned long theResult = 0;

	// Initialisation d'OT.
	InitOT();
	
	OTPortRecord theRecord;	// Enregistrement.

	unsigned long indexPorts = 0;

	while (true)
	{
		Boolean portAvailable = ::OTGetIndexedPort( &theRecord, indexPorts );
		
		if (portAvailable)
		{
			// Est-ce un alias?
			if (!(theRecord.fInfoFlags & kOTPortIsAlias))
			{
				// Est-ce bien un port série?
				OTPortRef thePortRef = theRecord.fRef;
				KUInt16 theDeviceType = ::OTGetDeviceTypeFromPortRef( thePortRef );
						
				if (theDeviceType == kOTSerialDevice)
				{
					theResult++;
				}
			}
			
			// Port suivant.
			indexPorts++;
		} else {
			break;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//  * Accept( void )
// ------------------------------------------------------------------------------------	//
TDCLPipe*
TDCLOTSerial::Accept( void )
{
	// Récupération de l'interface de communication OpenTransport.
	TDCLPipe* theResult = TDCLOpenTransport::Accept();
	
	// Ajout de la compression MNP.
	if (theResult)
	{
		theResult = new TDCLMNPPipe( theResult );
	}
	
	return theResult;
}

// ============================================================================== //
//         The programmers of old were mysterious and profound.  We cannot fathom //
// their thoughts, so all we do is describe their appearance.                     //
//         Aware, like a fox crossing the water.  Alert, like a general on the    //
// battlefield.  Kind, like a hostess greeting her guests. Simple, like uncarved  //
// blocks of wood.  Opaque, like black pools in darkened caves.                   //
//         Who can tell the secrets of their hearts and minds?                    //
//         The answer exists only in the Tao.                                     //
//                 -- Geoffrey James, "The Tao of Programming"                    //
// ============================================================================== //