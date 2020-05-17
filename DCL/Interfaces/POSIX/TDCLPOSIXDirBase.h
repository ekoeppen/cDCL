// ==============================
// Fichier:			TDCLPOSIXDirBase.h
// Projet:			Desktop Connection Library
// 
// Créé le:			20/9/2003
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
// The Original Code is TDCLPOSIXDirBase.h.
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
// $Id: TDCLPOSIXDirBase.h,v 1.3 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLPOSIXDIRBASE_H
#define _TDCLPOSIXDIRBASE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFolder.h>

// SUSv2
#include <sys/types.h>
#include <dirent.h>

// DCL
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>

///
/// Classe de base pour les répertoires POSIX.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLPOSIXDirBase
	:
		virtual public TDCLFolder
{
public:
	///
	/// Destructeur.
	/// Ferme le dossier.
	///
	virtual ~TDCLPOSIXDirBase( void );

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
	virtual	TDCLFSItemRef	GetItemByName( const KUInt16* inName, KSInt32 inVolRefNum = 0 );

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
	/// \return un tableau de n+1 TDCLFSItemRef. Le dernier élément est une référence
	///			sur \c nil. Le tableau est alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef*	GetItems( void );

protected:
	///
	/// Constructeur à partir d'un dossier parent (facultatif) et d'un chemin.
	/// Le dossier parent doit être compatible avec la classe du fichier.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFolder		dossier parent (peut être \c nil)
	/// \param inPath		chemin complet vers le dossier.
	///
	TDCLPOSIXDirBase(
			TDCLPOSIXFiles* inFilesIntf,
			TDCLFSItemRef inFolder,
			const char* inPath );

	///
	/// Accesseur sur le chemin.
	///
	/// \return une référence sur le chemin.
	///
	inline const char* GetPOSIXPath( void ) const
		{
			return mPath;
		}

private:
	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*	MakeName( void ) const;

	///
	/// Ouvre le dossier à partir du chemin.
	///
	void			OpenDir( void );

	DIR*			mDirRef;	///< Référence sur le dossier.
	const char*		mPath;		///< Chemin du dossier.
};

#endif
		// _TDCLPOSIXDIRBASE_H

// ======================================= //
// Building translators is good clean fun. //
//                 -- T. Cheatham          //
// ======================================= //
