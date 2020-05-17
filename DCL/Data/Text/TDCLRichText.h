// ==============================
// Fichier:			TDCLNSRichText.h
// Projet:			Desktop Connection Library
// 
// Créé le:			13/3/2004
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
// The Original Code is TDCLNSRichText.h.
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
// $Id: TDCLRichText.h,v 1.5 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLNSRICHTEXT_H
#define _TDCLNSRICHTEXT_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Data/Text/TDCLStyleRef.h>

///
/// Interface pour un paragraphe avec styles et graphismes.
/// Les graphismes sont représentés comme des styles sur un seul
/// caractère. Le caractère de la chaîne qui correspond à un graphisme
/// dépend des données. Par exemple, c'est 0xF701 pour l'encre dans
/// les paragraphes clParagraph et 0x2601 pour les données NewtWorks.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLRichText
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLRichText( void ) {};

	///
	/// Destructeur.
	///
	virtual ~TDCLRichText( void ) {};

	///
	/// Concatène avec un autre texte riche.
	///
	virtual void Catenate( const TDCLRichText& inAlter ) = 0;

	///
	/// Récupère la taille du texte, i.e. le nombre de
	/// caractères.
	///
	/// \return la taille du texte, en caractères.
	///
	virtual KUInt32 GetSize( void ) const = 0;

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
						KUInt16* outChars ) const = 0;

	///
	/// Détermine le nombre de styles.
	///
	/// \return le nombre de styles.
	///
	virtual KUInt32 CountStyleRuns( void ) const = 0;

	///
	/// Retourne l'indice du début du i-ème style.
	///
	/// \param inStyleRunIndex	indice du style dont on veut le début (base 0).
	/// \return l'indice du caractère où commence le i-ème style.
	///
	virtual KUInt32	GetStyleRunStart( KUInt32 inStyleRunIndex ) const = 0;

	///
	/// Retourne la longueur du i-ème style.
	///
	/// \param inStyleRunIndex	indice du style dont on veut la longueur.
	/// \return le nombre de caractères du i-ème style.
	///
	virtual KUInt32	GetStyleRunLength( KUInt32 inStyleRunIndex ) const = 0;

	///
	/// Retourne une référence sur le n-ième style du texte.
	///
	/// \param inStyleRunIndex	index du style à retourner (base 0).
	/// \return le n-ième style du paragraphe.
	///
	virtual TDCLStyleRef	GetStyle( KUInt32 inStyleRunIndex ) const = 0;

	///
	/// Modifie le style du texte entre deux caractères.
	/// Si le style ajouté est un graphique, le texte entre les
	/// 
	///
	/// \param inTextIndex	début du bout du texte dont on veut changer le style
	/// \param inTextLength	longueur du texte dont on veut changer le style
	/// \param inStyle		style à appliquer à ce texte.
	///
	virtual void			SetStyle(
								KUInt32 inTextIndex,
								KUInt32 inTextLength,
								TDCLStyleRef inStyle ) = 0;

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLRichText( const TDCLRichText& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLRichText& operator = ( const TDCLRichText& inCopy );
};

#endif
		// _TDCLNSRICHTEXT_H

// ============================================================================== //
// Dear Emily:                                                                    //
//         I'm having a serious disagreement with somebody on the net. I          //
// tried complaints to his sysadmin, organizing mail campaigns, called for        //
// his removal from the net and phoning his employer to get him fired.            //
// Everybody laughed at me.  What can I do?                                       //
//                 -- A Concerned Citizen                                         //
//                                                                                //
// Dear Concerned:                                                                //
//         Go to the daily papers.  Most modern reporters are top-notch computer  //
// experts who will understand the net, and your problems, perfectly.  They       //
// will print careful, reasoned stories without any errors at all, and surely     //
// represent the situation properly to the public.  The public will also all      //
// act wisely, as they are also fully cognizant of the subtle nature of net       //
// society.                                                                       //
//         Papers never sensationalize or distort, so be sure to point out things //
// like racism and sexism wherever they might exist.  Be sure as well that they   //
// understand that all things on the net, particularly insults, are meant         //
// literally.  Link what transpires on the net to the causes of the Holocaust, if //
// possible.  If regular papers won't take the story, go to a tabloid paper --    //
// they are always interested in good stories.                                    //
// ============================================================================== //
