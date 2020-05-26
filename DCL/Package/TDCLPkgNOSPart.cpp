// ==============================
// Fichier:			TDCLPkgNOSPart.cp
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
// The Original Code is TDCLPkgNOSPart.cp.
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
// $Id: TDCLPkgNOSPart.cp,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPkgNOSPart.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>
#include <DCL/Streams/TDCLMemStream.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLPkgNOSPart( const TDCLNSRef&, Boolean )
// -------------------------------------------------------------------------- //
TDCLPkgNOSPart::TDCLPkgNOSPart(
		const TDCLNSRef& inObject,
		Boolean inFourBytesPadding /* = false */ )
	:
		mObject( inObject ),
		mDirty( true ),
		mFourBytesPadding( inFourBytesPadding )
{
	if (!inObject.IsFrame())
	{
		throw DCLNS( kNSErrNotAFrame );
	}
}

// -------------------------------------------------------------------------- //
//  * TDCLPkgNOSPart( KUInt32, const void*, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLPkgNOSPart::TDCLPkgNOSPart(
			KUInt32 inOffset,
			const void* inData,
			KUInt32 inSize )
	:
		TDCLPkgRelocatablePart( inOffset, inData, inSize ),
		mDirty( false ),
		mFourBytesPadding( false )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPkgNOSPart( void )
// -------------------------------------------------------------------------- //
TDCLPkgNOSPart::~TDCLPkgNOSPart( void )
{
	// This space for rent.
}

// -------------------------------------------------------------------------- //
//  * GetObject( void )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLPkgNOSPart::GetObject( void )
{
	if (mObject.IsNIL())
	{
		// On décode avec un flux mémoire.
		TDCLMemStream theBufferStream( DoGetBuffer(), DoGetSize() );
		TDCLPkgDecoder thePkgDecoder( &theBufferStream, GetOffset() );
		mObject = thePkgDecoder.GetNextObject();

		// L'objet est synchronisé avec les données.
		mDirty = false;

		// On note l'alignement.
		mFourBytesPadding = thePkgDecoder.GetFourBytesPadding();
	}

	return mObject;
}

// -------------------------------------------------------------------------- //
//  * IsNOSPart( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgNOSPart::IsNOSPart( void ) const
{
	return true;
}

// -------------------------------------------------------------------------- //
//  * IsOffsetDependant( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgNOSPart::IsOffsetDependant( void ) const
{
	return true;
}

// -------------------------------------------------------------------------- //
//  * IsDirty( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgNOSPart::IsDirty( void ) const
{
	return mDirty;
}

// -------------------------------------------------------------------------- //
//  * Encode( KUInt32, void**, KUInt32* ) const
// -------------------------------------------------------------------------- //
void
TDCLPkgNOSPart::Encode(
		KUInt32 inOffset,
		void** ioBuffer,
		KUInt32* ioSize ) const
{
	// On encode avec un flux mémoire.
	TDCLMemStream theBufferStream;
	TDCLPkgEncoder thePkgEncoder(
			&theBufferStream,
			inOffset,
			mFourBytesPadding );
	thePkgEncoder.AddObject( mObject );
	mDirty = false;

	// Copie des données.
	KUInt32 theSize = theBufferStream.GetBufferSize();
	*ioSize = theSize;
	*ioBuffer = ::realloc( *ioBuffer, theSize );
	(void) ::memcpy( *ioBuffer, theBufferStream.GetBuffer(), theSize );
}

// ========================================================================= //
// Some people claim that the UNIX learning curve is steep, but at least you //
// only have to climb it once.                                               //
// ========================================================================= //
