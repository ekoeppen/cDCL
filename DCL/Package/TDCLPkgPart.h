// ==============================
// Fichier:			TDCLPkgPart.h
// Projet:			Desktop Connection Library
// 
// Créé le:			6/4/2004
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
// The Original Code is TDCLPkgPart.h.
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
// $Id: TDCLPkgPart.h,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLPKGPART_H
#define _TDCLPKGPART_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour une partie dans un paquet.
/// La partie est représentée par une mémoire tampon tant qu'elle
/// n'est pas analysée.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLPkgPart
{
public:
	///
	/// Constructeur par défaut.
	/// L'ensemble des données est vide.
	///
	TDCLPkgPart( void );

	///
	/// Constructeur à partir de données.
	/// Les données sont copiées.
	///
	/// \param inOffset	décalage dans le paquet.
	/// \param inData	données (copiées)
	/// \param inSize	taille des données.
	///
	TDCLPkgPart( KUInt32 inOffset, const void* inData, KUInt32 inSize );

	///
	/// Destructeur.
	///
	virtual ~TDCLPkgPart( void );

	///
	/// Détermine si la partie est un objet NewtonScript, i.e.
	/// si cette partie est de la classe TDCLPkgNOSPart.
	/// Par défaut, non.
	///
	/// \return \c true si l'objet est de la classe TDCLPkgNOSPart
	///
	virtual Boolean		IsNOSPart( void ) const;

	///
	/// Détermine la taille de la partie.
	/// Ré-encode les données si nécessaire.
	///
	/// \param inOffset		décalage par rapport au début du paquet.
	/// \return la taille des données.
	///
	KUInt32				GetSize( KUInt32 inOffset ) const;

	///
	/// Récupère un pointeur sur les données.
	/// Ré-encode les données si nécessaire.
	///
	/// \param inOffset		décalage par rapport au début du paquet.
	/// \return un pointeur sur les données.
	///
	const void*			GetBuffer( KUInt32 inOffset ) const;

	///
	/// Détermine la taille de la partie.
	/// Ré-encode les données si nécessaire.
	///
	/// \return la taille des données.
	///
	inline KUInt32		GetSize( void ) const
		{
			return GetSize( mOffset );
		}

	///
	/// Récupère un pointeur sur les données.
	/// Ré-encode les données si nécessaire.
	///
	/// \return un pointeur sur les données.
	///
	inline const void*	GetBuffer( void ) const
		{
			return GetBuffer( mOffset );
		}

	///
	/// Récupère le décalage dans le paquet.
	///
	/// \return le décalage dans le paquet.
	///
	inline KUInt32		GetOffset( void ) const
		{
			return mOffset;
		}

protected:
	///
	/// Détermine la taille de la partie afin de la décoder.
	///
	/// \return la taille des données.
	///
	inline KUInt32		DoGetSize( void ) const
		{
			return mSize;
		}

	///
	/// Récupère un pointeur sur les données afin de les décoder.
	///
	/// \return un pointeur sur les données.
	///
	inline const void*	DoGetBuffer( void ) const
		{
			return mBuffer;
		}

private:
	///
	/// Détermine si les données dépendent du décalage dans le paquet.
	/// Par défaut, non (retourne \c false).
	///
	/// \return \c true si les données dépendent du décalage dans le paquet.
	///
	virtual Boolean IsOffsetDependant( void ) const;

	///
	/// Détermine si la partie a été modifiée et doit être ré-encodée.
	/// Par défaut, non.
	///
	/// \return \c true si les données ont été modifiées et doivent être
	///			ré-encodées.
	///
	virtual Boolean IsDirty( void ) const;

	///
	/// Encode la partie pour un décalage donné (la décode si nécessaire).
	/// Par défaut, ne fait rien.
	///
	/// \param inOffset		décalage dans le paquet.
	/// \param ioBuffer		pointeur sur les données (alloué avec malloc)
	/// \param ioSize		taille des données (en entrée et en sortie)
	///
	virtual void Encode(
					KUInt32 inOffset,
					void** ioBuffer,
					KUInt32* ioSize ) const;

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPkgPart( const TDCLPkgPart& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPkgPart& operator = ( const TDCLPkgPart& inCopy );

	/// \name Variables
	mutable KUInt32		mOffset;			///< Décalage original.
	mutable void*		mBuffer;			///< Pointeur sur la mémoire tampon.
	mutable KUInt32		mSize;				///< Taille de la partie.
};

#endif
		// _TDCLPKGPART_H

// ======================================================================= //
// The nicest thing about the Alto is that it doesn't run faster at night. //
// ======================================================================= //
