// ==============================
// Fichier:			TDCLDockCmdPassword.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/08/2001
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
// The Original Code is TDCLDockCmdPassword.h.
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
// $Id: TDCLDockCmdPassword.h,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCMDPASSWORD__
#define __TDCLDOCKCMDPASSWORD__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour la gestion du mot de passe.
/// Permet de créer ou de vérifier un mot de passe.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdPassword
	:
		public TDCLDockCommand
{
public:

	///
	/// Constructeur à partir des données qui viennent du Newton.
	/// Ces données sont converties avec le boutien local.
	///
	/// \param inLength	taille des données
	/// \param inData	données
	///
	TDCLDockCmdPassword( KUInt32 inLength, const void* inData );

	///
	/// Constructeur pour une commande Password à envoyer.
	///
	/// \param inChallenge	défi
	/// \param inPassword	mot de passe (\c nil signifie le mot de passe vide)
	///
	TDCLDockCmdPassword(
						const KUInt32 inChallenge[2],
						const KUInt16* inPassword = nil );

	///
	/// Crée un défi à l'aide de deux nombres aléatoires.
	///
	/// \param outChallenge	en sortie, deux nombres aléatoires pour le défi.
	///
	static	void	CreateChallenge( KUInt32 outChallenge[2] );

	///
	/// Vérifie le mot de passe à partir du défi envoyé au Newton.
	///
	/// \param inChallenge	défi que nous avons envoyé au Newton.
	/// \param inPassword	mot de passe à vérifier (\c nil signifie le mot de
	///						passe vide)
	/// \return \c true si les données renvoyées par le Newton correspond au
	///			défi et au mot de passe.
	///
	Boolean	VerifyPassword(
						const KUInt32 inChallenge[2],
						const KUInt16* inPassword = nil );

protected:
	///
	/// Méthode appelée pour récupèrer la taille de la commande avant de
	/// l'envoyer.
	///
	/// \return la taille de la commande
	///
	virtual	KUInt32			GetLength( void ) const;

	///
	/// Méthode appelée pour récupèrer les données à envoyer.
	///
	/// \return un pointeur sur les données à envoyer.
	///
	virtual const void*		GetData( void );

private:
	/// \name Variables
	
	KUInt32	mCipheredChallenge[2];	///< Défi crypté par le mot de passe
									///< (en grand-boutien)
};

#endif
		// __TDCLDOCKCMDPASSWORD__

// ================================= //
// Put no trust in cryptic comments. //
// ================================= //
