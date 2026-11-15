// ==============================
// Fichier:			TDCLDockLink.h
// Projet:			Desktop Connection Library
//
// Créé le:			13/08/2001
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
// The Original Code is TDCLDockLink.h.
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
// $Id: TDCLDockLink.h,v 1.3 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLDOCKLINK_H
#define _TDCLDOCKLINK_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLink.h>

///
/// Classe pour un lien minimal.
/// Les commandes du Newton qui sont gérées sont kDHello et kDDisconnect.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockLink
	:
		public TDCLLink
{
public:
	///
	/// Icônes pour le paramètre inIcons du constructeur.
	/// Un jour, nous pourrons mettre nos propres icônes dans le dialogue du dock.
	///
	/// La présence de l'icône signifie que l'utilisateur peut la cliquer
	/// et donc qu'il faut gérer le protocole associé.
	///
	enum {
		kNoIcon				= 0,	///< Pas d'icône
		kBackupIcon			= 1,	///< Icône pour la sauvegarde.
		kRestoreIcon		= 2,	///< Icône de restauration.
		kInstallIcon		= 4,	///< Icône d'installation de paquet.
		kImportIcon			= 8,	///< Icône d'importation de documents (sauf Works)
	  	kSyncIcon			= 16,	///< Icône de synchronisation
		kKeyboardIcon		= 32,	///< Icône pour utiliser le clavier
		kAllIcons			= 63	///< Constante pour utiliser toutes les icônes.
	};

protected:
	///
	/// Constructeur unique.
	///
	/// \param inApplication	Référence sur l'application.	
	/// \param inIcons			les icônes que l'on veut montrer dans
	///							l'application Dock sur sur le Newton.
	///
	TDCLDockLink(
			TDCLApplication* inApplication,
			KUInt32 inIcons = kAllIcons );
							
	///
	/// Méthode synchrone pour connecter le Newton.
	///
	/// Appelle d'abord TDCLLink::DoConnect pour le début du dialogue:
	///
	/// Desktop			Newton
	///				<- kDRequestToDock
	/// kDInitiateDocking ->
	///				<- kDNewtonName
	///
	/// Puis continue avec le dialogue suivant:
	///
	/// kDDesktopInfo ->
	///				<- kDNewtonInfo
	/// kDWhichIcons ->
	///				<- kDResult
	/// kDSetTimeout ->
	///				<- kDPassword
	/// kDPassword ->
	///				<- kDResult
	///
	/// \throws TDCLException si un problème est survenu.
	///
	virtual void		DoConnect( void );	

	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	/// Cette classe ne gère que kDHello et kDDisconnect.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le prochain état du lien (l'état actuel étant kRunning),
	///			par exemple kRunning ou kDisconnecting (DoDisconnect est appelé
	///			dans la boucle)
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	EState		ProcessDockCommand(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	///	Méthode pour récupérer le mot de passe.
	/// Le mot de passe appartient au lien et n'est pas libéré par l'appelant.
	/// La méthode par défaut fournit \c nil qui veut dire pas de mot de passe
	/// (la chaîne vide). Au moment où cette méthode est appelée, l'identité du
	/// Newton est connu (NewtonName a été reçu et les méthodes de TDCLLink
	/// retournent des données cohérentes).
	///
	/// \return le mot de passe ou \c nil
	///
	virtual const KUInt16*	GetPassword( void );
	
	///
	/// Accesseur sur les icônes.
	///
	/// \return les icônes pour la prochaine connexion.
	///
	inline KUInt32 GetIcons( void ) const
		{
			return mIcons;
		}
	
	///
	/// Sélecteur sur les icônes.
	/// Remarque: ceci n'a d'effet que pour la prochaine connexion.
	///
	/// \param inNewIcons les nouvelles icônes pour la prochaine connexion.
	///
	inline void SetIcons( KUInt32 inNewIcons )
		{
			mIcons = inNewIcons;
		}

private:
	/// \name Variables privées.
	KUInt32				mIcons;	///< paramètre pour les icônes
};

#endif
		// _TDCLDOCKLINK_H

// ========================================================================= //
// Let the machine do the dirty work.                                        //
//                 -- "Elements of Programming Style", Kernighan and Ritchie //
// ========================================================================= //
