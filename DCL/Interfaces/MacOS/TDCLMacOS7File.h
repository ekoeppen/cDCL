// ==============================
// Fichier:			TDCLMacOS7File.h
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
// The Original Code is TDCLMacOS7File.h.
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
// $Id: TDCLMacOS7File.h,v 1.7 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef __TDCLMACOS7FILE__
#define __TDCLMACOS7FILE__

#include <DCL/Headers/DCLDefinitions.h>

// MacOS
#include <Files.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/MacOS/TDCLMacFile.h>
#include <DCL/Streams/TDCLStream.h>

///
/// Classe pour les fichiers sur Mac utilisant les FSSpec.
/// Requiert InterfaceLib 7.1 ou plus récent.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLMacOS7File
	:
		public TDCLMacFile
{
public:
	///
	/// Constructeur à partir d'un FSSpec uniquement.
	/// Crée l'objet représentant le dossier parent.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFSSpec	référence sur le fichier
	/// \throws TDCLException si l'ouverture a échoué.
	///
	TDCLMacOS7File(
					TDCLMacFiles* inFilesIntf,
					const FSSpec* inFSSpec );

	///
	/// Destructeur.
	/// Ferme le fichier.
	///
	virtual				~TDCLMacOS7File( void );

	///
	/// Ouvre le fichier.
	///
	/// \param inReadOnly	si le fichier doit être ouvert en lecture seule
	/// \throws TDCLException si l'ouverture a échoué.
	///
	virtual void		Open( Boolean inReadOnly );

	///
	/// Crée le fichier avec un code créateur et un type donnés.
	///
	/// \param inCreator	code créateur du fichier à créer.
	/// \param inFileType	type du fichier à créer.
	/// \throws TDCLException si la création a échoué.
	///
	virtual void		Create(
							OSType inCreator = TDCLMacFiles::kCreator,
							OSType inFileType = TDCLMacFiles::kFileType );

	///
	/// Change les codes type et créateur.
	///
	/// \param inCreator	nouveau code créateur du fichier.
	/// \param inFileType	nouveau type du fichier.
	/// \throws TDCLException si la création a échoué.
	///
	virtual void		SetMetaData(
							OSType inCreator,
							OSType inFileType );

	/// \name interface TDCLFile

	///
	/// Supprime le fichier.
	///
	/// \throws TDCLException si la suppression a échoué.
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
	/// \throws TDCLException si la fin du fichier (ou le début) est dépassée.
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
	/// \throws TDCLException	si un problème est survenu (autre que EOF)
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
	/// \throws TDCLException	si un problème est survenu.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Vide la mémoire tampon de sortie.
	///
	/// \throws TDCLException	si un problème est survenu.
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
	/// La classe TDCLMacOS7Folder crée les objets de type TDCLMacOS7File.
	///
	friend class TDCLMacOS7Folder;

	///
	/// Constructeur privé (cette classe ne peut pas être dérivée).
	/// Ce constructeur doit être appelé un objet TDCLMacOS7Folder.
	/// De plus, inFSSpec doit correspondre à inFolder.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFolder		dossier où se trouve le fichier.
	///						C'est aussi le dossier qui a créé l'objet.
	/// \param inFSSpec		spécification du dossier.
	///
	TDCLMacOS7File(
						TDCLMacFiles* inFilesIntf,
						TDCLFSItemRef inFolder,
						const FSSpec* inFSSpec );

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacOS7File( const TDCLMacOS7File& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacOS7File& operator = ( const TDCLMacOS7File& inCopy );

	///
	/// Crée le nom du fichier.
	///
	/// \return le nom du fichier en UTF-16 (cette chaîne est allouée avec
	///			malloc).
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*	MakeName( void ) const;

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

	/// \name Variables
		
	FSSpec		mFileFSSpec;	///< Spécification du fichier sur le disque.
	short		mRefNum;		///< Référence sur le fichier ouvert.
};

#endif
		// __TDCLMACOS7FILE__

// ======================================================================= //
// In a five year period we can get one superb programming language.  Only //
// we can't control when the five year period will begin.                  //
// ======================================================================= //