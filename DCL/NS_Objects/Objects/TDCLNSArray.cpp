// ==============================
// Fichier:			TDCLNSArray.cp
// Projet:			Desktop Connection Library
//
// Créé le:			07/08/2002
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
// The Original Code is TDCLNSArray.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLNSArray.cp,v 1.10 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>

// C++
#include <new>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSArray( void )
// ------------------------------------------------------------------------- //
TDCLNSArray::TDCLNSArray( void )
	:
		mClass( KDCLSYM::kSYMarray ),
		mSize( 0 ),
		mItems( nil )
{
	mItems = (TDCLNSRef*) ::malloc( 0 );
}

// ------------------------------------------------------------------------- //
//  * TDCLNSArray( KUInt32, const TDCLNSRef& = TDCLNSRef::kNILREF )
// ------------------------------------------------------------------------- //
TDCLNSArray::TDCLNSArray(
				KUInt32 inSize,
				const TDCLNSRef& inRef /* = TDCLNSRef::kNILREF */ )
	:
		mClass( KDCLSYM::kSYMarray ),
		mSize( inSize ),
		mItems( nil )
{
	mItems = (TDCLNSRef*) ::malloc( inSize * sizeof(TDCLNSRef) );
	
	if (inSize > 0)
	{
		if (mItems == nil)
		{
			throw DCLMemError;
		}
	
		// Initialisation des éléments.
		KUInt32 indexItems;
		for ( indexItems = 0; indexItems < inSize; indexItems++ )
		{
			new (&mItems[ indexItems ]) TDCLNSRef( inRef );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLNSArray( const TDCLNSArray& )
// ------------------------------------------------------------------------- //
TDCLNSArray::TDCLNSArray( const TDCLNSArray& inCopy )
	:
		TDCLNSObject( inCopy ),
		mClass( inCopy.mClass ),
		mSize( inCopy.mSize ),
		mItems( nil )
{
	KUInt32 theSize = mSize;
	mItems = (TDCLNSRef*) ::malloc( theSize * sizeof(TDCLNSRef) );
	
	if (theSize > 0)
	{
		if (mItems == nil)
		{
			throw DCLMemError;
		}
	
		// Copie des éléments (surface)
		KUInt32 indexItems;
		for ( indexItems = 0; indexItems < theSize; indexItems++ )
		{
			new (&mItems[ indexItems ]) TDCLNSRef( inCopy.mItems[ indexItems ] );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSArray( void )
// ------------------------------------------------------------------------- //
TDCLNSArray::~TDCLNSArray( void )
{
	// Suppression des éléments
	if (mItems != nil)
	{
		KUInt32 nbItems = mSize;
		KUInt32 indexItems;
		for (indexItems = 0; indexItems < nbItems; indexItems++)
		{
			mItems[indexItems].TDCLNSRef::~TDCLNSRef();
		}
	
		::free( mItems );
	}
}

// ------------------------------------------------------------------------- //
//  * operator = ( const TDCLNSArray& inCopy )
// ------------------------------------------------------------------------- //
TDCLNSArray&
TDCLNSArray::operator = ( const TDCLNSArray& inCopy )
{
	// Suppression des anciennes références.
	KUInt32 theSize = mSize;
	KUInt32 indexItems;
	for (indexItems = 0; indexItems < theSize; indexItems++)
	{
		mItems[indexItems].TDCLNSRef::~TDCLNSRef();
	}

	// Copie des nouvelles valeurs.
	theSize = inCopy.mSize;
	mSize = theSize;

	mItems = (TDCLNSRef*) ::realloc( mItems, theSize * sizeof(TDCLNSRef) );
	
	if (theSize > 0)
	{
		if (mItems == nil)
		{
			throw DCLMemError;
		}
	
		// Copie des éléments (surface)
		for ( indexItems = 0; indexItems < theSize; indexItems++ )
		{
			new (&mItems[ indexItems ]) TDCLNSRef( inCopy.mItems[ indexItems ] );
		}
	}
	
	return *this;
}

// ------------------------------------------------------------------------- //
//  * IsArray( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSArray::IsArray( void ) const
{
	return true;
}

// ------------------------------------------------------------------------- //
//  * Get( KUInt32 ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSArray::Get( KUInt32 inIndex ) const
{
	// Vérification de l'index.
	if (inIndex >= mSize)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}
	
	return mItems[ inIndex ];
}

// ------------------------------------------------------------------------- //
//  * Set( KUInt32, const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSArray::Set( KUInt32 inIndex, const TDCLNSRef& inItem )
{
	// Vérification de l'index.
	if (inIndex >= mSize)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}
	
	mItems[ inIndex ] = inItem;
}

// ------------------------------------------------------------------------- //
//  * Add( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSArray::Add( const TDCLNSRef& inItem )
{
	// Agrandissement du tableau.
	mSize++;
	mItems = (TDCLNSRef*) ::realloc( mItems, mSize * sizeof(TDCLNSRef) );
	
	if (mItems == nil)
	{
		throw DCLMemError;
	}
	
	// Ajout de l'élément (initialisation de l'élément dans le tableau)
	new (&mItems[ mSize - 1 ]) TDCLNSRef( inItem );
}

// ------------------------------------------------------------------------- //
//  * RemoveSlot( KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLNSArray::RemoveSlot( KUInt32 inIndex )
{
	// Vérification de l'index.
	if (inIndex >= mSize)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}
	
	// Suppression de l'ancien élément.
	mItems[ inIndex ].TDCLNSRef::~TDCLNSRef();

	mSize--;

	// ABC-EF
	// ABCEF
	// mSize = 5
	// inIndex = 3
	// mSize - inIndex = 2

	// Déplacement des éléments.
	(void) ::memmove(
				(void*) &mItems[inIndex],
				(const void*) &mItems[inIndex + 1],
				(mSize - inIndex) * sizeof(TDCLNSRef));
	
	// Rétrécicement du tableau.
	mItems = (TDCLNSRef*) ::realloc( mItems, mSize * sizeof(TDCLNSRef) );
	
	if (mItems == nil)
	{
		throw DCLMemError;
	}
}

// ------------------------------------------------------------------------- //
//  * Insert( KUInt32, const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSArray::Insert( KUInt32 inIndex, const TDCLNSRef& inItem )
{
	// Vérification de l'index.
	if (inIndex > mSize)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}
	
	// Agrandissement du tableau.
	mSize++;
	mItems = (TDCLNSRef*) ::realloc( mItems, mSize * sizeof(TDCLNSRef) );
	
	if (mItems == nil)
	{
		throw DCLMemError;
	}
	
	// ABCDEF
	// ABC-DEF
	// mSize = 7
	// inIndex = 3
	// mSize - inIndex - 1 = 3
	
	// Déplacement des anciens éléments.
	(void) ::memmove(
				(void*) &mItems[inIndex + 1],
				(const void*) &mItems[inIndex],
				(mSize - inIndex - 1) * sizeof(TDCLNSRef));
	
	// Ajout de l'élément (initialisation de l'élément dans le tableau)
	new (&mItems[ inIndex ]) TDCLNSRef( inItem );
}

// ------------------------------------------------------------------------- //
//  * Clone( void ) const
// ------------------------------------------------------------------------- //
TDCLNSObject*
TDCLNSArray::Clone( void ) const
{
	return new TDCLNSArray( *this );
}

// ------------------------------------------------------------------------- //
//  * ClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSArray::ClassOf( void ) const
{
	return mClass;
}

// ------------------------------------------------------------------------- //
//  * PrimClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSArray::PrimClassOf( void ) const
{
	return KDCLSYM::kSYMarray;
}

// ------------------------------------------------------------------------- //
//  * SetClass( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSArray::SetClass( const TDCLNSRef& inClass )
{
	mClass = inClass;
}

// ------------------------------------------------------------------------- //
//  * FromNSOF( TDCLNSOFDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSArray*
TDCLNSArray::FromNSOF(
				TDCLNSOFDecoder* inDecoder,
				Boolean isPlain )
{
	TDCLStream* theStream = inDecoder->GetInputStream();
	
	KUInt32 nbItems = theStream->GetXLong();

	// Création de l'objet. Je mets tout plein de NIL.
	TDCLNSArray* theResult = new TDCLNSArray( nbItems );

	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ) );

	// Récupération de la classe si nécessaire.
	if (!isPlain)
	{
		theResult->mClass = inDecoder->GetNextObject();
	}

	// Ensuite, on remplit.
	KUInt32 indexItems;
	for (indexItems = 0; indexItems < nbItems; indexItems++)
	{
		theResult->Set( indexItems, inDecoder->GetNextObject() );
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * FromPkg( TDCLPkgDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSArray*
TDCLNSArray::FromPkg( TDCLPkgDecoder* inDecoder )
{
	TDCLStream* theStream = inDecoder->GetInputStream();

	// Décalage de l'objet.
	KUInt32 theObjectOffset = inDecoder->GetCurrentStreamOffset();
	
	// Taille
	KUInt32 theFirstLong = theStream->GetLong();
	KUInt32 nbItems = ((theFirstLong >> TDCLPkgDecoder::kSizeShift) - 12) / 4;
	
	// On passe le deuxième long.
	(void) theStream->GetLong();

	// Création de l'objet. Je mets tout plein de NIL.
	TDCLNSArray* theResult = new TDCLNSArray( nbItems );
	
	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ), theObjectOffset );

	// Récupération de la classe.
	theResult->mClass = inDecoder->GetNextObject();

	// Ensuite, on remplit.
	KUInt32 indexItems;
	for (indexItems = 0; indexItems < nbItems; indexItems++)
	{
		theResult->Set( indexItems, inDecoder->GetNextObject() );
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * ToXML( TDCLXMLEncoder*, KUInt32 ) const
// ------------------------------------------------------------------------- //
void
TDCLNSArray::ToXML(
				TDCLXMLEncoder*		inEncoder,
				KUInt32				inObjectID ) const
{
	// Récupération du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();

	// == est un comparateur de surface, sauf pour les symboles.	
	Boolean isPlain = (mClass == KDCLSYM::kSYMarray);
	
	// Balise de début avec l'ID.
	char theOutputStr[32]; 	// Suffisant.
	(void) ::sprintf( theOutputStr, "<array id=\"n%lu\">", inObjectID );
	theOutputStream->PutString( theOutputStr );

	inEncoder->IncrementLevel();
	
	// Classe, au besoin.
	if (!isPlain)
	{
		inEncoder->PutTabulations();
		theOutputStream->PutString( "<class>" );
		inEncoder->AddObject( mClass );
		theOutputStream->PutString( "</class>" );
	}
	
	// Puis les éléments.
	KUInt32 indexItems;
	KUInt32 nbItems = mSize;
	for ( indexItems = 0; indexItems < nbItems; indexItems++ )
	{
		inEncoder->PutTabulations();
		inEncoder->AddObject( mItems[ indexItems ] );
	}	
	
	inEncoder->DecrementLevel();
	inEncoder->PutTabulations();
	
	// Balise de fin.
	theOutputStream->PutString( "</array>" );
}

// ------------------------------------------------------------------------- //
//  * ToNSOF( TDCLNSOFEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSArray::ToNSOF(
				TDCLNSOFEncoder* inEncoder ) const
{
	// Récupération du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();

	// == est un comparateur de surface, sauf pour les symboles.	
	Boolean isPlain = (mClass == KDCLSYM::kSYMarray);
	
	// Ecriture du type.
	if (isPlain)
	{
		theOutputStream->PutByte( KDCLNSOF::kPlainArray );
	} else {
		theOutputStream->PutByte( KDCLNSOF::kArray );
	}

	// Ecriture du nombre d'éléments.
	KUInt32 nbItems = mSize;
	theOutputStream->PutXLong( nbItems );

	// Puis la classe.
	if (!isPlain)
	{
		inEncoder->AddObject( mClass );
	}
	
	// Puis les éléments.
	KUInt32 indexItems;
	for ( indexItems = 0; indexItems < nbItems; indexItems++ )
	{
		inEncoder->AddObject( mItems[ indexItems ] );
	}
}

// ------------------------------------------------------------------------- //
//  * ToText( TDCLTextEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSArray::ToText(
				TDCLTextEncoder* /* inEncoder */ ) const
{
}

// ------------------------------------------------------------------------- //
//  * ToPkg( TDCLPkgEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSArray::ToPkg(
				TDCLPkgEncoder* inEncoder,
				KUInt32* ioOffset ) const
{
	// Récupération du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Ecriture du nombre d'éléments.
	KUInt32 nbItems = mSize;
	KUInt32 theSize = (mSize * 4) + 12;

	if (theSize > TDCLPkgEncoder::kMaxSize)
	{
		throw DCLLimitReachedError;
	}
	
	// Écriture de l'entête.
	theOutputStream->PutLong(
		(theSize << TDCLPkgEncoder::kSizeShift)
		| TDCLPkgEncoder::kObjFlagHeader
		| TDCLPkgEncoder::kObjSlotted );
	
	theOutputStream->PutLong( 0 );
	
	*ioOffset += 8;

	// Classe.
	inEncoder->AddObject( mClass );

	// Puis les éléments.
	KUInt32 indexItems;
	for ( indexItems = 0; indexItems < nbItems; indexItems++ )
	{
		inEncoder->AddObject( mItems[ indexItems ] );
	}
}

// ======================================================================== //
// ... computer hardware progress is so fast.  No other technology since    //
// civilization began has seen six orders of magnitude in performance-price //
// gain in 30 years.                                                        //
//                 -- Fred Brooks                                           //
// ======================================================================== //
