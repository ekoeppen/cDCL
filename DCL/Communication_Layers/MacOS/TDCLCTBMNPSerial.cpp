// ==============================
// Fichier:			TDCLCTBMNPSerial.cp
// Projet:			Desktop Connection Library
//
// Créé le:			22/01/2002
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
// The Original Code is TDCLCTBMNPSerial.cp.
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
// $Id: TDCLCTBMNPSerial.cp,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLCTBMNPSerial.h>

// MacOS
#include <CommResources.h>
#include <CRMSerialDevices.h>

// ANSI C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// ------------------------------------------------------------------------- //
//	* Constantes
// ------------------------------------------------------------------------- //
#define kAppleModemToolStr	"Apple Modem Tool"
#define kMNPConfigStr		"ModemType \"Newton Serial Connection\" Baud %i "\
							"DataBits 8 Parity None StopBits 1 Port \"%s\" "\
							"Handshake None"

// ------------------------------------------------------------------------- //
//	* TDCLCTBMNPSerial( IDCLThreads*, unsigned long, long, long )
// ------------------------------------------------------------------------- //
TDCLCTBMNPSerial::TDCLCTBMNPSerial(
		IDCLThreads* inThreadsIntf,
		unsigned long inPort,
		long inBaud /* = 38400 */,
		long inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLCommToolBox(
				inThreadsIntf,
				1,			// Une seule connexion à la fois
				inTimeout )
{
	// Récupération du nom du port série.
	char* thePortName = GetSerialPortName( inPort );
	
	// Si c'est nil, c'est que le port n'existe pas.
	if (thePortName == nil)
	{
		throw DCLNotAvailable;
	}
	
	// Mémoire tampon pour la configuration
	// taille de chacune des chaînes moins 2 pour le %s, 2 pour %i,
	// plus 1 pour le terminateur plus 10 pour la vitesse.
	char* theConfigString =
		(char*) ::malloc(
					::strlen(thePortName)
					+ ::strlen(kMNPConfigStr) + 11 );
	
	(void) ::sprintf( theConfigString, kMNPConfigStr, inBaud, thePortName );
	
	// Libération du nom.
	::free( thePortName );

	Init( kAppleModemToolStr, theConfigString );
	
	// Libération de la chaîne de configuration.
	::free( theConfigString );
}

// ------------------------------------------------------------------------- //
//	* IsAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCTBMNPSerial::IsAvailable( void )
{
	return TDCLCommToolBox::IsAvailable( kAppleModemToolStr );
}

// ------------------------------------------------------------------------- //
//	* GetSerialPortName( unsigned long )
// ------------------------------------------------------------------------- //
// Le code provient de l'exemple d'Apple que j'ai modifié.
/*
	File:		FindSerialPorts.c
	
	Description:
				This is a little snippet from the "Inside the Macintosh 
				Communications Toolbox" which demonstrates the correct 
				method for detecting which serial ports are present.


	Author:		BB

	Copyright: 	Copyright: © 1999 by Apple Computer, Inc.
				all rights reserved.
	
	Disclaimer:	You may incorporate this sample code into your applications
				without restriction, though the sample code has been provided
				"AS IS" and the responsibility for its operation is 100% yours.
				However, what you are not permitted to do is to redistribute
				the source as "DSC Sample Code" after having made changes.  If
				you're going to re-distribute the source, we require that you
				make it clear in the source that the code was descended from
				Apple Sample Code, but that you've made changes.
	
	Change History (most recent first):
		6/22/99 - updated for Metrowerks Codewarrior Pro 2.1(KMG)
		2/17/97 - recompiled in Metrowerks Codewarrior 11(BB)  
		

*/

char*
TDCLCTBMNPSerial::GetSerialPortName( unsigned long inIndex )
{
	char* theResult = nil;
	
	OSErr theErr = ::InitCRM();
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}
	
	CRMRec theRecord;							// Élément courant.
	CRMRecPtr theCurrentRecPtr = &theRecord;	// Pointeur sur l'élément
												// courant.

	theRecord.crmDeviceType = crmSerialDevice;
	theRecord.crmDeviceID   = 0;

	int indexPorts;
	for (indexPorts = 0; indexPorts <= inIndex; indexPorts++)
	{
		theCurrentRecPtr =
			(CRMRecPtr) ::CRMSearch( (CRMRecPtr)theCurrentRecPtr );
		if (theCurrentRecPtr)	// Port suivant.
		{
			if (indexPorts == inIndex)
			{
				// Pointeur sur les informations sur le port.
				CRMSerialPtr theSerialPtr =
					(CRMSerialPtr) theCurrentRecPtr->crmAttributes;
			
				// Pointeur sur le nom.
				char* theString = (char*) *(theSerialPtr->name);
				
				// Taille de la chaîne avec le terminateur.
				long theLen = theString[0] + 1;
				theResult = (char*) ::malloc( (unsigned long) theLen );
				if (theResult == nil)
				{
					throw DCLMemError;
				}
				
				// Copie du nom.
				(void) ::memcpy(
							theResult,
							&theString[1],
							(unsigned long) theLen - 1 );
				theResult[theLen - 1] = '\0';
				
				// Inutile de continuer.
				break;
			}
		} else {
			// Plus de port série.
			// theResult vaut déjà nil.
			break;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//	* CountSerialPorts( void )
// ------------------------------------------------------------------------- //
unsigned long
TDCLCTBMNPSerial::CountSerialPorts( void )
{
	unsigned long theResult = 0;

	OSErr theErr = ::InitCRM();
	if (theErr != noErr)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	CRMRec theRecord;							// Élément courant.
	CRMRecPtr theCurrentRecPtr = &theRecord;	// Pointeur sur l'élément
												// courant.

	theRecord.crmDeviceType = crmSerialDevice;
	theRecord.crmDeviceID   = 0;

	while (true)
	{
		theCurrentRecPtr =
			(CRMRecPtr) ::CRMSearch( (CRMRecPtr)theCurrentRecPtr );
		
		if (theCurrentRecPtr)
		{
			theResult++;
		} else {
			break;
		}
	}
	
	return theResult;
}

// ============================================================== //
// X windows:                                                     //
//         We will dump no core before its time.                  //
//         One good crash deserves another.                       //
//         A bad idea whose time has come.  And gone.             //
//         We make excuses.                                       //
//         It didn't even look good on paper.                     //
//         You laugh now, but you'll be laughing harder later!    //
//         A new concept in abuser interfaces.                    //
//         How can something get so bad, so quickly?              //
//         It could happen to you.                                //
//         The art of incompetence.                               //
//         You have nothing to lose but your lunch.               //
//         When uselessness just isn't enough.                    //
//         More than a mere hindrance.  It's a whole new barrier! //
//         When you can't afford to be right.                     //
//         And you thought we couldn't make it worse.             //
//                                                                //
// If it works, it isn't X windows.                               //
// ============================================================== //