// ==============================
// Fichier:			TDCLWin32File.cpp
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
// The Original Code is TDCLWin32File.cpp.
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
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Interfaces/Win32/TDCLWin32Dir.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <K/Unicode/UUTF16CStr.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <windows.h>

// Minutes between 1904 (Newton epoch) and 1970 (Unix/Windows epoch)
const KUInt32 kMinutesBetween1904and1970 = 34714080;

TDCLWin32File::TDCLWin32File( IDCLFiles* inFilesIntf, const char* inPath )
	: TDCLFile( inFilesIntf, nil ),
	  mPath( _strdup(inPath) ),
	  mFile( NULL ),
	  mReadOnly( false )
{
}

TDCLWin32File::~TDCLWin32File()
{
	Close();
	::free(mPath);
}

void TDCLWin32File::Open( Boolean inReadOnly )
{
	mFile = ::fopen(mPath, inReadOnly ? "rb" : "r+b");
	if (mFile == nil)
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
				
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	mReadOnly = inReadOnly;
}

void TDCLWin32File::Create()
{
	mFile = ::fopen(mPath, "w+b");
	if (mFile == nil)
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	mReadOnly = false;
}

void TDCLWin32File::Delete()
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	
	if (::remove( mPath ))
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case EACCES:
				throw DCLPlatformPermission( theErr );
				
			case EROFS:
				throw DCLPlatformReadOnly( theErr );
				
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
}

Boolean TDCLWin32File::IsOpen() const
{
	return mFile != NULL;
}

Boolean TDCLWin32File::IsReadOnly() const
{
	return (mFile != nil) && mReadOnly;
}

KUInt64 TDCLWin32File::GetLength()
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	struct _stat64 theFileInfos;
	if (_stat64(mPath, &theFileInfos) != 0)
	{
		int theErr = errno;
		
		switch (theErr)
		{
			case ENOENT:
				throw DCLPlatformDoesntExist( theErr );
			
			case EACCES:
				throw DCLPlatformPermission( theErr );
			
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	
	return (KUInt64) theFileInfos.st_size;
}

void TDCLWin32File::Close()
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	::fclose(mFile);
	mFile = NULL;
}

void TDCLWin32File::Read( void* outBuffer, KUInt32* ioCount )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	size_t toRead = *ioCount;
	size_t read = ::fread(outBuffer, 1, *ioCount, mFile);
	if (read != toRead)
	{
		*ioCount = read;
		if (!(feof(mFile)))
		{
			int theErr = ferror(mFile);
			clearerr( mFile );
			throw DCLPlatformUnknownError( theErr );
		}
	}
}

void TDCLWin32File::Write( const void* outBuffer, KUInt32* ioCount )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	if (mReadOnly)
	{
		throw DCLReadOnly;
	}

	size_t toWrite = *ioCount;
	size_t written = ::fwrite( outBuffer, 1, toWrite, mFile );
	if (written != toWrite)
	{
		*ioCount = written;

		int theErr = ferror(mFile);
		clearerr( mFile );
		throw DCLPlatformUnknownError( theErr );
	}
}

void TDCLWin32File::FlushOutput()
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	::fflush(mFile);
}

KUInt64 TDCLWin32File::GetPosition()
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	return _ftelli64(mFile);
}

void TDCLWin32File::SetPosition( KUInt64 inPosition )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	if (_fseeki64(mFile, inPosition, SEEK_SET) != 0)
	{
		throw DCLPosition;
	}
}

KSInt64 TDCLWin32File::GetCursor()
{
	return (KSInt64)GetPosition();
}

void TDCLWin32File::SetCursor( KSInt64 inOffset, ECursorMode inMode )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	int origin;
	switch (inMode) {
		case kFromStart: origin = SEEK_SET; break;
		case kFromCursor: origin = SEEK_CUR; break;
		case kFromLEOF: origin = SEEK_END; break;
		default: throw DCLPosition;
	}
	if (_fseeki64(mFile, inOffset, origin) != 0)
	{
		throw DCLPosition;
	}
}

TDCLNSRef TDCLWin32File::GetKind() const
{
	return TDCLNSRef::MakeString("File");
}

KUInt32 TDCLWin32File::GetCreationDate() const
{
	struct _stat64 theFileInfos;
	if (_stat64(mPath, &theFileInfos) != 0)
	{
		return 0;
	}

	return (KUInt32)(theFileInfos.st_ctime / 60) + kMinutesBetween1904and1970;
}

KUInt32 TDCLWin32File::GetModificationDate() const
{
	struct _stat64 theFileInfos;
	if (_stat64(mPath, &theFileInfos) != 0)
	{
		return 0;
	}

	return (KUInt32)(theFileInfos.st_mtime / 60) + kMinutesBetween1904and1970;
}

TDCLNSRef TDCLWin32File::GetStringPath() const
{
	return TDCLNSRef::MakeString(mPath);
}

const KUInt16* TDCLWin32File::GetName() const
{
	return nil;
}

KUInt16* TDCLWin32File::MakeName() const
{
	const char* theBaseName = ::strrchr(mPath, '\\');
	if (theBaseName)
	{
		theBaseName++;
	}
	else
	{
		theBaseName = mPath;
	}

	size_t theLength = ::strlen(theBaseName) + 1;
	KUInt16* theResult = (KUInt16*) ::malloc(theLength * sizeof(KUInt16));
	UUTF16CStr::FromASCII((const KUInt8*)theBaseName, theResult, theLength);

	return theResult;
}

TDCLFSItemRef TDCLWin32File::MakeParentFolder() const
{
	char* thePath = _strdup(mPath);
	char* theLastSlash = ::strrchr(thePath, '\\');
	TDCLFSItemRef theResult;

	if (theLastSlash)
	{
		*theLastSlash = '\0';
		theResult = TDCLFSItemRef(new TDCLWin32Dir((TDCLWin32Files*)GetFilesIntf(), TDCLFSItemRef(), thePath));
	}
	
	::free(thePath);
	return theResult;
}
