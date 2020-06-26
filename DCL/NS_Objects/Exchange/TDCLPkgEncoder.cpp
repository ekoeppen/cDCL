// ==============================
// Fichier:			TDCLPkgEncoder.cp
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
// The Original Code is TDCLPkgEncoder.cp.
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
// $Id: TDCLPkgEncoder.cp,v 1.10 2006/02/12 01:23:56 pguyot Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Math/UTInt64.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * TDCLPkgEncoder( TDCLRandomAccessStream*, KUInt32, Boolean )
// ------------------------------------------------------------------------- //
TDCLPkgEncoder::TDCLPkgEncoder(
					TDCLRandomAccessStream* inStream,
					KUInt32 inOffset,
					Boolean inFourBytesPadding /* = false */ )
	:
		TDCLNSEncoder( inStream ),
		mOriginOffset( inStream->GetCursor() - inOffset ),
		mOffset( inOffset ),
		mPartOffset( inOffset ),
		mFourBytesPadding( inFourBytesPadding ),
		mStack( nil ),
		mStackAllocatedSize( 0 ),
		mStackSize( 0 ),
		mInSlottedObject( false )
{
	Init();
}

// ------------------------------------------------------------------------- //
//  * TDCLPkgEncoder( TDCLRandomAccessStream*, Boolean )
// ------------------------------------------------------------------------- //
TDCLPkgEncoder::TDCLPkgEncoder(
					TDCLRandomAccessStream* inStream,
					Boolean inFourBytesPadding /* = false */ )
	:
		TDCLNSEncoder( inStream ),
		mOriginOffset( 0 ),
		mOffset( UTInt64::ToKUInt32( inStream->GetCursor() ) ),
		mPartOffset( mOffset ),
		mFourBytesPadding( inFourBytesPadding ),
		mStack( nil ),
		mStackAllocatedSize( 0 ),
		mStackSize( 0 ),
		mInSlottedObject( false )
{
	Init();
}

// ------------------------------------------------------------------------- //
//  * ~TDCLPkgEncoder( void )
// ------------------------------------------------------------------------- //
TDCLPkgEncoder::~TDCLPkgEncoder( void )
{
	if (mStack)
	{
		::free( mStack );
	}
}

// ------------------------------------------------------------------------- //
//  * AddObject( const TDCLNSRef& inObject )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::AddObject( const TDCLNSRef& inObject )
{
	// Est-ce une référence?
	if (inObject.IsRealPtr())
	{
		if (mInSlottedObject)
		{
			// Si on est dans un objet, on ajoute un pointeur et l'objet
			// dans la pile s'il n'a pas encore été encodé.

			// L'objet a-t-il déjà été encodé?
			KUIntPtr theOffset;
			if (WasObjectEncoded(
						inObject.GetPointer(),
						nil,
						&theOffset ))
			{
				// Oui.
				// Ajout du pointeur directement.
				GetOutputStream()->PutLong( theOffset + 1 );
			} else {
				// Non. On pousse dans la pile et on l'encodera plus tard.
				PushToStack( inObject );
		
				GetOutputStream()->PutLong( 0 ); // Pointeur à écrire plus tard.
			}
			
			mOffset += 4;
		} else {
			// Sinon, on ajoute donc l'objet directement.
			TDCLNSEncoder::AddObject( inObject );
		}
	} else {
		// C'est une référence.
		PutRef( inObject );
	}
}

// ------------------------------------------------------------------------- //
//  * PutObject( const TDCLNSObject*, KUInt32, KUIntPtr* )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::PutObject(
					const TDCLNSObject*	inObject,
					KUInt32 /* inObjectID */,
					KUIntPtr* outCookie )
{
	// On est dans un objet.
	mInSlottedObject = true;

	// On stocke le décalage pour les futurs pointeurs vers cet objet.
	*outCookie = mOffset;
	
	// Flux.
	TDCLRandomAccessStream* theStream = GetOutputStream();
	
	// Position actuelle.
	KSInt64 theCurrentOffset = theStream->GetCursor();

	// Écriture des pointeurs (on vide la pile).
	// en commençant par le début de la pile.
	KUInt32 newStackSize = mStackSize;
	KUInt32 stackIndexPlusOne;
	for (stackIndexPlusOne = mStackSize; stackIndexPlusOne > 0; stackIndexPlusOne--)
	{
		SStackItem* theItem = &mStack[stackIndexPlusOne - 1];
		if (theItem->fObjectPtr == inObject)
		{
			// Trouvé.
			// On écrit le pointeur.
			theStream->SetCursor(
						mOriginOffset + theItem->fPtrOffset,
						TDCLRandomAccessStream::kFromStart );
			
			// Écriture du pointeur.
			GetOutputStream()->PutLong( mOffset + 1 );
			
			// Suppression de cet élément de la pile.
			if (stackIndexPlusOne < newStackSize)
			{
				(void) ::memmove(
						(void*) theItem,
						(const void*) &mStack[stackIndexPlusOne],
						sizeof( SStackItem ) * (newStackSize - stackIndexPlusOne));
			}
			
			--newStackSize;
		}
	}
	
	if (newStackSize != mStackSize)
	{
		// On redimensionne.
		mStackSize = newStackSize;
		
		if ((mStackAllocatedSize - mStackSize) > kStackDecrThreshold)
		{
			mStackAllocatedSize = mStackSize + kStackChunkSize;
			mStack = (SStackItem*)
				::realloc( mStack, sizeof(SStackItem) * mStackAllocatedSize );
		}
			
		// Ça veut aussi dire qu'on a déplacé le curseur.
		theStream->SetCursor(
					theCurrentOffset,
					TDCLRandomAccessStream::kFromStart );
	}
	
	// Écriture de l'objet.
	inObject->ToPkg( this, &mOffset );
	
	AlignStream( &mOffset );
	
	// On n'est plus dans un objet.
	mInSlottedObject = false;
	
	// Ajout des éléments de la pile (par AddObject), tant qu'il y en a,
	// en commençant par la fin.
	while (mStackSize > 0)
	{
		TDCLNSRef theRef( mStack[mStackSize - 1].fObjectPtr );
		TDCLNSEncoder::AddObject( theRef );
	}
}

// ------------------------------------------------------------------------- //
//  * PutPrecedent( KUInt32, KUIntPtr )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::PutPrecedent( KUInt32 /* inRefID */, KUIntPtr inCookie )
{
	// inCookie est le décalage vers l'objet.
	// (qui est multiple de 4).
	// On rajoute 1 pour avoir un pointeur.
	GetOutputStream()->PutLong( inCookie + 1 );
	mOffset += 4;
}

// ------------------------------------------------------------------------- //
//  * PutRef( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::PutRef( const TDCLNSRef& inRef )
{
	GetOutputStream()->PutLong( inRef.RefOf() );
	mOffset += 4;
}

// ------------------------------------------------------------------------- //
//  * Init( void )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::Init( void )
{
	// Création de la pile.
	mStack = (SStackItem*) ::malloc( kStackChunkSize * sizeof( SStackItem ) );
	mStackAllocatedSize = kStackChunkSize;

	// Écriture des 12 premiers octets (il nous reste alors une référence
	// vers le premier objet).
	TDCLRandomAccessStream* theStream = GetOutputStream();
	
	theStream->PutLong( k16BytesArray );	// tableau de taille 16
	if (mFourBytesPadding)
	{
		theStream->PutLong( k4BytesPadding );	// alignement sur 4 octets
	} else {
		theStream->PutLong( k8BytesPadding );	// alignement sur 8 octets
	}
	theStream->PutLong( kNILClass );	// classe (NIL)

	mOffset += 16;
	
	// L'objet est juste après.
	theStream->PutLong( mOffset + 1 );	// pointeur sur l'objet (+1).
	
	mInSlottedObject = false;
}

// ------------------------------------------------------------------------- //
//  * PushToStack( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::PushToStack( const TDCLNSRef& inObject )
{
	// On redimensionne.
	if (mStackSize == mStackAllocatedSize)
	{
		mStackAllocatedSize += kStackChunkSize;
		mStack = (SStackItem*)
			::realloc( mStack, sizeof(SStackItem) * mStackAllocatedSize );
	}
	mStack[mStackSize].fObjectPtr = inObject.GetPointer();
	mStack[mStackSize].fPtrOffset = mOffset;

	mStackSize++;
}

// ------------------------------------------------------------------------- //
//  * AlignStream( KUInt32* ) const
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::AlignStream( KUInt32* ioOffset ) const
{
	// Alignement.
	if (mFourBytesPadding)
	{
		KUInt32 nbBytes = 4 - ((*ioOffset - mPartOffset) % 4);
		if (nbBytes != 4)
		{
			TDCLStream* theStream = GetOutputStream();
			
			*ioOffset += nbBytes;

			for ( ; nbBytes > 0; nbBytes-- )
			{
				theStream->PutByte( 0 );
			}
		}
	} else {
		KUInt32 nbBytes = 8 - ((*ioOffset - mPartOffset) % 8);
		if (nbBytes != 8)
		{
			TDCLStream* theStream = GetOutputStream();
			
			*ioOffset += nbBytes;

			for ( ; nbBytes > 0; nbBytes-- )
			{
				theStream->PutByte( 0 );
			}
		}
	}
}

// ------------------------------------------------------------------------- //
//  * AlignOffset( KUInt32* ) const
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::AlignOffset( KUInt32* ioOffset ) const
{
	// Alignement.
	if (mFourBytesPadding)
	{
		KUInt32 nbBytes = 4 - ((*ioOffset - mPartOffset) % 4);
		if (nbBytes != 4)
		{
			*ioOffset += nbBytes;
		}
	} else {
		KUInt32 nbBytes = 8 - ((*ioOffset - mPartOffset) % 8);
		if (nbBytes != 8)
		{
			*ioOffset += nbBytes;
		}
	}
}

// ------------------------------------------------------------------------- //
//  * AddMap( const TDCLNSFrame*, KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLPkgEncoder::AddMap( const TDCLNSFrame* frame, KUInt32 mapRef )
{
    mEncodedMaps.push_back(std::make_pair(frame, mapRef));
}

// ------------------------------------------------------------------------- //
//  * GetBestSupermap(const TDCLNSFrame*, KUInt32*, KUInt32*)
// ------------------------------------------------------------------------- //
Boolean
TDCLPkgEncoder::GetBestSupermap(const TDCLNSFrame* frame, KUInt32* outSharedKeysCount, KUInt32* outSupermapRef)
{
    KUInt32 thisSize = frame->GetLength();
    KUInt32 bestCandidateRef = 0;
    KUInt32 bestCandidateSize = 0;
    for (auto p : mEncodedMaps) {
        KUInt32 candidateSize = p.first->GetLength();
        bool mismatch = false;
        if (candidateSize > thisSize) continue;
        for (int i = 0; i < candidateSize; i++) {
            if (p.first->GetKey(i).ToSymbol().Compare(frame->GetKey(i).ToSymbol()) != 0) {
                mismatch = true;
                break;
            }
        }
        if (mismatch) continue;
        if (candidateSize > bestCandidateSize) {
            bestCandidateRef = p.second;
            bestCandidateSize = candidateSize;
        }
    }
    if (bestCandidateSize > 0) {
        *outSharedKeysCount = bestCandidateSize;
        *outSupermapRef = bestCandidateRef;
        return true;
    }
    return false;
}

// ==================================================================== //
// If the code and the comments disagree, then both are probably wrong. //
//                 -- Norm Schryer                                      //
// ==================================================================== //
