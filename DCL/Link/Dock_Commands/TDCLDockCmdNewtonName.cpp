// ==============================
// Fichier:			TDCLDockCmdNewtonName.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			02/02/2001
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
// The Original Code is TDCLDockCmdNewtonName.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdNewtonName.cp,v 1.7 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>

// ANSI C
#include <string.h>
#include <stdlib.h>

// K
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>

// ------------------------------------------------------------------------- //
//  * DŽverminage
// ------------------------------------------------------------------------- //

#undef KERROR_ENABLED
#if defined(error_TDCLDockCmdNewtonName) && error_TDCLDockCmdNewtonName
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLDockCmdNewtonName) && trace_TDCLDockCmdNewtonName
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLDockCmdNewtonName) && debug_TDCLDockCmdNewtonName
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

// ------------------------------------------------------------------------- //
//  * TDCLDockCmdNewtonName( KUInt32 inLength, const void* inData )
// ------------------------------------------------------------------------- //
TDCLDockCmdNewtonName::TDCLDockCmdNewtonName( KUInt32 inLength, const void* inData )
		:
			TDCLDockCommand( kDNewtonName ),
			mNewtonName( nil ),
			mNameLength( 0 )
{
	KUInt32 theLength = UByteSex_FromBigEndian( ((SVersionInfo*) inData)->fLength );

	KDEBUG2( "sizeof( SVersionInfo ) = %i; fLength = %i",
		(int) sizeof( SVersionInfo ),
		(int) theLength );

	if (theLength > sizeof( SVersionInfo ))
	{
		theLength = sizeof( SVersionInfo );
	}
	
	// Copie des informations.
	(void) ::memcpy( &mVersionInfo, inData, theLength );

	// Then, I allocate the name and I copy it.

	// The name is said to be null terminated. This is checked here.
	// (the principle here is that I'm not sure of the data sent by the Newton).
	KUInt32	theNewtonNameLength = inLength - theLength - sizeof(mVersionInfo.fLength);
	
	// The Length should me a multiple of two (the name is encoded in wide chars)
	if (theNewtonNameLength & 0x1)
	{
		throw DCLBadDockCmd;
	}
	
	mNewtonName = (KUInt16*) ::malloc( theNewtonNameLength );
	if ((theNewtonNameLength > 0) && (mNewtonName == nil))
	{
		throw DCLMemError;
	}
	
	(void) ::memcpy(
			(void*) mNewtonName,
			&((const char*) inData)[theLength + sizeof(mVersionInfo.fLength)],
			theNewtonNameLength );
	
	// Let's check it's null terminated.
	// I divide the length by two.
	mNameLength = (theNewtonNameLength / 2) - 1;	// Without the terminator.

	// I check the null terminator.
	if (mNewtonName[mNameLength])
	{
		throw DCLBadDockCmd;
	}
}

// ------------------------------------------------------------------------- //
//  * ~TDCLDockCmdNewtonName( void )
// ------------------------------------------------------------------------- //
TDCLDockCmdNewtonName::~TDCLDockCmdNewtonName( void )
{
	// I free the name pointer.
	if (mNewtonName)
	{
		::free( (void*) mNewtonName );
	}
}

// =============================== //
// MOUNT TAPE U1439 ON B3, NO RING //
// =============================== //
