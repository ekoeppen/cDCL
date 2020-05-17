// ==============================
// Fichier:			TDCLNSObject.h
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
// The Original Code is TDCLNSObject.h.
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
// $Id: TDCLNSObject.h,v 1.4 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSOBJECT__
#define __TDCLNSOBJECT__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/IStreamable.h>
#include <K/Misc/TRefCounter.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/TDCLNSException.h>

class TDCLNSWeakRef;
class TDCLNSRef;

///
/// Classe racine pour les objets NewtonScript qui ne sont pas des immédiats.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
/// \todo	gérer les références faibles.
///
class TDCLNSObject
			:
				public IStreamable,
				private TRefCounter 
{
	friend class TDCLNSRef;

public:
	///
	/// Ajoute une référence faible.
	/// Attention: ne pas ajouter une référence deux fois.
	///
	/// \param inWeakRef	référence faible à ajouter.
	///
	void AddWeakRef( TDCLNSWeakRef* inWeakRef );
	
	///
	/// Supprime une référence faible.
	/// Attention: ne pas supprimer une référence qui n'était pas déjà là.
	///
	/// \param inWeakRef	référence faible à supprimer.
	///
	void RemoveWeakRef( TDCLNSWeakRef* inWeakRef );

	///
	/// Crée une copie de surface de l'objet.
	/// Les sous-classes doivent dériver cette méthode et appeler leur
	/// constructeur par copie.
	///
	/// \return une copie de surface de this.
	///
	virtual TDCLNSObject*	Clone( void ) const = 0;

	/// \name Type de l'objet

	///
	/// Détermine si cet objet est une structure.
	///
	/// \return \c false
	///
	virtual Boolean IsFrame( void ) const;

	///
	/// Détermine si cet objet est une fonction.
	///
	/// \return \c false
	///
	virtual Boolean IsFunction( void ) const;

	///
	/// Détermine si cet objet est un symbole.
	///
	/// \return \c false
	///
	virtual Boolean IsSymbol( void ) const;

	///
	/// Détermine si cet objet est un tableau.
	///
	/// \return \c false
	///
	virtual Boolean IsArray( void ) const;

	///
	/// Détermine si cet objet est un binaire.
	///
	/// \return \c false
	///
	virtual Boolean IsBinary( void ) const;

	///
	/// Détermine si cet objet est une chaîne.
	///
	/// \return \c false
	///
	virtual Boolean IsString( void ) const;

	///
	/// Détermine la classe de cet objet.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de cet objet.
	///
	virtual TDCLNSRef ClassOf( void ) const = 0;

	///
	/// Détermine la classe primaire de cet objet.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de cet objet.
	///
	virtual TDCLNSRef PrimClassOf( void ) const = 0;

	///
	/// Change la classe de cet objet.
	/// Ce n'est pas forcément un symbole.
	///
	/// \param	inClass un objet qui sert de nouvelle classe à cet objet.
	/// \throws TDCLNSException si une erreur est survenue.
	///
	virtual void SetClass( const TDCLNSRef& inClass ) = 0;
};

#endif
		// __TDCLNSOBJECT__

// ========================================================== //
// Computer programs expand so as to fill the core available. //
// ========================================================== //
