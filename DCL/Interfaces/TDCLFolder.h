// ==============================
// Fichier:			TDCLFolder.h
// Projet:			Desktop Connection Library
//
// Créé le:			09/02/2003
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
// The Original Code is TDCLFolder.h.
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
// $Id: TDCLFolder.h,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __TDCLFOLDER__
#define __TDCLFOLDER__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFSItem.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

class TDCLFile;

///
/// Classe pour l'abstraction d'un dossier.
/// Un dossier contient des fichiers. On peut parcourir la liste des éléments,
/// ouvrir/créer/supprimer un élément particulier.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFolder
	:
		public TDCLFSItem
{
public:
	///
	/// Récupère une référence sur un élément dans ce dossier.
	/// Retourne nil si l'élément n'existe pas.
	///
	/// \param inName		nom de l'élément.
	/// \param inVolRefNum	référence du volume (utilisé pour le bureau).
	///						Ce paramètre peut être ignoré si le Newton ne nous prend
	///						pas pour un Mac.
	/// \return un objet TDCLFSItem représentant cet élément ou \c nil si aucun
	///			élément de ce nom n'existe. Cet élément est la propriété de l'appelant
	///			qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual	TDCLFSItemRef	GetItemByName( const KUInt16* inName, KSInt32 inVolRefNum = 0 ) = 0;

	///
	/// Crée un fichier d'un nom donné dans ce dossier.
	///
	/// \param inName		nom du fichier à créer.
	/// \return un objet TDCLFile représentant ce nouveau fichier. Ce fichier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le fichier
	///			existe déjà.
	///
	virtual	TDCLFSItemRef	CreateFile( const KUInt16* inName ) = 0;

	///
	/// Crée un dossier d'un nom donné dans ce dossier.
	///
	/// \param inName		nom du dossier à créer.
	/// \return un objet TDCLFolder représentant ce nouveau dossier. Ce dossier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le dossier
	///			existe déjà.
	///
	virtual	TDCLFSItemRef	CreateFolder( const KUInt16* inName ) = 0;

	///
	/// Compte les éléments dans ce dossier.
	/// Notez que ce nombre peut changer juste après l'appel à cette fonction.
	///
	/// \return le nombre d'élément dans le dossier.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt32			CountElements( void ) = 0;

	///
	/// Retourne un tableau avec tous les éléments du dossier.
	/// Notez que ces éléments peuvent disparaître juste après l'appel à cette fonction
	/// et d'autres éléments peuvent apparaître.
	///
	/// \return un tableau de n+1 TDCLFSItemRef. Le dernier élément est une référence
	///			sur \c nil. Le tableau est alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef*	GetItems( void ) = 0;

	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Retourne une structure avec type: 'folder.
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef		ToFrame( void );

protected:
	///
	/// Constructeur à partir d'un dossier parent (optionnel).
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inParentFolder	dossier parent (ou \c nil si celui-ci sera créé plus tard).
	///
	TDCLFolder( IDCLFiles* inFilesIntf, TDCLFSItemRef inParentFolder = TDCLFSItemRef() );
};

#endif
		// __TDCLFOLDER__

// =========================================================================== //
// Algol-60 surely must be regarded as the most important programming language //
// yet developed.                                                              //
//                 -- T. Cheatham                                              //
// =========================================================================== //
