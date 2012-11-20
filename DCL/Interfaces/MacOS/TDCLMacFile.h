// ==============================
// Fichier:			TDCLMacFile.h
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
// The Original Code is TDCLMacFile.h.
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
// $Id: TDCLMacFile.h,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef __TDCLMACFILE__
#define __TDCLMACFILE__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Interfaces/MacOS/TDCLMacFiles.h>
#include <DCL/Streams/TDCLStream.h>

// MacOS
#if TARGET_OS_MACOS
	#include <Files.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

class TDCLFolder;

///
/// Classe de base pour les fichiers sous MacOS. Cette classe est dérivée
/// en une classe pour les fichiers version classique et une classe pour les
/// fichiers version porno.
///
/// La particularité de cette classe ne réside que dans la méthode Create qui
/// a le code type et le code créateur.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLMacFile
	:
		public TDCLFile
{
public:
	///
	/// Crée le fichier avec un code créateur et un type donnés.
	///
	/// \param inCreator	code créateur du fichier à créer.
	/// \param inFileType	type du fichier à créer.
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void		Create(
							OSType inCreator,
							OSType inFileType ) = 0;

	///
	/// Crée le fichier.
	///
	/// \throws TDCLIOException si la création a échoué.
	///
	inline virtual void		Create( void )
		{
			Create( TDCLMacFiles::kCreator, TDCLMacFiles::kFileType );
		}

	///
	/// Change les codes type et créateur.
	///
	/// \param inCreator	nouveau code créateur du fichier.
	/// \param inFileType	nouveau type du fichier.
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void	SetMetaData(
							OSType inCreator,
							OSType inFileType ) = 0;

	/// \name interface TDCLFile

	///
	/// Détermine si le fichier est ouvert.
	///
	/// \return \c true si le fichier est ouvert, \c false sinon.
	///
	virtual Boolean		IsOpen( void ) const;

	///
	/// Détermine si le fichier est ouvert en lecture seule.
	///
	/// \return \c true si le fichier est ouvert en lecture seule,
	///			\c false sinon. Le résultat est indéterminé si le fichier
	///			n'est pas ouvert.
	///
	virtual Boolean		IsReadOnly( void ) const;

	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Ajoute 'whichVol à la structure si le fichier est sur le bureau.
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef	ToFrame( void );

protected:
	///
	/// Constructeur à partir d'un dossier parent.
	/// Le dossier parent doit être compatible avec la classe du fichier.
	///
	/// \param inFilesIntf	interface pour les fichiers.
	/// \param inFolder		dossier parent.
	///						(\c nil signifie que MakeParentFolder sera
	///							appelé plus tard).
	///
	TDCLMacFile( TDCLMacFiles* inFilesIntf, TDCLFSItemRef inFolder );

	///
	/// Méthode appelée par les sous-classes lorsque le fichier est ouvert.
	///
	/// \param inReadOnly	si le fichier est en lecture seule.
	///
	void DoOpen( Boolean inReadOnly );
	
	///
	/// Méthode appelée par les sous-classes lorsque le fichier est fermé.
	///
	void DoClose( void );

	///
	/// Change les codes type et créateur d'un fichier représenté par
	/// son FSSpec.
	///
	/// \param inFSSpec		référence sur le fichier.
	/// \param inCreator	nouveau code créateur du fichier.
	/// \param inFileType	nouveau type du fichier.
	/// \throws TDCLIOException si la création a échoué.
	///
	static void	DoSetMetaData(
							const FSSpec* inFSSpec,
							OSType inCreator,
							OSType inFileType );
	
private:
	///
	/// Détermine si le fichier est sur le bureau.
	///
	/// \return \c true si le fichier est sur le bureau, \c false sinon.
	///
	virtual Boolean		IsOnDesktop( void ) const = 0;
	
	///
	/// Récupère la référence sur le volume où se trouve le fichier.
	///
	/// \return le volume où se trouve le fichier.
	///
	virtual short		GetVRefNum( void ) const = 0;

	/// \name variables privées
	
	Boolean				mReadOnly;		///< Si le fichier a été ouvert en
										///< lecture seule.
	Boolean				mIsOpen;		///< Si le fichier est ouvert.
};

#endif
		// __TDCLMACFILE__

// ========================================================================= //
// An interpretation I satisfies a sentence in the table language if and     //
// only if each entry in the table designates the value of the function      //
// designated by the function constant in the upper-left corner applied to   //
// the objects designated by the corresponding row and column labels.        //
//              -- Genesereth & Nilsson, "Logical foundations of Artificial  //
//                    Intelligence"                                          //
// ========================================================================= //
