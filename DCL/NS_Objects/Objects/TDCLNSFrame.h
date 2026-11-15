// ==============================
// Fichier:			TDCLNSFrame.h
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
// The Original Code is TDCLNSFrame.h.
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
// $Id: TDCLNSFrame.h,v 1.9 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSFRAME__
#define __TDCLNSFRAME__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/IStreamable.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/Exceptions/TDCLException.h>

class TDCLNSOFDecoder;
class TDCLXMLEncoder;
class TDCLNSOFEncoder;
class TDCLTextEncoder;
class TDCLPkgEncoder;

///
/// Classe pour une Frame. Une Frame est un dictionnaire d'objets NewtonScript,
/// les clés sont des symboles.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.9 $
///
/// \test	UTestNS::GetWalterSmith
/// \test	UTestNS::TestFrameKeysSort
///
class TDCLNSFrame
	:
		public TDCLNSObject
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLNSFrame( void );

	///
	/// Constructeur par copie.
	/// Equivalent à Clone.
	/// Duplique les clés et les valeurs (les références seront incrémentées)
	///
	/// \param inCopy	objet à copier.
	///
	explicit TDCLNSFrame( const TDCLNSFrame& inCopy );

	///
	/// Destructeur.
	///
	virtual ~TDCLNSFrame( void );

	///
	/// Opérateur d'assignation.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSFrame& operator = ( const TDCLNSFrame& inCopy );

	///
	/// Détermine si cet objet est une structure.
	///
	/// \return \c true
	///
	virtual Boolean 	IsFrame( void ) const;

	///
	/// Détermine si cet objet est une fonction.
	///
	/// \return \c mIsFunction
	///
	virtual Boolean 	IsFunction( void ) const;

	///
	/// Détermine si une clé existe dans le dictionnaire.
	///
	/// \param inSymbol	clé à tester.
	/// \return \c true si la clé existe, \c false sinon.
	///
	Boolean				HasSlot( const TDCLNSSymbol& inSymbol ) const;

	///
	/// Détermine si une clé existe dans le dictionnaire.
	///
	/// \param inSymbol	clé à tester, sous forme de chaîne ISO-8859-1.
	/// \return \c true si la clé existe, \c false sinon.
	///
	inline Boolean		HasSlot( const char* inSymbol ) const
		{
			return HasSlot( TDCLNSSymbol( inSymbol ) );
		}

	///
	/// Supprime une clé du dictionnaire.
	///
	/// \param inSymbol	clé à supprimer.
	/// \return \c true si la clé existait, \c false sinon.
	///
	Boolean				RemoveSlot( const TDCLNSSymbol& inSymbol );

	///
	/// Supprime une clé du dictionnaire.
	///
	/// \param inSymbol	clé à supprimer, sous forme de chaîne ISO-8859-1.
	/// \return \c true si la clé existait, \c false sinon.
	///
	inline Boolean		RemoveSlot( const char* inSymbol )
		{
			return RemoveSlot( TDCLNSSymbol( inSymbol ) );
		}

	///
	/// Accesseur sur un élément.
	///
	/// \param inSymbol	clé pour accéder à l'élément.
	/// \return la référence de l'objet référencé par la clé ou
	///			NIL s'il n'y en a pas.
	///
	TDCLNSRef			Get( const TDCLNSSymbol& inSymbol ) const;

	///
	/// Accesseur sur un élément.
	///
	/// \param inSymbol		clé pour accéder à l'élément, sous forme de chaîne
	///						ISO 8859-1
	/// \return la référence de l'objet référencé par la clé ou
	///			NIL s'il n'y en a pas.
	///
	inline TDCLNSRef	Get( const char* inSymbol ) const
		{
			return Get( TDCLNSSymbol( inSymbol ) );
		}		

	///
	/// Accesseur sur une clé à partir d'un indice.
	/// Utilisé pour itérer sur les éléments de la structure.
	///
	/// \param inIndex	indice de la clé à récupérer.
	/// \return la référence de la inIndex-ième clé
	/// \throws TDCLException si la structure est plus petite que ça
	///
	TDCLNSRef			GetKey( KUInt32 inIndex ) const;

	///
	/// Accesseur sur un élément à partir d'un indice.
	/// Utilisé pour itérer sur les éléments de la structure.
	///
	/// \param inIndex	indice de l'élément à récupérer.
	/// \return la référence du inIndex-ième objet
	/// \throws TDCLException si la structure est plus petite que ça
	///
	TDCLNSRef			GetValue( KUInt32 inIndex ) const;

	///
	/// Sélecteur sur un élément.
	///
	/// \param inSymbol		clé pour accéder à l'élément.
	/// \param inNewValue	nouvelle valeur pour cette clé.
	///
	void				Set(
							const TDCLNSRef& inSymbol,
							const TDCLNSRef& inNewValue );

	///
	/// Sélecteur sur un élément.
	///
	/// \param inSymbol		clé pour accéder à l'élément.
	/// \param inNewValue	nouvelle valeur pour cette clé.
	///
	inline void			Set(
							const TDCLNSSymbol& inSymbol,
							const TDCLNSRef& inNewValue )
		{
			Set( TDCLNSRef(inSymbol.Clone()), inNewValue );
		}

	///
	/// Sélecteur sur un élément.
	///
	/// \param inSymbol		clé pour accéder à l'élément, sous forme de chaîne
	///						ISO 8859-1
	/// \param inNewValue	nouvelle valeur pour cette clé.
	///
	inline void		Set( const char* inSymbol, const TDCLNSRef& inNewValue )
		{
			Set( TDCLNSRef::MakeSymbol( inSymbol ), inNewValue );
		}

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
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSFrame*	FromNSOF( TDCLNSOFDecoder* inDecoder );
		
	///
	/// Crée un objet rectangle à partir d'un flux NSOF.
	/// Ça n'est pas clair dans Newton Formats, mais les 4 coordonnées
	/// doivent être entre 0 et 255 (ce sont des octets non signés).
	///
	/// \param inDecoder	décodeur NSOF associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSFrame*	FromNSOFAsRect( TDCLNSOFDecoder* inDecoder );
		
	///
	/// Crée un objet à partir d'un paquet.
	///
	/// \param inDecoder	décodeur Pkg associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSFrame*	FromPkg( TDCLPkgDecoder* inDecoder );

	///
	/// Détermine la classe de cette structure.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de cette structure.
	///
	virtual TDCLNSRef ClassOf( void ) const;

	///
	/// Détermine la classe primaire de cette structure.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de cette structure.
	///
	virtual TDCLNSRef PrimClassOf( void ) const;

	///
	/// Change la classe de cette structure.
	/// Ce n'est pas forcément un symbole.
	///
	/// \param	inClass un objet qui sert de nouvelle classe à cette structure.
	/// \throws TDCLNSException si une erreur est survenue.
	///
	virtual void SetClass( const TDCLNSRef& inClass );

	///
	/// Accesseur sur la taille de la structure.
	///
	/// \return la taille de la structure (le nombre de paire)
	///
	inline KUInt32	GetLength( void ) const
		{
			return mLength;
		}

protected:
	///
	/// Sélecteur sur mKeysAreSorted
	///
	/// \param inKeysAreSorted	si les clés sont triées.
	///
	inline void		SetKeysAreSorted( Boolean inKeysAreSorted )
		{
			mKeysAreSorted = inKeysAreSorted;
		}

	///
	/// Sélecteur sur mIsFunction
	///
	/// \param inIsFunction		si la structure est une fonction.
	///
	inline void		SetIsFunction( Boolean inIsFunction )
		{
			mIsFunction = inIsFunction;
		}

private:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kListChunkSize	= 10,	///< Incrément des listes lorsqu'elles sont pleines.
		kSortThreshold	= 20	///< Seuil à partir duquel il vaut mieux trier la liste.
	};

	///
	/// Constructeur à partir d'une capacité.
	///
	/// \param inCapacity	nombre de paire à allouer.
	///
	TDCLNSFrame( KUInt32 inCapacity );

	///
	/// Détermine l'index d'une clé, si elle est présente (ou l'index ou insérer
	/// la clé si elle n'est pas dans la liste).
	///
	/// \param inSymbol		clé à chercher
	/// \param outIndex		index de la clé ou index où mettre la clé si elle
	///						n'est pas présente (taille de la structure si elle n'est
	///						pas triée).
	/// \return \c true si la clé a été trouvée, \c false sinon.
	///
	Boolean		GetKeyIndex( const TDCLNSSymbol& inSymbol, KUInt32* outIndex ) const;

	/// \name Interface IStreamable
	// @{
	///
	/// Crée une représentation XML de la structure.
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
	/// Crée une représentation NSOF de la structure.
	/// Comprend le type et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToNSOF( TDCLNSOFEncoder* inEncoder ) const;

	///
	/// Crée une représentation sous forme de texte de la structure.
	/// Comprend les accolades et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToText( TDCLTextEncoder* inEncoder ) const;

	///
	/// Crée une représentation de la structure pour mettre dans un
	/// paquet.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder	encodeur pour le paquet.
	/// \param ioOffset		en entrée et en sortie, décalage par rapport
	///						au paquet.
	///
	virtual void ToPkg( TDCLPkgEncoder* inEncoder, KUInt32* ioOffset ) const;
	// @}

	// Même format que sur le Newton, sauf pour le coup des cartes.
	// taille, clés, valeurs.
	KUInt32					mLength;		///< Nombre d'éléments.
	KUInt32					mCapacity;		///< Capacité des listes.
	TDCLNSRef*				mKeys;			///< Liste des clés
	TDCLNSRef*				mValues;		///< Liste des valeurs.
	Boolean					mKeysAreSorted;	///< Indique que les clés sont triées.
	Boolean					mIsFunction;	///< Si la structure est une fonction.
};

#endif
		// __TDCLNSFRAME__

// =========================================================================== //
// Some programming languages manage to absorb change, but withstand progress. //
//                 -- Epigrams in Programming, ACM SIGPLAN Sept. 1982          //
// =========================================================================== //
