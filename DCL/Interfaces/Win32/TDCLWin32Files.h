// ==============================
// Fichier:			TDCLWin32Files.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/05/2026
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
// The Original Code is TDCLWin32Files.h.
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

#ifndef __TDCLWIN32FILES__
#define __TDCLWIN32FILES__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/TDCLFile.h>

#if !TARGET_OS_WIN32
	#error TDCLWin32Files requiert Windows
#endif

///
/// Mise en place de l'interface IDCLFiles pour Win32.
///
class TDCLWin32Files
	:
		public IDCLFiles
{
public:
	///
	/// Constructeur unique.
	///
	TDCLWin32Files();

	///
	/// Destructeur.
	///
	virtual ~TDCLWin32Files( void );

	///
	/// Indique au Newton qu'on est sur Windows.
	///
	/// \return TDCLDockCommand::kWin_DType
	///
	virtual KUInt32	GetKind( void );

	///
	/// Détermine le dossier racine.
	///
	virtual TDCLFSItemRef	GetRootFolder( const KUInt16* inRootName = nil );

	///
	/// Détermine le nombre de dossiers racine.
	///
	virtual KUInt32			CountRootFolders( void ) const;

	///
	/// Récupère le nom du n-ième dossier racine.
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const;

private:
	TDCLFSItemRef			mRootFolder;		///< Dossier racine
	static const KUInt16	kRootFolderName[3];	///< 'C:'
};

#endif
		// __TDCLWIN32FILES__
