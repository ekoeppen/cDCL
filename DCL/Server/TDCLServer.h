// ==============================
// Fichier:			TDCLServer.h
// Projet:			Desktop Connection Library
//
// Créé le:			14/01/2002
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
// The Original Code is TDCLServer.h.
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
// $Id: TDCLServer.h,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef __TDCLSERVER__
#define __TDCLSERVER__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Interfaces/TDCLThread.h>

class TDCLLink;

///
/// Classe abstraite pour les serveurs.
///
/// Un serveur écoute sur une ou plusieurs interfaces (pipes) et les associe à
/// un lien (link) lorsque le Newton se connecte.
/// Il peut alors créer plus d'interfaces ou tuer les interfaces précédentes ou
/// leur dire de ne plus écouter. Tout ceci dépend de la sous-classe.
///
/// L'accès aux interfaces et aux liens doit être fait en
/// verrouillant/déverrouillant les listes. Verrouiller les listes empêche le
/// serveur de créer des nouveaux liens ou des nouvelles interfaces. Donc vous
/// devez les déverrouiller dès que possible.
///
/// Par ailleurs, le serveur comprend un sémaphore mutex pour protéger les
/// accès aux méthodes Start/Stop/Kill.
///
/// Le serveur est représenté par un processus léger qui généralement reste en
/// attente. La boucle est:
/// - Tant qu'il n'y a pas de connexion ou qu'il ne faut pas quitter, attendre.
/// - Faire ce qu'il y a à faire.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLServer
	:
		private TDCLThread
{
public:
	///
	/// Le constructeur est public, mais la classe est abstraite.
	///
	/// \param inApplication	interface avec le système hôte. Utilisé
	///							pour le sémaphore.
	///
	TDCLServer( TDCLApplication* inApplication );

	///
	/// Destructeur.
	/// Tue le serveur s'il est en train de tourner.
	///
	virtual	~TDCLServer( void );


	///
	/// Etats du serveur.
	///
	enum EState {
		kStopped,	///< Etat initial.
		kStarting,	///< Etat juste après que Start ait été appelé.
		kRunning,	///< Etat une fois que DoStart a terminé.
		kStopping,	///< En cours d'arrêt.
		kUnknown	///< Etat inconnu (en cas d'exception)
	};	

	///
	/// Démarre le serveur.
	/// Ne fait rien si le serveur n'est pas à l'arrêt.
	/// Change l'état en kStarting si tout va bien.
	///
	/// \throws	TDCLException si un problème est survenu. Dans ce cas
	///			le serveur est maintenu dans l'état arrêté.
	///
	void				Start( void );

	///
	/// Commence l'arrêt du serveur en douceur.
	/// Normalement asynchrone, mais ceci peut être synchrone.
	/// Ne fait rien si le serveur ne tourne pas.
	/// Change l'état en kStopping ou kStopped suivant que l'opération
	/// était asynchrone ou synchrone.
	///
	/// \throws	TDCLException si un problème est survenu. Dans ce cas
	///			le serveur est dans l'état inconnu (ne devrait pas arriver).
	///
	void				Stop( void );
	
	///
	/// Termine brutalement toute connexion (opération synchrone).
	/// Ne fait rien si le serveur ne tourne pas.
	/// Tue aussi les interfaces de communication qui sont en train de
	/// se fermer. Change l'état en kStopped.
	///
	/// \throws	TDCLException si un problème est survenu. Dans ce cas
	///			le serveur est dans l'état inconnu (ne devrait pas arriver).
	///
	void				Kill( void );

	///
	/// Récupère l'état du serveur.
	/// Note: cette méthode utilise le sémaphore comme Start/Stop/Kill.
	///
	/// \return l'état actuel du serveur
	///
	EState				GetState( void );

	///
	/// Méthode appelée par les couches de communication pour indiquer qu'une
	/// requête est présente. Cette méthode envoie un événement au processus
	/// léger du serveur.
	///
	/// \param inCommLayer	couche de communication qui a reçu une requête.
	///
	void				RequestPresent( TDCLCommLayer* inCommLayer );

	///
	/// Méthode appelée par les couches de communication pour indiquer qu'elles
	/// attendent une connexion. C'est le moment pour l'utilisateur de demander
	/// au Newton de se connecter.
	///
	/// \param inCommLayer	couche de communication qui attend.
	///
	void				WaitingConnection( TDCLCommLayer* inCommLayer );

	///
	/// Méthode appelée par les liens pour indiquer qu'ils sont déconnectés
	/// (i.e. que la communication avec le Newton est terminée).
	///
	/// \param inLink		lien qui est déconnecté.
	///
	void				Disconnected( TDCLLink* inLink );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'une/de la couche de communication.
	/// Cette méthode prévient l'application.
	///
	/// \param inCommLayer	couche qui s'est vautrée.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 		HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException );

protected:
	///
	/// Méthode appelée lorsqu'on a reçu un événement.
	/// Cette méthode peut être dérivée si un serveur a besoin de recevoir
	/// des événements non prévus.
	/// Par défaut
	///
	/// \param 	inEvent	l'événement reçu.
	/// \param 	outProcessed en sortie, \c true si l'événement a été géré,
	///			inchangé sinon. Si l'événement n'est pas géré, une exception
	///			sera levée.
	/// \return le nouvel état du serveur.
	///
	virtual EState		ProcessEvent(
							TDCLEvent* inEvent,
							Boolean* outProcessed );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée.
	/// Lorsque cette méthode retourne, le processus est terminé.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 		HandleException( TDCLException* inException );

	/// \name Interface TDCLThread
	
	///
	/// Cette méthode est appelée dans le processus léger du serveur.
	/// C'est la boucle principale du serveur.
	/// Cette boucle est terminée lorsqu'une exception survient ou lorsque
	/// le serveur est arrêté.
	///
	virtual void		Run( void );

	/// \name Interface TDCLServer
	
	///
	/// Commence à écouter.
	/// Cette méthode est appelée lorsque le serveur est démarré (depuis
	/// le processus léger du serveur).
	///
	virtual	void		DoStartListening( void ) = 0;

	///
	/// Arrête le serveur.
	///
	virtual	void		DoStop( void ) = 0;
	
	///
	/// Ferme toutes les connexions brutalement.
	///
	virtual	void		DoKill( void ) = 0;

	///
	/// Gère l'arrivée d'une nouvelle connexion.
	///
	/// \param inCommLayer	interface de communication qui a reçu la requête.
	/// \return le nouvel état du serveur.
	/// \throws TDCLException si un problème est survenu.
	///
	virtual EState		HandleIncomingConnection(
								TDCLCommLayer* inCommLayer ) = 0;

	///
	/// Méthode appelée dans le processus léger du serveur pour indiquer
	/// qu'un lien a été déconnecté et qu'il faut fermer l'interface de
	/// communication.
	///
	/// \param inLink	le lien qui a été déconnecté.
	/// \return le nouvel état du serveur.
	/// \throws TDCLException si un problème est survenu.
	///
	virtual	EState		HandleLinkDisconnection( TDCLLink* inLink ) = 0;

	///
	/// Méthode appelée par le serveur lorsque rien ne se passe.
	/// Elle permet d'appeler Idle sur les interfaces de communication qui en
	/// ont besoin.
	///
	virtual	void		DoIdle( void ) = 0;

	inline	TDCLApplication*	GetApplication( void )
			{
				return mApplication;
			}
	
private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLServer( const TDCLServer& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLServer& operator = ( const TDCLServer& inCopy );

	/// \name Variables

	TDCLApplication*			mApplication;	///< Référence sur
												///< l'application.
	IDCLThreads::ISemaphore*	mStateMutex;	///< Mutex sur l'état.
	EState						mState;			///< Etat.
};

#endif
		// __TDCLSERVER__

// ============================================================= //
// APL is a natural extension of assembler language programming; //
// ...and is best for educational purposes.                      //
//                 -- A. Perlis                                  //
// ============================================================= //
