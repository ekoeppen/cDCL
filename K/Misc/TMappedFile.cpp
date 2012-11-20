// ==============================
// Fichier:			TMappedFile.cp
// Projet:			K
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// CrŽŽ le:			14/2/2005
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
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TMappedFile.cp,v 1.4 2005/06/07 05:53:31 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TMappedFile.h"

// ANSI C & POSIX
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TMappedFile( const char*, size_t, int )
// -------------------------------------------------------------------------- //
TMappedFile::TMappedFile(
				const char* inFilePath,
				size_t inSize  /* = 0 */,
				int inFlags /* = O_RDONLY */,
				void* preferredAddress /* = NULL */ )
	:
		mBuffer( NULL ),
		mSize( inSize ),
		mMapped( false ),
		mReadOnly( (inFlags & O_ACCMODE) == O_RDONLY ),
		mCreated( false ),
		mFileFd( -1 )
{
	do {
		// Open the file.
		mFileFd = ::open( inFilePath, inFlags, 0777 );

		if (mFileFd < 0)
		{
			break;
		}

		// Get the size.
		off_t theSize = ::lseek( mFileFd, 0, SEEK_END );
		if (theSize < 0)
		{
			break;
		}
		(void) ::lseek( mFileFd, 0, SEEK_SET );

		// If the size is 0, set mSize to its size.
		if (inSize == 0)
		{
			mSize = theSize;
		} else if (inSize > (size_t) theSize) {
			// otherwise, grow the file if required/permitted.
			if (mReadOnly)
			{
				mSize = theSize;
			} else {
				(void) ::lseek( mFileFd, inSize - 1, SEEK_SET );
				char someByte = 0;
				(void) ::write( mFileFd, &someByte, 1 );
				(void) ::lseek( mFileFd, 0, SEEK_SET );
				mCreated = true;
			}
		}

		// (Try to) map the file.
		int theProt = 0;
		int theMode = inFlags & O_ACCMODE;
#ifdef MAP_FILE
		int theFlags = MAP_FILE;
#else
		// Zaurus headers don't define MAP_FILE.
		int theFlags = 0;
#endif
		if (theMode == O_RDONLY)
		{
			theProt = PROT_READ;
		} else if (theMode == O_WRONLY) {
			theProt = PROT_WRITE;
			theFlags |= MAP_SHARED;
		} else if (theMode == O_RDWR) {
			theProt = PROT_READ | PROT_WRITE;
			theFlags |= MAP_SHARED;
		}
		
		mBuffer = ::mmap(
						preferredAddress /* addr */,
						mSize,
						theProt,
						theFlags,
						mFileFd,
						0 );
		mMapped = true;
	
		if (((int) mBuffer) == -1)
		{
			mMapped = false;

			// Read it the usual way.
			mBuffer = ::malloc( mSize );
			if (mBuffer)
			{
				if (((size_t)::read( mFileFd, mBuffer, mSize )) != mSize)
				{
					// Read failed.
					::free( mBuffer );
					mBuffer = NULL;
					break;
				}
			}
		}
	} while (false);
}

// -------------------------------------------------------------------------- //
//  * ~TMappedFile( void )
// -------------------------------------------------------------------------- //
TMappedFile::~TMappedFile( void )
{
	if (mBuffer)
	{
		// Unmap the file/free the pointer.
		if (mMapped)
		{
			// use munmap
			if (::munmap( mBuffer, mSize ) < 0)
			{
				(void) ::fprintf(
							stderr,
							"Error with munmap (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
		} else {
			// Write the content if it wasn't read only.
			if (!mReadOnly)
			{
				(void) ::lseek( mFileFd, 0, SEEK_SET );
				(void) ::write( mFileFd, mBuffer, mSize );
			}

			::free( mBuffer );
		}
	}
	
	// Close the file.
	if (mFileFd > 0)
	{
		::close( mFileFd );
	}
}

// -------------------------------------------------------------------------- //
//  * Sync( void ) const
// -------------------------------------------------------------------------- //
void
TMappedFile::Sync( void ) const
{
	if ((mBuffer) && (!mReadOnly))
	{
		if (mMapped)
		{
			// use msync
			if (::msync( mBuffer, mSize, MS_SYNC ) < 0)
			{
				(void) ::fprintf(
							stderr,
							"Error with msync (%i) - %s:%i\n",
							errno, __FILE__, __LINE__ );
				::abort();
			}
		} else {
			// Write the content.
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
