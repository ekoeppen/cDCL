// ==============================
// Fichier:			TDCLFontSpecification.h
// Projet:			Desktop Connection Library
// 
// Créé le:			17/3/2004
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
// The Original Code is TDCLFontSpecification.h.
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
// $Id: TDCLFontSpecification.h,v 1.5 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLFONTSPECIFICATION_H
#define _TDCLFONTSPECIFICATION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <K/Misc/TRefCounter.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour une spécification de police sur le Newton.
/// Cette spécification peut-être en deux formats:
/// -> packed font integer specification (un entier)
/// -> structure:
///		{
///			size: taille (entier)
///			face: style (entier)
///			family: police (symbole)
///		}
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLFontSpecification
	:
		private TRefCounter
{
	/// La classe qui s'occupe des références est une amie.
	friend class TDCLStyleRef;

public:
	///
	/// Constantes pour les styles.
	/// Ce sont les constantes sur le Newton pour les styles gérés sur le
	/// Newton. (en fait, les 5 premières sont dans MacTypes.h :)
	///
	enum {
		kFaceNormal			= 0x000,	///< Normal
		kFaceBold			= 0x001,	///< Gras
		kFaceItalic			= 0x002,	///< Italique
		kFaceUnderline		= 0x004,	///< Souligné
		kFaceOutline		= 0x008,	///< Entouré
		kFaceSuperscript	= 0x080,	///< Exposant
		kFaceSubscript		= 0x100		///< Indice
	};

	///
	/// Constructeur à partir d'un objet Newton.
	///
	TDCLFontSpecification( const TDCLNSRef& inFontSpec );

	///
	/// Constructeur à partir d'une police, d'une taille et d'un style.
	///
	/// \param inFontName	nom de la police (symbole)
	/// \param inSize		taille de la police
	/// \param inFace		style de la police
	///
	TDCLFontSpecification(
				const TDCLNSRef& inFontName,
				KUInt32 inSize,
				KUInt32 inFace = kFaceNormal );

	///
	/// Constructeur par copie.
	///
	/// \param inCopy		autre spécification TDCLFontSpecification
	///
	TDCLFontSpecification( const TDCLFontSpecification& inCopy );

	///
	/// Accesseur sur l'objet NewtonScript.
	///
	/// \return l'objet NewtonScript
	///
	inline TDCLNSRef	GetNSRef( void ) const
		{
			return mFontSpec;
		}

	///
	/// Accesseur sur le nom de la police, sous forme de symbole.
	///
	/// \return le nom de la police dans un symbole.
	///
	TDCLNSRef		GetFamily( void ) const;

	///
	/// Accesseur sur la taille de la police.
	///
	/// \return la taille de la police, à l'échelle du Newton.
	///
	KUInt32			GetSize( void ) const;

	///
	/// Accesseur sur le style de la police
	///
	/// \return le style de la police, combinaison des constantes définies plus
	///			haut.
	///
	KUInt32			GetFace( void ) const;

	///
	/// Sélecteur sur le nom de la police.
	///
	/// \param inFamilyName		nom de la police (symbole)
	///
	void			SetFamily( const TDCLNSRef& inFamilyName );

	///
	/// Sélecteur sur la taille de la police
	///
	/// \param inSize			taille de la police
	///
	void			SetSize( KUInt32 inSize );

	///
	/// Sélecteur sur le style de la police.
	/// Cette méthode remplace tout style préalablement défini (i.e. on ne peut
	/// pas ajouter du gras en appelant juste cette méthode, il faut appeler
	/// GetFace puis faire un ou logique).
	///
	/// \param inFace			style de police.
	///
	void			SetFace( KUInt32 inFace );

	///
	/// Opérateur d'égalité.
	///
	/// \param inAlter	spécification avec laquelle comparer \c this.
	/// \return \c true si \c this et \c inAlter spécifient la même police.
	///
	inline bool operator == (const TDCLFontSpecification& inAlter) const
		{
			return (GetSize() == inAlter.GetSize())
				&& (GetFace() == inAlter.GetFace())
				&& (GetFamily() == inAlter.GetFamily());
		}

	///
	/// Opérateur d'inégalité.
	///
	/// \param inAlter	spécification avec laquelle comparer \c this.
	/// \return \c true si \c this et \c inAlter ne spécifient pas la même
	///			police.
	///
	inline bool operator != (const TDCLFontSpecification& inAlter) const
		{
			return !( *this == inAlter );
		}

private:
	enum {
		ktsSystem		= 0,			///< Police système (Espy)
		ktsFancy		= 1,			///< New York
		ktsSimple		= 2,			///< Geneva
		ktsHWFont		= 3,			///< Casual
		kFamilyMask		= 0x000003FF,	///< Masque pour la police.
		kSizeShift		= 10,			///< Nombre de décalages à gauche pour
										///< la taille.
		kSizeMask		= 0x000FFC00,		///< Masque pour la taille.
		kFaceShift		= 20,			///< Nombre de décalages à gauche pour
										///< le style.
		kFaceMask		= 0x3FF00000,	///< Masque pour le style.
		kSignBit		= 0x20000000,	///< Signe pour un entier sur 30 bit.
		kSignExtension	= 0xC0000000	///< Extension du signe (pour que MakeInt
										///< retrouve ses petits)
	};

	///
	/// Création d'une structure (ou d'un entier).
	///
	/// \param inFontName		nom de la police (symbole).
	/// \param inSize			taille de la police.
	/// \param inFace			style de la police.
	///
	void				CreateFontSpec(
								const TDCLNSRef& inFontName,
								KUInt32 inSize,
								KUInt32 inFace );

	/// \name Variables
	TDCLNSRef			mFontSpec;	///< Spécification (entier ou structure)
};

#endif
		// _TDCLFONTSPECIFICATION_H

// ======================================================================= //
// The day-to-day travails of the IBM programmer are so amusing to most of //
// us who are fortunate enough never to have been one -- like watching     //
// Charlie Chaplin trying to cook a shoe.                                  //
// ======================================================================= //
