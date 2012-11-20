// ==============================
// Fichier:			TDCLFullDockLink.h
// Projet:			Desktop Connection Library
//
// Créé le:			22/06/2002
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
// The Original Code is TDCLFullDockLink.h.
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
// $Id: TDCLFullDockLink.h,v 1.3 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLFULLDOCKLINK_H
#define _TDCLFULLDOCKLINK_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Link/TDCLDockLink.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

// Pré-déclarations

class TDCLApplication;
class TDCLFile;
class TDCLBrowseEngine;
class TDCLKeyboardEngine;
class TDCLLoadPkgEngine;
class TDCLLinkEngine;
class TDCLSyncEngine;

///
/// Classe pour un lien qui gère l'ensemble des commandes documentées.
/// 
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFullDockLink
	:
		public TDCLDockLink
{
public:
	///
	/// La classe TDCLBrowseEngine doit pouvoir appeler les méthodes virtuelles
	/// de cette classe pour effectuer des opérations demandées pendant le butinage.
	///
	friend class TDCLBrowseEngine;
	
	enum {
		// Command sets this class supports.
		kNilSet				= 0,
		kFileBrowsingSet	= 1,	// Depending on the other sets,
									// the request to browse command will lead to an error or not.
		kInstallPackageSet	= 2,
		kSyncSet			= 4,
		kKeyboardSet		= 8,
		kAllSets			= 15
	};

	///
	/// Constructeur unique.
	///
	/// \param inApplication	application qui gère ce lien
	/// \param inWhatToProcess	entier qui détermine les ensembles de commandes à gérer
	/// \param inIcons			entier qui détermine les icônes à afficher dans le dock du Newton
	///
	TDCLFullDockLink(
					TDCLApplication* inApplication,
					KUInt32 inWhatToProcess = kAllSets,
					KUInt32 inIcons = kAllIcons );

	///
	/// Destructeur.
	///
	~TDCLFullDockLink( void );

	/// \name Interface avec l'appliaction.
	
	///
	/// Indique au Newton que l'on souhaite installer un paquet et installe
	/// un paquet. (Cette méthode est en fait asynchrone et poste un événement
	/// pour le lien).
	///
	/// \param inPackage	paquet à installer.
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean InstallPackage( TDCLFSItemRef inPackage );
		
	///
	/// Indique au Newton que l'on souhaite utiliser le clavier et
	/// utilise le clavier.
	///
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean StartUsingKeyboard( void );

	///
	/// Envoie une chaîne au Newton. Asynchrone aussi.
	///
	/// \remark cette chaîne est envoyée si le lien est connecté et sera traitée
	/// 		si le lien était en mode clavier, sinon une erreur sera retournée
	///			à l'application dans AppCommandProcessed.
	/// \param inString		chaîne à envoyer (copiée)
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean SendKeyboardString( const KUInt16* inString );
		
	///
	/// Envoie un caractère au Newton.
	///
	/// \remark comme la méthode précédente.
	/// \param inChar		caractère à envoyer
	/// \param inFlags		drapeaux (permet en théorie d'envoyer des modificateurs)
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean SendKeyboardChar( KUInt16 inChar, KUInt16 inFlags );
		
	///
	/// Arrête d'utiliser le clavier.
	///
	/// \remark comme les deux méthodes précédentes.
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean StopUsingKeyboard( void );
		
	///
	/// Indique au Newton que l'on souhaite synchroniser et effectue
	/// cette synchronisation.
	///
	/// \remark comme les méthodes précédentes.
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean Synchronize( void );

protected:
	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	/// Cette classe gère les commandes en fonction de l'ensemble
	/// déterminé par le constructeur.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le prochain état du lien (l'état actuel étant kRunning),
	///			par exemple kRunning ou kDisconnecting (DoDisconnect est appelé
	///			dans la boucle)
	///
	virtual	EState		ProcessDockCommand(
								TDCLDockCommand* inCommand,
								Boolean* outProcessed );

	///
	/// Méthode appelée pour gérer des commandes venant de l'application.
	/// Toutes les commandes sont gérées, quelque soit mHandledCommandSets.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le prochain état du lien (l'état actuel étant kRunning)
	///
	virtual	EState		ProcessAppCommand(
								TDCLAppCommand* inCommand,
								Boolean* outProcessed );

	///
	/// Installe véritablement un paquet.
	///
	/// \param inPackage	paquet à installer.
	/// \throws TDCLException si une erreur est survenue.
	///
	void DoInstallPackage( TDCLFSItemRef inPackage );
	
	///
	/// Effectue le butinage.
	///
	/// \param inFileTypes	types des fichiers que le Newton veut butiner
	/// \throws TDCLException si une erreur est survenue.
	///
	void DoBrowse( const TDCLNSRef& inFileTypes );

	///
	/// Effectue véritablement la synchronisation.
	///
	/// \throws TDCLException si une erreur est survenue.
	///
	void DoSynchronize( void );

	///
	/// Utilise véritablement le clavier.
	///
	/// \throws TDCLException si une erreur est survenue.
	///
	void DoUseKeyboard( void );

	///
	/// Détermine les commandes de l'application que le lien
	/// peut traiter. Cette méthode est appelée à chaque fois que le
	/// lien va attendre un événement.
	///
	/// Demande à un éventuel moteur de lien courant, sinon
	/// on demande à la classe mère.
	///
	/// \return le masque pour les commandes de l'application
	///
	virtual KUInt32				GetAppCommandMask( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFullDockLink( const TDCLFullDockLink& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFullDockLink& operator = ( const TDCLFullDockLink& inCopy );

	///
	/// Récupère le moteur de butinage.
	/// Par défaut, retourne mDefaultBrowseEngine.
	///
	/// \return une référence sur le moteur de butinage.
	///
	virtual TDCLBrowseEngine*	GetBrowseEngine( void );

	///
	/// Récupère le moteur d'installation des paquets.
	/// Par défaut, retourne mDefaultLoadPkgEngine;
	///
	/// \return une référence sur le moteur d'installation des paquets.
	///
	virtual TDCLLoadPkgEngine*	GetLoadPkgEngine( void );

	///
	/// Récupère le gestionnaire de clavier.
	/// Par défaut, retourne \c nil.
	///
	/// \return une référence sur le moteur de synchronisation.
	///
	virtual TDCLKeyboardEngine*	GetKeyboardEngine( void );

	///
	/// Récupère le moteur de synchronisation.
	/// Par défaut, retourne \c nil.
	///
	/// \return une référence sur le moteur de synchronisation.
	///
	virtual TDCLSyncEngine*		GetSyncEngine( void );

	///
	/// Envoie une requête (pour commencer une opération) et lit le
	/// message d'erreur.
	///
	/// \param inRequest	requête à envoyer.
	/// \throws TDCLException si une erreur est survenue.
	///
	void SendRequest( KUInt32 inRequest );

	/// \name Variables
	KUInt32				mHandledCommandSets;	///< Ensemble des commandes
												///< à gérer.
	TDCLLinkEngine*		mCurrentEngine;			///< le moteur courant (\c nil
												///< si aucun moteur ne gère les commandes)
	TDCLBrowseEngine*	mDefaultBrowseEngine;	///< Moteur de butinage par défaut.
	TDCLLoadPkgEngine*	mDefaultLoadPkgEngine;	///< Moteur d'installation par défaut.
};

#endif
		// _TDCLFULLDOCKLINK_H

// ==================================================================== //
// "The algorithm to do that is extremely nasty.  You might want to mug //
// someone with it."                                                    //
//                 -- M. Devine, Computer Science 340                   //
// ==================================================================== //
