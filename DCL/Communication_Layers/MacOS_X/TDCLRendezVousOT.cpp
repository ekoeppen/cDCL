// ==============================
// Fichier:			TDCLRendezVousOT.cp
// Projet:			Desktop Connection Library
//
// Créé le:			25/1/2003
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
// The Original Code is TDCLRendezVousOT.cp.
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
// $Id: TDCLRendezVousOT.cp,v 1.4 2004/11/24 18:11:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS_X/TDCLRendezVousOT.h>

// ANSI C
#include <stdlib.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/MacOS/UDCLMacMachineName.h>

// ------------------------------------------------------------------------- //
//  * TDCLRendezVousOT( IDCLThreads*, const char*, unsigned short, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLRendezVousOT::TDCLRendezVousOT(
				   IDCLThreads*	inThreadsIntf,
				   const char*		inMachineName /* = nil*/,
				   unsigned short	inPort /* = kIANAAssignedPort*/,
				   KUInt32			inMaxPipeCount /* = kDefaultPipeCount*/ )
	:
		TDCLOTTCP( inPort, inThreadsIntf, inMaxPipeCount ),
		mNetServiceRef( nil )
{
	// Récupération du nom de la machine si nécessaire.
	const char* theMachineName;
	if (inMachineName == nil)
	{
		theMachineName = UDCLMacMachineName::GetMachineName();
	} else {
		theMachineName = inMachineName;
	}
	
	CFStringRef theMachineNameRef = ::CFStringCreateWithCString(
			kCFAllocatorDefault,
			theMachineName,
			kCFStringEncodingMacRoman );
	
	if (inMachineName == nil)
	{
		::free( (void*) theMachineName );
	}
	
	if (theMachineNameRef == nil)
	{
		throw DCLUnknownError;
	}
	
	mNetServiceRef = ::CFNetServiceCreate( kCFAllocatorDefault,
						CFSTR(""),
						CFSTR("_newton-dock._tcp"),
						theMachineNameRef,
						inPort);

	if (mNetServiceRef == nil)
	{
		throw DCLUnknownError;
	}

	CFNetServiceClientContext myContext;
		myContext.version = 0;
		myContext.info = this;
		myContext.retain = NULL;
		myContext.release = NULL;
		myContext.copyDescription = NULL;

	if (!::CFNetServiceSetClient(mNetServiceRef, 
			(CFNetServiceClientCallBack)&NetServiceClientCallBack,
			&myContext))
	{
		throw DCLUnknownError;
	}

    ::CFNetServiceScheduleWithRunLoop(
    				mNetServiceRef,
					CFRunLoopGetCurrent(),
					kCFRunLoopCommonModes);
}

// -------------------------------------------------------------------------- //
//  * ~TDCLRendezVousOT( void )
// -------------------------------------------------------------------------- //
TDCLRendezVousOT::~TDCLRendezVousOT( void )
{
	if (mNetServiceRef)
	{
		::CFNetServiceUnscheduleFromRunLoop(
				mNetServiceRef,
				CFRunLoopGetCurrent(),
				kCFRunLoopCommonModes);
		(void) ::CFNetServiceSetClient(mNetServiceRef, NULL, NULL);
		::CFRelease( mNetServiceRef );
	}
}

// ------------------------------------------------------------------------- //
//  * DoStartListening( void )
// ------------------------------------------------------------------------- //
void
TDCLRendezVousOT::DoStartListening( void )
{
	TDCLOTTCP::DoStartListening();
		
	// Enregistrement du service.	
	if (!CFNetServiceRegister(mNetServiceRef, NULL))
	{
		throw DCLUnknownError;
	}
}

// ------------------------------------------------------------------------- //
//  * DoStopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLRendezVousOT::DoStopListening( void )
{
	// On se désenregistre.
	CFNetServiceCancel(mNetServiceRef);

	TDCLOTTCP::DoStopListening();		
};

// ------------------------------------------------------------------------- //
//  * NetServiceClientCallBack( CFNetServiceRef, CFStreamError, void* )
// ------------------------------------------------------------------------- //
void
TDCLRendezVousOT::NetServiceClientCallBack(
				CFNetServiceRef /* theService */,
				CFStreamError* /* error */,
				void* /* info */)
{
#warning TODO.
	// This place for rent.
}

// ======================================================= //
// Hacking's just another word for nothing left to kludge. //
// ======================================================= //