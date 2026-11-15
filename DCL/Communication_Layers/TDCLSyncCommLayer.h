// ==============================
// Fichier:			TDCLSyncCommLayer.h
// Projet:			Desktop Connection Library
//
// Créé le:			28/3/2003
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
// The Original Code is TDCLSyncCommLayer.h.
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
// $Id: TDCLSyncCommLayer.h,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef _TDCLSYNCCOMMLAYER_H
#define _TDCLSYNCCOMMLAYER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Interfaces/TDCLThread.h>
#include <DCL/Communication_Layers/TDCLPipe.h>

// Pré-déclarations.
class IDCLThreads;

///
/// Classe pour une interface de communication synchrone qui requiert
/// donc un processus léger séparé.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLSyncCommLayer
	:
		public TDCLCommLayer,
		protected TDCLThread
{
public:
	///
	/// Constructeur.
	///
	/// Enregistre le nombre maximum de connexion dans \c mMaxPipeCount.
	/// Enregistre aussi \p inFlushAvailable et \p inTimeout dans les champs
	/// privés correspondants.
	///
	/// \remark ce constructeur est publique, mais la classe est abstraite.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inFlushAvailable	si la couche de communication sait vider la
	///							mémoire tampon de sortie.
	///							Si la couche de communication ne sait pas le
	///							faire, l'appelant devra attendre avant de
	///							fermer la connexion.
	/// \param inMaxPipeCount	nombre maximum de connexion simultanées.
	///							0 signifie pas de limite.
	///
	TDCLSyncCommLayer(
			IDCLThreads* inThreadsIntf,
			Boolean	inFlushAvailable = false,
			KUInt32	inMaxPipeCount = kDefaultPipeCount
			);
	
	///
	/// Commence à écouter.
	/// Lance le processus léger.
	///
	/// \param inServer	serveur à prévenir lorsqu'une requête est arrivée.
	/// \throws TDCLException si le serveur est déjà en train d'écouter.
	///
	virtual void		StartListening( TDCLServer* inServer );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	/// Dit au processus léger de s'arrêter une fois que DoStopListening a terminé.
	/// Ne fait rien si on n'est pas en train d'écouter.
	///
	virtual void		StopListening( void );

protected:
	///
	/// Méthode appelée dans la méthode Run juste avant la boucle qui appelle
	/// WaitForIncomingRequest.
	///
	virtual	void		DoStartListening( void ) = 0;

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	/// Cette méthode doit d'abord interrompre l'attente dans la méthode Run
	/// et ceci ne doit pas avoir de conséquence si on n'est pas en train d'attendre dans
	/// cette méthode.
	/// Attention aux possible problème d'attente/de réveil.
	///
	virtual	void		DoStopListening( void ) = 0;

	///
	/// Attend qu'une requête se présente. Cette méthode est synchrone (c'est l'intérêt
	/// de cette classe). Elle retourne si une requête est présente ou si elle a été
	/// interrompue par DoStopListening.
	///
	/// \return \c true si une requête est présente, \c false si la méthode a été
	/// interrompue (ou si on n'écoute plus).
	///
	virtual Boolean 	WaitForIncomingRequest( void ) = 0;

	///
	/// Retourne un objet connexion représentant le canal de communication avec le client
	/// ou \c nil s'il n'y a pas de requête de connexion présente.
	/// C'est un objet créé avec \c new. L'appelant devra le supprimer.
	/// Cette méthode appelle DoAccept et ensuite réveille le processus léger pour attendre la
	/// présence de nouvelles requêtes.
	///
	/// \return un nouvel objet TDCLPipe créé avec \c new représentant la connexion ou \c nil.
	///
	virtual	TDCLPipe*	Accept( void );
	
	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	/// Cette méthode appelle DoRefuse et ensuite réveille le processus léger pour attendre la
	/// présence de nouvelles requêtes.
	///
	virtual	void		Refuse( void );
	
	///
	/// Retourne un objet connexion représentant le canal de communication avec le client
	/// ou \c nil s'il n'y a pas de requête de connexion présente.
	/// C'est un objet créé avec \c new. L'appelant devra le supprimer.
	///
	/// \return un nouvel objet TDCLPipe créé avec \c new représentant la connexion ou \c nil.
	///
	virtual	TDCLPipe*	DoAccept( void ) = 0;
	
	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	///
	virtual	void		DoRefuse( void ) = 0;
	
	///
	/// Méthode exécutée dans le processus.
	/// Boucle tant que WaitForIncomingRequest retourne \c true. À chaque fois que
	/// cette dernière méthode retourne \c true, envoie un événement au serveur.
	///
	virtual void Run( void );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée.
	/// Lorsque cette méthode retourne, le processus est terminé.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void HandleException( TDCLException* inException );

	///
	/// On donne tous les droits a la classe fille.
	///
	class TSyncPipe;
	friend class TSyncPipe;

	///
	/// Class pour les connexions synchrone.
	///
	class TSyncPipe
		:
			public TDCLPipe,
			protected TDCLThread
	{
	public:
		/// La classe mère est une amie.
		friend class TDCLSyncCommLayer;

		///
		/// Constructeur.
		/// Remplit les champs privés.
		///
		/// \param inThreadsIntf	interface pour les processus légers.
		/// \param inCommLayer		couche de communication à laquelle est
		///							rattachée la connexion.
		///
		TSyncPipe(
				IDCLThreads* inThreadsIntf,
				TDCLSyncCommLayer*	inCommLayer );

		///
		/// Attend que des données se présentent. Cette méthode est synchrone (c'est l'intérêt
		/// de cette classe). Elle retourne si des données sont présentes ou si elle a été
		/// interrompue par Disconnect.
		///
		/// \return \c true si des données sont présentes, \c false si la méthode a été
		/// interrompue.
		///
		virtual Boolean 	WaitForIncomingData( void ) = 0;

		///
		/// Déconnecte le canal de communication avec le client.
		/// Appelle DoDisconnect et attend que le processus léger ait terminé.
		///
		/// \remark	une fois que cette méthode est appelée, l'objet connexion est inutilisable.
		/// 		Il peut être supprimé par l'appelant à tout instant.
		///
		virtual	void		Disconnect( void );

		///
		/// Déconnecte effectivement le canal de communication avec le client.
		/// Cette méthode doit envoyer une interruption pour que WaitForIncomingData
		/// retourne.
		///
		virtual	void		DoDisconnect( void ) = 0;

		///
		/// Méthode appelée par le lien pour dire qu'il est connecté
		/// via cette connexion. Cette méthode est appelée juste avant
		/// que des données soient échangées.
		///
		/// Lance le processus léger et appelle la méthode par défaut.
		///
		/// \return l'interface de communication.
		///
		virtual	TDCLCommLayer*	Connected( TDCLLink* inLink );

		///
		/// Méthode appelée par le lien pour dire qu'il souhaite
		/// être prévenu dès que des données sont présentes.
		/// Au départ, l'interface de communication ne prévient
		/// pas le lien de la présence de données.
		///
		virtual	void		ClearDataPresent( void );

		///
		/// Méthode appelée lorsqu'une exception n'est pas interceptée.
		/// Lorsque cette méthode retourne, le processus est terminé.
		///
		/// \param inException	l'exception si c'est une TDCLException,
		///			\c nil sinon.
		///
		virtual void HandleException( TDCLException* inException );

	protected:
		///
		/// Méthode exécutée dans le processus.
		/// Boucle tant que WaitForIncomingData retourne \c true. À chaque fois que
		/// cette dernière méthode retourne \c true, envoie un événement au serveur.
		///
		virtual void Run( void );
	};
};

#endif
		// _TDCLSYNCCOMMLAYER_H

// ====================================================================== //
// Established technology tends to persist in the face of new technology. //
//                 -- G. Blaauw, one of the designers of System 360       //
// ====================================================================== //
