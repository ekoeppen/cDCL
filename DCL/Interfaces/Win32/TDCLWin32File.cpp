// ==============================
// Fichier:			TDCLWin32File.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			10/05/2026
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLGenericIOException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <K/Unicode/UUTF16CStr.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

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
	free(mPath);
}

void TDCLWin32File::Open( Boolean inReadOnly )
{
	mFile = fopen(mPath, inReadOnly ? "rb" : "r+b");
	if (!mFile) throw DCLIOException;
	mReadOnly = inReadOnly;
}

void TDCLWin32File::Create()
{
	mFile = fopen(mPath, "w+b");
	if (!mFile) throw DCLIOException;
	mReadOnly = false;
}

void TDCLWin32File::Delete()
{
	Close();
	remove(mPath);
}

Boolean TDCLWin32File::IsOpen() const
{
	return mFile != NULL;
}

Boolean TDCLWin32File::IsReadOnly() const
{
	return mReadOnly;
}

KUInt64 TDCLWin32File::GetLength()
{
	if (!mFile) return 0;
	struct _stat64 st;
	if (_stat64(mPath, &st) != 0) return 0;
	return st.st_size;
}

void TDCLWin32File::Close()
{
	if (mFile) {
		fclose(mFile);
		mFile = NULL;
	}
}

void TDCLWin32File::Read( void* outBuffer, KUInt32* ioCount )
{
	if (!mFile) throw DCLIOException;
	size_t readCount = fread(outBuffer, 1, *ioCount, mFile);
	*ioCount = (KUInt32)readCount;
}

void TDCLWin32File::Write( const void* inBuffer, KUInt32* ioCount )
{
	if (!mFile) throw DCLIOException;
	size_t writtenCount = fwrite(inBuffer, 1, *ioCount, mFile);
	*ioCount = (KUInt32)writtenCount;
}

void TDCLWin32File::FlushOutput()
{
	if (mFile) fflush(mFile);
}

KUInt64 TDCLWin32File::GetPosition()
{
	if (!mFile) return 0;
	return _ftelli64(mFile);
}

void TDCLWin32File::SetPosition( KUInt64 inPosition )
{
	if (!mFile) throw DCLPosition;
	if (_fseeki64(mFile, inPosition, SEEK_SET) != 0) throw DCLPosition;
}

KSInt64 TDCLWin32File::GetCursor()
{
	return (KSInt64)GetPosition();
}

void TDCLWin32File::SetCursor( KSInt64 inOffset, ECursorMode inMode )
{
	if (!mFile) throw DCLPosition;
	int origin;
	switch (inMode) {
		case kFromStart: origin = SEEK_SET; break;
		case kFromCursor: origin = SEEK_CUR; break;
		case kFromLEOF: origin = SEEK_END; break;
		default: throw DCLPosition;
	}
	if (_fseeki64(mFile, inOffset, origin) != 0) throw DCLPosition;
}

TDCLNSRef TDCLWin32File::GetKind() const
{
	return TDCLNSRef::MakeString("File");
}

KUInt32 TDCLWin32File::GetCreationDate() const
{
	return 0; // Stub
}

KUInt32 TDCLWin32File::GetModificationDate() const
{
	return 0; // Stub
}

TDCLNSRef TDCLWin32File::GetStringPath() const
{
	return TDCLNSRef::MakeString(mPath);
}

const KUInt16* TDCLWin32File::GetName() const
{
	return nil; // Stub
}

KUInt16* TDCLWin32File::MakeName() const
{
	return nil; // Stub
}
