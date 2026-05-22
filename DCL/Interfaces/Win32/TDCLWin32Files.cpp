// ==============================
// Fichier:			TDCLWin32Files.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			10/05/2026
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

const KUInt16 TDCLWin32Files::kRootFolderName[3] = { 'C', ':', 0 };

TDCLWin32Files::TDCLWin32Files()
	: mRootFolder(nil)
{
}

TDCLWin32Files::~TDCLWin32Files()
{
}

KUInt32 TDCLWin32Files::GetKind()
{
	return 4; // TDCLDockCommand::kWin_DType
}

TDCLFSItemRef TDCLWin32Files::GetRootFolder(const KUInt16* inRootName)
{
	return mRootFolder;
}

KUInt32 TDCLWin32Files::CountRootFolders() const
{
	return 1;
}

const KUInt16* TDCLWin32Files::GetRootFolderName(KUInt32 inFolderIndex) const
{
	return kRootFolderName;
}
