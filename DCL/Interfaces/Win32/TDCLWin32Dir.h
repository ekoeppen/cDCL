// ==============================
// Fichier:			TDCLWin32Dir.h
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
// The Original Code is TDCLWin32Dir.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef __TDCLWIN32DIR__
#define __TDCLWIN32DIR__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Win32/TDCLWin32DirBase.h>

class TDCLWin32Dir
	:
		public TDCLWin32DirBase
{
public:
	TDCLWin32Dir( TDCLWin32Files* inFilesIntf, TDCLFSItemRef inParentFolder, const char* inPath );

	virtual	TDCLFSItemRef	GetItemByName( const KUInt16* inName, KSInt32 inVolRefNum = 0 ) { return TDCLWin32DirBase::GetItemByName(inName, inVolRefNum); }
	virtual	TDCLFSItemRef	CreateFile( const KUInt16* inName ) { return TDCLWin32DirBase::CreateFile(inName); }
	virtual	TDCLFSItemRef	CreateFolder( const KUInt16* inName ) { return TDCLWin32DirBase::CreateFolder(inName); }
	virtual KUInt32			CountElements( void ) { return TDCLWin32DirBase::CountElements(); }
	virtual TDCLFSItemRef*	GetItems( void ) { return TDCLWin32DirBase::GetItems(); }
};

#endif
