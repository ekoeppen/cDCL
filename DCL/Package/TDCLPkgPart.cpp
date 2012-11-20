// ==============================
// Fichier:			TDCLPkgPart.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			6/4/2004
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
// The Original Code is TDCLPkgPart.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPkgPart.cp,v 1.3 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPkgPart.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLPkgPart( void )
// -------------------------------------------------------------------------- //
TDCLPkgPart::TDCLPkgPart( void )
	:
		mOffset( 0 ),
		mBuffer( nil ),
		mSize( 0 )
{
	mBuffer = ::malloc( 0 );
}

// -------------------------------------------------------------------------- //
//  * TDCLPkgPart( KUInt32, const void*, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLPkgPart::TDCLPkgPart( KUInt32 inOffset, const void* inData, KUInt32 inSize )
	:
		mOffset( inOffset ),
		mBuffer( nil ),
		mSize( inSize )
{
	mBuffer = ::malloc( inSize );
	if ((inSize != 0) && (mBuffer == nil))
	{
		throw DCLMemError;
	}
	
	// Copie.
	(void) ::memcpy( mBuffer, inData, inSize );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPkgPart( void )
// -------------------------------------------------------------------------- //
TDCLPkgPart::~TDCLPkgPart( void )
{
	if (mBuffer)
	{
		::free( mBuffer );
	}
}

// -------------------------------------------------------------------------- //
//  * IsNOSPart( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgPart::IsNOSPart( void ) const
{
	return false;
}

// -------------------------------------------------------------------------- //
//  * GetSize( KUInt32 ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLPkgPart::GetSize( KUInt32 inOffset ) const
{
	// Faut-il ré-encoder?
	if (((inOffset != mOffset) && IsOffsetDependant())
		|| (IsDirty()))
	{
		Encode( inOffset, &mBuffer, &mSize );
	}

	// Mise à jour du décalage.
	mOffset = inOffset;

	return mSize;
}

// -------------------------------------------------------------------------- //
//  * GetBuffer( KUInt32 ) const
// -------------------------------------------------------------------------- //
const void*
TDCLPkgPart::GetBuffer( KUInt32 inOffset ) const
{
	// Faut-il ré-encoder?
	if (((inOffset != mOffset) && IsOffsetDependant())
		|| (IsDirty()))
	{
		Encode( inOffset, &mBuffer, &mSize );
	}
	
	// Mise à jour du décalage.
	mOffset = inOffset;

	return mBuffer;
}

// -------------------------------------------------------------------------- //
//  * IsOffsetDependant( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgPart::IsOffsetDependant( void ) const
{
	return false;
}

// -------------------------------------------------------------------------- //
//  * IsDirty( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgPart::IsDirty( void ) const
{
	return false;
}

// -------------------------------------------------------------------------- //
//  * Encode( KUInt32, void**, KUInt32* ) const
// -------------------------------------------------------------------------- //
void
TDCLPkgPart::Encode(
		KUInt32 /* inOffset */,
		void** /* ioBuffer */,
		KUInt32* /* ioSize */ ) const
{
}

// ============================================ //
// Avoid strange women and temporary variables. //
// ============================================ //
