// ==============================
// Fichier:			TDCLLogApplication.h
// Projet:			Desktop Connection Library
//
// Créé le:			31/3/2003
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
// The Original Code is TDCLLogApplication.h.
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
// $Id: TDCLLogApplication.h,v 1.3 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef _TDCLLOGAPPLICATION_H
#define _TDCLLOGAPPLICATION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLApplication.h>

// ANSI C
#include <stdio.h>

///
/// Classe pour une application qui écrit tous les messages dans un fichier.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLLogApplication
	:
		public TDCLApplication
{
public:
	///
	/// Constructeur à partir d'une référence sur le fichier à utiliser.
	///
	/// \param inLogFile	fichier où écrire tout ce qui se passe.
	///
	TDCLLogApplication( FILE* inLogFile );

	///
	/// Méthode appelée lorsque le serveur commence à écouter sur une interface donnée.
	/// Cette méthode est appelé par le serveur.
	/// Elle permet d'afficher un dialogue.
	///
	/// \param inServer	serveur qui attend la connexion.
	/// \param inLayer	couche de communication qui attend la connexion.
	///
	virtual	void	WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer );

	///
	/// Méthode appelée par le serveur pour indiquer que la connexion a été établie.
	/// Le paramètre est l'interface de communication avec le Newton.
	///
	/// \param inLink	le lien courant
	/// \param inLayer	couche de communication connectée.
	///
	virtual	void	Connected( TDCLLink* inLink, TDCLCommLayer* inLayer );

	///
	/// Méthode appelée par le serveur une fois que le nom est connu.
	/// Cette méthode permet de copier le nom et d'afficher le nom pour l'utilisateur
	/// afin de confirmer la bonne connexion avec le Newton.
	///
	/// \param inLink	le lien courant
	/// \param inName	nom de l'utilisateur (officiellement, nom du Newton). Ce pointeur
	///					est la propriété du lien.
	///	
	virtual	void	ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName );

	///
	/// Méthode appelée lorsque le lien est déconnecté
	/// (ceci peut prendre du temps).
	///
	/// \param inLink	le lien courant
	///
	virtual void	Disconnecting( TDCLLink* inLink );
	
	///
	/// Méthode appelée lorsque le lien a été déconnecté (juste avant
	/// que le nom ne soit libéré).
	///
	/// \param inLink	le lien courant
	///
	virtual void	Disconnected( TDCLLink* inLink );

	///
	/// Méthode appelée lorsque le serveur quitte.
	/// Cette méthode est appelée par le processus léger du serveur juste avant
	/// qu'il ne se termine (remarque: si une exception est arrivée, cette
	/// méthode ne sera pas appelée).
	///
	/// \param inServer	serveur qui quitte
	///
	virtual	void	ServerIsDown( TDCLServer* inServer );

	///
	/// Méthode appelée lorsque l'utilisateur a annulé.
	///
	/// \param inLink	le lien courant
	///
	virtual void	Cancel( TDCLLink* inLink );
	
	///
	/// Méthode appelée lorsqu'un délai de temporisation
	/// est dépassé.
	///
	/// \param inLink	le lien courant
	///
	virtual	void	Timeout( TDCLLink* inLink );

	///
	/// Méthode appelée lorsqu'une commande de l'application commence à être exécutée.
	///
	/// \param inLink		le lien qui exécute la commande.
	/// \param inCommand	la commande qui est en train d'être exécutée
	///
	virtual void 	AppCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand );

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
							TDCLException* inException );

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
							TDCLLinkEngine* inEngine );

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
							TDCLException* inException );

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
							TDCLException* inException );

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
							TDCLException* inException );

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
							TDCLException* inException );

	///
	/// Accesseur sur le fichier de log.
	/// Permet d'envoyer d'autres informations sur le fichier.
	///
	inline FILE*	GetLogFile( void )
		{
			return mLogFile;
		}

protected:
	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger. Cette méthode est appelée dans ce processus léger
	/// qui se termine lorsque la méthode retourne.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleException( TDCLException* inException );

protected:
	/// \name Protected variables
	FILE*			mLogFile;		///< Fichier journal.

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLogApplication( const TDCLLogApplication& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLogApplication& operator = ( const TDCLLogApplication& inCopy );

	///
	/// Affiche une exception sur le fichier journal.
	///
	/// \param inException	l'exception à afficher (peut être \c nil si
	///						ce n'est pas une TDCLException)
	///
	virtual void	LogException( TDCLException* inException );

};

#endif
		// _TDCLLOGAPPLICATION_H

// ========================================================================== //
// You can now buy more gates with less specifications than at any other time //
// in history.                                                                //
//                 -- Kenneth Parker                                          //
// ========================================================================== //
