// ==============================
// Fichier:			TDCLNSArray.h
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
// The Original Code is TDCLNSArray.h.
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
// $Id: TDCLNSArray.h,v 1.7 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSARRAY__
#define __TDCLNSARRAY__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLStream;
class TDCLNSEncoder;
class TDCLNSSymbol;
class TDCLNSOFDecoder;
class TDCLPkgDecoder;

///
/// Classe pour un tableau NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLNSArray
					:	public TDCLNSObject
{
public:
	///
	/// Constructeur par défaut.
	/// Crée un tableau vide.
	///
	TDCLNSArray( void );

	///
	/// Constructeur par copie.
	/// Duplique les éléments (les références seront incrémentées)
	///
	/// \param inCopy	objet à copier.
	///
	explicit TDCLNSArray( const TDCLNSArray& inCopy );

	///
	/// Constructeur par à partir d'une taille et d'un élément.
	/// Crée un tableau rempli de cet élément.
	///
	/// \param inSize	taille du tableau
	/// \param inRef	référence sur l'élément à mettre dans le tableau.
	///
	TDCLNSArray( KUInt32 inSize, const TDCLNSRef& inRef = TDCLNSRef::kNILREF );

	///
	/// Destructeur.
	///
	virtual ~TDCLNSArray( void );

	///
	/// Opérateur d'assignation.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSArray& operator = ( const TDCLNSArray& inCopy );

	///
	/// Détermine si cet objet est un tableau.
	///
	/// \return \c false
	///
	virtual Boolean IsArray( void ) const;

	///
	/// Accesseur sur un élément du tableau
	///
	/// \param inIndex	index de l'élément à récupérer.
	/// \throws TDCLNSException si l'index est en dehors du domaine de ce tableau.
	///
	TDCLNSRef	Get( KUInt32 inIndex ) const;

	///
	/// Sélecteur sur un élément du tableau
	///
	/// \param inIndex	index de l'élément à changer.
	/// \param inItem	élément à mettre dans le tableau.
	/// \throws TDCLNSException si l'index est en dehors du domaine de ce tableau.
	///
	void		Set( KUInt32 inIndex, const TDCLNSRef& inItem );

	///
	/// Ajout d'un élément dans le tableau (à la fin).
	///
	/// \param inItem	élément à ajouter au tableau.
	///
	void		Add( const TDCLNSRef& inItem );

	///
	/// Suppression d'un élément dans le tableau (à la fin).
	/// Le tableau est rétréci.
	///
	/// \param inIndex	indice de l'élément à supprimer du tableau.
	///
	void		RemoveSlot( KUInt32 inIndex );

	///
	/// Ajout d'un élément au milieu du tableau.
	/// Le tableau est agrandi.
	///
	/// \param inIndex	indice de l'élément une fois inséré dans le tableau.
	///					0 insèrera l'élément au début du tableau. mLength
	///					l'insèrera à la fin.
	/// \param inItem	élément à insérer.
	///
	void		Insert( KUInt32 inIndex, const TDCLNSRef& inItem );

	///
	/// Crée une copie de surface de la structure.
	/// Appelle le constructeur par copie.
	///
	/// \return une copie de surface de this.
	///
	virtual TDCLNSObject*	Clone( void ) const;

	///
	/// Crée un objet à partir d'un flux NSOF
	///
	/// \param inDecoder	décodeur NSOF associé à un flux donné.
	/// \param isPlain		\c true si la classe du tableau est 'array
	///						et qu'il ne faut pas la lire, \c false sinon
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSArray*	FromNSOF(
								TDCLNSOFDecoder* inDecoder,
								Boolean isPlain );

	///
	/// Crée un objet à partir d'un paquet.
	///
	/// \param inDecoder	décodeur Pkg associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSArray*	FromPkg( TDCLPkgDecoder* inDecoder );

	///
	/// Détermine la classe de ce tableau.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de ce tableau.
	///
	virtual TDCLNSRef ClassOf( void ) const;

	///
	/// Détermine la classe primaire de ce tableau.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de ce tableau.
	///
	virtual TDCLNSRef PrimClassOf( void ) const;

	///
	/// Change la classe de ce tableau.
	/// Ce n'est pas forcément un symbole.
	///
	/// \param	inClass un objet qui sert de nouvelle classe à ce tableau.
	/// \throws TDCLNSException si une erreur est survenue.
	///
	virtual void SetClass( const TDCLNSRef& inClass );

	///
	/// Accesseur sur la taille du tableau.
	///
	/// \return la taille du tableau (le nombre d'éléments dans le tableau)
	///
	inline KUInt32	GetLength( void ) const
		{
			return mSize;
		}

private:
	/// \name Interface IStreamable
	
	///
	/// Crée une représentation XML du tableau.
	/// Comprend la balise de début, les données et la balise de fin.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	/// \param inObjectID		ID de l'objet dans ce flux.
	///
	virtual void ToXML(
				TDCLXMLEncoder* inEncoder,
				KUInt32			inObjectID ) const;

	///
	/// Crée une représentation NSOF du tableau.
	/// Comprend le type et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToNSOF(
				TDCLNSOFEncoder* inEncoder ) const;

	///
	/// Crée une représentation sous forme de texte du tableau.
	/// Comprend les crochets et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToText(
				TDCLTextEncoder* inEncoder ) const;
	
	///
	/// Crée une représentation du tableau pour mettre dans un
	/// paquet.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder	encodeur pour le paquet.
	/// \param ioOffset		en entrée et en sortie, décalage par rapport
	///						au paquet.
	///
	virtual void ToPkg( TDCLPkgEncoder* inEncoder, KUInt32* ioOffset ) const;

	/// \name Variables Privées
	
	TDCLNSRef			mClass;		///< Classe du tableau
	KUInt32				mSize;		///< Nombre d'éléments dans le tableau
	TDCLNSRef*			mItems;		///< Eléments du tableau
};

#endif
		// __TDCLNSARRAY__

// ========================================================================= //
// Don't get suckered in by the comments -- they can be terribly misleading. //
// Debug only code.                                                          //
//                 -- Dave Storer                                            //
// ========================================================================= //
