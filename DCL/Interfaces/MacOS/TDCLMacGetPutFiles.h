// ==============================
// Fichier:			TDCLMacGetPutFiles.h
// Projet:			Desktop Connection Library
//
// Créé le:			30/01/2003
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
// The Original Code is TDCLMacGetPutFiles.h.
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
// $Id: TDCLMacGetPutFiles.h,v 1.6 2004/11/24 18:11:19 paul Exp $
// ===========

#ifndef __TDCLMACGETPUTFILES__
#define __TDCLMACGETPUTFILES__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
#endif

// MacOS
#if TARGET_OS_MACOS
	#include <Files.h>
	#include <Navigation.h>
#else
	#include <CoreServices/CoreServices.h>
	#include <Carbon/Carbon.h>
#endif

#ifdef __MWERKS__
	#pragma	warn_resultnotused	reset
#endif

// DCL
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Interfaces/MacOS/TDCLMacFile.h>

///
/// Classe pour récupérer une référence vers un fichier existant ou un fichier
/// à créer. Utilise les NavServices ou StandardFile suivant ce qui est
/// disponible.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
class TDCLMacGetPutFiles
{
public:
	///
	/// Constructeur à partir de l'interface pour les fichiers.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	///
	TDCLMacGetPutFiles( TDCLMacFiles* inFilesIntf );

	///
	/// Destructeur.
	///
	virtual ~TDCLMacGetPutFiles( void );

	///
	/// Affiche un dialogue pour choisir un fichier (existant) sur le disque.
	/// Le fichier n'est pas ouvert.
	///
	/// \param inPrompt		message affiché pour spécifier cette action
	/// \return un objet TDCLMacFile représentant ce fichier
	/// \throw TDCLException si un problème est survenu ou si l'utilisateur
	///			a annulé l'opération.
	///
	inline TDCLFSItemRef	GetFile( const char* inPrompt )
		{
			return DoGetFile( inPrompt, false );
		}

	///
	/// Affiche un dialogue pour choisir un paquet (existant) sur le disque.
	/// Le fichier n'est pas ouvert.
	///
	/// \param inPrompt		message affiché pour spécifier cette action
	/// \return un objet TDCLMacFile représentant ce fichier
	/// \throw TDCLException si un problème est survenu ou si l'utilisateur
	///			a annulé l'opération.
	///
	inline TDCLFSItemRef	GetPackage( const char* inPrompt )
		{
			return DoGetFile( inPrompt, true );
		}

	///
	/// Affiche un dialogue pour définir un nouveau fichier sur le disque.
	/// Le fichier n'est pas créé. En revanche, si le fichier existait déjà,
	/// il est au préalable supprimé.
	///
	/// \param inPrompt			message affiché pour spécifier cette action
	/// \param inDefaultName	nom par défaut pour le nouveau fichier
	/// \return un objet TDCLMacFile représentant ce fichier
	/// \throw TDCLException si un problème est survenu ou si l'utilisateur
	///			a annulé l'opération.
	///
	TDCLFSItemRef		PutFile(
								const char* inPrompt,
								const char* inDefaultName );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacGetPutFiles( const TDCLMacGetPutFiles& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacGetPutFiles& operator = ( const TDCLMacGetPutFiles& inCopy );

	///
	/// Affiche un dialogue pour choisir un fichier (existant) ou un paquet sur
	/// le disque.
	/// Le fichier n'est pas ouvert.
	///
	/// \param inPrompt		message affiché pour spécifier cette action
	/// \param inOnlyPackages	si on ne veut que les paquets
	/// \return un objet TDCLMacFile représentant ce fichier
	/// \throw TDCLException si un problème est survenu ou si l'utilisateur
	///			a annulé l'opération.
	///
	TDCLFSItemRef		DoGetFile(
								const char* inPrompt,
								Boolean inOnlyPackages );

	///
	/// Filtre pour les paquets avec les NavigationServices.
	///
	/// \param inItem			élément à filtrer.
	/// \param inInfo			information des NavServices
	/// \param inCallBackUD		cookie (ignoré)
	/// \param inFilterMode		mode de filtrage (ignoré)
	/// \return \c true si le fichier est un paquet, \c false sinon.
	///
	static pascal Boolean	PackageNavServicesFilterProc(
									AEDesc* inItem,
									void* inInfo, 
									NavCallBackUserData inCallBackUD,
									NavFilterModes inFilterMode );

	/// \name Variables

	TDCLMacFiles*	mFilesIntf;
	Boolean			mNavServicesAvailable;	///< Si les NavServices sont
											///< disponibles.
};

#endif
		// __TDCLMACGETPUTFILES__

// =============================================================== //
// Never test for an error condition you don't know how to handle. //
//                 -- Steinbach                                    //
// =============================================================== //