// ==============================
// Fichier:			TMappedFile.cp
// Projet:			K
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			14/2/2005
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
// The Original Code is TMappedFile.cp.
// 
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2005 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org>
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TMappedFile.cp,v 1.4 2005/06/07 05:53:31 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TMappedFile.h"

// ANSI C & POSIX
#include <fcntl.h>
#include <sys/types.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#if TARGET_OS_WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifndef O_ACCMODE
#define O_ACCMODE 0x0003
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TMappedFile( const char*, size_t, int, void* )
// -------------------------------------------------------------------------- //
TMappedFile::TMappedFile(
				const char* inFilePath,
				size_t inSize /* = 0 */,
				int inFlags /* = O_RDONLY */,
				void* preferredAddress /* = nullptr */ )
{
	(void) Map( inFilePath, inSize, inFlags, preferredAddress );
}

// -------------------------------------------------------------------------- //
//  * ~TMappedFile( void )
// -------------------------------------------------------------------------- //
TMappedFile::~TMappedFile( void )
{
	Unmap();
}

// -------------------------------------------------------------------------- //
//  * Map()
// -------------------------------------------------------------------------- //
int
TMappedFile::Map(
				const char* inFilePath,
				size_t inSize,
				int inFlags,
				void* preferredAddress )
{
	// Unmap any existing mapping first
	Unmap();

	mReadOnly = ((inFlags & O_ACCMODE) == O_RDONLY);
	mSize = inSize;
	mCreated = false;

	int openFlags = inFlags | O_BINARY;
#if TARGET_OS_WIN32
	mFileFd = ::open( inFilePath, openFlags, _S_IREAD | _S_IWRITE );
#else
	mFileFd = ::open( inFilePath, openFlags, 0777 );
#endif

	if (mFileFd < 0)
	{
		return -1;
	}

	// Get the current file size.
	off_t theSize = ::lseek( mFileFd, 0, SEEK_END );
	if (theSize < 0)
	{
		Unmap();
		return -1;
	}
	(void) ::lseek( mFileFd, 0, SEEK_SET );

	// If the caller asked for 0 size, map the whole existing file.
	if (inSize == 0)
	{
		mSize = (size_t) theSize;
	} else if (inSize > (size_t) theSize) {
		// If growing the file is requested and allowed:
		if (mReadOnly)
		{
			mSize = (size_t) theSize;
		} else {
			(void) ::lseek( mFileFd, inSize - 1, SEEK_SET );
			char someByte = 0;
			(void) ::write( mFileFd, &someByte, 1 );
			(void) ::lseek( mFileFd, 0, SEEK_SET );
			mCreated = true;
		}
	}

	if (mSize == 0)
	{
		// Cannot map an empty file (mmap fails with EINVAL)
		mBuffer = nullptr;
		mMapped = false;
		return 0;
	}

#if TARGET_OS_WIN32
	HANDLE hFile = (HANDLE) ::_get_osfhandle( mFileFd );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		Unmap();
		return -1;
	}

	DWORD flProtect = mReadOnly ? PAGE_READONLY : PAGE_READWRITE;
	mFileMapping = ::CreateFileMappingA(
			hFile,
			nullptr,
			flProtect,
			(DWORD) (mSize >> 32),
			(DWORD) (mSize & 0xFFFFFFFF),
			nullptr );

	if (mFileMapping != nullptr)
	{
		DWORD dwDesiredAccess = mReadOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE);
		if (preferredAddress != nullptr)
		{
			mBuffer = ::MapViewOfFileEx( mFileMapping, dwDesiredAccess, 0, 0, mSize, preferredAddress );
		}
		if (mBuffer == nullptr)
		{
			mBuffer = ::MapViewOfFile( mFileMapping, dwDesiredAccess, 0, 0, mSize );
		}

		if (mBuffer != nullptr)
		{
			mMapped = true;
		}
	}
#else
	// POSIX mmap
	int theProt = 0;
	int theFlags = MAP_SHARED;
	int theMode = inFlags & O_ACCMODE;

	if (theMode == O_RDONLY)
	{
		theProt = PROT_READ;
	} else if (theMode == O_WRONLY) {
		theProt = PROT_WRITE;
	} else if (theMode == O_RDWR) {
		theProt = PROT_READ | PROT_WRITE;
	}

	mBuffer = ::mmap(
				preferredAddress,
				mSize,
				theProt,
				theFlags,
				mFileFd,
				0 );

	if (mBuffer != MAP_FAILED)
	{
		mMapped = true;
	} else {
		mBuffer = nullptr;
		mMapped = false;
	}
#endif

	// Fallback to malloc & read if mapping failed
	if (!mMapped)
	{
		mBuffer = ::malloc( mSize );
		if (mBuffer != nullptr)
		{
			if (((size_t) ::read( mFileFd, mBuffer, mSize )) != mSize)
			{
				::free( mBuffer );
				mBuffer = nullptr;
				Unmap();
				return -1;
			}
		} else {
			Unmap();
			return -1;
		}
	}

	return 0;
}

// -------------------------------------------------------------------------- //
//  * Unmap()
// -------------------------------------------------------------------------- //
void
TMappedFile::Unmap( void )
{
	if (mBuffer != nullptr)
	{
		if (mMapped)
		{
#if TARGET_OS_WIN32
			::UnmapViewOfFile( mBuffer );
#else
			if (::munmap( mBuffer, mSize ) < 0)
			{
				(void) ::fprintf(
							stderr,
							"Error with munmap (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
#endif
		} else {
			// Write the content if it wasn't read only.
			if (!mReadOnly && mFileFd >= 0)
			{
				(void) ::lseek( mFileFd, 0, SEEK_SET );
				(void) ::write( mFileFd, mBuffer, mSize );
			}

			::free( mBuffer );
		}
		mBuffer = nullptr;
	}

#if TARGET_OS_WIN32
	if (mFileMapping != nullptr)
	{
		::CloseHandle( mFileMapping );
		mFileMapping = nullptr;
	}
#endif

	// Close the file.
	if (mFileFd >= 0)
	{
		::close( mFileFd );
		mFileFd = -1;
	}

	mSize = 0;
	mMapped = false;
	mReadOnly = false;
	mCreated = false;
}

// -------------------------------------------------------------------------- //
//  * Sync( void ) const
// -------------------------------------------------------------------------- //
void
TMappedFile::Sync( void ) const
{
	if ((mBuffer != nullptr) && (!mReadOnly))
	{
		if (mMapped)
		{
#if TARGET_OS_WIN32
			if (!::FlushViewOfFile( mBuffer, mSize ))
			{
				(void) ::fprintf(
							stderr,
							"Error with FlushViewOfFile (%lu) - %s:%i\n",
							::GetLastError(), __FILE__, __LINE__ );
				::abort();
			}
#else
			if (::msync( mBuffer, mSize, MS_SYNC ) < 0)
			{
				(void) ::fprintf(
							stderr,
							"Error with msync (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
#endif
		} else {
			// Write the content for non-mapped buffer.
			if (::lseek( mFileFd, 0, SEEK_SET ) < 0)
			{
				(void) ::fprintf(
							stderr,
							"Error with lseek (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
			if (::write( mFileFd, mBuffer, mSize ) != (ssize_t) mSize)
			{
				(void) ::fprintf(
							stderr,
							"Error with write (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
		}
	}
}

// ============================= //
// Disk crisis, please clean up! //
// ============================= //
