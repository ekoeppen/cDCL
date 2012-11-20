// ==============================
// Fichier:			TDCLClParagraph.h
// Projet:			Desktop Connection Library
// 
// Créé le:			20/3/2004
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
// The Original Code is TDCLClParagraph.h.
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
// $Id: TDCLClParagraph.h,v 1.3 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLCLPARAGRAPH_H
#define _TDCLCLPARAGRAPH_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Data/Text/TDCLRichText.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLNSFrame;
class TDCLNSString;

///
/// Classe pour un paragraphe avec style clParagraph.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLClParagraph
	:
		public TDCLRichText
{
public:
	///
	/// Constructeur par défaut.
	/// Crée un texte vide.
	///
	TDCLClParagraph( void );

	///
	/// Constructeur à partir de données Newton.
	///
	/// \param inDataFrame	structure Newton telle qu'on peut la trouver dans
	///						les notes (clParagraph).
	///
	TDCLClParagraph( const TDCLNSRef& inDataFrame );

	///
	/// Constructeur par copie.
	///
	/// \param inCopy	autre paragraphe Newton (copié)
	///
	TDCLClParagraph( const TDCLClParagraph& inCopy );

	///
	/// Constructeur à partir d'un autre texte avec styles.
	///
	/// \param inCopy	autre texte avec styles.
	///
	TDCLClParagraph( const TDCLRichText& inCopy );

	///
	/// Destructeur.
	///
	virtual ~TDCLClParagraph( void ) {};

	///
	/// Concatène avec un autre texte riche.
	///
	virtual void Catenate( const TDCLRichText& inAlter );

	///
	/// Récupère la taille du texte, i.e. le nombre de
	/// caractères.
	///
	/// \return la taille du texte, en caractères.
	///
	virtual KUInt32 GetSize( void ) const;

	///
	/// Copie n caractères du texte.
	///
	/// \param inOffset		décalage à partir duquel copier les caractères.
	/// \param inCount		nombre de caractères à copier.
	/// \param outChars		mémoire tampon où stocker les caractères.
	///
	virtual void	GetText(
						KUInt32 inOffset,
						KUInt32 inCount,
						KUInt16* outChars ) const;

	///
	/// Détermine le nombre de styles.
	///
	/// \return le nombre de styles.
	///
	virtual KUInt32 CountStyleRuns( void ) const;

	///
	/// Retourne l'indice du début du i-ème style.
	///
	/// \param inStyleRunIndex	indice du style dont on veut le début (base 0).
	/// \return l'indice du caractère où commence le i-ème style.
	///
	virtual KUInt32	GetStyleRunStart( KUInt32 inStyleRunIndex ) const;

	///
	/// Retourne la longueur du i-ème style.
	///
	/// \param inStyleRunIndex	indice du style dont on veut la longueur.
	/// \return le nombre de caractères du i-ème style.
	///
	virtual KUInt32	GetStyleRunLength( KUInt32 inStyleRunIndex ) const;

	///
	/// Retourne une référence sur le n-ième style du texte.
	///
	/// \param inStyleRunIndex	index du style à retourner (base 0).
	/// \return le n-ième style du paragraphe.
	///
	virtual TDCLStyleRef	GetStyle( KUInt32 inStyleRunIndex ) const;

	///
	/// Modifie le style du texte entre deux caractères.
	/// Si le style ajouté est un graphique, le texte entre les ...
	/// 
	///
	/// \param inTextIndex	début du bout du texte dont on veut changer le style
	/// \param inTextLength	longueur du texte dont on veut changer le style
	/// \param inStyle		style à appliquer à ce texte.
	///
	virtual void			SetStyle(
								KUInt32 inTextIndex,
								KUInt32 inTextLength,
								TDCLStyleRef inStyle );

private:
	/// \name Variables
	TDCLNSRef				mFrameRef;	///< Référence sur la structure.
	TDCLNSFrame*			mFrame;		///< La structure (évite les
										///< conversions multiples)
};

#endif
		// _TDCLCLPARAGRAPH_H

// ========================================================================== //
// Real computer scientists don't write code.  They occasionally tinker with  //
// `programming systems', but those are so high level that they hardly count  //
// (and rarely count accurately; precision is for applications).              //
// ========================================================================== //
