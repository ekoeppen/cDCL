// ==============================
// Fichier:			TDCLIOKitSerialPort.cp
// Projet:			Desktop Connection Library
//
// Créé le:			4/4/2003
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
// The Original Code is TDCLIOKitSerialPort.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
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
// $Id: TDCLIOKitSerialPort.cp,v 1.3 2004/11/24 14:08:55 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS_X/TDCLIOKitSerialPort.h>

// MacOS X
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#include <paths.h>	// BSD

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// -------------------------------------------------------------------------- //
//  * TDCLIOKitSerialPort( void )
// -------------------------------------------------------------------------- //
TDCLIOKitSerialPort::TDCLIOKitSerialPort(
						IDCLThreads* inThreadsIntf,
						KUInt32 inIndex,
						long inBaud /* = 38400 */ )
	:
		TDCLFDSerialPort(
				inThreadsIntf,
				nil,
				inBaud )
{
	// Récupération.
	CFStringRef theBSDPathAsCFString = GetSerialPortPath( inIndex );

	// Vérification que le port existe.
	if (theBSDPathAsCFString == nil)	
	{
		throw DCLNotAvailable;
	}

	// Traduction.
	char* theBSDPath = CFStringRefToCStr( theBSDPathAsCFString );

	// Libération.
	::CFRelease( theBSDPathAsCFString );

	// Initialisation.
	Init( theBSDPath );

	// Libération.
	::free( theBSDPath );
}

// -------------------------------------------------------------------------- //
//  * TDCLIOKitSerialPort( IDCLThreads*, CFStringRef, long, long )
// -------------------------------------------------------------------------- //
TDCLIOKitSerialPort::TDCLIOKitSerialPort(
						IDCLThreads* inThreadsIntf,
						CFStringRef inDevPath,
						long inBaud /* = 38400 */ )
	:
		TDCLFDSerialPort(
				inThreadsIntf,
				nil,
				inBaud )
{
	// Traduction.
	char* theBSDPath = CFStringRefToCStr( inDevPath );

	// Initialisation.
	Init( theBSDPath );

	// Libération.
	::free( theBSDPath );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLIOKitSerialPort( void )
// -------------------------------------------------------------------------- //
TDCLIOKitSerialPort::~TDCLIOKitSerialPort( void )
{
}

// ------------------------------------------------------------------------------------	//
//	* CountSerialPorts( void )
// ------------------------------------------------------------------------------------	//
KUInt32
TDCLIOKitSerialPort::CountSerialPorts( void )
{
	KUInt32 theResult = 0;
	
	try {
		// Création d'un itérateur sur les ports série.
		io_iterator_t theIterator;
		GetSerialPortsIterator( &theIterator );
		
		// On itère pour compter.
		io_object_t	theSerialService;
		while ((theSerialService = IOIteratorNext(theIterator)) != 0)
		{
			CFTypeRef theBSDPathAsCFString =
					::IORegistryEntryCreateCFProperty(
						theSerialService,
						CFSTR(kIOCalloutDeviceKey),
						kCFAllocatorDefault,
						0 /* options */ );
			if (theBSDPathAsCFString)
			{
				// Libération.
				CFRelease(theBSDPathAsCFString);
				
				// Un de plus.
				theResult++;
	        }

			// Libération du service.
			(void) IOObjectRelease(theSerialService);
	    }
	} catch ( TDCLNotAvailableException& inNotAvailableException ) {
		// Ignorée.
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortName( KUInt32 )
// ------------------------------------------------------------------------------------	//
CFStringRef
TDCLIOKitSerialPort::GetSerialPortName( KUInt32 inIndex )
{
	KUInt32 theCount = 0;
	CFStringRef theResult = nil;
	
	try {
		// Création d'un itérateur sur les ports série.
		io_iterator_t theIterator;
		GetSerialPortsIterator( &theIterator );
		
		// On itère pour compter.
		io_object_t	theSerialService;
		while ((theSerialService = IOIteratorNext(theIterator)) != 0)
		{
			CFTypeRef theBSDPathAsCFString =
					::IORegistryEntryCreateCFProperty(
						theSerialService,
						CFSTR(kIOCalloutDeviceKey),
						kCFAllocatorDefault,
						0 /* options */ );
			if (theBSDPathAsCFString)
			{
				// Libération.
				CFRelease(theBSDPathAsCFString);

				if (theCount == inIndex)
				{
					// Récupération du nom.
					theResult = (CFStringRef)
						::IORegistryEntryCreateCFProperty(
							theSerialService,
							CFSTR(kIOTTYDeviceKey),
							kCFAllocatorDefault,
							0 /* options */ );;
				} else {
					// Un de plus.
					theCount++;
				}
	        }

			// Libération du service.
			(void) IOObjectRelease(theSerialService);

			if (theResult)
			{
				// Trouvé.
				break;
			}
	    }
	} catch ( TDCLNotAvailableException& inNotAvailableException ) {
		// Ignorée.
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortName( CFStringRef )
// ------------------------------------------------------------------------------------	//
CFStringRef
TDCLIOKitSerialPort::GetSerialPortName( CFStringRef inDevPath )
{
	CFStringRef theResult = nil;
	
	try {
		// Création d'un itérateur sur les ports série.
		io_iterator_t theIterator;
		GetSerialPortsIterator( &theIterator );
		
		// On itère pour compter.
		io_object_t	theSerialService;
		while ((theSerialService = IOIteratorNext(theIterator)) != 0)
		{
			CFTypeRef theBSDPathAsCFString =
					::IORegistryEntryCreateCFProperty(
						theSerialService,
						CFSTR(kIOCalloutDeviceKey),
						kCFAllocatorDefault,
						0 /* options */ );
			if (theBSDPathAsCFString)
			{
				if (::CFStringCompare(
							(CFStringRef) theBSDPathAsCFString,
							inDevPath,
							0 /* options */) == kCFCompareEqualTo)
				{
					// Récupération du nom.
					theResult = (CFStringRef)
						::IORegistryEntryCreateCFProperty(
							theSerialService,
							CFSTR(kIOTTYDeviceKey),
							kCFAllocatorDefault,
							0 /* options */ );;
				}

				// Libération.
				CFRelease(theBSDPathAsCFString);
	        }

			// Libération du service.
			(void) IOObjectRelease(theSerialService);

			if (theResult)
			{
				// Trouvé.
				break;
			}
	    }
	} catch ( TDCLNotAvailableException& inNotAvailableException ) {
		// Ignorée.
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* IsAvailable( CFStringRef )
// ------------------------------------------------------------------------------------	//
Boolean
TDCLIOKitSerialPort::IsAvailable( CFStringRef inDevPath )
{
	Boolean theResult = false;
	
	try {
		// Création d'un itérateur sur les ports série.
		io_iterator_t theIterator;
		GetSerialPortsIterator( &theIterator );
		
		// On itère pour compter.
		io_object_t	theSerialService;
		while ((theSerialService = IOIteratorNext(theIterator)) != 0)
		{
			CFTypeRef theBSDPathAsCFString =
					::IORegistryEntryCreateCFProperty(
						theSerialService,
						CFSTR(kIOCalloutDeviceKey),
						kCFAllocatorDefault,
						0 /* options */ );
			if (theBSDPathAsCFString)
			{
				if (::CFStringCompare(
							(CFStringRef) theBSDPathAsCFString,
							inDevPath,
							0 /* options */) == kCFCompareEqualTo)
				{
					theResult = true;
				}

				// Libération.
				CFRelease(theBSDPathAsCFString);
	        }

			// Libération du service.
			(void) IOObjectRelease(theSerialService);
	
			if (theResult)
			{
				// Trouvé.
				break;
			}
	    }
	} catch ( TDCLNotAvailableException& inNotAvailableException ) {
		// Ignorée.
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortPath( KUInt32 )
// ------------------------------------------------------------------------------------	//
CFStringRef
TDCLIOKitSerialPort::GetSerialPortPath( KUInt32 inIndex )
{
	KUInt32 theCount = 0;
	CFStringRef theResult = nil;
	
	try {
		// Création d'un itérateur sur les ports série.
		io_iterator_t theIterator;
		GetSerialPortsIterator( &theIterator );
		
		// On itère pour compter.
		io_object_t	theSerialService;
		while ((theSerialService = IOIteratorNext(theIterator)) != 0)
		{
			CFTypeRef theBSDPathAsCFString =
					::IORegistryEntryCreateCFProperty(
						theSerialService,
						CFSTR(kIOCalloutDeviceKey),
						kCFAllocatorDefault,
						0 /* options */ );
			if (theBSDPathAsCFString)
			{
				if (theCount == inIndex)
				{
					theResult = (CFStringRef) theBSDPathAsCFString;
				} else {
					// Libération.
					CFRelease(theBSDPathAsCFString);
				
					// Un de plus.
					theCount++;
				}
	        }

			// Libération du service.
			(void) IOObjectRelease(theSerialService);

			if (theResult)
			{
				// Trouvé.
				break;
			}
	    }
	} catch ( TDCLNotAvailableException& inNotAvailableException ) {
		// Ignorée.
	}
	
	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* CFStringRefToCStr( CFStringRef )
// ------------------------------------------------------------------------------------	//
char*
TDCLIOKitSerialPort::CFStringRefToCStr( CFStringRef inStringRef )
{	
	// Détermination de la taille.
	CFIndex theLength = ::CFStringGetLength( inStringRef );
	
	// Allocation de la chaîne avec malloc.
	char* theResult = (char*) ::malloc( (size_t) (theLength + 1) );
	
	// Copie des données (transcription en US-ASCII)
	if (!::CFStringGetCString(
						inStringRef, theResult, theLength + 1, kCFStringEncodingASCII ))
	{
		throw DCLUnknownError;
	}

	return theResult;
}

// ------------------------------------------------------------------------------------	//
//	* GetSerialPortsIterator( io_iterator_t* )
// ------------------------------------------------------------------------------------	//
void
TDCLIOKitSerialPort::GetSerialPortsIterator( io_iterator_t* outIterator )
{
	// Récupération du port maître.
	mach_port_t	theMasterPort;
	kern_return_t theErr = ::IOMasterPort(MACH_PORT_NULL, &theMasterPort);
	if (theErr != KERN_SUCCESS)
	{
		throw DCLPlatformUnknownError( theErr );
	}

	// Récupération du dictionnaire sur les ports BSD.
	CFMutableDictionaryRef theSerialPortsDict =
		::IOServiceMatching( kIOSerialBSDServiceValue );
	if (theSerialPortsDict == NULL)
	{
		throw DCLNotAvailable;
	}
	
	// Ensuite, on filtre pour n'avoir que les ports série.
	::CFDictionarySetValue(theSerialPortsDict,
								CFSTR(kIOSerialBSDTypeKey),
								CFSTR(kIOSerialBSDRS232Type));


	// Création de l'itérateur.
	theErr = ::IOServiceGetMatchingServices(
					theMasterPort, theSerialPortsDict, outIterator );    
	if (theErr != KERN_SUCCESS)
	{
		throw DCLPlatformUnknownError( theErr );
	}
}

// ======================================================================== //
// Unix is a lot more complicated (than CP/M) of course -- the typical Unix //
// hacker can never remember what the PRINT command is called this week --  //
// but when it gets right down to it, Unix is a glorified video game.       //
// People don't do serious work on Unix systems; they send jokes around the //
// world on USENET or write adventure games and research papers.            //
//                 -- E. Post                                               //
//                 "Real Programmers Don't Use Pascal", Datamation, 7/83    //
// ======================================================================== //