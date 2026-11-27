// ==============================
// File:            TAIFFile.cpp
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
// The Original Code is TAIFFile.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen (eck@40hz.org).
// Portions created by the Initial Developer are Copyright (C) 2026
// the Initial Developer. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****
// ===========

#include "TAIFFile.h"

#include <cstring>
#include <stdexcept>

// -------------------------------------------------------------------------- //
//  * TAIFFile( IDCLFiles*, const char* )
// -------------------------------------------------------------------------- //
TAIFFile::TAIFFile( TDCLPlatformFiles* inFilesIntf, const char* inPath )
    : mFile( inFilesIntf, inPath ),
      mExecutable( false ),
      mPayloadBaseAddress( 0 ),
      mPayloadFileSize( 0 ),
      mPayloadSize( 0 )
{
}

// -------------------------------------------------------------------------- //
//  * ~TAIFFile( void )
// -------------------------------------------------------------------------- //
TAIFFile::~TAIFFile( void )
{
	if ( mFile.IsOpen() )
	{
		mFile.Close();
	}
}

// -------------------------------------------------------------------------- //
//  * Open( void )
// -------------------------------------------------------------------------- //
void
TAIFFile::Open( void )
{
	mFile.Open( true );

	KUInt64 fileLength = mFile.GetLength();
	if ( fileLength < kAIFHeaderSize )
	{
		throw std::runtime_error( "Input file is too small to be an AIF image" );
	}

	mFile.SetCursor( 0, TDCLFile::kFromStart );

	KUInt32 header[32];
	for ( int i = 0; i < 32; i++ )
	{
		header[i] = mFile.GetLong();
	}

	KUInt32 decompressCode = header[0];
	KUInt32 selfRelocCode = header[1];
	KUInt32 entryPoint = header[3];
	KUInt32 roSize = header[5];
	KUInt32 rwSize = header[6];
	KUInt32 debugSize = header[7];
	KUInt32 zeroInitSize = header[8];
	KUInt32 imageBase = header[10];
	KUInt32 dataBase = header[13];

	if ( decompressCode != kARMNop )
	{
		throw std::runtime_error( "Compressed or non-big-endian AIF files are not supported" );
	}

	mExecutable = ( ( entryPoint & kARMBLMask ) == kARMBLOpcode );

	KUInt32 roUserSize = 0;
	if ( mExecutable )
	{
		if ( roSize < kAIFHeaderSize )
		{
			throw std::runtime_error( "Invalid executable AIF header (ROSize < 0x80)" );
		}
		roUserSize = roSize - kAIFHeaderSize;
		mPayloadBaseAddress = imageBase + kAIFHeaderSize;
	}
	else
	{
		roUserSize = roSize;
		mPayloadBaseAddress = imageBase;
	}

	KUInt64 payloadFileSize64 = (KUInt64)roUserSize + rwSize;
	if ( payloadFileSize64 > 0xFFFFFFFFULL )
	{
		throw std::runtime_error( "AIF image payload size overflows 32 bits" );
	}
	KUInt64 expectedEnd = (KUInt64)kAIFHeaderSize + payloadFileSize64;
	if ( expectedEnd > fileLength )
	{
		throw std::runtime_error( "AIF image payload exceeds file size" );
	}

	if ( rwSize > 0 && dataBase != 0 && dataBase != imageBase + roSize )
	{
		throw std::runtime_error( "Split RO/RW base addresses are not supported" );
	}

	KUInt64 totalPayloadSize64 = payloadFileSize64 + zeroInitSize;
	if ( totalPayloadSize64 > 0xFFFFFFFFULL )
	{
		throw std::runtime_error( "AIF image payload size overflows 32 bits" );
	}

	mPayloadFileSize = (KUInt32)payloadFileSize64;
	mPayloadSize = (KUInt32)totalPayloadSize64;
	mRelocations.clear();

	if ( mExecutable )
	{
		if ( selfRelocCode == kARMNop )
		{
			return;
		}
		if ( ( selfRelocCode & kARMBLMask ) != kARMBLOpcode )
		{
			throw std::runtime_error( "Unsupported executable AIF self-relocation header" );
		}

		// PC = instruction address (0x04) + 8 = 0x0C; signed 24-bit word offset.
		KSInt32 branchOffset = (KSInt32)( ( selfRelocCode & 0x00FFFFFF ) << 8 ) >> 6;
		KSInt64 selfRelocOffset = 0x0C + branchOffset;
		if ( selfRelocOffset < (KSInt64)expectedEnd || (KUInt64)selfRelocOffset + kSelfRelocStubSize + 4 > fileLength )
		{
			throw std::runtime_error( "Invalid executable AIF self-relocation stub offset" );
		}

		mFile.SetCursor( selfRelocOffset + kSelfRelocListAddInstrOffset, TDCLFile::kFromStart );
		KUInt32 addInstr = mFile.GetLong();
		if ( addInstr != kSelfRelocListAddR2PC )
		{
			throw std::runtime_error( "Unrecognized executable AIF self-relocation stub" );
		}

		KUInt64 relocListOffset = (KUInt64)selfRelocOffset + kSelfRelocStubSize;
		mFile.SetCursor( (KSInt64)relocListOffset, TDCLFile::kFromStart );
		bool foundSentinel = false;
		while ( relocListOffset + 4 <= fileLength )
		{
			KUInt32 reloc = mFile.GetLong();
			relocListOffset += 4;
			if ( reloc == 0xFFFFFFFF )
			{
				foundSentinel = true;
				break;
			}
			// Executable AIF relocation offsets are relative to the start of the
			// AIF file (including the 0x80 header). Ignore debug-section entries.
			if ( reloc >= kAIFHeaderSize )
			{
				KUInt32 payloadReloc = reloc - kAIFHeaderSize;
				if ( payloadReloc < mPayloadFileSize )
				{
					if ( mPayloadFileSize < 4 || payloadReloc > mPayloadFileSize - 4 )
					{
						throw std::runtime_error( "AIF relocation offset exceeds payload word bounds" );
					}
					if ( payloadReloc & 0x3 )
					{
						throw std::runtime_error( "Unaligned AIF relocation offset" );
					}
					mRelocations.push_back( payloadReloc );
				}
			}
		}
		if ( !foundSentinel )
		{
			throw std::runtime_error( "Truncated executable AIF relocation list (missing 0xFFFFFFFF terminator)" );
		}
	}
	else
	{
		if ( selfRelocCode == kARMNop || selfRelocCode == 0 )
		{
			return;
		}
		KUInt64 relocTableOffset = selfRelocCode;
		KUInt64 minRelocOffset = expectedEnd + debugSize;
		if ( relocTableOffset < minRelocOffset || relocTableOffset + 4 > fileLength )
		{
			throw std::runtime_error( "Invalid non-executable AIF relocation table offset" );
		}

		mFile.SetCursor( (KSInt64)relocTableOffset, TDCLFile::kFromStart );
		KUInt32 relocCount = mFile.GetLong();
		if ( relocTableOffset + 4 + (KUInt64)relocCount * 4 > fileLength )
		{
			throw std::runtime_error( "AIF relocation table exceeds file size" );
		}

		for ( KUInt32 i = 0; i < relocCount; i++ )
		{
			KUInt32 reloc = mFile.GetLong();
			// Non-executable AIF relocation offsets are relative to the start
			// of the user image (after the 0x80 header). Ignore debug-section entries.
			if ( reloc < mPayloadFileSize )
			{
				if ( mPayloadFileSize < 4 || reloc > mPayloadFileSize - 4 )
				{
					throw std::runtime_error( "AIF relocation offset exceeds payload word bounds" );
				}
				if ( reloc & 0x3 )
				{
					throw std::runtime_error( "Unaligned AIF relocation offset" );
				}
				mRelocations.push_back( reloc );
			}
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Close( void )
// -------------------------------------------------------------------------- //
void
TAIFFile::Close( void )
{
	mFile.Close();
}

// -------------------------------------------------------------------------- //
//  * ReadPayload( void* )
// -------------------------------------------------------------------------- //
void
TAIFFile::ReadPayload( void* outBuffer )
{
	::memset( outBuffer, 0, mPayloadSize );
	if ( mPayloadFileSize > 0 )
	{
		mFile.SetCursor( kAIFHeaderSize, TDCLFile::kFromStart );
		KUInt32 nbRead = mPayloadFileSize;
		mFile.Read( outBuffer, &nbRead );
		if ( nbRead != mPayloadFileSize )
		{
			throw std::runtime_error( "Short read while reading AIF payload" );
		}
	}
}
