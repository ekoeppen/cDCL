// ==============================
// Fichier:			TDCLDockCmdBackupIDs.h
// Projet:			Desktop Connection Library
// 
// Créé le:			23/9/2003
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
// The Original Code is TDCLDockCmdBackupIDs.h.
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
// $Id: TDCLDockCmdBackupIDs.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLDOCKCMDBACKUPIDS_H
#define _TDCLDOCKCMDBACKUPIDS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour la commande Newton comprenant une liste d'ID pour la
/// sauvegarde.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdBackupIDs
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur unique à partir des ID.
	/// TDCLDockCommand::ReceiveCommand a
	/// Les ID sont déjà dans l'indien local.
	///
	/// \param inNbShorts	nombre d'ID.
	/// \param inShorts		liste des identifiants.
	///
	TDCLDockCmdBackupIDs( KUInt32 inNbShorts, const KUInt16* inShorts );

	///
	/// Destructeur.
	///
	virtual ~TDCLDockCmdBackupIDs( void );

	///
	/// Accesseur sur le nombre de shorts.
private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdBackupIDs( const TDCLDockCmdBackupIDs& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdBackupIDs& operator = ( const TDCLDockCmdBackupIDs& inCopy );

	/// \name Variables

//	KUInt32			mPackageCount;		///< Nombre de paquets.
//	SPackageInfo**	mInfos;				///< Pointeurs sur les informations.
};

#endif
		// _TDCLDOCKCMDBACKUPIDS_H

// ======================================================= //
// As a computer, I find your faith in technology amusing. //
// ======================================================= //
