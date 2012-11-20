// ==============================
// Fichier:			TDCLSimpleServer.h
// Projet:			Desktop Connection Library
//
// Créé le:			15/1/2002
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
// The Original Code is TDCLSimpleServer.h.
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
// $Id: TDCLSimpleServer.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLSIMPLESERVER_H
#define _TDCLSIMPLESERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>

class TDCLLink;

///
/// Classe pour un serveur simple qui n'écoute que sur une seule
/// interface de communication et qui ne sert qu'une seule connexion
/// à la fois.
/// C'est un serveur de type NPI/NTK.
///
/// Il faut d'abord créer le lien (parce que le lien suppose que le
/// serveur pré-existe).
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLSimpleServer
	:
		public TDCLServer
{
public:
	///
	/// Créateur à partir d'une application et d'une couche de communication.
	///
	/// \param inApplication	application prévenue de l'activité du serveur.
	/// \param inCommLayer		couche de communication utilisée pour le
	///							serveur.
	///
	TDCLSimpleServer(
			TDCLApplication* inApplication,
			TDCLCommLayer* inCommLayer
		);

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
	virtual	~TDCLSimpleServer( void );
	
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
	///						(ignoré)
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
	TDCLSimpleServer( const TDCLSimpleServer& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLSimpleServer& operator = ( const TDCLSimpleServer& inCopy );

	/// \name Variables

	TDCLLink*			mLink;			///< Référence sur l'unique lien.
	TDCLCommLayer*		mCommLayer;		///< Référence sur l'interface de
										///< communication.
	TDCLPipe*			mPipe;			///< Référence sur la connexion, \c nil
										///< si on n'est pas connecté.
};

#endif
		// _TDCLSIMPLESERVER_H

// ======================================================== //
// Blinding speed can compensate for a lot of deficiencies. //
//                 -- David Nichols                         //
// ======================================================== //
