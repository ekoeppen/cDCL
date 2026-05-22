// ==============================
// Fichier:			TDCLWin32Files.cpp
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
// The Original Code is TDCLWin32Files.cpp.
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
#include <DCL/Interfaces/Win32/TDCLWin32RootDir.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <windows.h>

TDCLWin32Files::TDCLWin32Files()
	: mRootFolder(nil)
{
	mRootFolder = GetRootFolder(nil);
}

TDCLWin32Files::~TDCLWin32Files()
{
}

KUInt32 TDCLWin32Files::GetKind()
{
	return TDCLDockCommand::kWin_DType;
}

TDCLFSItemRef TDCLWin32Files::GetRootFolder(const KUInt16* inRootName)
{
	if (inRootName == nil) {
		KUInt32 theCount = CountRootFolders();
		if (theCount > 0) {
			return TDCLFSItemRef(new TDCLWin32RootDir(this, (const char*)GetRootFolderName(0)));
		}
		return TDCLFSItemRef();
	}
	
	char thePath[10];
	KUInt32 i = 0;
	while (inRootName[i] != 0 && i < 9) {
		thePath[i] = (char)inRootName[i];
		i++;
	}
	thePath[i] = '\0';
	
	if (i > 0 && thePath[i-1] != '\\' && i < 9) {
		thePath[i] = '\\';
		thePath[i+1] = '\0';
	}

	return TDCLFSItemRef(new TDCLWin32RootDir(this, thePath));
}

KUInt32 TDCLWin32Files::CountRootFolders() const
{
	KUInt32 theDrives = ::GetLogicalDrives();
	KUInt32 theCount = 0;
	for (int i = 0; i < 32; i++) {
		if (theDrives & (1 << i)) {
			theCount++;
		}
	}
	return theCount;
}

const KUInt16* TDCLWin32Files::GetRootFolderName(KUInt32 inFolderIndex) const
{
	KUInt32 theDrives = ::GetLogicalDrives();
	KUInt32 theCurrentIndex = 0;
	
	static KUInt16 theName[4]; // "X:\"

	for (int i = 0; i < 32; i++) {
		if (theDrives & (1 << i)) {
			if (theCurrentIndex == inFolderIndex) {
				theName[0] = (KUInt16)('A' + i);
				theName[1] = ':';
				theName[2] = '\\';
				theName[3] = 0;
				return theName;
			}
			theCurrentIndex++;
		}
	}
	return nil;
}
