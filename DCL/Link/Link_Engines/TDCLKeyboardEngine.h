// ==============================
// Fichier:			TDCLKeyboardEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/4/2003
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
// The Original Code is TDCLKeyboardEngine.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLKeyboardEngine.h,v 1.3 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLKEYBOARDENGINE_H
#define _TDCLKEYBOARDENGINE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLLinkEngine.h>

///
/// Classe pour le gestionnaire de clavier.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLKeyboardEngine
	:
		public TDCLLinkEngine
{
public:
	///
	/// Constructeur à partir d'un lien.
	///
	/// \param inLink	référence sur le lien.
	///
	TDCLKeyboardEngine( TDCLFullDockLink* inLink );
	
	///
	/// Méthode appelée pour gérer des commandes venant de l'application.
	///
	/// \param inCommand	commande à exécuter.
	/// \param outProcessed	si la commande a été traitée.
	/// \return \c true si le moteur continue, \c false sinon
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean				ProcessAppCommand(
									TDCLAppCommand* inCommand,
									Boolean* outProcessed );

	///
	/// Méthode appelée par le lien pour commencer à utiliser le clavier.
	/// Cette méthode doit ouvrir la fenêtre qui demande de taper des touches
	/// à l'utilisateur ou un truc dans ce genre.
	///
	virtual void				StartUsingKeyboard( void ) = 0;

	///
	/// Méthode appelée par le lien pour signifier que le travail du
	/// moteur est terminé.
	/// Cette méthode doit fermer la fenêtre qui demande de taper des touches
	/// à l'utilisateur ou un truc dans ce genre.
	///
	virtual void				WorkIsOver( void ) = 0;

	///
	/// Détermine les commandes de l'application que le lien
	/// peut traiter. Cette méthode est appelée à chaque fois que le
	/// lien va attendre un événement.
	/// On n'accepte que les commandes relatives au clavier.
	///
	/// \return le masque pour les commandes de l'application
	///
	virtual KUInt32				GetAppCommandMask( void );
};

#endif
		// _TDCLKEYBOARDENGINE_H

// =========================================================================== //
// Try to find the real tense of the report you are reading:  Was it done, is  //
// it being done, or is something to be done?  Reports are now written in four //
// tenses:  past tense, present tense, future tense, and pretense.  Watch for  //
// novel uses of CONGRAM (CONtractor GRAMmar), defined by the imperfect past,  //
// the insufficient present, and the absolutely perfect future.                //
//                 -- Amrom Katz                                               //
// =========================================================================== //
