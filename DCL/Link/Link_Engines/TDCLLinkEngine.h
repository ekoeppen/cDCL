// ==============================
// Fichier:			TDCLLinkEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/4/2003
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
// The Original Code is TDCLLinkEngine.h.
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
// $Id: TDCLLinkEngine.h,v 1.3 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLLINKENGINE_H
#define _TDCLLINKENGINE_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Link/TDCLLink.h>

// Pré-déclarations.
class TDCLDockCommand;
class TDCLFullDockLink;

///
/// Classe de base pour les moteurs du lien.
/// Ces moteurs prennent le relai du FullDockLink lorsqu'une commande est en route.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLLinkEngine
{
public:
	///
	/// Constructeur à partir d'une référence sur le lien.
	///
	/// \param inLink	référence sur le lien.
	///
	TDCLLinkEngine( TDCLFullDockLink* inLink );

	///
	/// Destructeur.
	///
	virtual ~TDCLLinkEngine( void );

	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	/// Par défaut retourne \c true et met outProcessed à \c false.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return \c true si le moteur continue, \c false sinon
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean		ProcessDockCommand(
									TDCLDockCommand* inCommand,
									Boolean* outProcessed );

	///
	/// Méthode appelée pour gérer des commandes venant de l'application.
	/// Par défaut retourne \c true et met outProcessed à \c false.
	///
	/// \param inCommand	commande à exécuter.
	/// \param outProcessed	si la commande a été traitée.
	/// \return \c true si le moteur continue, \c false sinon
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean		ProcessAppCommand(
									TDCLAppCommand* inCommand,
									Boolean* outProcessed );

	///
	/// Méthode appelée par le lien pour signifier que le travail du
	/// moteur est terminé.
	/// Par défaut ne fait rien.
	///
	virtual void				WorkIsOver( void );

	///
	/// Détermine les commandes de l'application que le lien
	/// peut traiter. Cette méthode est appelée à chaque fois que le
	/// lien va attendre un événement. Par défaut, on accepte toutes
	/// les commandes d'application sauf les commandes envoyées lorsqu'on
	/// est oisif.
	///
	/// \return le masque pour les commandes de l'application
	///
	virtual KUInt32				GetAppCommandMask( void );

protected:
	///
	/// Accesseur sur le lien.
	///
	inline TDCLFullDockLink*	GetLink( void ) const
		{
			return mLink;
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLinkEngine( const TDCLLinkEngine& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLinkEngine& operator = ( const TDCLLinkEngine& inCopy );

	/// \name Variables
	TDCLFullDockLink*			mLink;			///< Référence sur le lien.
};

#endif
		// _TDCLLINKENGINE_H

// ============================================================================ //
// I think there's a world market for about five computers.                     //
//                 -- attr. Thomas J. Watson (Chairman of the Board, IBM), 1943 //
// ============================================================================ //
