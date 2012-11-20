// ==============================
// Fichier:			UDCLMacMachineName.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			26/01/2002
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
// The Original Code is UDCLMacMachineName.cp.
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
// $Id: UDCLMacMachineName.cp,v 1.5 2004/11/24 14:09:01 paul Exp $
// ===========

#include <DCL/Interfaces/MacOS/UDCLMacMachineName.h>

#if TARGET_OS_OPENSTEP
	// MacOS X
	#include <CoreFoundation/CFString.h>
	#include <CoreFoundation/CFBase.h>
	#include <CoreFoundation/CFURL.h>
	#include <CoreFoundation/CFURLAccess.h>
	#include <CoreFoundation/CFPropertyList.h>
	#include <CoreServices/CoreServices.h>
#endif

#if TARGET_OS_MACOS
	// MacOS
	#include <Resources.h>
	#include <TextUtils.h>
#endif

// ANSI
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// K
#include <K/Misc/UPStrings.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/MacOS_X/UDCLMacCarbonUtils.h>

#if TARGET_OS_OPENSTEP
// ------------------------------------------------------------------------- //
//  * ReadMachineNameFromFile( void )
// ------------------------------------------------------------------------- //
const char*
UDCLMacMachineName::ReadMachineNameFromFile( void )
{
    // This routine is called when the client tries to get the
    // machine name on Mac OS X 10.0.x.  Because CSCopyMachineName returns
    // un-useful information on those systems, it reads the name
    // directly from the System Configuration preferences file.

    // *** IMPORTANT ***
    //
    // The location and format of the System Configuration framework
    // preferences file in the code below is subject to change
    // at any time.  Do not write any code which makes any assumptions
    // on this file since your app WILL BREAK at some point in time.
    // The only reason we can do this safely in this code is because
    // we know we're running on Mac OS 10.0 through 10.0.4.  This is
    // enforced by the asserts above.  For Mac OS X 10.1 and above you
    // must use the public System Configuration framework APIs (or
    // APIs, like CSCopyMachineName, which are layered on top of
    // System Configuration framework).

	CFURLRef thePrefsURL =
    	::CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
			CFSTR( "/var/db/SystemConfiguration/preferences.xml" ),
			kCFURLPOSIXPathStyle,
			false );
	if (thePrefsURL == nil)
	{
		throw DCLNotAvailable;
	}

	CFDataRef	thePrefsData;	// Data from the pref URL.
    if (!::CFURLCreateDataAndPropertiesFromResource(
				kCFAllocatorDefault,
				thePrefsURL,
				&thePrefsData,
				nil,
				nil,
				nil )
		)
	{
		// Release the prefs anyway.
		::CFRelease( thePrefsURL );
		
		throw DCLNotAvailable;
	}

	// I no longer need the URL.
	::CFRelease( thePrefsURL );
	
	CFDictionaryRef theDictionary =
		(CFDictionaryRef) ::CFPropertyListCreateFromXMLData(
				kCFAllocatorSystemDefault,
				thePrefsData,
				kCFPropertyListImmutable,
				nil );
	
	// I no longer need the data.
	::CFRelease( thePrefsData );
	
	if (theDictionary == nil)
	{
		throw DCLNotAvailable;
	}
	
	// Then go system/system.
	CFDictionaryRef theSystem1Dict = 
		(CFDictionaryRef)
			::CFDictionaryGetValue( theDictionary, CFSTR("System") );
	if (theSystem1Dict == nil)
	{
		// Dispose the dictionary
		::CFRelease( theDictionary );

 		throw DCLNotAvailable;
	}

	CFDictionaryRef theSystem2Dict =
		(CFDictionaryRef)
			::CFDictionaryGetValue( theSystem1Dict, CFSTR("System") );
	if (theSystem2Dict == nil)
	{
		// Dispose the dictionary (and sub-objects)
		::CFRelease( theDictionary );

 		throw DCLNotAvailable;
	}
	
	// Finally, grab the string.
	CFStringRef theMachineName =
		(CFStringRef)
			::CFDictionaryGetValue( theSystem2Dict, CFSTR("ComputerName") );
	
	if (theMachineName == nil)
	{
		// Dispose the dictionary (and sub-objects)
		::CFRelease( theDictionary );

 		throw DCLNotAvailable;
	}
	
	const char* theResult = UDCLMacCarbonUtils::CFToCString( theMachineName );

	// Dispose the dictionary (and sub-objects)
	::CFRelease( theDictionary );

    return theResult;
}
#endif

#if TARGET_OS_MACOS
// -------------------------------------------------------------------------- //
//  * ReadMachineNameFromResource( void )
// -------------------------------------------------------------------------- //
const char*
UDCLMacMachineName::ReadMachineNameFromResource( void )
{
	// Let's get the name from the resource.
	StringHandle theStringHdl = ::GetString( kMachineNameSTRID );
	
	// I check the resource is present.
	if ( theStringHdl == nil )
		throw DCLNotAvailable;
	
	// I lock it.
	::HLock( (Handle) theStringHdl );
	
	// I dereference it.
	unsigned char* theStringPtr = *theStringHdl;
	
	// Then I convert it to a newly allocated CString.
	char* theMachineName =
			(char*) ::malloc( (unsigned long) (theStringPtr[0] + 1) );
	
	if (theMachineName == nil)
	{
		// I release the resource.
		::ReleaseResource( (Handle) theStringHdl );

		throw DCLMemError;
	}
	
	UPStrings::P2CStrCopy( (char*) theMachineName, theStringPtr );

	// I release the resource.
	::ReleaseResource( (Handle) theStringHdl );

	return theMachineName;
}
#endif

// -------------------------------------------------------------------------- //
//  * GetMachineName( void )
// -------------------------------------------------------------------------- //
const char*
UDCLMacMachineName::GetMachineName( void )
{
#if TARGET_OS_OPENSTEP
	// Get the system version to know what to choose.
	// Check that we have the function, or throw NotAvailable if we don't.
	KUInt32 theMacOSXVersion;	// Version of the system.
	
	OSErr theErr =
			::Gestalt( gestaltSystemVersion, (KSInt32 *) &theMacOSXVersion );
	if (theErr)
	{
		// Shouldn't happen, unless they remove Gestalt :)
		throw DCLPlatformUnknownError( theErr );
	}
	
	const char* theResult;	// The result as a CString.
	
	if ( theMacOSXVersion < kMacOSX10point1 )
	{
		// File method
		theResult = ReadMachineNameFromFile();
	} else {
		// Carbon function method.
		// Check that this function is available.
		if ( CSCopyMachineName == (void *) kUnresolvedCFragSymbolAddress )
		{
			throw DCLNotAvailable;
        }
        
		CFStringRef theMachineName = ::CSCopyMachineName();

		theResult = UDCLMacCarbonUtils::CFToCString( theMachineName );

		// I no longer need the CF string.
		::CFRelease( theMachineName );
	}
	
	return theResult;
	
#else	// TARGET_OS_MACOS
	// MacOS: just use the resource method.
	return ReadMachineNameFromResource();
#endif
}

// ================================================================== //
// CCI Power 6/40: one board, a megabyte of cache, and an attitude... //
// ================================================================== //