// ==============================
// Fichier:			TDCLWin32Dir.cpp
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
// The Original Code is TDCLWin32Dir.cpp.
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
#include <DCL/Interfaces/Win32/TDCLWin32DirBase.h>
#include <DCL/Interfaces/Win32/TDCLWin32Dir.h>
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <K/Unicode/UUTF16CStr.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <new>

TDCLWin32DirBase::TDCLWin32DirBase( TDCLWin32Files* inFilesIntf, TDCLFSItemRef inParentFolder, const char* inPath )
	: TDCLFolder( (IDCLFiles*) inFilesIntf, inParentFolder ),
	  mPath( _strdup(inPath) )
{
}

TDCLWin32DirBase::~TDCLWin32DirBase()
{
	::free( mPath );
}

TDCLFSItemRef TDCLWin32DirBase::GetItemByName( const KUInt16* inName, KSInt32 inVolRefNum )
{
	char theItemName[MAX_PATH];
	KUInt32 i = 0;
	while (inName[i] != 0 && i < MAX_PATH - 1) {
		theItemName[i] = (char)inName[i];
		i++;
	}
	theItemName[i] = '\0';

	char theFullPath[MAX_PATH];
	const char* theSeparator = (mPath[::strlen(mPath)-1] == '\\' ? "" : "\\");
	::sprintf(theFullPath, "%s%s%s", mPath, theSeparator, theItemName);

	DWORD theAttributes = ::GetFileAttributesA(theFullPath);
	if (theAttributes == INVALID_FILE_ATTRIBUTES) {
		return TDCLFSItemRef();
	}

	if (theAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TDCLFSItemRef(new TDCLWin32Dir((TDCLWin32Files*)GetFilesIntf(), TDCLFSItemRef(this), theFullPath));
	} else {
		return TDCLFSItemRef(new TDCLWin32File(GetFilesIntf(), theFullPath));
	}
}

TDCLFSItemRef TDCLWin32DirBase::CreateFile( const KUInt16* inName )
{
	char theItemName[MAX_PATH];
	KUInt32 i = 0;
	while (inName[i] != 0 && i < MAX_PATH - 1) {
		theItemName[i] = (char)inName[i];
		i++;
	}
	theItemName[i] = '\0';

	char theFullPath[MAX_PATH];
	const char* theSeparator = (mPath[::strlen(mPath)-1] == '\\' ? "" : "\\");
	::sprintf(theFullPath, "%s%s%s", mPath, theSeparator, theItemName);

	// Ensure the file doesn't exist
	if (::GetFileAttributesA(theFullPath) != INVALID_FILE_ATTRIBUTES) {
		throw DCLPlatformExistsAlready( 0 );
	}

	TDCLFSItemRef theResult(new TDCLWin32File(GetFilesIntf(), theFullPath));
	((TDCLFile*) theResult)->Create();
	return theResult;
}

TDCLFSItemRef TDCLWin32DirBase::CreateFolder( const KUInt16* inName )
{
	char theItemName[MAX_PATH];
	KUInt32 i = 0;
	while (inName[i] != 0 && i < MAX_PATH - 1) {
		theItemName[i] = (char)inName[i];
		i++;
	}
	theItemName[i] = '\0';

	char theFullPath[MAX_PATH];
	const char* theSeparator = (mPath[::strlen(mPath)-1] == '\\' ? "" : "\\");
	::sprintf(theFullPath, "%s%s%s", mPath, theSeparator, theItemName);

	if (::CreateDirectoryA(theFullPath, NULL)) {
		return TDCLFSItemRef(new TDCLWin32Dir((TDCLWin32Files*)GetFilesIntf(), TDCLFSItemRef(this), theFullPath));
	}
	
	throw DCLPlatformUnknownError(::GetLastError());
}

KUInt32 TDCLWin32DirBase::CountElements()
{
	KUInt32 theCount = 0;
	WIN32_FIND_DATAA theFindData;
	char theSearchPath[MAX_PATH];
	
	::sprintf( theSearchPath, "%s\\*", mPath );
	
	HANDLE hFind = ::FindFirstFileA( theSearchPath, &theFindData );
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (::strcmp( theFindData.cFileName, "." ) != 0
				&& ::strcmp( theFindData.cFileName, ".." ) != 0)
			{
				theCount++;
			}
		} while (::FindNextFileA( hFind, &theFindData ));
		::FindClose( hFind );
	}
	
	return theCount;
}

TDCLFSItemRef* TDCLWin32DirBase::GetItems()
{
	KUInt32 theCount = CountElements();
	TDCLFSItemRef* theResult = (TDCLFSItemRef*) ::malloc( (theCount + 1) * sizeof( TDCLFSItemRef ) );
	if (theResult == nil)
	{
		throw DCLMemError;
	}

	// Initialize with nil
	for (KUInt32 i = 0; i <= theCount; i++)
	{
		new (&theResult[i]) TDCLFSItemRef();
	}

	WIN32_FIND_DATAA theFindData;
	char theSearchPath[MAX_PATH];
	::sprintf( theSearchPath, "%s\\*", mPath );
	
	HANDLE hFind = ::FindFirstFileA( theSearchPath, &theFindData );
	if (hFind != INVALID_HANDLE_VALUE)
	{
		KUInt32 theIndex = 0;
		do
		{
			if (::strcmp( theFindData.cFileName, "." ) != 0
				&& ::strcmp( theFindData.cFileName, ".." ) != 0)
			{
				char theItemPath[MAX_PATH];
				::sprintf( theItemPath, "%s\\%s", mPath, theFindData.cFileName );
				
				TDCLFSItem* theItem = nil;
				if (theFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					theItem = new TDCLWin32Dir( (TDCLWin32Files*) GetFilesIntf(), TDCLFSItemRef( this ), theItemPath );
				}
				else
				{
					theItem = new TDCLWin32File( GetFilesIntf(), theItemPath );
				}
				
				if (theItem)
				{
					theResult[theIndex] = TDCLFSItemRef( theItem );
					theIndex++;
				}
			}
		} while (::FindNextFileA( hFind, &theFindData ) && theIndex < theCount);
		::FindClose( hFind );
	}
	
	return theResult;
}

KUInt16* TDCLWin32DirBase::MakeName() const
{
	// Simple implementation for now
	size_t theLength = ::strlen( mPath ) + 1;
	KUInt16* theResult = (KUInt16*) ::malloc( theLength * sizeof( KUInt16 ) );
	UUTF16CStr::FromASCII( (const KUInt8*) mPath, theResult, theLength );
	return theResult;
}

TDCLFSItemRef TDCLWin32DirBase::MakeParentFolder() const
{
	return TDCLFSItemRef();
}
