//==============================
// Fichier:			TTestServer.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			17/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: TTestServer.h,v 1.5 2003/09/17 10:43:49 paul Exp $
// ===========

#ifndef _TTESTSERVER_H
#define _TTESTSERVER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

// ANSI C
#include <stdio.h>

class TDCLLogApplication;

///
/// Classe pour un serveur qui raconte sa vie sur radio stdout.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
class TTestServer
	:
		public TDCLServer
{
public:
	///
	/// Constructeur à partir d'une référence sur l'application.
	///
	/// \param inApplication	interface avec le système hôte.
	///
	TTestServer( TDCLLogApplication* inApplication );

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
	/// \return le nouvel état du serveur.
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

private:
	FILE*				mLogFile; ///< Reference sur le flux.
};

#endif
		// _TTESTSERVER_H

// =============================================================================== //
// "This is lemma 1.1.  We start a new chapter so the numbers all go back to one." //
//                 -- Prof. Seager, C&O 351                                        //
// =============================================================================== //
