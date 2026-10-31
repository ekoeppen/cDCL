// ==============================
// Fichier:			UTestKMappedFile.cpp
// Projet:			cDCL
// Ecrit par:		Eckhart Koeppen (eck@40hz.org)
// 
// Cr le:			31/10/2026
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
// The Original Code is UTestKMappedFile.cpp.
// 
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
// 
// ***** END LICENSE BLOCK *****

#include "UTestKMappedFile.h"
#include <K/Misc/TMappedFile.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static const char* kTestTempPath = "/tmp/cdcl_test_mappedfile.bin";

void
UTestKMappedFile::Run(void)
{
	(void) ::printf("--- Testing TMappedFile ---\n");
	TestMapAndGrow();
	TestReadWriteSync();
	TestReadOnly();
	(void) ::printf("--- TMappedFile tests passed ---\n");
}

void
UTestKMappedFile::TestMapAndGrow(void)
{
	(void) ::printf("Testing Map and Grow...\n");
	(void) ::unlink(kTestTempPath);

	{
		// Create a new mapped file with initial size 1024 bytes
		TMappedFile mapped(kTestTempPath, 1024, O_RDWR | O_CREAT);
		assert(mapped.GetBuffer() != nullptr);
		assert(mapped.GetSize() == 1024);
		assert(mapped.GetCreated() == true);

		// Write known pattern
		char* buf = (char*) mapped.GetBuffer();
		for (size_t i = 0; i < 1024; ++i)
		{
			buf[i] = (char)(i & 0xFF);
		}
		mapped.Sync();
	}

	// Reopen and grow file to 2048 bytes
	{
		TMappedFile mapped;
		int err = mapped.Map(kTestTempPath, 2048, O_RDWR);
		assert(err == 0);
		assert(mapped.GetBuffer() != nullptr);
		assert(mapped.GetSize() == 2048);
		assert(mapped.GetCreated() == true); // Grown

		char* buf = (char*) mapped.GetBuffer();
		// Verify original 1024 bytes
		for (size_t i = 0; i < 1024; ++i)
		{
			assert(buf[i] == (char)(i & 0xFF));
		}

		// Write to second half
		for (size_t i = 1024; i < 2048; ++i)
		{
			buf[i] = (char)((i ^ 0xAA) & 0xFF);
		}
		mapped.Sync();
	}

	(void) ::unlink(kTestTempPath);
}

void
UTestKMappedFile::TestReadWriteSync(void)
{
	(void) ::printf("Testing Read/Write/Sync...\n");
	(void) ::unlink(kTestTempPath);

	const size_t kSize = 4096;
	{
		TMappedFile mapped(kTestTempPath, kSize, O_RDWR | O_CREAT);
		assert(mapped.GetBuffer() != nullptr);
		assert(mapped.GetSize() == kSize);

		char* buf = (char*) mapped.GetBuffer();
		(void) ::memset(buf, 0x42, kSize);
		mapped.Sync();
		mapped.Unmap();
		assert(mapped.GetBuffer() == nullptr);
		assert(mapped.GetSize() == 0);
	}

	// Verify persisted contents directly via POSIX read
	{
		int fd = ::open(kTestTempPath, O_RDONLY);
		assert(fd >= 0);
		char readBuf[kSize];
		ssize_t bytesRead = ::read(fd, readBuf, kSize);
		assert(bytesRead == (ssize_t)kSize);
		for (size_t i = 0; i < kSize; ++i)
		{
			assert(readBuf[i] == 0x42);
		}
		::close(fd);
	}

	(void) ::unlink(kTestTempPath);
}

void
UTestKMappedFile::TestReadOnly(void)
{
	(void) ::printf("Testing Read-Only mapping...\n");
	(void) ::unlink(kTestTempPath);

	const size_t kSize = 512;
	// Create file first
	{
		TMappedFile mapped(kTestTempPath, kSize, O_RDWR | O_CREAT);
		char* buf = (char*) mapped.GetBuffer();
		(void) ::memset(buf, 0x5A, kSize);
		mapped.Sync();
	}

	// Open read-only with size 0 (map whole file)
	{
		TMappedFile mapped(kTestTempPath, 0, O_RDONLY);
		assert(mapped.GetBuffer() != nullptr);
		assert(mapped.GetSize() == kSize);
		assert(mapped.GetCreated() == false);

		const char* buf = (const char*) mapped.GetBuffer();
		for (size_t i = 0; i < kSize; ++i)
		{
			assert(buf[i] == 0x5A);
		}
	}

	(void) ::unlink(kTestTempPath);
}
