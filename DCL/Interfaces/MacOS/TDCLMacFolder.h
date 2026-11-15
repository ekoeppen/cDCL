// ==============================
// Fichier:			TDCLMacFolder.h
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
// The Original Code is TDCLMacFolder.h.
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
// $Id: TDCLMacFolder.h,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef _TDCLMACFOLDER_H
#define _TDCLMACFOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFolder.h>

// MacOS
#if TARGET_OS_MACOS
	#include <Files.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>

///
/// Classe pour l'abstraction d'un dossier sous MacOS
/// (y compris en version porno).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLMacFolder
	:
		public TDCLFolder
{
public:
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
								OSType inCreator,
								OSType inFileType ) = 0;

	///
	/// Crée un fichier d'un nom donné dans ce dossier. Appelle la méthode
	/// CreateFile avec le type et le code créateur par défaut.
	///
	/// \param inName		nom du fichier à créer.
	/// \return un objet TDCLFile représentant ce nouveau fichier. Ce fichier
	///			est la propriété de l'appelant qui doit le supprimer.
	/// \throw TDCLException si un problème est survenu, y compris si le fichier
	///			existe déjà.
	///
	inline virtual	TDCLFSItemRef	CreateFile( const KUInt16* inName )
		{
			return CreateFile(
				inName, TDCLMacFiles::kCreator, TDCLMacFiles::kFileType );
		}

protected:
	///
	/// Constructeur à partir d'un dossier parent (optionnel).
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inParentFolder	dossier parent (ou \c nil si celui-ci sera créé
	///							plus tard).
	///
	TDCLMacFolder(
				TDCLMacFiles* inFilesIntf,
				TDCLFSItemRef inParentFolder = TDCLFSItemRef() );

	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Ajoute les informations sur le volume si on est sur le bureau.
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef	ToFrame( void );

private:
	///
	/// Détermine si le dossier est un disque.
	///
	/// \return \c true si le dossier est le dossier racine d'un disque.
	///
	virtual Boolean		IsVolume( void ) const = 0;

	///
	/// Détermine si le dossier est sur le bureau.
	///
	/// \return \c true si le dossier est sur le bureau, \c false sinon.
	///
	virtual Boolean		IsOnDesktop( void ) const = 0;

	///
	/// Récupère la référence sur le volume où se trouve le dossier.
	///
	/// \return le volume où se trouve le dossier.
	///
	virtual short		GetVRefNum( void ) const = 0;
};

#endif
		// _TDCLMACFOLDER_H

// =================================================================== //
// The value of a program is proportional to the weight of its output. //
// =================================================================== //
