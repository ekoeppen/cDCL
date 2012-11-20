// ==============================
// Fichier:			TDCLObjCApplication.h
// Projet:			Desktop Connection Library
//
// Créé le:			30/3/2003
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
// The Original Code is TDCLObjCApplication.h.
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
// $Id: TDCLObjCApplication.h,v 1.3 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _TDCLOBJCAPPLICATION_H
#define _TDCLOBJCAPPLICATION_H

#include <DCL/Interfaces/TDCLApplication.h>

#if !TARGET_OS_OPENSTEP
	#error TDCLObjCApplication ne fonctionne que sur OSX/OpenSTEP.
#endif

#include <objc/objc.h>
#include <CoreFoundation/CFString.h>

// Pré-déclarations
class TDCLLink;
class TDCLAppCommand;
class TDCLException;
class TDCLDockCommand;
class TDCLLinkEngine;

//
// L'objet Cocoa gérant les messages doit suivre le protocole IDCLObjCApplication
//
// Prototypes des eveloppes. Ces enveloppes sont définies dans
// TDCLObjCApplication.mm
//
void TDCLObjCAppl_WaitConnection( id inCocoaObject, TDCLServer* inServer, TDCLCommLayer* inLayer );
void TDCLObjCAppl_Connected( id inCocoaObject, TDCLLink* inLink, TDCLCommLayer* inLayer );
void TDCLObjCAppl_ConnectedToNewtonDevice( id inCocoaObject, TDCLLink* inLink, CFStringRef inName );
void TDCLObjCAppl_Disconnecting( id inCocoaObject, TDCLLink* inLink );
void TDCLObjCAppl_Disconnected( id inCocoaObject, TDCLLink* inLink );
void TDCLObjCAppl_ServerIsDown( id inCocoaObject, TDCLServer* inServer );
void TDCLObjCAppl_Cancel( id inCocoaObject, TDCLLink* inLink );
void TDCLObjCAppl_Timeout( id inCocoaObject, TDCLLink* inLink );
void TDCLObjCAppl_AppCommandBeingProcessed( id inCocoaObject, TDCLLink* inLink, TDCLAppCommand* inCommand );
void TDCLObjCAppl_AppCommandProcessed( id inCocoaObject, TDCLLink* inLink, TDCLAppCommand* inCommand, TDCLException* inException );
void TDCLObjCAppl_DockCommandBeingProcessed( id inCocoaObject, TDCLLink* inLink, TDCLDockCommand* inCommand, TDCLLinkEngine* inEngine );
void TDCLObjCAppl_DockCommandProcessed( id inCocoaObject, TDCLLink* inLink, TDCLLinkEngine* inEngine, TDCLException* inException );
void TDCLObjCAppl_HandleServerException( id inCocoaObject, TDCLServer* inServer, TDCLException* inException );
void TDCLObjCAppl_HandleCommLayerException( id inCocoaObject, TDCLCommLayer* inCommLayer, TDCLException* inException );
void TDCLObjCAppl_HandleLinkException( id inCocoaObject, TDCLLink* inLink, TDCLException* inException );
void TDCLObjCAppl_HandleException( id inCocoaObject, TDCLException* inException );
IDCLFiles* TDCLObjCAppl_CreateFilesIntf( id inCocoaObject );
IDCLThreads* TDCLObjCAppl_CreateThreadsIntf( id inCocoaObject );

///
/// Classe pour une enveloppe autour d'un objet ObjC gérant l'interface TDCLApplication.
/// Cette classe appelle les méthodes de l'objet Cocoa passé en paramètre du
/// constructeur (comme id).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLObjCApplication
	:
		public TDCLApplication
{
public:
	///
	/// Constructeur à partir d'un id d'objet Cocoa.
	///
	/// \param inCocoaObject	object cocoa appelé
	///
	TDCLObjCApplication( id inCocoaObject );

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
	/// Méthode appelée lorsqu'une commande commence à être exécutée.
	///
	/// \param inLink		le lien qui a exécute la commande.
	/// \param inCommand	la commande qui est en train d'être exécutée
	///
	virtual void 	AppCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand );

	///
	/// Méthode appelée lorsqu'une commande exécutée par l'application est terminée.
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
	/// \param inLink		le lien qui a exécute la commande.
	/// \param inCommand	la commande qui est en train d'être exécutée
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

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLObjCApplication( const TDCLObjCApplication& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLObjCApplication& operator = ( const TDCLObjCApplication& inCopy );

	///
	/// Retourne l'abstraction pour les fichiers.
	/// Cette méthode est appelée la première fois que
	/// GetFilesIntf est appelée.
	///
	/// \return une abstraction pour utiliser les fichiers.
	///
	virtual IDCLFiles*		CreateFilesIntf( void );
	
	///
	/// Retourne l'abstraction pour les processus légers.
	/// Cette méthode est appelée la première fois que
	/// GetThreadsIntf est appelée.
	///
	/// \return une abstraction pour les processus légers.
	///
	virtual IDCLThreads*	CreateThreadsIntf( void );

	/// \name Variables privées
	id								mCocoaObject;
};

#endif
		// _TDCLOBJCAPPLICATION_H

// ============================================================================== //
// Dear Emily:                                                                    //
//         I recently read an article that said, "reply by mail, I'll summarize." //
// What should I do?                                                              //
//                 -- Doubtful                                                    //
//                                                                                //
// Dear Doubtful:                                                                 //
//         Post your response to the whole net.  That request applies only to     //
// dumb people who don't have something interesting to say.  Your postings are    //
// much more worthwhile than other people's, so it would be a waste to reply by   //
// mail.                                                                          //
//                 -- Emily Postnews Answers Your Questions on Netiquette         //
// ============================================================================== //