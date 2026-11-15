// ==============================
// Fichier:			TDCLNSFrame.cp
// Projet:			Desktop Connection Library
//
// Créé le:			06/08/2002
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
// The Original Code is TDCLNSFrame.cp.
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
// $Id: TDCLNSFrame.cp,v 1.11 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// C++
#include <new>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSFrame::TDCLNSFrame( void )
	:
		mLength( 0 ),
		mCapacity( 0 ),
		mKeys( nil ),
		mValues( nil ),
		mKeysAreSorted( true ),
		mIsFunction( false )
{
	mKeys = (TDCLNSRef*) ::malloc( kListChunkSize * sizeof (TDCLNSRef) );
	mValues = (TDCLNSRef*) ::malloc( kListChunkSize * sizeof (TDCLNSRef) );
	
	mCapacity = kListChunkSize;

	if ((mKeys == nil) || (mValues == nil))
	{
		throw DCLMemError;
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLNSFrame( KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSFrame::TDCLNSFrame( KUInt32 inCapacity )
	:
		mLength( 0 ),
		mCapacity( inCapacity ),
		mKeys( nil ),
		mValues( nil ),
		mKeysAreSorted( true ),
		mIsFunction( false )
{
	mKeys = (TDCLNSRef*) ::malloc( inCapacity * sizeof (TDCLNSRef) );
	mValues = (TDCLNSRef*) ::malloc( inCapacity * sizeof (TDCLNSRef) );

	if (inCapacity > 0)
	{
		if ((mKeys == nil) || (mValues == nil))
		{
			throw DCLMemError;
		}
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLNSFrame( const TDCLNSFrame& )
// ------------------------------------------------------------------------- //
TDCLNSFrame::TDCLNSFrame( const TDCLNSFrame& inCopy )
	:
		TDCLNSObject( inCopy ),
		mLength( inCopy.mLength ),
		mCapacity( inCopy.mLength ),
		mKeys( nil ),
		mValues( nil ),
		mKeysAreSorted( inCopy.mKeysAreSorted ),
		mIsFunction( inCopy.mIsFunction )
{
	// Copie des valeurs et des clés.
	KUInt32 nbPairs = mLength;
	mKeys = (TDCLNSRef*) ::malloc( nbPairs * sizeof (TDCLNSRef) );
	mValues = (TDCLNSRef*) ::malloc( nbPairs * sizeof (TDCLNSRef) );
	
	if (nbPairs > 0)
	{
		if ((mKeys == nil) || (mValues == nil))
		{
			throw DCLMemError;
		}

		KUInt32 indexPairs;
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			// Copie de la clé.
			new ( &mKeys[indexPairs] ) TDCLNSRef( inCopy.mKeys[indexPairs] );

			// Copie de la valeur.
			new ( &mValues[indexPairs] ) TDCLNSRef( inCopy.mValues[indexPairs] );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSFrame::~TDCLNSFrame( void )
{
	// Suppression des clés et des références.
	if ((mKeys != nil) && (mValues != nil))
	{
		KUInt32 nbPairs = mLength;
		KUInt32 indexPairs;
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			mKeys[indexPairs].TDCLNSRef::~TDCLNSRef();
			mValues[indexPairs].TDCLNSRef::~TDCLNSRef();
		}
	
		::free( mKeys );
		::free( mValues );
	} else {
		// Echec lors de l'initialisation.
		if (mKeys != nil)
		{
			::free( mKeys );
		}

		if (mValues != nil)
		{
			::free( mValues );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * operator = ( const TDCLNSFrame& inCopy )
// ------------------------------------------------------------------------- //
TDCLNSFrame&
TDCLNSFrame::operator = ( const TDCLNSFrame& inCopy )
{
	// Suppression des anciennes références.
	KUInt32 nbPairs = mLength;
	KUInt32 indexPairs;
	for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
	{
		mKeys[indexPairs].TDCLNSRef::~TDCLNSRef();
		mValues[indexPairs].TDCLNSRef::~TDCLNSRef();
	}

	// Copie des nouvelles valeurs et des clés.
	nbPairs = inCopy.mLength;
	mLength = nbPairs;
	mKeys = (TDCLNSRef*) ::realloc( mKeys, nbPairs * sizeof (TDCLNSRef) );
	mValues = (TDCLNSRef*) ::realloc( mValues, nbPairs * sizeof (TDCLNSRef) );
	
	if (nbPairs > 0)
	{
		if ((mKeys == nil) || (mValues == nil))
		{
			throw DCLMemError;
		}

		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			// Copie de la clé.
			new ( &mKeys[indexPairs] ) TDCLNSRef( inCopy.mKeys[indexPairs] );

			// Copie de la valeur.
			new ( &mValues[indexPairs] ) TDCLNSRef( inCopy.mValues[indexPairs] );
		}
	}
	
	return *this;
}

// ------------------------------------------------------------------------- //
//  * IsFrame( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSFrame::IsFrame( void ) const
{
	return true;
}

// ------------------------------------------------------------------------- //
//  * IsFunction( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSFrame::IsFunction( void ) const
{
	return mIsFunction;
}

// ------------------------------------------------------------------------- //
//  * HasSlot( const TDCLNSSymbol& ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSFrame::HasSlot( const TDCLNSSymbol& inSymbol ) const
{
	Boolean theResult = false;	// Supposons que la clé ne soit pas là.
	
	// Index de la clé, si elle est dans le dictionnaire.
	KUInt32 indexKey;
	theResult = GetKeyIndex( inSymbol, &indexKey );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * RemoveSlot( const TDCLNSSymbol& )
// ------------------------------------------------------------------------- //
Boolean
TDCLNSFrame::RemoveSlot( const TDCLNSSymbol& inSymbol )
{
	Boolean theResult = false;	// Supposons que la clé ne soit pas là.
	
	// Index de la clé, si elle est dans le dictionnaire.
	KUInt32 indexKey;
	theResult = GetKeyIndex( inSymbol, &indexKey );
	
	if (theResult)
	{
		mValues[indexKey].TDCLNSRef::~TDCLNSRef();
		mKeys[indexKey].TDCLNSRef::~TDCLNSRef();
		
		mLength--;

		// Il faut donc tirer les clés/valeurs avant le point
		// de suppression avec un memmove (les zones se recouvrent).
		(void) ::memmove(
				(void*) &mKeys[indexKey],
				(const void*) &mKeys[indexKey + 1],
				(mLength - indexKey) * sizeof (TDCLNSRef) );
		(void) ::memmove(
				(void*) &mValues[indexKey],
				(const void*) &mValues[indexKey + 1],
				(mLength - indexKey) * sizeof (TDCLNSRef) );
		
		
		// Si la structure était une fonction et que l'élément
		// supprimé est la classe, alors la structure n'est plus
		// une fonction.
		if (mIsFunction && (inSymbol == KDCLSYM::kSYMclass.ToSymbol()))
		{
			mIsFunction = false;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * Get( const TDCLNSSymbol& ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSFrame::Get( const TDCLNSSymbol& inSymbol ) const
{
	TDCLNSRef theResult;	// Supposons que la clé ne soit pas là.

	// Index de la clé, si elle est dans le dictionnaire.
	KUInt32 indexKey;
	if (GetKeyIndex( inSymbol, &indexKey ))
	{
		theResult = mValues[indexKey];
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetKey( KUInt32 ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSFrame::GetKey( KUInt32 inIndex ) const
{
	if (inIndex >= mLength)
	{
		throw DCLBadParamError;
	}

	return mKeys[inIndex];
}

// ------------------------------------------------------------------------- //
//  * GetValue( KUInt32 ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSFrame::GetValue( KUInt32 inIndex ) const
{
	if (inIndex >= mLength)
	{
		throw DCLBadParamError;
	}

	return mValues[inIndex];
}

// ------------------------------------------------------------------------- //
//  * Set( const TDCLNSRef&, const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::Set( const TDCLNSRef& inSymbol, const TDCLNSRef& inNewValue )
{
	TDCLNSSymbol& theSymbol = inSymbol.ToSymbol();

	// Index de la clé, si elle est dans le dictionnaire.
	KUInt32 indexKey = mLength;
	if (GetKeyIndex( theSymbol, &indexKey ))
	{
		// Ceci appelle l'opérateur d'assignation.
		// Libération de l'ancienne valeur, copie de la nouvelle.
		mValues[indexKey] = inNewValue;
	} else {
		// Ici, il y a une petite astuce.
		// Si la structure n'est pas triée mais devrait l'être avec l'ajout
		// de la nouvelle paire, il faut la trier.
		
		// Agrandissement de la structure si nécessaire.
		if (mLength == mCapacity)
		{
			mCapacity += kListChunkSize;
			mKeys = (TDCLNSRef*) ::realloc(
						mKeys,
						mCapacity * sizeof (TDCLNSRef) );
			mValues = (TDCLNSRef*) ::realloc(
						mValues,
						mCapacity * sizeof (TDCLNSRef) );
		}

		if (indexKey < mLength)
		{
			// Je rajoute la nouvelle clé au milieu.
			// Il faut donc pousser les clés/valeurs avant le point
			// d'insertion avec un memmove (les zones se recouvrent).
			(void) ::memmove(
					(void*) &mKeys[indexKey + 1],
					(const void*) &mKeys[indexKey],
					(mLength - indexKey) * sizeof (TDCLNSRef) );
			(void) ::memmove(
					(void*) &mValues[indexKey + 1],
					(const void*) &mValues[indexKey],
					(mLength - indexKey) * sizeof (TDCLNSRef) );
		}
		mLength++;

		// Initialisation de ces éléments dans le tableau.
		// (c'est une fausse assignation, les éléments ici sont
		// des copies de ce qu'on a en indexKey + 1 ou les derniers
		// éléments non intialisés).
		new ( &mKeys[indexKey] ) TDCLNSRef( inSymbol );
		new ( &mValues[indexKey] ) TDCLNSRef( inNewValue );
		
		// Tri si nécessaire de la structure.
		if (!mIsFunction && !mKeysAreSorted && (mLength > kSortThreshold))
		{
			// Vue la taille de la liste, on peut y aller avec un insertion sort.
			KUInt32 indexRemoved;
			for (indexRemoved = 0; indexRemoved < mLength; indexRemoved++)
			{
				TDCLNSRef removedKey = mKeys[indexRemoved];
				TDCLNSRef removedValue = mValues[indexRemoved];
				TDCLNSSymbol& removedKeySym = removedKey.ToSymbol();
				
				KUInt32 indexInserted;
				for (
						indexInserted = indexRemoved;
						indexInserted > 0;
						indexInserted--)
				{
					// Si la clé supprimée est plus petite que la clé précédente,
					// on inverse.
					if (
						removedKeySym.Compare(
							mKeys[indexInserted - 1].ToSymbol()) < 0)
					{
						mKeys[indexInserted] = mKeys[indexInserted - 1];
						mValues[indexInserted] = mValues[indexInserted - 1];
					} else {
						break;
					}
				}
				if (indexInserted != indexRemoved)
				{
					mKeys[indexInserted] = removedKey;
					mValues[indexInserted] = removedValue;
				}
			}
			
			mKeysAreSorted = true;
		}
	}
}

// ------------------------------------------------------------------------- //
//  * Clone( void ) const
// ------------------------------------------------------------------------- //
TDCLNSObject*
TDCLNSFrame::Clone( void ) const
{
	return new TDCLNSFrame( *this );
}

// ------------------------------------------------------------------------- //
//  * GetKeyIndex( const TDCLNSSymbol&, KUInt32* ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSFrame::GetKeyIndex( const TDCLNSSymbol& inSymbol, KUInt32* outIndex ) const
{
	// On suppose que la clé n'est pas présente.
	Boolean theResult = false;
	
	// Boucle sur tous les symboles, sauf si le dictionnaire est trié.
	// (remarque: on n'a pas d'index).
	KUInt32 indexKeys;			// index pour itérer.
	KUInt32 nbKeys = mLength;	// Nombre de clés (plus rapide)
	Boolean sorted = mKeysAreSorted;	// Si les clés sont triées.
	
	for (indexKeys = 0; indexKeys < nbKeys; indexKeys++)
	{
		// Cet entier vaut 0 si les symboles sont égaux,
		// > 0 si la clé est supérieure au symbole, < 0 dans le cas inverse.
		int comparison = ((TDCLNSSymbol&) mKeys[indexKeys]).Compare( inSymbol );
		
		if (comparison == 0)
		{
			theResult = true;
			break;
		}
		
		if (sorted && (comparison > 0))
		{
			// Les clés sont triées et le paramètre est supérieur à une clé:
			// il n'est pas dans le dictionnaire.
			break;
		}
	}
	
	// Dans tous les cas, on met dans outIndex l'index courant.
	// C'est l'index de la clé si elle a été trouvée, l'index suivant si la
	// structure est triée, la taille de la structure sinon.
	*outIndex = indexKeys;
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * ClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSFrame::ClassOf( void ) const
{
	TDCLNSRef theResult;
	if (HasSlot( KDCLSYM::kSYMclass ))
	{
		theResult = Get( KDCLSYM::kSYMclass );
	} else {
		theResult = KDCLSYM::kSYMframe;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PrimClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSFrame::PrimClassOf( void ) const
{
	return KDCLSYM::kSYMframe;
}

// ------------------------------------------------------------------------- //
//  * SetClass( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::SetClass( const TDCLNSRef& inClass )
{
	Set( KDCLSYM::kSYMclass, inClass );
}

// ------------------------------------------------------------------------- //
//  * FromNSOF( TDCLNSOFDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSFrame*
TDCLNSFrame::FromNSOF( TDCLNSOFDecoder* inDecoder )
{
	TDCLStream* theStream = inDecoder->GetInputStream();
	
	KUInt32 nbPairs = theStream->GetXLong();
	KUInt32 indexPairs;

	// Création de l'objet.
	TDCLNSFrame* theResult = new TDCLNSFrame( nbPairs );
	
	// Cette structure n'est pas triée (vu que je vais ajouter les clés sans
	// appeler la méthode Set).
	theResult->mKeysAreSorted = false;

	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ) );

	// Création des symboles
	// Attention au risque de fuite de mémoire. On se protège avec un try/catch.
	try {
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			new ( &theResult->mKeys[ indexPairs ] )
				TDCLNSRef( inDecoder->GetNextObject() );
		}
	} catch ( ... ) {
		nbPairs = indexPairs;
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			theResult->mKeys[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}		

		throw;	// rethrow
	}

	// Création des valeurs
	try {
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			new ( &theResult->mValues[ indexPairs ] )
				TDCLNSRef( inDecoder->GetNextObject() );
		}
	} catch ( ... ) {
		KUInt32 nbValues = indexPairs;
		
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			theResult->mKeys[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}		
		for (indexPairs = 0; indexPairs < nbValues; indexPairs++)
		{
			theResult->mValues[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}
		
		throw;	// rethrow
	}
	
	// Mise à jour de la taille.
	theResult->mLength = nbPairs;
	
	// Est-ce une fonction?
	// (même test que sur le Newton)
	if (nbPairs >= 1)
	{
		if ((theResult->mKeys[0] == KDCLSYM::kSYMclass)
			&& ((theResult->mValues[0] == TDCLNSRef::Ref( 50 ))
			|| (theResult->mValues[0] == KDCLSYM::kSYMCodeBlock)))
		{
			theResult->mIsFunction = true;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * FromNSOFAsRect( TDCLNSOFDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSFrame*
TDCLNSFrame::FromNSOFAsRect( TDCLNSOFDecoder* inDecoder )
{
	TDCLStream* theStream = inDecoder->GetInputStream();
	
	// Création de l'objet.
	TDCLNSFrame* theResult = new TDCLNSFrame( 4 );

	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ) );

	// Ici, on pourrait trier les clés.
	theResult->mKeysAreSorted = false;
	
	// Haut
	KUInt8 theByte =theStream->GetByte();
	new ( &theResult->mKeys[0] ) TDCLNSRef( KDCLSYM::kSYMtop );
	new ( &theResult->mValues[0] ) TDCLNSRef( TDCLNSRef::MakeInt( theByte ) );
	theResult->mLength++;

	// Gauche
	theByte = theStream->GetByte();
	new ( &theResult->mKeys[1] ) TDCLNSRef( KDCLSYM::kSYMleft );
	new ( &theResult->mValues[1] ) TDCLNSRef( TDCLNSRef::MakeInt( theByte ) );
	theResult->mLength++;

	// Bas
	theByte = theStream->GetByte();
	new ( &theResult->mKeys[2] ) TDCLNSRef( KDCLSYM::kSYMbottom );
	new ( &theResult->mValues[2] ) TDCLNSRef( TDCLNSRef::MakeInt( theByte ) );
	theResult->mLength++;

	// Droite
	theByte = theStream->GetByte();
	new ( &theResult->mKeys[3] ) TDCLNSRef( KDCLSYM::kSYMright );
	new ( &theResult->mValues[3] ) TDCLNSRef( TDCLNSRef::MakeInt( theByte ) );
	theResult->mLength++;

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * FromPkg( TDCLPkgDecoder* )
// ------------------------------------------------------------------------- //
TDCLNSFrame*
TDCLNSFrame::FromPkg( TDCLPkgDecoder* inDecoder )
{
	TDCLRandomAccessStream* theStream = inDecoder->GetInputStream();

	// Décalage de l'objet.
	KUInt32 theObjectOffset = inDecoder->GetCurrentStreamOffset();
	
	// D'abord, c'est la structure avec les valeurs.

	// Taille
	KUInt32 theFirstLong = theStream->GetLong();
	KUInt32 nbPairs = ((theFirstLong >> TDCLPkgDecoder::kSizeShift) - 12) / 4;
	
	// On passe le deuxième long.
	(void) theStream->GetLong();

	// Création de l'objet. Je mets tout plein de NIL.
	TDCLNSFrame* theResult = new TDCLNSFrame( nbPairs );
	
	// Ajout de l'objet dans la liste des objets référencés.
	inDecoder->AddObjectToList( TDCLNSRef( theResult ), theObjectOffset );

	// Récupération de la carte.
	size_t pos = theStream->GetCursor();
    KUInt32 mapRef = theStream->GetLong();
	theStream->SetCursor(pos, TDCLRandomAccessStream::kFromStart);
	TDCLNSRef theMap = inDecoder->GetNextObject();

	// Remplissage des clés à partir de la fin.
	KUInt32 indexPairs = nbPairs - 1;

    try {
        do {
            const TDCLNSArray& theMapAsArray = theMap.ToArray();
            KSInt32 theMapFlags = 0;
            if (theMapAsArray.ClassOf().IsInt()) {
                // BookMaker apparently generates packages with maps of class 'Array.
                // Just consider the flags are 0 in this case.
                theMapFlags = theMapAsArray.ClassOf().ToInt();
            }
            if (theMapAsArray.GetLength() == nbPairs + 1) {
                theResult->mKeysAreSorted = (theMapFlags & TDCLPkgDecoder::kMapSorted) != 0;
            } else {
                theResult->mKeysAreSorted = false;
            }

            KUInt32 indexKeys = theMapAsArray.GetLength() - 1;
            for (; indexKeys > 0; indexKeys--)
            {
                new ( &theResult->mKeys[indexPairs--] )
                    TDCLNSRef( theMapAsArray.Get(indexKeys) );
            }

            if (theMapAsArray.Get(0) == TDCLNSRef::kNILREF)
            {
                break;
            } else {
                theMap = theMapAsArray.Get(0);
            }
        } while (true);
        if (indexPairs != (KUInt32) -1) {
            throw DCLBadStateError;
        }
	} catch ( ... ) {
		for (; indexPairs < nbPairs; indexPairs++) {
			theResult->mKeys[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}
		throw;	// rethrow
	}
	
	// Ensuite, on remplit les valeurs.
	try {
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			new ( &theResult->mValues[ indexPairs ] )
				TDCLNSRef( inDecoder->GetNextObject() );
		}
	} catch ( ... ) {
		KUInt32 nbValues = indexPairs;
		
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			theResult->mKeys[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}		
		for (indexPairs = 0; indexPairs < nbValues; indexPairs++)
		{
			theResult->mValues[ indexPairs ].TDCLNSRef::~TDCLNSRef();
		}
		
		throw;	// rethrow
	}
	
	// Mise à jour de la taille.
	theResult->mLength = nbPairs;
	
	// Est-ce une fonction?
	// (même test que sur le Newton)
	if ((nbPairs >= 1) && (!theResult->mKeysAreSorted))
	{
		if ((theResult->mKeys[0] == KDCLSYM::kSYMclass)
			&& ((theResult->mValues[0] == TDCLNSRef::Ref( 50 ))
			|| (theResult->mValues[0] == KDCLSYM::kSYMCodeBlock)))
		{
			theResult->mIsFunction = true;
		}
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * ToXML( TDCLXMLEncoder*, KUInt32 ) const
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::ToXML(
				TDCLXMLEncoder*		inEncoder,
				KUInt32				inObjectID ) const
{
	// Récupération du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Balise de début avec l'ID.
	char theOutputStr[32]; 	// Suffisant.
	(void) ::sprintf( theOutputStr, "<frame id=\"n%lu\">", (unsigned long) inObjectID );
	theOutputStream->PutString( theOutputStr );

	inEncoder->IncrementLevel();
	
	// Ensuite, chacun des éléments.
	KUInt32 nbPairs = mLength;
	KUInt32 indexPairs;
	for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
	{
		inEncoder->PutTabulations();
		
		// Balise de début, première partie.
		theOutputStream->PutString( "<slot symbol=\"" );
		
		// Ajout du symbole.
		const char* theSymbol = ((TDCLNSSymbol&) mKeys[indexPairs]).GetString();
		inEncoder->Print8BitsWithEntities( (KUInt8*) theSymbol );

		// Balise de début, fin.
		theOutputStream->PutString( "\">" );

		inEncoder->IncrementLevel();
		inEncoder->PutTabulations();
		
		// Ajout de la valeur.
		inEncoder->AddObject( mValues[indexPairs] );
		
		inEncoder->DecrementLevel();
		inEncoder->PutTabulations();
		
		// Balise de fin.
		theOutputStream->PutString( "</slot>" );
	}
	
	inEncoder->DecrementLevel();
	inEncoder->PutTabulations();
	
	// Balise de fin.
	theOutputStream->PutString( "</frame>" );
}

// ------------------------------------------------------------------------- //
//  * ToNSOF( TDCLNSOFEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::ToNSOF(
				TDCLNSOFEncoder* inEncoder ) const
{
	// Récupération du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// C'est peut-être un rectangle.
	KUInt32 nbPairs = mLength;
	Boolean isRect = false;
	KUInt8 rectVals[4];
	if ( nbPairs == 4 )
	{
		do {
			TDCLNSRef theTop = Get( KDCLSYM::kSYMtop );
			if (!theTop.IsInt())
			{
				break;
			}
			
			KSInt32 theValue = theTop.ToInt();
			if ((theValue >= 0) && (theValue <= 255))
			{
				rectVals[0] = (KUInt8) theValue;
			}

			TDCLNSRef theLeft = Get( KDCLSYM::kSYMleft );
			if (!theLeft.IsInt())
			{
				break;
			}
			
			theValue = theLeft.ToInt();
			if ((theValue >= 0) && (theValue <= 255))
			{
				rectVals[1] = (KUInt8) theValue;
			}

			TDCLNSRef theBottom = Get( KDCLSYM::kSYMbottom );
			if (!theBottom.IsInt())
			{
				break;
			}
			
			theValue = theBottom.ToInt();
			if ((theValue >= 0) && (theValue <= 255))
			{
				rectVals[2] = (KUInt8) theValue;
			}

			TDCLNSRef theRight = Get( KDCLSYM::kSYMright );
			if (!theRight.IsInt())
			{
				break;
			}
			
			theValue = theRight.ToInt();
			if ((theValue >= 0) && (theValue <= 255))
			{
				rectVals[3] = (KUInt8) theValue;
			}
			
			isRect = true;
		} while ( false );
	}
	
	if (isRect)
	{
		// Ecriture du type.
		theOutputStream->PutByte( KDCLNSOF::kSmallRect );
		
		// Ecriture des 4 valeurs.
		int indexVals;
		for (indexVals = 0; indexVals < 4; indexVals++)
		{
			theOutputStream->PutByte( rectVals[indexVals] );
		}
	} else {
		// Ecriture du type.
		theOutputStream->PutByte( KDCLNSOF::kFrame );
	
		// Ecriture de la taille.
		theOutputStream->PutXLong( nbPairs );
	
		// Ecriture des clés.
		KUInt32 indexPairs;
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			// Ajout de la clé.
			inEncoder->AddObject( mKeys[indexPairs] );
		}

		// Ecriture des valeurs
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			// Ajout des références.
			inEncoder->AddObject( mValues[indexPairs] );
		}
	}
}

// ------------------------------------------------------------------------- //
//  * ToText( TDCLTextEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::ToText(
				TDCLTextEncoder* /* inEncoder */ ) const
{
}

// ------------------------------------------------------------------------- //
//  * ToPkg( TDCLPkgEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSFrame::ToPkg(
				TDCLPkgEncoder* inEncoder,
				KUInt32* ioOffset ) const
{
	// Récupération du flux de sortie.
	TDCLRandomAccessStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Les valeurs d'abord.
	// Ecriture du nombre d'éléments.
	KUInt32 nbItems = mLength;
	KUInt32 theSize = (nbItems * 4) + 12;

	if (theSize > TDCLPkgEncoder::kMaxSize)
	{
		throw DCLLimitReachedError;
	}

	KUInt32 endValueOffset = *ioOffset + theSize;
	inEncoder->AlignOffset( &endValueOffset );

    // Look for suitable shared map or supermap
    KUInt32 sharedKeysCount = 0;
    KUInt32 superMapRef = 2; // TDCLNSRef::kNILREF;
    KUInt32 mapRef = endValueOffset + 1;
    if (inEncoder->GetBestSupermap(this, &sharedKeysCount, &superMapRef)) {
        if (sharedKeysCount == nbItems) {
            mapRef = superMapRef;
        }
    }

	// Écriture de l'entête.
	theOutputStream->PutLong(
		(theSize << TDCLPkgEncoder::kSizeShift)
		| TDCLPkgEncoder::kObjFlagHeader
		| TDCLPkgEncoder::kObjSlotted 
		| TDCLPkgEncoder::kObjFrame );
	
	theOutputStream->PutLong( 0 );
	
	// Pointeur sur la carte.
	theOutputStream->PutLong( mapRef );
	
	// Mise à jour du décalage.
	*ioOffset += 12;

	// Ajout des valeurs.
	KUInt32 indexItems;
	for ( indexItems = 0; indexItems < nbItems; indexItems++ )
	{
		inEncoder->AddObject( mValues[ indexItems ] );
	}

	// Alignement.
	inEncoder->AlignStream( ioOffset );

	KASSERT( *ioOffset == endValueOffset );

	// Ensuite, les clés, si nécessaire.
    if (sharedKeysCount < nbItems) {
        KSInt32 theMapClass = 0;
        if (mKeysAreSorted)
        {
            theMapClass = TDCLPkgEncoder::kMapSorted;
        }
        if (HasSlot( KDCLSYM::kSYM_proto ))
        {
            theMapClass |= TDCLPkgEncoder::kMapProto;
        }

        theSize = ((nbItems - sharedKeysCount) * 4) + 16;

        theOutputStream->PutLong(
            (theSize << TDCLPkgEncoder::kSizeShift)
            | TDCLPkgEncoder::kObjFlagHeader
            | TDCLPkgEncoder::kObjSlotted );

        theOutputStream->PutLong( 0 );

        // Mise à jour du décalage.
        *ioOffset += 8;

        // Classe.
        inEncoder->AddObject( TDCLNSRef::MakeInt( theMapClass ) );

        theOutputStream->PutLong( superMapRef );
        *ioOffset += 4;

        // Ajout des symboles.
        for ( indexItems = sharedKeysCount; indexItems < nbItems; indexItems++ )
        {
            inEncoder->AddObject( mKeys[ indexItems ] );
        }
        inEncoder->AddMap(this, mapRef);
    }

    // Note that supermap is shared.
    if (sharedKeysCount > 0) {
        KUInt64 pos = theOutputStream->GetCursor();
        KUInt64 superMapPos = pos - *ioOffset + superMapRef - 1;
        theOutputStream->SetCursor(superMapPos + 8, TDCLRandomAccessStream::kFromStart);
	    KUInt32 classRef = theOutputStream->GetLong();
	    KASSERT((classRef & 0x3) == 0);
	    KUInt32 newClassRef = classRef | (TDCLPkgEncoder::kMapShared << 2);
	    if (newClassRef != classRef) {
            theOutputStream->SetCursor(superMapPos + 8, TDCLRandomAccessStream::kFromStart);
            theOutputStream->PutLong(newClassRef);
	    }
	    theOutputStream->SetCursor(pos, TDCLRandomAccessStream::kFromStart);
    }
}

// ====================================================================== //
// Real computer scientists don't program in assembler.  They don't write //
// in anything less portable than a number two pencil.                    //
// ====================================================================== //
