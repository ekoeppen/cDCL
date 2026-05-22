// ==============================
// Fichier:			TDCLWin32RootDir.cpp
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
// The Original Code is TDCLWin32RootDir.cpp.
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

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Win32/TDCLWin32RootDir.h>
#include <K/Unicode/UUTF16CStr.h>
#include <string.h>
#include <stdlib.h>

TDCLWin32RootDir::TDCLWin32RootDir( TDCLWin32Files* inFilesIntf, const char* inPath )
	: TDCLFolder( (IDCLFiles*) inFilesIntf ),
	  TDCLWin32DirBase( inFilesIntf, TDCLFSItemRef(), inPath ),
	  TDCLRootFolder( (IDCLFiles*) inFilesIntf )
{
}

KUInt16* TDCLWin32RootDir::MakeName() const
{
	const char* theName = "c:\\";
	size_t theLength = ::strlen( theName ) + 1;
	KUInt16* theResult = (KUInt16*) ::malloc( theLength * sizeof( KUInt16 ) );
	UUTF16CStr::FromASCII( (const KUInt8*) theName, theResult, theLength );
	
	return theResult;	
}

TDCLFSItemRef TDCLWin32RootDir::MakeParentFolder() const
{
	return TDCLFSItemRef();
}
