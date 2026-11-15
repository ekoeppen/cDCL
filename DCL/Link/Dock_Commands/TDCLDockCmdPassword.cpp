// ==============================
// Fichier:			TDCLDockCmdPassword.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/08/2001
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
// The Original Code is TDCLDockCmdPassword.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdPassword.cp,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPassword.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// K
#include <K/Crypto/UDES.h>
#include <K/Defines/UByteSex.h>
#include <K/Math/UTInt64.h>

// ------------------------------------------------------------------------- //
//  * TDCLDockCmdPassword( KUInt32, const void* )
// ------------------------------------------------------------------------- //
TDCLDockCmdPassword::TDCLDockCmdPassword( KUInt32 inLength, const void* inData )
	:
		TDCLDockCommand( kDPassword )
{
	// I check that I have 8 bytes for length.
	if (inLength == 8)
	{
		(void) ::memcpy( mCipheredChallenge, inData, 8 );
	} else {
		// I set the data to 0, so I'll surely say bad password.
		mCipheredChallenge[0] = 0;
		mCipheredChallenge[1] = 0;
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLDockCmdPassword( const KUInt32 [2], const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLDockCmdPassword::TDCLDockCmdPassword(
		const KUInt32 inChallenge[2],
		const KUInt16* inPassword /* = nil */ )
	:
		TDCLDockCommand( kDPassword )
{
	// On copie le défi (en traduisant l'indien)
	KUInt64 theCipheredChallenge =
		UTInt64::CreateKUInt64(
			UByteSex_FromBigEndian( inChallenge[0] ),
			UByteSex_FromBigEndian( inChallenge[1] ) );

	// Détermination du pointeur sur le mot de passe.
	// Si inPassword vaut nil, on prend un pointeur sur 0.
	const KUInt16 theEmptyStr = 0;
	const KUInt16* thePasswordString;
	if (inPassword)
	{
		thePasswordString = inPassword;
	} else {
		thePasswordString = &theEmptyStr;
	}

	// On crée la clé à partir du mot de passe.
	KUInt64 theKey;
	UDES::CreateNewtonKey( thePasswordString, &theKey );
	
	// On crypte le défi.
	UDES::NewtonEncodeBlock( theKey, &theCipheredChallenge );
	
	// Puis on recopie en faisant attention à l'indien.
	mCipheredChallenge[0] = UByteSex_ToBigEndian(
								UTInt64::GetHi( theCipheredChallenge ) );
	mCipheredChallenge[1] = UByteSex_ToBigEndian(
								UTInt64::GetLo( theCipheredChallenge ) );
}

// ------------------------------------------------------------------------- //
//  * CreateChallenge( KUInt32[2] )
// ------------------------------------------------------------------------- //
void
TDCLDockCmdPassword::CreateChallenge( KUInt32 outChallenge[2] )
{
	// Pseudo random number generation
	outChallenge[0] =
		(KUInt32) ((::rand() & 0xFF) << 24)
				| ((::rand() & 0xFF) << 16)
				| ((::rand() & 0xFF) << 8)
				| (::rand() & 0xFF);
	outChallenge[1] =
		(KUInt32) ((::rand() & 0xFF) << 24)
				| ((::rand() & 0xFF) << 16)
				| ((::rand() & 0xFF) << 8)
				| (::rand() & 0xFF);
}

// ------------------------------------------------------------------------- //
//  * VerifyPassword( const KUInt32[2], const KUInt16* )
// ------------------------------------------------------------------------- //
Boolean
TDCLDockCmdPassword::VerifyPassword( const KUInt32 inChallenge[2],
									const KUInt16* inPassword /* = nil */ )
{
	// On copie le défi (en traduisant l'indien)
	KUInt64 theCipheredChallenge =
		UTInt64::CreateKUInt64(
			UByteSex_FromBigEndian( inChallenge[0] ),
			UByteSex_FromBigEndian( inChallenge[1] ) );

	// Détermination du pointeur sur le mot de passe.
	// Si inPassword vaut nil, on prend un pointeur sur 0.
	const KUInt16 theEmptyStr = 0;
	const KUInt16* thePasswordString;
	if (inPassword)
	{
		thePasswordString = inPassword;
	} else {
		thePasswordString = &theEmptyStr;
	}

	// Création de la clé à partir du mot de passe.
	KUInt64 theKey;
	UDES::CreateNewtonKey( thePasswordString, &theKey );

	// On crypte le défi.
	UDES::NewtonEncodeBlock( theKey, &theCipheredChallenge );

	// On teste en faisant attention à l'indien.
	return (
		(UByteSex_FromBigEndian( mCipheredChallenge[0] )
				== UTInt64::GetHi( theCipheredChallenge ))
		&& (UByteSex_FromBigEndian( mCipheredChallenge[1] )
				== UTInt64::GetLo( theCipheredChallenge )));
}

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLDockCmdPassword::GetLength( void ) const
{
	return sizeof( mCipheredChallenge );
}

// ------------------------------------------------------------------------- //
//  * GetData( void )
// ------------------------------------------------------------------------- //
const void*
TDCLDockCmdPassword::GetData( void )
{
	// mCipheredChallenge est en grand indien.
	return mCipheredChallenge;
}

// ====================================================================== //
// Okay, Okay -- I admit it.  You didn't change that program that worked  //
// just a little while ago; I inserted some random characters into the    //
// executable.  Please forgive me.  You can recover the file by typing in //
// the code over again, since I also removed the source.                  //
// ====================================================================== //
