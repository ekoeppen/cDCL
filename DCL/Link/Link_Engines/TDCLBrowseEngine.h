// ==============================
// Fichier:			TDCLBrowseEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			8/4/2003
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
// The Original Code is TDCLBrowseEngine.h.
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
// $Id: TDCLBrowseEngine.h,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef _TDCLBROWSEENGINE_H
#define _TDCLBROWSEENGINE_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Link/Link_Engines/TDCLLinkEngine.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

// Pré-déclarations
class TDCLFolder;
class TDCLPipe;
class IDCLFiles;

///
/// Classe pour le moteur de butinage.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
/// \todo	gérer le butinage sur Windoze.
///
class TDCLBrowseEngine
	:
		public TDCLLinkEngine
{
public:
	///
	/// Constructeur à partir d'un dossier par défaut.
	///
	/// \param inLink			référence sur le lien dont nous faisons le travail.
	/// \param inDefaultFolder	dossier de départ (si \c nil, on prend le dossier racine)
	///
	TDCLBrowseEngine(
				TDCLFullDockLink* inLink,
				TDCLFSItemRef inDefaultFolder = TDCLFSItemRef() );

	///
	/// Commence le butinage. Cette méthode est appelée juste après
	/// avoir accepté le butinage auprès du Newton. Elle doit
	/// envoyer une commande kDResult au Newton pour accepter le butinage.
	///
	/// \param inFileTypes	type de fichiers
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual void StartBrowsing( const TDCLNSRef& inFileTypes );

	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le prochain état du lien (l'état actuel étant kRunning),
	///			par exemple kRunning ou kDisconnecting (DoDisconnect est appelé
	///			dans la boucle)
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean		ProcessDockCommand(
									TDCLDockCommand* inCommand,
									Boolean* outProcessed );

protected:
	///
	/// Récupère les éléments du dossier courant.
	///
	/// \return un tableau contenant les éléments du dossier courant.
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual TDCLNSRef GetFilesAndFolders( void );


	///
	/// Installe un paquet.
	/// Appelle la méthode du lien.
	/// Cette méthode est surtout là pour les sous-classes de cette classe.
	///
	/// \param inPackage	paquet à installer.
	/// \throws TDCLException si une erreur est survenue.
	///
	inline void DoInstallPackage( TDCLFSItemRef inPackage )
		{
			GetLink()->DoInstallPackage( inPackage );
		}

	///
	/// Récupération d'un élément à partir du dossier courant et d'une information
	/// de nom venant du Newton.
	///
	/// \param inFileName	nom (chaîne ou structure avec le nom dans Name et d'autres
	///						informations comme le volume dans whichVol)
	/// \return une référence vers le fichier ou \c nil.
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual TDCLFSItemRef	GetFile( const TDCLNSRef& inFileName );

	///
	/// Récupération d'informations sur un élément.
	///
	/// \param inFileName	nom (chaîne ou structure, passé à GetFile)
	/// \return une structure NewtonScript comprenant les informations à envoyer au Newton.
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual TDCLNSRef		GetFileInfo( const TDCLNSRef& inFileName );

	///
	/// Changement du chemin courant à partir d'un nouveau chemin.
	///
	/// \param inPath		chemin tel qu'envoyé par le Newton.
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual void 			SetPath( const TDCLNSRef& inPath );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLBrowseEngine( const TDCLBrowseEngine& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLBrowseEngine& operator = ( const TDCLBrowseEngine& inCopy );

	/// \name Variables

	IDCLFiles*				mFilesIntf;			///< Interface pour les fichiers.
	TDCLFSItemRef			mCurrentFolderRef;	///< Référence sur le dossier courant.
	TDCLFolder*				mCurrentFolder;		///< Dossier courant.
	Boolean					mIsMac;				///< Si on réagit comme un Mac.
	TDCLNSRef				mFileTypes;			///< Types des fichiers.
};

#endif
		// _TDCLBROWSEENGINE_H

// ========================================================================= //
// It must be remembered that there is nothing more difficult to plan, more  //
// doubtful of success, nor more dangerous to manage, than the creation of   //
// a new system.  For the initiator has the emnity of all who would profit   //
// by the preservation of the old institutions and merely lukewarm defenders //
// in those who would gain by the new ones.                                  //
//                 -- Niccolo Machiavelli, 1513                              //
// ========================================================================= //
