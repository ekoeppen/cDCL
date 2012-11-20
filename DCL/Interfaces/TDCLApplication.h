// ==============================
// Fichier:			TDCLApplication.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLApplication.h.
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
// $Id: TDCLApplication.h,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __TDCLAPPLICATION__
#define __TDCLAPPLICATION__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Headers/DCLErrorMessages.h>

// Pré-déclarations
class IDCLFiles;
class IDCLThreads;
class TDCLAppCommand;
class TDCLCommLayer;
class TDCLDockCommand;
class TDCLException;
class TDCLLink;
class TDCLLinkEngine;
class TDCLServer;

///
/// Classe pour gérer à la fois les abstractions spécifiques à une
/// plateforme et l'intéraction des éléments de la DCL avec l'application
/// hôte.
///
/// Cette classe doit être dérivée pour chaque application. Elle
/// fournit des objets qui permettent d'effectuer les tâches de
/// bases. Ces objets sont détruits par le constructeur de TDCLApplication.
///
/// <b>Attention</b>: les méthodes appelées pour prévenir d'événements
/// de la DCL sont appelées dans des processus légers différents. Il faut
/// sans doute un mécanisme de notification pour récupérer ces événements
/// dans le processus léger principal de l'application. Cf l'exemple
/// TDCLObjCApplication.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLApplication
{
public:
	/// \name constructeur et destructeur

	///
	/// Constructeur par défaut.
	///
	TDCLApplication( void );

	///
	/// Destructeur.
	///
	virtual ~TDCLApplication( void );

	/// \name abstractions
	
	///
	/// Retourne l'abstraction pour les fichiers.
	///
	/// \return une abstraction pour utiliser les fichiers.
	///
	IDCLFiles*		GetFilesIntf( void );
	
	///
	/// Retourne l'abstraction pour les processus légers.
	///
	/// \return une abstraction pour les processus légers.
	///
	IDCLThreads*	GetThreadsIntf( void );

	/// \name Interface TDCLApplication

	///
	/// Méthode appelée lorsque le serveur commence à écouter sur une interface donnée.
	/// Cette méthode est appelé par le serveur.
	/// Elle permet, par exemple, d'afficher un dialogue.
	///
	/// \param inServer	serveur qui commence à écouter
	/// \param inLayer	couche de communication qui attend la connexion.
	///
	virtual	void	WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer ) = 0;

	///
	/// Méthode appelée par le serveur pour indiquer que la connexion a été établie.
	/// Le paramètre est l'interface de communication avec le Newton.
	///
	/// \param inLink	le lien courant
	/// \param inLayer	couche de communication connectée.
	///
	virtual	void	Connected( TDCLLink* inLink, TDCLCommLayer* inLayer ) = 0;

	///
	/// Méthode appelée par le serveur une fois que le nom est connu.
	/// Cette méthode permet de copier le nom et d'afficher le nom pour l'utilisateur
	/// afin de confirmer la bonne connexion avec le Newton.
	///
	/// \param inLink	le lien courant
	/// \param inName	nom de l'utilisateur (officiellement, nom du Newton). Ce pointeur
	///					est la propriété du lien.
	///	
	virtual	void	ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName ) = 0;

	///
	/// Méthode appelée lorsque le lien est déconnecté
	/// (ceci peut prendre du temps).
	///
	/// \param inLink	le lien courant
	///
	virtual void	Disconnecting( TDCLLink* inLink ) = 0;
	
	///
	/// Méthode appelée lorsque le lien a été déconnecté (juste avant
	/// que le nom ne soit libéré).
	///
	/// \param inLink	le lien courant
	///
	virtual void	Disconnected( TDCLLink* inLink ) = 0;

	///
	/// Méthode appelée lorsque le serveur quitte.
	/// Cette méthode est appelée par le processus léger du serveur juste avant
	/// qu'il ne se termine (remarque: si une exception est arrivée, cette
	/// méthode ne sera pas appelée).
	///
	/// \param inServer	serveur qui quitte
	///
	virtual	void	ServerIsDown( TDCLServer* inServer ) = 0;

	///
	/// Méthode appelée lorsque l'utilisateur a annulé.
	///
	/// \param inLink	le lien courant
	///
	virtual void	Cancel( TDCLLink* inLink ) = 0;
	
	///
	/// Méthode appelée lorsqu'un délai de temporisation
	/// est dépassé.
	///
	/// \param inLink	le lien courant
	///
	virtual	void	Timeout( TDCLLink* inLink ) = 0;

	///
	/// Méthode appelée lorsqu'une commande de l'application commence à être exécutée.
	///
	/// \param inLink		le lien qui exécute la commande.
	/// \param inCommand	la commande qui est en train d'être exécutée
	///
	virtual void 	AppCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand ) = 0;

	///
	/// Méthode appelée lorsqu'une commande de l'application exécutée par le lien
	/// est terminée.
	///
	/// \param inLink		le lien qui a exécuté la commande.
	/// \param inCommand	la commande qui a été exécutée
	/// \param inException	l'exception si la commande a échoué (nil si elle a réussi).
	///
	virtual void 	AppCommandProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand,
							TDCLException* inException ) = 0;

	///
	/// Méthode appelée lorsqu'une commande du Newton commence à être exécutée.
	///
	/// \param inLink		le lien qui exécute la commande.
	/// \param inCommand	la commande qui commence à être exécutée
	/// \param inEngine		le moteur qui exécute la commande.
	///
	virtual void 	DockCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLDockCommand* inCommand,
							TDCLLinkEngine* inEngine ) = 0;

	///
	/// Méthode appelée lorsqu'une commande du Newton exécutée par le lien
	/// est terminée.
	///
	/// \param inLink		le lien qui a exécuté la commande.
	/// \param inEngine		le moteur qui a exécuté la commande.
	/// \param inException	l'exception si la commande a échoué (nil si elle a réussi).
	///
	virtual void 	DockCommandProcessed(
							TDCLLink* inLink,
							TDCLLinkEngine* inEngine,
							TDCLException* inException ) = 0;

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'un/du serveur.
	///
	/// \param inServer		serveur qui s'est vautré.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleServerException(
							TDCLServer* inServer,
							TDCLException* inException ) = 0;

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'une/de la couche de communication.
	///
	/// \param inCommLayer	couche qui s'est vautrée.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException ) = 0;

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'un/du lien.
	///
	/// \param inLink		lien qui s'est vautré.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleLinkException(
							TDCLLink* inLink,
							TDCLException* inException ) = 0;

protected:
	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger. Cette méthode est appelée dans ce processus léger
	/// qui se termine lorsque la méthode retourne.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleException( TDCLException* inException ) = 0;

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLApplication( const TDCLApplication& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLApplication& operator = ( const TDCLApplication& inCopy );

	///
	/// Retourne l'abstraction pour les fichiers.
	/// Cette méthode est appelée la première fois que
	/// GetFilesIntf est appelée.
	///
	/// \return une abstraction pour utiliser les fichiers.
	///
	virtual IDCLFiles*		CreateFilesIntf( void ) = 0;
	
	///
	/// Retourne l'abstraction pour les processus légers.
	/// Cette méthode est appelée la première fois que
	/// GetThreadsIntf est appelée.
	///
	/// \return une abstraction pour les processus légers.
	///
	virtual IDCLThreads*	CreateThreadsIntf( void ) = 0;

	/// \name Variables privées
	
	IDCLFiles*		mFilesIntf;		///< Abstraction pour les fichiers.
	IDCLThreads*	mThreadsIntf;	///< Abstraction pour les processus légers.
};

#endif
		// __TDCLAPPLICATION__

// ========================================================================== //
// You can now buy more gates with less specifications than at any other time //
// in history.                                                                //
//                 -- Kenneth Parker                                          //
// ========================================================================== //
