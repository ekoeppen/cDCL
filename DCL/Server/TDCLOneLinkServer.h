// ==============================
// Fichier:			TDCLOneLinkServer.h
// Projet:			Desktop Connection Library
//
// Créé le:			3/4/2003
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
// The Original Code is TDCLOneLinkServer.h.
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
// $Id: TDCLOneLinkServer.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLONELINKSERVER_H
#define _TDCLONELINKSERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

///
/// Classe pour un serveur avec un seul lien et plusieurs couches de
/// communication. C'est un serveur de type NCU.
///
/// Il faut d'abord créer le lien (parce que le lien suppose que le
/// serveur pré-existe).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLOneLinkServer
	:
		public TDCLServer
{
public:
	///
	/// Créateur à partir d'une application.
	///
	/// \param inApplication	application prévenue de l'activité du serveur.
	///
	TDCLOneLinkServer( TDCLApplication* inApplication );
	
	///
	/// Fixe le lien.
	///
	/// \param inLink		lien à utiliser lorsqu'une connexion est établie.
	///
	inline void			SetLink( TDCLLink* inLink )
		{
			mLink = inLink;
		}

	///
	/// Destructeur.
	///
	virtual	~TDCLOneLinkServer( void );

	///
	/// Ajoute une couche de communication.
	/// Cette méthode peut être appelée même si le serveur tourne.
	/// La couche de communication devra être supprimée par l'application.
	///
	/// \param inLayer	la couche à ajouter au serveur.
	///
	void		AddCommLayer( TDCLCommLayer* inLayer );
	
	///
	/// Enlève une couche de communication.
	/// Cette méthode peut être appelée même si le serveur tourne.
	/// La couche de communication devra être supprimée par l'application.
	/// Ne fait rien si la couche n'est pas dans la liste.
	///
	/// \param inLayer	la couche à supprimer du serveur.
	///
	void		RemoveCommLayer( TDCLCommLayer* inLayer );
	
	///
	/// Fonction utilisée pour déterminer si le serveur est "actif" ou non.
	///
	/// \return true si le serveur est déjà connecté, false sinon.
	///
	TDCLLink*			GetConnectionLink( void );

protected:
	///
	/// Commence à écouter.
	/// Cette méthode est appelée lorsque le serveur est démarré (depuis
	/// le processus léger du serveur).
	///
	virtual	void		DoStartListening( void );

	///
	/// Arrête le serveur.
	///
	virtual	void		DoStop( void );
	
	///
	/// Ferme toutes les connexions brutalement.
	///
	virtual	void		DoKill( void );

	///
	/// Gère l'arrivée d'une nouvelle connexion.
	///
	/// \param inCommLayer	interface de communication qui a reçu la requête.
	/// \throws TDCLException si un problème est survenu.
	///
	virtual EState		HandleIncomingConnection(
								TDCLCommLayer* inCommLayer );

	///
	/// Méthode appelée dans le processus léger du serveur pour indiquer
	/// qu'un lien a été déconnecté et qu'il faut fermer l'interface de
	/// communication.
	///
	/// \param inLink	le lien qui a été déconnecté.
	/// \return le nouvel état du serveur.
	/// \throws TDCLException si un problème est survenu.
	///
	virtual	EState		HandleLinkDisconnection( TDCLLink* inLink );

	///
	/// Méthode appelée par le serveur lorsque rien ne se passe.
	/// Elle permet d'appeler Idle sur les interfaces de communication qui en
	/// ont besoin.
	///
	virtual	void		DoIdle( void );

	///
	/// Accesseur sur le lien.
	///
	/// \return le lien à utiliser lorsqu'une connexion est établie.
	///
	inline TDCLLink*		GetLink( void )
		{
			return mLink;
		}
private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLOneLinkServer( const TDCLOneLinkServer& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLOneLinkServer& operator = ( const TDCLOneLinkServer& inCopy );

	/// \name Variables
	
	TDCLLink*					mLink;			///< Référence sur l'unique
												///< lien.
	TDCLPipe*					mPipe;			///< Référence sur la
												///< connexion, \c nil si on
												///< n'est pas connecté.
	TDCLCommLayer**				mCommLayers;	///< Liste des couches de
												///< communication.
	KUInt32						mNbCommLayers;	///< Nombre d'éléments dans la
												///< liste précédente.
	IDCLThreads::ISemaphore*	mListMutex;		///< Mutex sur la liste
												///< précédente.
};

#endif
		// _TDCLONELINKSERVER_H

// =========================== //
// Memory fault -- brain fried //
// =========================== //
