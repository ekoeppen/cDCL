// ==============================
// Fichier:			TDCLDockCmdDesktopInfo.cp
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
// The Original Code is TDCLDockCmdDesktopInfo.cp.
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
// $Id: TDCLDockCmdDesktopInfo.cp,v 1.4 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Link/Dock_Commands/TDCLDockCmdDesktopInfo.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/Streams/TDCLMemStream.h>

// --------------------------------------------------------------------------------	//
//  * TDCLDockCmdDesktopInfo( TDCLNSArray&, KUInt32, KUInt32, Boolean, KUInt32 )
// --------------------------------------------------------------------------------	//
TDCLDockCmdDesktopInfo::TDCLDockCmdDesktopInfo(
					TDCLNSArray& inDesktopAppArray,
					KUInt32 inEncKey1,
					KUInt32 inEncKey2,
					Boolean inSelectiveBackup,
					KUInt32 inSessionType,
					KUInt32 inPlatformKind /* = kThis_DType */ )
	:
		TDCLDockCommand( kDDesktopInfo ),
		mDesktopArraySize( 0 ),
		mDesktopArrayPtr( nil ),
		mLength( 0 )
{
	mDesktopInfo.fProtocolVersion = kProtocolVersion;
	mDesktopInfo.fDesktopType = inPlatformKind;
	mDesktopInfo.fEncryptedKey[0] = inEncKey1;
	mDesktopInfo.fEncryptedKey[1] = inEncKey2;
	mDesktopInfo.fSessionType = inSessionType;
	mDesktopInfo.fAllowSelectiveSync = inSelectiveBackup;
	
	// Conversion du tableau en donnŽes NSOF.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( TDCLNSRef( inDesktopAppArray ) );
	
	KUInt32 theArraySize = mDesktopArraySize = theMemStream.GetBufferSize();
	void* theArrayPtr = mDesktopArrayPtr = ::malloc( theArraySize );
	
	(void) ::memcpy( theArrayPtr, theMemStream.GetBuffer(), theArraySize );
	
	// Calcul de la taille totale des donnŽes.
	mLength = sizeof(SDesktopInfo) + theArraySize;
}

// --------------------------------------------------------------------------------	//
//  * ~TDCLDockCmdDesktopInfo( void )
// --------------------------------------------------------------------------------	//
TDCLDockCmdDesktopInfo::~TDCLDockCmdDesktopInfo( void )
{
	if (mDesktopArrayPtr)
	{
		::free( mDesktopArrayPtr );
	}
}

// --------------------------------------------------------------------------------	//
//  * GetLength( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLDockCmdDesktopInfo::GetLength( void ) const
{
	return mLength;
}

// --------------------------------------------------------------------------------	//
//  * SendBody( TDCLPipe*, KUInt32, ProgressFuncPtr, void* )
// --------------------------------------------------------------------------------	//
void
TDCLDockCmdDesktopInfo::SendBody(
					TDCLStream* inStream,
					KUInt32 inLength,
					ProgressFuncPtr inProgressFuncPtr,
					void* inRefCon )
{
	// A - Send the desktop info.
	KUInt32 theLength = sizeof( mDesktopInfo );
	inStream->Write( &mDesktopInfo, &theLength );
	
	// Si inProgressFuncPtr n'est pas nil, on prŽvient ici.
	if (inProgressFuncPtr)
	{
		inStream->FlushOutput();
		(*inProgressFuncPtr)(
				inRefCon,
				((double) (16 + theLength)) / ((double) (16 + inLength)) );
	}

	// B - Send the desktop array.
	theLength = mDesktopArraySize;
	inStream->Write( mDesktopArrayPtr, &theLength );
}

// ===================================================================== //
// There are two major products that come out of Berkeley: LSD and UNIX. //
// We don't believe this to be a coincidence.                            //
//                 -- Jeremy S. Anderson                                 //
// ===================================================================== //
