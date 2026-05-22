// ==============================
// Fichier:			TDCLWin32Files.h
// Projet:			Desktop Connection Library
//
// Created:			10/05/2026
// Tab width:		4 spaces
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
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef __TDCLWIN32FILES__
#define __TDCLWIN32FILES__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/TDCLFile.h>

///
/// Implementation of IDCLFiles interface for Win32.
///
class TDCLWin32Files
	:
		public IDCLFiles
{
public:
	///
	/// Constructor.
	///
	TDCLWin32Files();

	///
	/// Destructor.
	///
	virtual ~TDCLWin32Files( void );

	///
	/// Indicate Windows as desktop type.
	///
	/// \return TDCLDockCommand::kWin_DType
	///
	virtual KUInt32	GetKind( void );

	///
	/// Return the root folder by name.
	///
	/// \param inRootName	root name (can be nil).
	/// \return instance of TDCLFolder.
	///
	///
	virtual TDCLFSItemRef	GetRootFolder( const KUInt16* inRootName = nil );

	///
	/// Return the number of root folders.
	///
	/// \return 1
	///
	///
	virtual KUInt32			CountRootFolders( void ) const;

	///
	/// Return the name of the root folder (currently hard coded to 'C:').
	///
	/// \param inFolderIndex	index to the root folders array, between 0 and
	///							CountRootFolders() - 1.
	/// \return name of the corresponding root folder.
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const;

private:
	TDCLFSItemRef			mRootFolder;		///< Root folder
	static const KUInt16	kRootFolderName[3];	///< 'C:'
};

#endif
		// __TDCLWIN32FILES__
