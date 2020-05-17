// ==============================
// Fichier:			TDCLPOSIXFiles.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLPOSIXFiles.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPOSIXFiles.h,v 1.10 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef __TDCLPOSIXFILES__
#define __TDCLPOSIXFILES__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/TDCLFile.h>

#if !TARGET_OS_COMPAT_POSIX
	#error TDCLPOSIXFiles requiert la compatibilité POSIX
#endif

///
/// Mise en place de l'interface IDCLFiles pour POSIX.
/// Pour avoir plusieurs racines, il faut dériver cette classe
/// et mettre en place les méthodes GetRootFolder, CountRootFolders et
/// GetRootFolderName.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.10 $
///
/// \test	aucun test défini.
///
class TDCLPOSIXFiles
	:
		public IDCLFiles
{
public:
	///
	/// Constructeur unique.
	///
	/// \param inFollowSymLinks		si on suit les liens symboliques.
	///
	TDCLPOSIXFiles( Boolean inFollowSymLinks = true );

	///
	/// Destructeur.
	///
	virtual ~TDCLPOSIXFiles( void );

	///
	/// Indique au Newton qu'on est sur Windoze.
	///
	/// \return TDCLDockCommand::kWin_DType
	///
	virtual KUInt32	GetKind( void );

	///
	/// Détermine le dossier racine.
	///
	/// Cette méthode retourne le dossier racine. Ce dossier appartient à l'objet
	/// TDCLPOSIXFiles.
	///
	virtual TDCLFSItemRef	GetRootFolder( const KUInt16* inRootName = nil );

	///
	/// Détermine le nombre de dossiers racine.
	///
	/// \return 1
	///
	virtual KUInt32			CountRootFolders( void ) const;

	///
	/// Récupère le nom du n-ième dossier racine.
	/// Le nom appartient à l'objet IDCLFiles.
	/// Ce nom doit obligatoirement se terminer par :
	///
	/// \param inFolderIndex	un indice de dossier racine, entre 0 et
	///							CountRootFolders() - 1.
	/// \return /:
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const;

	///
	/// Si on suit les liens symboliques (pour les dossiers seulement,
	/// on suit tous les fichiers).
	///
	inline Boolean GetFollowSymLinks( void )
		{
			return mFollowSymLinks;
		}

	///
	/// Nombre de minutes entre le 1er janvier 1904 et le 1er janvier 1970.
	/// Pour les conversions de date.
	///
	static const KUInt32 kMinutesBetween1904and1970;

private:
	/// \name variables privées
	
	TDCLFSItemRef			mRootFolder;		///< Dossier racine
	Boolean					mFollowSymLinks;	///< Si on suit les liens symboliques.
	static const KUInt16	kRootFolderName[3];	///< '/:'
};

#endif
		// __TDCLPOSIXFILES__

// ============================================================================ //
// My God, I'm depressed!  Here I am, a computer with a mind a thousand times   //
// as powerful as yours, doing nothing but cranking out fortunes and sending    //
// mail about softball games.  And I've got this pain right through my ALU.     //
// I've asked for it to be replaced, but nobody ever listens.  I think it would //
// be better for us both if you were to just log out again.                     //
// ============================================================================ //
