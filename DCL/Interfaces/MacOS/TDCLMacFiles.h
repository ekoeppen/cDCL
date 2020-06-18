// ==============================
// Fichier:			TDCLMacFiles.h
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
// The Original Code is TDCLMacFiles.h.
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
// $Id: TDCLMacFiles.h,v 1.5 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef __TDCLMACFILES__
#define __TDCLMACFILES__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>

// MacOS
#if TARGET_OS_MACOS
#include <Files.h>
#else
#include <CoreServices/CoreServices.h>
#endif

// DCL
#include <DCL/Interfaces/MacOS/TDCLMacDesktopFolder.h>

///
/// Mise en place de l'interface IDCLFiles pour MacOS.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLMacFiles
	:
		public IDCLFiles
{
public:
	///
	/// Constantes pour les valeurs par défaut.
	///
	enum {
		kCreator		= '****',	///< Créateur par défaut.
		kFileType		= 'BINA'	///< Type de fichier par défaut.
	};
	
	///
	/// Constructeur par défaut.
	///
	TDCLMacFiles( void );

	///
	/// Destructeur.
	///
	virtual ~TDCLMacFiles( void );
	
	///
	/// Indique au Newton qu'on est sur MacOS.
	///
	/// \return TDCLDockCommand::kMac_DType
	///
	virtual KUInt32	GetKind( void );

	///
	/// Détermine le dossier racine.
	///
	/// Cette méthode retourne le dossier racine. Ce dossier appartient à l'objet
	/// TDCLMacFiles.
	///
	virtual TDCLFSItemRef	GetRootFolder( const KUInt16* inRootName = nil );

	///
	/// Détermine le nombre de dossiers racine.
	///
	/// \return rien du tout
	/// \throws DCLNotImplementedError dans tous les cas.
	///
	virtual KUInt32			CountRootFolders( void ) const;

	///
	/// Récupère le nom du n-ième dossier racine.
	/// Le nom appartient à l'objet IDCLFiles.
	///
	/// \param inFolderIndex	un indice de dossier racine, entre 0 et
	///							CountRootFolders() - 1.
	/// \return rien du tout
	/// \throws DCLNotImplementedError dans tous les cas.
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const;

private:
	/// \name variables privées
	
	TDCLFSItemRef	mRootFolder;			///< Dossier racine
};

#endif
		// __TDCLMACFILES__

// ============================================================================== //
//         One of the questions that comes up all the time is: How enthusiastic   //
// is our support for UNIX?                                                       //
//         Unix was written on our machines and for our machines many years ago.  //
// Today, much of UNIX being done is done on our machines. Ten percent of our     //
// VAXs are going for UNIX use.  UNIX is a simple language, easy to understand,   //
// easy to get started with. It's great for students, great for somewhat casual   //
// users, and it's great for interchanging programs between different machines.   //
// And so, because of its popularity in these markets, we support it.  We have    //
// good UNIX on VAX and good UNIX on PDP-11s.                                     //
//         It is our belief, however, that serious professional users will run    //
// out of things they can do with UNIX. They'll want a real system and will end   //
// up doing VMS when they get to be serious about programming.                    //
//         With UNIX, if you're looking for something, you can easily and quickly //
// check that small manual and find out that it's not there.  With VMS, no matter //
// what you look for -- it's literally a five-foot shelf of documentation -- if   //
// you look long enough it's there.  That's the difference -- the beauty of UNIX  //
// is it's simple; and the beauty of VMS is that it's all there.                  //
//                 -- Ken Olsen, president of DEC, DECWORLD Vol. 8 No. 5, 1984    //
// [It's been argued that the beauty of UNIX is the same as the beauty of Ken     //
// Olsen's brain.  Ed.]                                                           //
// ============================================================================== //