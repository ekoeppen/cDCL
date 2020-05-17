// ==============================
// Fichier:			TDCLPkgDecoder.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			21/2/2004
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
// The Original Code is TDCLPkgDecoder.cp.
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
// $Id: TDCLPkgDecoder.cp,v 1.10 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgDecoder.h>

// K
#include <K/Math/UTInt64.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSBinary.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLPkgDecoder( TDCLRandomAccessStream*, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLPkgDecoder::TDCLPkgDecoder(
			TDCLRandomAccessStream* inStream,
			KUInt32 inOffset )
	:
		TDCLNSDecoder( inStream ),
		mTopLevel( true )
{
	mOriginOffset = inStream->GetCursor() - inOffset;
	mMaxOffset = mOriginOffset;
}

// -------------------------------------------------------------------------- //
//  * TDCLPkgDecoder( TDCLRandomAccessStream* )
// -------------------------------------------------------------------------- //
TDCLPkgDecoder::TDCLPkgDecoder( TDCLRandomAccessStream* inStream )
	:
		TDCLNSDecoder( inStream ),
		mTopLevel( true )
{
	mOriginOffset = inStream->GetCursor();
	mMaxOffset = mOriginOffset;
}

// -------------------------------------------------------------------------- //
//  * ReadFirstArray( TDCLRandomAccessStream* )
// -------------------------------------------------------------------------- //
void
TDCLPkgDecoder::ReadFirstArray( TDCLRandomAccessStream* inStream )
{
	// Lecture du premier tableau.
	KUInt32 theLong = inStream->GetLong();
	if (theLong != k16BytesArray)
	{
		(void) ::printf( "%.8X != %.8X", theLong, k16BytesArray );
		throw DCLNS( kError_Bad_Package );
	}

	theLong = inStream->GetLong();
	if (theLong == k8BytesPadding)
	{
		mFourBytesPadding = false;
	} else if (theLong == k4BytesPadding) {
		mFourBytesPadding = true;
	} else {
		throw DCLNS( kError_Bad_Package );
	}
	
	// Ensuite, on doit avoir NIL.
	theLong = inStream->GetLong();
	if (theLong != kNILClass)
	{
		throw DCLNS( kError_Bad_Package );
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPkgDecoder( void )
// -------------------------------------------------------------------------- //
TDCLPkgDecoder::~TDCLPkgDecoder( void )
{
}

// -------------------------------------------------------------------------- //
//  * GetNextObject( void )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLPkgDecoder::GetNextObject( void )
{
	Boolean topLevel = mTopLevel;
	TDCLNSRef theResult;
	
	TDCLRandomAccessStream* theStream = GetInputStream();
	
	if (topLevel)
	{
		mTopLevel = false;
		ReadFirstArray( theStream );
	}
	
	// Lecture de l'objet.
	KUInt32 theLong = theStream->GetLong();
	KSInt64 theCurrentPosition = theStream->GetCursor();
	if (theCurrentPosition > mMaxOffset)
	{
		mMaxOffset = theCurrentPosition;
	}	
	
	if ((theLong & kTagMask) == kPointerTag)
	{
		// Félicitation, c'est un pointeur.
		// Allons à l'objet.
		theLong -= kPointerTag;
		
		// Un tel objet a-t-il déjà été encodé?
		theResult = GetNextObjectWithCookie( theLong );
		
		// Non.
		if (theResult.IsNIL())
		{
			theStream->SetCursor(
					mOriginOffset + theLong,
					TDCLRandomAccessStream::kFromStart );
			
			// Entête.
			theLong = theStream->GetLong();
			KUInt32 theNextLong = theStream->GetLong();
			if (theNextLong != 0)
			{
				throw DCLNS( kError_Bad_Package );
			}

			KUInt32 theFlags = theLong & kObjFlagsMask;
			if ((theFlags & kObjFlagHeader) == 0)
			{
				throw DCLNS( kError_Bad_Package );
			}
			
			if (theFlags & kObjSlotted)
			{
				// Retour arrière.
				theStream->SetCursor( -8, TDCLRandomAccessStream::kFromCursor );

				// Tableau ou structure.
				if (theFlags & kObjFrame)
				{
					// Structure.
					theResult = TDCLNSFrame::FromPkg( this );
				} else {
					// Tableau.
					theResult = TDCLNSArray::FromPkg( this );
				}
			} else {
				if (theFlags & kObjFrame)
				{
					throw DCLNS( kError_Bad_Package );
				}
				
				// Binaire ou symbole.
				KUInt32 theClass = theStream->GetLong();
				
				// Retour arrière.
				theStream->SetCursor( -12, TDCLRandomAccessStream::kFromCursor );
				if (theClass == kSymbolClass)
				{
					// Symbole
					theResult = TDCLNSSymbol::FromPkg( this );
				} else {
					// Binaire.
					theResult = TDCLNSBinary::FromPkg( this );
				}
			}

			KSInt64 thePostObjectPosition = theStream->GetCursor();
			if (thePostObjectPosition > mMaxOffset)
			{
				mMaxOffset = thePostObjectPosition;
			}
			
			// On revient juste après le pointeur.
			theStream->SetCursor(
					theCurrentPosition,
					TDCLRandomAccessStream::kFromStart );
		}
	} else {
		theResult = TDCLNSRef::Ref( theLong );
	}

	if (topLevel)
	{
		mTopLevel = true;
		
		// Déplacement du flux à la fin.
		if (mFourBytesPadding)
		{
			KSInt64 padding = mMaxOffset % 4;
			if (padding != 0)
			{
				mMaxOffset += 4 - padding;
			}
		} else {
			KSInt64 padding = mMaxOffset % 8;
			if (padding != 0)
			{
				mMaxOffset += 8 - padding;
			}
		}

		theStream->SetCursor(
			mMaxOffset,
			TDCLRandomAccessStream::kFromStart );
	}

	return theResult;
}

// ============================= //
// Pause for storage relocation. //
// ============================= //
