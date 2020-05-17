// ==============================
// Fichier:			TDCLMacCarbonDesktopFolder.h
// Projet:			Desktop Connection Library
//
// Créé le:			13/1/2003
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
// The Original Code is TDCLMacCarbonDesktopFolder.h.
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
// $Id: TDCLMacCarbonDesktopFolder.h,v 1.4 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _TDCLMACCARBONDESKTOPFOLDER_H
#define _TDCLMACCARBONDESKTOPFOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h>

// Pré-déclarations
class TDCLMacFiles;

///
/// Classe pour le bureau sous MacOS X.
/// C'est un faux dossier.
/// Contient tous les éléments sur le bureau de l'utilisateur plus les volumes.
/// Numérotation: d'abord les éléments sur le bureau puis les volumes.
///
/// \author Nicolas Zinovieff <krugazor@free.fr>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLMacCarbonDesktopFolder
	:
		public TDCLMacDesktopFolder
{
public:
	///
	/// Récupère une référence sur un élément dans ce dossier.
	/// Retourne nil si l'élément n'existe pas.
	///
	/// \param inName		nom de l'élément.
	/// \param inVolRefNum	référence du volume. Ce paramètre est utilisé en concordance
	///						avec la méthode ToFrame.
	/// \return un objet TDCLFSItem représentant cet élément ou \c nil si aucun
	///			élément de ce nom n'existe. Cet élément est la propriété de l'appelant
	///			qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual	TDCLFSItemRef	GetItemByName(
									const KUInt16* inName,
									KSInt32 inVolRefNum = 0 );

	///
	/// Crée un fichier d'un nom donné dans ce dossier.
	///
	/// \param inName		nom du fichier à créer.
	/// \return un objet TDCLFile représentant ce nouveau fichier. Ce fichier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le fichier
	///			existe déjà.
	///
	virtual	TDCLFSItemRef	CreateFile( const KUInt16* inName );

	///
	/// Crée un dossier d'un nom donné dans ce dossier.
	///
	/// \param inName		nom du dossier à créer.
	/// \return un objet TDCLFolder représentant ce nouveau dossier. Ce dossier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le dossier
	///			existe déjà.
	///
	virtual	TDCLFSItemRef	CreateFolder( const KUInt16* inName );

	///
	/// Compte les éléments dans ce dossier.
	/// Notez que ce nombre peut changer juste après l'appel à cette fonction.
	///
	/// \return le nombre d'élément dans le dossier.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt32			CountElements( void );

	///
	/// Retourne un tableau avec tous les éléments du dossier.
	/// Notez que ces éléments peuvent disparaître juste après l'appel à cette fonction
	/// et d'autres éléments peuvent apparaître.
	///
	/// \return un tableau de n+1 pointeurs sur des TDCLFSItem. Le
	///			dernier pointeur est \c nil. Le tableau est alloué avec malloc.
	///			Ces éléments sont la propriété de l'appelant qui doit les supprimer.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef*	GetItems( void );

private:
	/// Seule la classe TDCLMacFiles peut construire des objets de notre classe.
	friend class TDCLMacFiles;

	///
	/// Constructeur à partir de l'interface sur les fichiers.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	///
	TDCLMacCarbonDesktopFolder( TDCLMacFiles* inFilesIntf );
	
	/// \name Variables
	FSRef					mDesktopFolderRef; ///< Référence sur le bureau.
};

#endif
		// _TDCLMACCARBONDESKTOPFOLDER_H

// ===================================================================== //
// If there is a possibility of several things going wrong, the one that //
// will cause the most damage will be the one to go wrong.               //
// ===================================================================== //