// ==============================
// Fichier:			TDCLMacCarbonFile.h
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
// The Original Code is TDCLMacCarbonFile.h.
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
// $Id: TDCLMacCarbonFile.h,v 1.9 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _TDCLMACCARBONFILE_H
#define _TDCLMACCARBONFILE_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacFile.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonFolder.h>
#include <DCL/Interfaces/MacOS_X/Carbon_Files/TDCLMacCarbonDesktopFolder.h>

// MacOS
#if TARGET_OS_MACOS
#include <Files.h>
#else
#include <CoreServices/CoreServices.h>
#endif

///
/// Classe pour les fichiers sur Mac utilisant les FSRef.
/// Requiert CarbonLib ou MacOS X.
///
/// \author Nicolas Zinovieff <krugazor@free.fr>
/// \version $Revision: 1.9 $
///
/// \test	aucun test défini.
///
class TDCLMacCarbonFile
	:
		public TDCLMacFile
{
public:
	///
	/// Constructeur pour un fichier existant à partir d'un FSRef uniquement.
	/// Crée l'objet représentant le dossier parent.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFSRef	référence sur le fichier
	///
	TDCLMacCarbonFile( TDCLMacFiles* inFilesIntf, const FSRef* inFSRef );

	///
	/// Destructeur.
	/// Ferme le fichier.
	///
	virtual ~TDCLMacCarbonFile( void );

	///
	/// Ouvre le fichier.
	///
	/// \param inReadOnly	si le fichier doit être ouvert en lecture seule
	/// \throws TDCLIOException si l'ouverture a échoué.
	///
	virtual void		Open( Boolean inReadOnly );

	///
	/// Crée le fichier avec un code créateur et un type donnés.
	///
	/// \param inCreator	code créateur du fichier à créer.
	/// \param inFileType	type du fichier à créer.
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void		Create(
							OSType inCreator = TDCLMacFiles::kCreator,
							OSType inFileType = TDCLMacFiles::kFileType );

	///
	/// Change les codes type et créateur.
	///
	/// \param inCreator	nouveau code créateur du fichier.
	/// \param inFileType	nouveau type du fichier.
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void		SetMetaData(
							OSType inCreator,
							OSType inFileType );

	/// \name interface TDCLFile

	///
	/// Supprime le fichier.
	///
	/// \throws TDCLIOException si la suppression a échoué.
	///
	virtual void		Delete( void );

	///
	/// Retourne la longueur (logique) du fichier.
	///
	/// \return la taille du fichier, en octets.
	///
	virtual KUInt64		GetLength( void );
	
	///
	/// Déplace le curseur dans le fichier. Peut lancer une exception
	/// pour indiquer que la fin du fichier a été atteinte.
	///
	/// \param inPos	nouvelle position du curseur
	/// \param inMode	définit l'origine de inPos
	/// \throws TDCLPositionException si la fin du fichier (ou le début) est
	///			dépassée.
	/// \throws TDCLBadStateError si le fichier n'est pas ouvert.
	///
	virtual void		SetCursor( KSInt64 inPos, ECursorMode inMode );

	///
	/// Détermine la position du curseur dans le fichier.
	///
	/// \return la position du curseur dans le fichier.
	/// \throws TDCLBadStateError si le fichier n'est pas ouvert.
	///
	virtual KSInt64		GetCursor( void );

	/// \name interface entrée/sortie

	///
	/// Lit des octets.
	///
	/// \param outBuffer	mémoire tampon pour les octets lus.
	/// \param ioCount		nombre d'octets à lire en entrée, lus en sortie.
	///						Cette valeur est mise à jour avant que 
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLIOException	si un problème est survenu (autre que EOF)
	///
	virtual	void		Read( void* outBuffer, KUInt32* ioCount );
	
	///
	/// Ecrit des octets.
	///
	/// \param inBuffer		mémoire tampon pour les octets à écrire.
	/// \param ioCount		nombre d'octets à écrire en entrée, écris en
	///						sortie. Cette valeur est mise à jour avant que
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Vide la mémoire tampon de sortie.
	///
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual void		FlushOutput( void );
	
	///
	/// Ferme le fichier.
	/// Ne fait rien si le fichier n'était pas ouvert.
	/// Cette méthode est appelée par le destructeur.
	///
	virtual void		Close( void );

	///
	/// Récupère le type du fichier sous forme de chaîne NewtonScript.
	/// C'est ce qui est retourné au Newton.
	///
	/// \return une chaîne NewtonScript avec le type du fichier.
	///
	virtual TDCLNSRef	GetKind( void ) const;
	
	///
	/// Récupère la date de création au format des dates Newton
	/// (minutes depuis 1904).
	///
	/// \return la date de création du fichier.
	///
	virtual KUInt32		GetCreationDate( void ) const;

	///
	/// Récupère la date de modification au format des dates Newton
	/// (minutes depuis 1904).
	///
	/// \return la date de modification du fichier.
	///
	virtual KUInt32		GetModificationDate( void ) const;

	///
	/// Récupère le chemin du fichier dans une forme lisible par
	/// l'utilisateur. Ce chemin n'a pas d'autre fonction que
	/// d'être affiché sur le Newton.
	///
	/// \return le chemin du fichier dans une chaîne NewtonScript.
	///
	virtual TDCLNSRef	GetStringPath( void ) const;

	///
	/// Récupère l'icône du fichier sous forme de binaire noir et blanc
	/// 32x32.
	///
	/// \return l'icône du fichier.
	///
	virtual TDCLNSRef	GetIcon( void ) const;

private:
	///
	/// La classe TDCLMacCarbonFolder crée les objets de type TDCLMacCarbonFile.
	///
	friend class TDCLMacCarbonFolder;
	
	/// Pareil pour la classe TDCLMacCarbonDesktopFolder.
	friend class TDCLMacCarbonDesktopFolder;

	///
	/// Constructeur à partir d'un dossier parent et d'un FSRef pour un
	/// fichier *existant* dans un dossier.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFolder	dossier où se trouve le fichier (peut-être le bureau)
	/// \param inFSRef	référence sur le fichier
	///
	TDCLMacCarbonFile(
					TDCLMacFiles* inFilesIntf,
					TDCLFSItemRef inFolder,
					const FSRef* inFSRef );

	///
	/// Constructeur à partir d'un dossier parent et d'un FSRef pour un
	/// fichier n'existant pas forcément dans un dossier.
	/// Ce constructeur doit être appelé par un objet TDCLMacCarbonFolder.
	/// Le nom du fichier est copié.
	/// De plus, inFSRef doit correspondre à inFolder et inName.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFolder	dossier où se trouve le fichier.
	/// \param inFSRef	référence sur le fichier.
	///					Si c'est nul, la référence est créée.
	/// \param inName	nom du fichier (chaîne UTF-16 avec terminateur nul).
	///
	TDCLMacCarbonFile(
					TDCLMacFiles* inFilesIntf,
					TDCLFSItemRef inFolder,
					const FSRef* inFSRef,
					const KUInt16* inName );

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacCarbonFile( const TDCLMacCarbonFile& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacCarbonFile& operator = ( const TDCLMacCarbonFile& inCopy );

	///
	/// \brief Crée l'objet représentant le dossier parent d'un objet donné.
	///
	/// \param	inFilesIntf	interface pour les fichiers.
	/// \param	outIsOnDesktop en sortie, si le fichier est sur le bureau.
	/// \param	inFSRef référence sur l'objet dont on veut le dossier parent.
	/// \return le dossier parent (peut-être le bureau).
	///
	static TDCLFSItemRef	DoMakeParentFolder(
								TDCLMacFiles* inFilesIntf,
								Boolean* outIsOnDesktop,
								const FSRef* inFSRef );

	///
	/// Crée le nom du fichier.
	///
	/// \return le nom du fichier en UTF-16 (cette chaîne est allouée avec
	///			malloc).
	/// \throw TDCLIOException si un problème est survenu.
	///
	virtual KUInt16*	MakeName( void ) const;

	///
	/// Crée le FSRef.
	/// Méthode appelée lorsque mRefIsValid est \c false et qu'on a besoin
	/// de mRef.
	///
	/// \throw TDCLIOException si un problème est survenu (en particulier
	///			si le fichier n'existe pas).
	///
	void				CreateFSRef( void );

	///
	/// Détermine si le fichier est sur le bureau.
	///
	/// \return \c true si le fichier est sur le bureau, \c false sinon.
	///
	virtual Boolean		IsOnDesktop( void ) const;

	///
	/// Récupère la référence sur le volume où se trouve le fichier.
	///
	/// \return le volume où se trouve le fichier.
	///
	virtual short		GetVRefNum( void ) const;

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est
	/// appelée que si l'objet parent qui a été fourni est \c nil.
	/// Met à jour mIsOnDesktop.
	///
	/// \return le parent de \c this.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;

	/// \name Variables
	
	FSRef			mRef;			///< Le FSRef représentant ce fichier.
	Boolean			mRefIsValid;	///< Si le FSRef est valide.
	FSRef			mParentRef;		///< Le FSRef sur le parent.
	KSInt16			mOpenedFork;	///< the ID of opened fork
	FSVolumeRefNum	mVRefNum;		///< Référence du volume.
	mutable Boolean	mIsOnDesktop;	///< Si le fichier est sur le bureau.
};

#endif
		// _TDCLMACCARBONFILE_H

// ================================================================ //
// Government lies, and newspapers lie, but in a democracy they are //
// different lies.                                                  //
// ================================================================ //