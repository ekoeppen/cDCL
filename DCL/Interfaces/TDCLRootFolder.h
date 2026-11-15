// ==============================
// Fichier:			TDCLRootFolder.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/1/2003
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
// The Original Code is TDCLRootFolder.h.
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
// $Id: TDCLRootFolder.h,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef _TDCLROOTFOLDER_H
#define _TDCLROOTFOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFolder.h>

///
/// Classe de base pour la racine d'exploration.
/// Cette classe doit être dérivée pour chaque TDCLFiles.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLRootFolder
	:
		virtual public TDCLFolder
{
public:
	///
	/// Récupère le chemin de l'élément.
	/// Cette méthode retourne un tableau avec un seul élément.
	///
	/// \return le chemin de l'objet pour le butinage.
	///
	virtual TDCLNSRef	GetPath( void );

	///
	/// Récupère le dossier parent de l'élément.
	/// Retourne \c nil.
	///
	/// \return une référence sur \c nil.
	///
	virtual TDCLFSItemRef	GetParentFolder( void );

protected:
	///
	/// Constructeur à partir de l'interface sur les fichiers.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	///
	TDCLRootFolder( IDCLFiles* inFilesIntf );

private:
	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est
	/// jamais appelée.
	///
	/// \return une référence sur \c nil.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;
};

#endif
		// _TDCLROOTFOLDER_H

// ===================================== //
// Don't hit the keys so hard, it hurts. //
// ===================================== //
