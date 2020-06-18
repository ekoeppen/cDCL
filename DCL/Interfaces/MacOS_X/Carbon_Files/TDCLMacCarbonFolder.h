// ==============================
// Fichier:			TDCLMacCarbonFolder.h
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
// The Original Code is TDCLMacCarbonFolder.h.
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
// $Id: TDCLMacCarbonFolder.h,v 1.5 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLMACCARBONFOLDER_H
#define _TDCLMACCARBONFOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacFolder.h>

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>

// MacOS
#if TARGET_OS_MACOS
#include <Folders.h>
#else
#include <CoreServices/CoreServices.h>
#endif

///
/// Classe pour .
///
/// \author Nicolas Zinovieff <krugazor@free.fr>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLMacCarbonFolder
	:
		public TDCLMacFolder
{
public:
	///
	/// Constructeur à partir d'un FSRef.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inRef			référence sur le fichier.
	///
	TDCLMacCarbonFolder( TDCLMacFiles* inFilesIntf, const FSRef* inRef );

	///
	/// Récupère une référence sur un élément dans ce dossier.
	/// Retourne nil si l'élément n'existe pas.
	///
	/// \param inName		nom de l'élément.
	/// \param inVolRefNum	référence du volume (utilisé pour le bureau).
	///						Ce paramètre est ignoré.
	/// \return un objet TDCLFSItem représentant cet élément ou \c nil si aucun
	///			élément de ce nom n'existe. Cet élément est la propriété de l'appelant
	///			qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual	TDCLFSItemRef		GetItemByName(
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
	virtual	TDCLFSItemRef		CreateFile(
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
	virtual	TDCLFSItemRef		CreateFolder( const KUInt16* inName );

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
	/// \return un tableau de n+1 TDCLFSItemRef. Le dernier élément est une référence
	///			sur \c nil. Le tableau est alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef*	GetItems( void );

	///
	/// Retourne un tableau avec tous les éléments d'un dossier.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inRef		référence sur le dossier dont on veut les éléments.
	/// \param outCount		en sortie, si ce n'est pas \c nil, le nombre
	///						d'éléments (n)
	/// \return un tableau de n+1 TDCLFSItemRef. Le dernier élément est une référence
	///			sur \c nil. Le tableau est alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	static TDCLFSItemRef*		GetFolderItems(
								TDCLMacFiles* inFilesIntf,
								const FSRef* inRef,
								KUInt32* outCount = nil );

private:
	/// On autorise TDCLMacCarbonFile d'appeler GetRef.
	friend class TDCLMacCarbonFile;

	///
	/// La classe TDCLMacCarbonDesktopFolder crée des objets de type TDCLMacCarbonFile.
	///
	friend class TDCLMacCarbonDesktopFolder;

	///
	/// Constructeur à partir d'un dossier parent et d'un FSRef.
	/// La référence doit correspondre au dossier.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inParentFolder	dossier parent.
	/// \param inRef			référence sur le fichier.
	///
	TDCLMacCarbonFolder(
						TDCLMacFiles* inFilesIntf,
						TDCLFSItemRef inParentFolder,
						const FSRef* inRef );

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacCarbonFolder( const TDCLMacCarbonFolder& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacCarbonFolder& operator = ( const TDCLMacCarbonFolder& inCopy );

	///
	/// Récupère le FSRef sur le dossier.
	///
	/// \return mRef
	///
	inline const FSRef* GetRef( void ) const
		{
			return &mRef;
		};

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est appelée
	/// que si nécessaire.
	///
	/// \return une référence sur un objet représentant le dossier parent ou sur \c nil.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;

	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*	MakeName( void ) const;

	///
	/// Détermine si le dossier est un disque.
	///
	/// \return \c true si le dossier est le dossier racine d'un disque.
	///
	virtual Boolean		IsVolume( void ) const;

	///
	/// Détermine si le dossier est sur le bureau.
	///
	/// \return \c true si le dossier est sur le bureau, \c false sinon.
	///
	virtual Boolean		IsOnDesktop( void ) const;

	///
	/// Récupère la référence sur le volume où se trouve le fichier.
	///
	/// \return le volume où se trouve le fichier.
	///
	virtual short		GetVRefNum( void ) const;

	///
	/// Initialise mVRefNum, mIsVolume, mParentRef et mIsOnDesktop.
	///
	/// \throws TDCLException si un problème est survenu.
	///
	void				Init( void );

	/// \name Variables
	FSRef 				mRef;			///< the FSRef
	FSVolumeRefNum		mVRefNum;		///< Référence du volume.
	Boolean				mIsVolume;		///< Si on est un volume.
	FSRef				mParentRef;		///< Référence sur le parent (invalide
										///< si on a mIsVolume)
	Boolean				mIsOnDesktop;	///< Si on est sur le bureau.
};

#endif
		// _TDCLMACCARBONFOLDER_H

// ====================================================================== //
// "The first rule of magic is simple.  Don't waste your time waving your //
// hands and hoping when a rock or a club will do."                       //
//   -- McCloctnik the Lucid                                              //
// ====================================================================== //