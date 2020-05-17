// ==============================
// Fichier:			TDCLPkgNOSPart.h
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
// The Original Code is TDCLPkgNOSPart.h.
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
// $Id: TDCLPkgNOSPart.h,v 1.7 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLPKGNOSPART_H
#define _TDCLPKGNOSPART_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPkgPart.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour une partie contenant un objet NS encodé au format paquet.
///
/// Les modifications apportées à l'objet doivent être notifiées pour que
/// l'objet soit ré-encodé. Par ailleurs, l'objet ne doit être accédé que
/// par un processus léger à la fois, i.e. il ne doit pas être encodé
/// pendant qu'il est modifié.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLPkgNOSPart
	:
		public TDCLPkgPart
{
public:
	///
	/// Constructeur à partir d'un objet NewtonScript.
	/// Si l'objet est modifié après la construction de la partie, il faut
	/// appeler la méthode Dirty avant tout encodage. (en fait, ce n'est pas
	/// nécessaire la première fois).
	///
	/// \param inObject	objet en question.
	/// \param inFourBytesPadding	si on encode avec un alignement sur 32 bits
	///								(vs 64).
	///
	TDCLPkgNOSPart(
			const TDCLNSRef& inObject,
			Boolean inFourBytesPadding = false );

	///
	/// Constructeur à partir de données et d'un décalage.
	/// L'objet n'est décodé que lorsque ce sera demandé.
	///
	TDCLPkgNOSPart( KUInt32 inOffset, const void* inData, KUInt32 inSize );

	///
	/// Destructeur.
	///
	virtual ~TDCLPkgNOSPart( void );

	///
	/// Détermine si la partie est un objet NewtonScript, i.e.
	/// si cette partie est de la classe TDCLPkgNOSPart.
	/// Oui.
	///
	/// \return \c true.
	///
	virtual Boolean	IsNOSPart( void ) const;

	///
	/// Retourne l'objet de cette partie (la décode si nécessaire).
	/// Si l'objet a été modifié, la méthode Dirty doit être appelée
	/// avant tout encodage pour s'assurer que l'objet sera réencodé.
	///
	TDCLNSRef		GetObject( void );

	///
	/// Indique que l'objet a été modifié.
	///
	void			Dirty( void );

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
	/// \param inOffset				décalage dans le paquet.
	/// \param ioBuffer				pointeur sur les données
	///								(alloué avec malloc)
	/// \param ioSize				taille des données (en entrée et en sortie)
	///
	virtual void Encode(
					KUInt32 inOffset,
					void** ioBuffer,
					KUInt32* ioSize ) const;

	/// \name Variables
	TDCLNSRef				mObject;	///< Objet (ou nil s'il n'est pas encore
										///< décodé)
	mutable Boolean			mDirty;		///< Si l'objet a été modifié.
	Boolean			mFourBytesPadding;	///< Si on aligne sur 32 bits (vs 64).
};

#endif
		// _TDCLPKGNOSPART_H

// ===================================================================== //
// The algorithm for finding the longest path in a graph is NP-complete. //
// For you systems people, that means it's *real slow*.                  //
//                 -- Bart Miller                                        //
// ===================================================================== //
