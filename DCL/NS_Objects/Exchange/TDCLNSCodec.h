// ==============================
// Fichier:			TDCLNSCodec.h
// Projet:			Desktop Connection Library
//
// Créé le:			22/10/2002
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
// The Original Code is TDCLNSCodec.h.
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
// $Id: TDCLNSCodec.h,v 1.6 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __TDCLNSCODEC__
#define __TDCLNSCODEC__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLNSObject;
class TDCLStream;

///
/// Classe de base pour les encodeurs/décodeurs d'objets NS sur un flux.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	UTestNSOF
///
class TDCLNSCodec
{
public:
	///
	/// Destructeur.
	///
	virtual ~TDCLNSCodec( void );

	///
	/// Accesseur sur le flux associé.
	///
	/// \return le flux où on écrit/lit les données.
	///
	inline TDCLStream* GetStream( void ) const
		{
			return mStream;
		}

protected:
	///
	/// Constructeur à partir d'un flux.
	///
	/// \param inStream		flux où lire/écrire les données.
	///
	TDCLNSCodec( TDCLStream* inStream );

	///
	/// Accesseur sur le nombre d'éléments.
	///
	/// \return le nombre d'éléments dans la liste.
	///
	inline KUInt32	GetListSize( void )
		{
			return mListSize;
		}

	///
	/// Accesseur sur un élément donné.
	///
	/// \param inIndex	index de l'élément à retourner.
	/// \return le n-ième élément de la liste.
	///
	inline TDCLNSRef	GetListItemObject( KUInt32 inIndex )
		{
			return mList[inIndex].fObject;
		}

	///
	/// Accesseur sur le pointeur sur le biscuit.
	///
	/// \param inIndex	index du biscuit à retourner.
	/// \return un pointeur sur le n-ième biscuit de la liste.
	///
	inline KUIntPtr*	GetListItemCookie( KUInt32 inIndex )
		{
			return &mList[inIndex].fCookie;
		}

	///
	/// Ajoute un objet dans la liste des objets.
	/// Retourne son ID.
	///
	/// \param inObject		objet à ajouter. Ceci doit être un pointeur.
	/// \param inCookie		biscuit à stocker avec l'objet.
	/// \return ID de cet objet.
	///
	KUInt32				AddObjectToList(
								const TDCLNSRef& inObject,
								KUIntPtr inCookie = 0 );

private:
	///
	/// Constructeur par défaut non disponible.
	///
	TDCLNSCodec( void );

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSCodec( const TDCLNSCodec& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSCodec& operator = ( const TDCLNSCodec& inCopy );

	///
	/// Constantes pour cette classe.
	///
	enum {
		kListChunkSize	= 10	///< Incrément de la liste lorsqu'elle est pleine.
	};

	///
	/// Structure pour la liste avec l'objet et le biscuit.
	///
	struct SListItem {
		TDCLNSRef	fObject;	///< Objet.
		KUIntPtr	fCookie;	///< Biscuit.
	};

	/// \name Variables Privées

	SListItem*		mList;				///< Liste des objets. L'indice dans
										///< cette liste est l'ID.
	KUInt32			mListAllocatedSize;	///< Nombre d'éléments que la liste
										///< peut contenir.
	KUInt32			mListSize;			///< Nombre d'éléments que la liste
										///< contient.
	TDCLStream*		mStream;			///< Flux utilisé par ce codec.
};

#endif
		// __TDCLNSCODEC__

// ========================================================================= //
// It is easier to write an incorrect program than understand a correct one. //
// ========================================================================= //
