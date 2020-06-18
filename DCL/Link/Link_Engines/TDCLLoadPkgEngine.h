// ==============================
// Fichier:			TDCLLoadPkgEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/5/2003
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
// The Original Code is TDCLLoadPkgEngine.h.
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
// $Id: TDCLLoadPkgEngine.h,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLLOADPKGENGINE_H
#define _TDCLLOADPKGENGINE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLLinkEngine.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

///
/// Classe pour le moteur d'installation de paquet.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLLoadPkgEngine
	:
		public TDCLLinkEngine
{
public:
	///
	/// Constructeur à partir d'un lien.
	///
	/// \param inLink	référence sur le lien.
	///
	TDCLLoadPkgEngine( TDCLFullDockLink* inLink );

	///
	/// Méthode appelée par le lien pour installer un paquet.
	///
	/// \param inPackage	paquet à installer.
	/// \throws TDCLException si une erreur est survenue.
	///
	void			InstallPackage(
								TDCLFSItemRef inPackage );

	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return \c true si le moteur continue, \c false sinon
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean	ProcessDockCommand(
									TDCLDockCommand* inCommand,
									Boolean* outProcessed );

protected:
	///
	/// Méthode appelée pour indiquer qu'on commence à installer le paquet.
	/// Cette méthode doit initialiser la progression d'installation ou un truc
	/// approchant. Par défaut ne fait rien.
	///
	/// \param inPackageName	nom du paquet installé.
	///
	virtual void		StartInstallingPackage( const KUInt16* inPackageName );

	///
	/// Méthode appelée pour indiquer la progression de l'installation.
	/// Par défaut ne fait rien.
	///
	/// \param inProgress	progression de l'installation (valeur entre 0 et 1)
	///
	virtual void		InstallationProgress( double inProgress );

private:
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
		// _TDCLLOADPKGENGINE_H

// ============================================================================ //
// To say that UNIX is doomed is pretty rabid, OS/2 will certainly play a role, //
// but you don't build a hundred million instructions per second multiprocessor //
// micro and then try to run it on OS/2.  I mean, get serious.                  //
//                 -- William Zachmann, International Data Corp                 //
// ============================================================================ //
