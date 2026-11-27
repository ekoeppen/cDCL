// ==============================
// File:            UTestAIF.cpp
// Project:         Desktop Connection Library
// Written by:      Eckhart Koeppen (eck@40hz.org)
//
// Tab width:       4 spaces
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
// The Original Code is UTestAIF.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen (eck@40hz.org).
// Portions created by the Initial Developer are Copyright (C) 2026
// the Initial Developer. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****
// ===========

#include <doctest/doctest.h>

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgRelocatablePart.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <K/Defines/UByteSex.h>

#include "TAIFFile.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
class ScopedTempFile
{
public:
	explicit ScopedTempFile( const char* inName, const std::vector<KUInt8>& inBytes )
	{
		std::filesystem::path dir = std::filesystem::temp_directory_path();
		mPath = ( dir / inName ).string();
		FILE* f = ::fopen( mPath.c_str(), "wb" );
		REQUIRE( f != nullptr );
		if ( !inBytes.empty() )
		{
			REQUIRE( ::fwrite( inBytes.data(), 1, inBytes.size(), f ) == inBytes.size() );
		}
		::fclose( f );
	}

	~ScopedTempFile()
	{
		std::error_code ec;
		std::filesystem::remove( mPath, ec );
	}

	const char*
	Path() const
	{
		return mPath.c_str();
	}

private:
	std::string mPath;
};

void
WriteBE32( std::vector<KUInt8>& ioBuf, size_t inOffset, KUInt32 inValue )
{
	KUInt32 be = UByteSex_ToBigEndian( inValue );
	std::memcpy( ioBuf.data() + inOffset, &be, 4 );
}

KUInt32
ReadBE32( const KUInt8* inPtr )
{
	KUInt32 be = 0;
	std::memcpy( &be, inPtr, 4 );
	return UByteSex_FromBigEndian( be );
}

void
AppendBE32( std::vector<KUInt8>& ioBuf, KUInt32 inValue )
{
	size_t off = ioBuf.size();
	ioBuf.resize( off + 4 );
	WriteBE32( ioBuf, off, inValue );
}

std::vector<KUInt8>
BuildExecutableAIF( KUInt32 inImageBase, const std::vector<KUInt32>& inRelocList, bool inIncludeSentinel,
                    KUInt32 inDebugSize = 0, KUInt32 inZeroInitSize = 0 )
{
	const KUInt32 kHeaderSize = 0x80;
	const KUInt32 kROUserSize = 0x20;
	const KUInt32 kRWSize = 0x08;
	const KUInt32 kStubSize = 0xB8;
	const KUInt32 kStubOffset = kHeaderSize + kROUserSize + kRWSize + inDebugSize;

	std::vector<KUInt8> buf( kStubOffset + kStubSize, 0 );
	WriteBE32( buf, 0x00, 0xE1A00000 ); // NOP decompress
	KUInt32 blWordOffset = ( kStubOffset - 0x0C ) >> 2;
	WriteBE32( buf, 0x04, 0xEB000000 | ( blWordOffset & 0x00FFFFFF ) );
	WriteBE32( buf, 0x08, 0xE1A00000 );
	WriteBE32( buf, 0x0C, 0xEB00001B ); // BL entry
	WriteBE32( buf, 0x10, 0xEF000011 ); // SWI OS_Exit
	WriteBE32( buf, 0x14, kHeaderSize + kROUserSize );
	WriteBE32( buf, 0x18, kRWSize );
	WriteBE32( buf, 0x1C, inDebugSize );
	WriteBE32( buf, 0x20, inZeroInitSize );
	WriteBE32( buf, 0x28, inImageBase );
	WriteBE32( buf, 0x30, 0x00000020 );

	// User RO (0x80..0x9F) and RW (0xA0..0xA7)
	WriteBE32( buf, 0x80, 0xE3A00000 );
	WriteBE32( buf, 0x90, 0xE3A0002A );         // MyCallback at payload offset 0x10
	WriteBE32( buf, 0x98, inImageBase + 0x90 ); // pointer at payload offset 0x18
	WriteBE32( buf, 0x9C, inImageBase + 0xA0 ); // pointer at payload offset 0x1C
	WriteBE32( buf, 0xA0, 0x12345678 );
	WriteBE32( buf, 0xA4, inImageBase + 0x90 ); // pointer at payload offset 0x24

	// Self-reloc stub signature instruction at stub + 0x98
	WriteBE32( buf, kStubOffset + 0x98, 0xE28F2018 );

	for ( KUInt32 r : inRelocList )
	{
		AppendBE32( buf, r );
	}
	if ( inIncludeSentinel )
	{
		AppendBE32( buf, 0xFFFFFFFF );
	}
	return buf;
}

std::vector<KUInt8>
BuildNonExecutableAIF( KUInt32 inImageBase, const std::vector<KUInt32>& inRelocList, KUInt32 inDebugSize = 0,
                       KUInt32 inZeroInitSize = 0 )
{
	const KUInt32 kHeaderSize = 0x80;
	const KUInt32 kROSize = 0x20;
	const KUInt32 kRWSize = 0x08;
	const KUInt32 kRelocTableOffset = kHeaderSize + kROSize + kRWSize + inDebugSize;

	std::vector<KUInt8> buf( kRelocTableOffset, 0 );
	WriteBE32( buf, 0x00, 0xE1A00000 );
	WriteBE32( buf, 0x04, kRelocTableOffset );
	WriteBE32( buf, 0x08, 0xE1A00000 );
	WriteBE32( buf, 0x0C, 0x00000000 ); // non-BL entry offset
	WriteBE32( buf, 0x10, 0xEF000011 );
	WriteBE32( buf, 0x14, kROSize );
	WriteBE32( buf, 0x18, kRWSize );
	WriteBE32( buf, 0x1C, inDebugSize );
	WriteBE32( buf, 0x20, inZeroInitSize );
	WriteBE32( buf, 0x28, inImageBase );
	WriteBE32( buf, 0x30, 0x00000020 );

	WriteBE32( buf, 0x80, 0xE3A00000 );
	WriteBE32( buf, 0x90, 0xE3A0002A );
	WriteBE32( buf, 0x98, inImageBase + 0x10 );
	WriteBE32( buf, 0x9C, inImageBase + 0x20 );
	WriteBE32( buf, 0xA0, 0x12345678 );
	WriteBE32( buf, 0xA4, inImageBase + 0x10 );

	AppendBE32( buf, (KUInt32)inRelocList.size() );
	for ( KUInt32 r : inRelocList )
	{
		AppendBE32( buf, r );
	}
	return buf;
}
} // namespace

TEST_SUITE( "TAIFFile" )
{
	TEST_CASE( "Parses executable self-relocatable AIF and filters debug entries" )
	{
		std::vector<KUInt8> aifBytes =
		    BuildExecutableAIF( 0x8000, { 0x000000C8, 0x000000A4, 0x0000009C, 0x00000098 }, true, 0x20, 0x04 );
		ScopedTempFile tmp( "cdcl_test_exec.aif", aifBytes );

		TDCLPlatformFiles filesIntf;
		TAIFFile aif( &filesIntf, tmp.Path() );
		aif.Open();

		CHECK( aif.IsExecutable() == true );
		CHECK( aif.GetPayloadBaseAddress() == 0x8080 );
		CHECK( aif.GetPayloadSize() == 0x2C ); // 0x20 RO + 0x08 RW + 0x04 ZI

		std::vector<KUInt8> payload( aif.GetPayloadSize(), 0xAA );
		aif.ReadPayload( payload.data() );
		CHECK( ReadBE32( payload.data() + 0x28 ) == 0 ); // ZeroInit tail zeroed

		const std::vector<KUInt32>& relocs = aif.GetRelocations();
		REQUIRE( relocs.size() == 3 );
		CHECK( relocs[0] == 0x24 );
		CHECK( relocs[1] == 0x1C );
		CHECK( relocs[2] == 0x18 );

		// Round-trip through TDCLPackage -> Rex relocation at 0x00800100
		TDCLPkgRelocatablePart* part =
		    new TDCLPkgRelocatablePart( aif.GetPayloadBaseAddress(), payload.data(), aif.GetPayloadSize() );
		part->SetRelocations( (KUInt32)relocs.size(), relocs.data(), aif.GetPayloadBaseAddress() );

		TDCLPackage pkg;
		pkg.SetNOS1Compatible( false );
		pkg.SetPackageID( TDCLPackage::kDefaultID );
		pkg.SetPackageVersion( 1 );
		pkg.SetPackageName( "ExecAIFTest" );
		pkg.AddPart( 'prot', TDCLPackage::kPartProtocolPart, part );

		TDCLMemStream pkgStream;
		pkg.WriteToStream( &pkgStream );
		pkgStream.SetCursor( 0, TDCLMemStream::kFromStart );

		TDCLPackage reloaded( &pkgStream );
		TDCLMemStream rexStream;
		const KUInt32 rexLoadAddr = 0x00800100;
		reloaded.WriteToStream( &rexStream, rexLoadAddr );

		const KUInt8* rexBytes = (const KUInt8*)rexStream.GetBuffer();
		KUInt32 partOffset = rexStream.GetBufferSize() - aif.GetPayloadSize();
		CHECK( ReadBE32( rexBytes + partOffset + 0x18 ) == rexLoadAddr + partOffset + 0x10 );
		CHECK( ReadBE32( rexBytes + partOffset + 0x1C ) == rexLoadAddr + partOffset + 0x20 );
		CHECK( ReadBE32( rexBytes + partOffset + 0x24 ) == rexLoadAddr + partOffset + 0x10 );
	}

	TEST_CASE( "Parses non-executable AIF and filters debug entries" )
	{
		std::vector<KUInt8> aifBytes =
		    BuildNonExecutableAIF( 0x0000, { 0x00000030, 0x00000024, 0x0000001C, 0x00000018 }, 0x10, 0x04 );
		ScopedTempFile tmp( "cdcl_test_bin.aif", aifBytes );

		TDCLPlatformFiles filesIntf;
		TAIFFile aif( &filesIntf, tmp.Path() );
		aif.Open();

		CHECK( aif.IsExecutable() == false );
		CHECK( aif.GetPayloadBaseAddress() == 0 );
		CHECK( aif.GetPayloadSize() == 0x2C );

		const std::vector<KUInt32>& relocs = aif.GetRelocations();
		REQUIRE( relocs.size() == 3 );
		CHECK( relocs[0] == 0x24 );
		CHECK( relocs[1] == 0x1C );
		CHECK( relocs[2] == 0x18 );
	}

	TEST_CASE( "Parses non-relocatable AIF with zero relocations" )
	{
		std::vector<KUInt8> aifBytes( 0xA8, 0 );
		WriteBE32( aifBytes, 0x00, 0xE1A00000 );
		WriteBE32( aifBytes, 0x04, 0xE1A00000 );
		WriteBE32( aifBytes, 0x08, 0xE1A00000 );
		WriteBE32( aifBytes, 0x0C, 0x00000000 );
		WriteBE32( aifBytes, 0x14, 0x00000020 );
		WriteBE32( aifBytes, 0x18, 0x00000008 );
		ScopedTempFile tmp( "cdcl_test_noreloc.aif", aifBytes );

		TDCLPlatformFiles filesIntf;
		TAIFFile aif( &filesIntf, tmp.Path() );
		aif.Open();
		CHECK( aif.GetRelocations().empty() );
	}

	TEST_CASE( "Rejects malformed AIF headers and relocation tables" )
	{
		TDCLPlatformFiles filesIntf;

		SUBCASE( "Truncated executable relocation list missing 0xFFFFFFFF sentinel" )
		{
			std::vector<KUInt8> aifBytes = BuildExecutableAIF( 0x8000, { 0x00000098 }, false );
			ScopedTempFile tmp( "cdcl_test_missing_sentinel.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}

		SUBCASE( "32-bit overflow in ZeroInitSize" )
		{
			std::vector<KUInt8> aifBytes = BuildNonExecutableAIF( 0, {}, 0, 0xFFFFFFF0 );
			ScopedTempFile tmp( "cdcl_test_overflow.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}

		SUBCASE( "Relocation offset in trailing partial word of unaligned payload" )
		{
			// RO=6 (not 4-byte aligned), reloc=4 leaves only 2 bytes in payload.
			std::vector<KUInt8> aifBytes( 0x80 + 6 + 8, 0 );
			WriteBE32( aifBytes, 0x00, 0xE1A00000 );
			WriteBE32( aifBytes, 0x04, 0x80 + 6 );
			WriteBE32( aifBytes, 0x08, 0xE1A00000 );
			WriteBE32( aifBytes, 0x14, 6 );
			WriteBE32( aifBytes, 0x80 + 6, 1 );
			WriteBE32( aifBytes, 0x80 + 6 + 4, 4 );
			ScopedTempFile tmp( "cdcl_test_partial_word.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}

		SUBCASE( "Unaligned relocation offset" )
		{
			std::vector<KUInt8> aifBytes = BuildNonExecutableAIF( 0, { 0x00000012 } );
			ScopedTempFile tmp( "cdcl_test_unaligned.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}

		SUBCASE( "Compressed or non-big-endian AIF" )
		{
			std::vector<KUInt8> aifBytes = BuildNonExecutableAIF( 0, {} );
			WriteBE32( aifBytes, 0x00, 0xEB000000 );
			ScopedTempFile tmp( "cdcl_test_compressed.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}

		SUBCASE( "Non-executable relocation table overlapping debug section" )
		{
			std::vector<KUInt8> aifBytes = BuildNonExecutableAIF( 0, {}, 0x10 );
			// Point selfRelocCode inside debug section before expectedEnd + debugSize
			WriteBE32( aifBytes, 0x04, 0xA8 );
			ScopedTempFile tmp( "cdcl_test_bad_table_off.aif", aifBytes );
			TAIFFile aif( &filesIntf, tmp.Path() );
			CHECK_THROWS_AS( aif.Open(), std::runtime_error );
		}
	}
}
