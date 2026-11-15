// ==============================
// Fichier:			TDCLLoadPackageLink.h
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
// The Original Code is TDCLLoadPackageLink.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLLoadPackageLink.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef __TDCLLOADPACKAGELINK__
#define __TDCLLOADPACKAGELINK__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLink.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

// Pré-déclarations.
class TDCLApplication;

///
/// Classe pour un lien qui gère le sous-protocole d'installation de paquets.
/// 
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
//
class TDCLLoadPackageLink
	:
		public TDCLLink
{
public:
	///
	/// Constructeur à partir d'une application.
	///
	/// \param inApplication	application qui gère ce line.
	///
	TDCLLoadPackageLink( TDCLApplication* inApplication );

	///
	/// Méthode synchrone pour connecter le Newton.
	/// Appelle d'abord TDCLLink::DoConnect pour le début.
	/// Ensuite, on lance l'installation de paquets:
	///
	/// kDSetTimeout ->
	///				<- kDResult
	///
	/// Normalement, vous n'avez pas besoin de surcharger cette méthode.
	///
	virtual void		DoConnect ( void );

	///
	/// Installe un paquet. (Cette méthode est en fait asynchrone et poste un événement
	/// pour le lien).
	///
	/// \param inPackage	paquet à installer.
	/// \return \c true si le lien était connecté, \c false sinon (auquel cas
	///			le paquet ne sera pas installé).
	///
	Boolean InstallPackage( TDCLFSItemRef inPackage );

protected:
	///
	/// Méthode appelée pour indiquer qu'on commence à installer le paquet.
	/// Cette méthode doit initialiser la progression d'installation ou un truc
	/// approchant. Par défaut ne fait rien.
	///
	virtual void		StartInstallingPackage( void );

	///
	/// Méthode appelée pour indiquer la progression de l'installation.
	/// Par défaut ne fait rien.
	///
	/// \param inProgress	progression de l'installation (valeur entre 0 et 1)
	///
	virtual void		InstallationProgress( double inProgress );

private:
	///
	/// Installe véritablement un paquet.
	///
	/// \param inPackage	paquet à installer.
	/// \throws TDCLException si une erreur est survenue.
	///
	void DoInstallPackage( TDCLFSItemRef inPackage );

	///
	/// Méthode appelée lorsqu'on a reçu une commande de l'application.
	/// La commande est supprimée une fois que cette méthode retourne.
	/// Cette classe ne traite que la commande d'installation de paquet.
	/// Remarque: cette commande n'est pas traitée comme le fait TDCLFullDockLink.
	/// Ici, nous n'indiquons pas au Newton que nous allons installer un paquet.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée. Si la valeur est
	///						\c false, une exception est levée dans la boucle.
	/// \return le prochain état du lien (l'état actuel étant kRunning)
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	EState	ProcessAppCommand(
								TDCLAppCommand* inCommand,
								Boolean* outProcessed );

	///
	/// Méthode appelée par \c SendCommand pour savoir la progression de
	/// l'envoi du paquet.
	///
	/// \param inRefCon		biscuit (en fait \c this)
	/// \param inProgress	progression entre 0 et 1
	///
	static void					ProgressFunc(
										void* inRefCon,
										double inProgress );
};

#endif
		// __TDCLLOADPACKAGELINK__

// ====================================================================== //
// Systems have sub-systems and sub-systems have sub-systems and so on ad //
// infinitum -- which is why we're always starting over.                  //
//                 -- Epigrams in Programming, ACM SIGPLAN Sept. 1982     //
// ====================================================================== //
