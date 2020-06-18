// ==============================
// Fichier:			TDCLPOSIXFile.h
// Projet:			Desktop Connection Library
// 
// Créé le:			22/6/2003
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
// The Original Code is TDCLPOSIXFile.h.
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
// $Id: TDCLPOSIXFile.h,v 1.9 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLPOSIXFILE_H
#define _TDCLPOSIXFILE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFile.h>

// ANSI C
#include <stdio.h>

class TDCLPOSIXFiles;

///
/// Classe pour les fichiers dans un environnement POSIX.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.9 $
///
/// \test	aucun test défini.
///
class TDCLPOSIXFile
	:
		public TDCLFile
{
public:
	///
	/// Constructeur à partir de l'interface POSIXFiles et d'un chemin complet.
	/// Le dossier parent sera créé.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inPath			chemin complet vers le fichier.
	///
	TDCLPOSIXFile(
				TDCLPOSIXFiles* inFilesIntf,
				const char* inPath );

	///
	/// Destructeur.
	/// Ferme le fichier.
	///
	virtual ~TDCLPOSIXFile( void );

	///
	/// Ouvre le fichier.
	///
	/// \param inReadOnly	si le fichier doit être ouvert en lecture seule
	/// \throws TDCLIOException si l'ouverture a échoué.
	///
	virtual void		Open( Boolean inReadOnly );	

	///
	/// Crée le fichier.
	///
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void		Create( void );	

	///
	/// Supprime le fichier.
	///
	/// \throws TDCLIOException si la suppression a échoué.
	///
	virtual void		Delete( void );	

	///
	/// Determine si le fichier est ouvert.
	///
	/// \return \c true si le fichier est ouvert, \c false sinon.
	///
	virtual Boolean		IsOpen( void ) const;

	///
	/// Determine si le fichier est ouvert en lecture seule.
	///
	/// \return \c true si le fichier est ouvert en lecture seule,
	///			\c false sinon. Le résultat est indéterminé si le fichier
	///			n'est pas ouvert.
	///
	virtual Boolean		IsReadOnly( void ) const;

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

	///
	/// Retourne la longueur (logique) du fichier.
	///
	/// \return la taille du fichier, en octets.
	///
	virtual KUInt64		GetLength( void );
	
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
	/// Vous souhaitez probablement appeler la méthode close dans votre
	/// destructeur.
	/// Ne fait rien si le fichier n'était pas ouvert.
	///
	virtual void		Close( void );

	///
	/// Récupère le type du fichier sous forme de chaîne NewtonScript.
	/// C'est ce qui est retourné au Newton pour affichage dans
	/// le butineur.
	///
	/// \return une chaîne NewtonScript avec le type du fichier.
	///
	virtual TDCLNSRef	GetKind( void ) const;
	
	///
	/// Récupère la date de création au format des dates Newton
	/// (minutes depuis 1904).
	/// Ce n'est pas vraiment la date de création pour les fichiers POSIX,
	/// mais ça ira vu l'utilisation qu'on en fait.
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

private:
	///
	/// La classe TDCLPOSIXDirBase crée des objets de type TDCLPOSIXFile
	///
	friend class TDCLPOSIXDirBase;

	///
	/// Constructeur à partir de l'interface POSIXFiles, d'un dossier parent
	/// et d'un chemin complet.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inParentFolder	dossier parent.
	///							(\c nil signifie que MakeParentFolder sera
	///							appelé plus tard).
	/// \param inPath			chemin complet vers le fichier.
	///
	TDCLPOSIXFile(
				TDCLPOSIXFiles* inFilesIntf,
				TDCLFSItemRef inParentFolder,
				const char* inPath );

	///
	/// Crée l'objet représentant le dossier parent d'un objet donné.
	///
	/// \param	inFilesIntf	interface pour les fichiers.
	/// \param	inPath chemin sur l'objet dont on veut le dossier parent.
	/// \return le dossier parent.
	///
	static TDCLFSItemRef	DoMakeParentFolder(
								TDCLPOSIXFiles* inFilesIntf,
								const char* inPath );

	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*	MakeName( void ) const;

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est
	/// appelée que si l'objet parent qui a été fourni est \c nil.
	///
	/// \return le parent de \c this.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;

	/// \name Variables
	FILE*				mFile;		///< Référence sur le fichier (C 89),
									///< \c nil si le fichier n'a pas été
									///< ouvert.
	const char*			mPath;		///< Chemin du fichier.
	Boolean				mReadOnly;	///< Si le fichier a été ouvert en lecture seule.
};

#endif
		// _TDCLPOSIXFILE_H

// =========================================== //
// Your mode of life will be changed to ASCII. //
// =========================================== //
