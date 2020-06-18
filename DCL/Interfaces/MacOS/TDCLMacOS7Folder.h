// ==============================
// Fichier:			TDCLMacOS7Folder.h
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
// The Original Code is TDCLMacOS7Folder.h.
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
// $Id: TDCLMacOS7Folder.h,v 1.3 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef _TDCLMACOS7FOLDER_H
#define _TDCLMACOS7FOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacFolder.h>

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>

///
/// Classe pour les dossiers sous MacOS.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLMacOS7Folder
	:
		public TDCLMacFolder
{
public:
	///
	/// Constructeur à partir d'un volume et d'un ID de dossier.
	///
	/// \param	inFilesIntf	interface pour les fichiers
	/// \param	inVRefNum	référence sur le volume.
	/// \param	inDirID		ID du dossier.
	///
	TDCLMacOS7Folder(
					TDCLMacFiles* inFilesIntf,
					short inVRefNum,
					long inDirID );

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
	/// Crée un fichier d'un nom, d'un code créateur et d'un type donnés dans
	/// ce dossier.
	///
	/// \param inName		nom du fichier à créer.
	/// \param inCreator	code créateur du fichier à créer.
	/// \param inFileType	type du fichier à créer.
	/// \return un objet TDCLFile représentant ce nouveau fichier. Ce fichier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le fichier
	///			existe déjà.
	///
	virtual	TDCLFSItemRef	CreateFile(
								const KUInt16* inName,
								OSType inCreator = TDCLMacFiles::kCreator,
								OSType inFileType = TDCLMacFiles::kFileType );

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
	///			dernier pointeur est \c nil. Le tableau est alloué avec new.
	///			Ces éléments sont la propriété de l'appelant qui doit les supprimer.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef*	GetItems( void );

private:
	///
	/// Constructeur par défaut volontairement indisponible.
	///
	TDCLMacOS7Folder( void );

	///
	/// Constructeur à partir d'un dossier parent et d'un ID de dossier.
	///
	/// \param	inFilesIntf	interface pour les fichiers
	/// \param	inParentFolder	dossier parent.
	/// \param	inDirID			ID du dossier.
	///
	TDCLMacOS7Folder(
				TDCLMacFiles* inFilesIntf,
				TDCLFSItemRef inParentFolder,
				long inDirID );

	///
	/// Retourne l'ID du dossier parent.
	///
	/// \return l'ID du dossier parent.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual long			GetParentID( void ) const;

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est appelée
	/// que si nécessaire.
	///
	/// \return un objet représentant le dossier parent ou \c nil.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;

	///
	/// Construit un FSSpec pour un élément dans ce dossier à partir
	/// de son nom.
	///
	/// \param inName		le nom de l'élément en UTF-16.
	/// \param outFSSPec	en sortie, le FSSpec.
	/// \return \c true si l'élément existe, \c false si le fichier
	///			n'existe pas.
	/// \throw TDCLException si un problème est survenu.
	///
	Boolean					MakeFSSpecForItem(
									const KUInt16* inName,
									FSSpec* outFSSPec ) const;

	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*		MakeName( void ) const;

	///
	/// Détermine si le dossier est un disque.
	///
	/// \return \c true si le dossier est le dossier racine d'un disque.
	///
	virtual Boolean			IsVolume( void ) const;

	///
	/// Détermine si le dossier est sur le bureau.
	///
	/// \return \c true si le dossier est sur le bureau, \c false sinon.
	///
	virtual Boolean			IsOnDesktop( void ) const;

	///
	/// Récupère la référence sur le volume où se trouve le dossier.
	///
	/// \return le volume où se trouve le dossier.
	///
	virtual short			GetVRefNum( void ) const;

	/// \name Variables

	short		mVRefNum;	///< Identifiant du volume où se trouve le dossier
	long		mDirID;		///< Identifiant du dossier sur le volume
};

#endif
		// _TDCLMACOS7FOLDER_H

// ========================================================================== //
// On two occasions I have been asked [by members of Parliament!], "Pray, Mr. //
// Babbage, if you put into the machine wrong figures, will the right answers //
// come out?"  I am not able rightly to apprehend the kind of confusion of    //
// ideas that could provoke such a question.                                  //
//                 -- Charles Babbage                                         //
// ========================================================================== //