// ==============================
// Fichier:			TDCLStyleRef.h
// Projet:			Desktop Connection Library
// 
// Créé le:			19/3/2004
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
// The Original Code is TDCLStyleRef.h.
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
// $Id: TDCLStyleRef.h,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLSTYLEREF_H
#define _TDCLSTYLEREF_H

#include <DCL/Headers/DCLDefinitions.h>

class TDCLFontSpecification;
class TDCLPicture;

///
/// Classe pour une référence sur un spécification de style texte ou une
/// image.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLStyleRef
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLStyleRef( void );

	///
	/// Constructeur à partir d'une spécification de style de texte.
	///
	/// \param inTextStyle	style de texte.
	///
	TDCLStyleRef( TDCLFontSpecification* inTextStyle );

	///
	/// Constructeur à partir d'une image.
	///
	/// \param inPicture	image.
	///
	TDCLStyleRef( TDCLPicture* inPicture );

	///
	/// Destructeur.
	///
	~TDCLStyleRef( void );

	///
	/// Constructeur par copie.
	/// Incrémente la référence.
	///
	/// \param inCopy		objet à copier
	///
	TDCLStyleRef( const TDCLStyleRef& inCopy );

	///
	/// Opérateur d'assignation.
	/// Idem.
	///
	/// \param inCopy		objet à copier
	///
	TDCLStyleRef& operator = ( const TDCLStyleRef& inCopy );

	///
	/// Détermine si cette référence pointe sur un style de texte.
	///
	/// \return \c true si cette référence pointe sur un style de texte,
	///			\c false si elle pointe sur une image ou sur rien.
	///
	inline Boolean	IsFontSpecification( void ) const
		{
			return (mFontSpecification != nil);
		}

	///
	/// Détermine si cette référence pointe sur une image.
	///
	/// \return \c true si cette référence pointe sur une image,
	///			\c false si elle pointe sur un style ou sur rien.
	///
	inline Boolean	IsPicture( void ) const
		{
			return (mPicture != nil);
		}

	///
	/// Retourne la référence sous forme de style de texte
	/// (vérifie que c'est un style de texte)
	///
	/// \throws TDCLClassCastException si la référence ne pointe pas sur
	///			une spécification de style de texte.
	///
	operator TDCLFontSpecification& () const;

	///
	/// Retourne la référence sous forme d'image.
	/// (vérifie que c'est une image)
	///
	/// \throws TDCLClassCastException si la référence ne pointe pas sur
	///			une image.
	///
	operator TDCLPicture& () const;

private:
	/// \name Variables
	TDCLFontSpecification*	mFontSpecification;	///< Spécification sur le style
												///< de texte ou \c nil si ce
												///< n'est pas un style de
												///< texte.
	TDCLPicture*			mPicture;			///< Spécification sur
												///< l'image ou \c nil si ce
												///< n'est pas une image.
};

#endif
		// _TDCLSTYLEREF_H

// ============================================== //
// The Macintosh is Xerox technology at its best. //
// ============================================== //
