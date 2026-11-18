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

#include <K/Misc/TMappedFile.h>

#include <doctest/doctest.h>

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <system_error>
#include <unistd.h>

namespace {

struct TempFileGuard
{
	std::string path;

	explicit TempFileGuard(const char* inName)
	{
		std::error_code ec;
		std::filesystem::path tempDir = std::filesystem::temp_directory_path(ec);
		if (ec)
		{
			path = inName;
		}
		else
		{
			path = (tempDir / inName).string();
		}
		std::filesystem::remove(path, ec);
	}

	~TempFileGuard()
	{
		std::error_code ec;
		std::filesystem::remove(path, ec);
	}

	const char* c_str() const { return path.c_str(); }
};

} // namespace

TEST_SUITE("KMappedFile")
{
	TEST_CASE("TMappedFile map and grow")
	{
		TempFileGuard tempFile("cdcl_test_mappedfile_grow.bin");

		{
			// Create a new mapped file with initial size 1024 bytes
			TMappedFile mapped(tempFile.c_str(), 1024, O_RDWR | O_CREAT);
			REQUIRE(mapped.GetBuffer() != nullptr);
			CHECK(mapped.GetSize() == 1024);
			CHECK(mapped.GetCreated() == true);

			// Write known pattern
			char* buf = (char*) mapped.GetBuffer();
			for (size_t i = 0; i < 1024; ++i)
			{
				buf[i] = (char) (i & 0xFF);
			}
			mapped.Sync();
		}

		// Reopen and grow file to 2048 bytes
		{
			TMappedFile mapped;
			int err = mapped.Map(tempFile.c_str(), 2048, O_RDWR);
			REQUIRE(err == 0);
			REQUIRE(mapped.GetBuffer() != nullptr);
			CHECK(mapped.GetSize() == 2048);
			CHECK(mapped.GetCreated() == true); // Grown

			char* buf = (char*) mapped.GetBuffer();
			// Verify original 1024 bytes
			for (size_t i = 0; i < 1024; ++i)
			{
				REQUIRE(buf[i] == (char) (i & 0xFF));
			}

			// Write to second half
			for (size_t i = 1024; i < 2048; ++i)
			{
				buf[i] = (char) ((i ^ 0xAA) & 0xFF);
			}
			mapped.Sync();
		}
	}

	TEST_CASE("TMappedFile read write sync")
	{
		TempFileGuard tempFile("cdcl_test_mappedfile_sync.bin");

		const size_t kSize = 4096;
		{
			TMappedFile mapped(tempFile.c_str(), kSize, O_RDWR | O_CREAT);
			REQUIRE(mapped.GetBuffer() != nullptr);
			CHECK(mapped.GetSize() == kSize);

			char* buf = (char*) mapped.GetBuffer();
			(void) ::memset(buf, 0x42, kSize);
			mapped.Sync();
			mapped.Unmap();
			CHECK(mapped.GetBuffer() == nullptr);
			CHECK(mapped.GetSize() == 0);
		}

		// Verify persisted contents directly via POSIX read
		{
			int fd = ::open(tempFile.c_str(), O_RDONLY);
			REQUIRE(fd >= 0);
			char readBuf[kSize];
			ssize_t bytesRead = ::read(fd, readBuf, kSize);
			::close(fd);
			REQUIRE(bytesRead == (ssize_t) kSize);
			for (size_t i = 0; i < kSize; ++i)
			{
				REQUIRE(readBuf[i] == 0x42);
			}
		}
	}

	TEST_CASE("TMappedFile read only")
	{
		TempFileGuard tempFile("cdcl_test_mappedfile_ro.bin");

		const size_t kSize = 512;
		// Create file first
		{
			TMappedFile mapped(tempFile.c_str(), kSize, O_RDWR | O_CREAT);
			REQUIRE(mapped.GetBuffer() != nullptr);
			char* buf = (char*) mapped.GetBuffer();
			(void) ::memset(buf, 0x5A, kSize);
			mapped.Sync();
		}

		// Open read-only with size 0 (map whole file)
		{
			TMappedFile mapped(tempFile.c_str(), 0, O_RDONLY);
			REQUIRE(mapped.GetBuffer() != nullptr);
			CHECK(mapped.GetSize() == kSize);
			CHECK(mapped.GetCreated() == false);

			const char* buf = (const char*) mapped.GetBuffer();
			for (size_t i = 0; i < kSize; ++i)
			{
				REQUIRE(buf[i] == 0x5A);
			}
		}
	}
}
