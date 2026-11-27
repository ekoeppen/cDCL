// ==============================
// File:            TAIFFile.h
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
// The Original Code is TAIFFile.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen (eck@40hz.org).
// Portions created by the Initial Developer are Copyright (C) 2026
// the Initial Developer. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef _TAIFFILE_H
#define _TAIFFILE_H

#include <DCL/Headers/DCLDefinitions.h>

#ifdef _WIN32
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
typedef TDCLWin32File TDCLPlatformFile;
typedef TDCLWin32Files TDCLPlatformFiles;
#else
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
typedef TDCLPOSIXFile TDCLPlatformFile;
typedef TDCLPOSIXFiles TDCLPlatformFiles;
#endif

#include <vector>

///
/// Class for reading an ARM Image Format (AIF) file produced by ARMLink.
///
class TAIFFile
{
public:
	///
	/// Constructor from a file interface and path.
	///
	TAIFFile( TDCLPlatformFiles* inFilesIntf, const char* inPath );

	///
	/// Destructor.
	///
	~TAIFFile( void );

	///
	/// Open and parse the AIF header and relocation list.
	///
	void Open( void );

	///
	/// Close the underlying file.
	///
	void Close( void );

	///
	/// Whether this is an executable AIF (-AIF) vs non-executable (-BIN -AIF).
	///
	bool
	IsExecutable( void ) const
	{
		return mExecutable;
	}

	///
	/// Link-time base address corresponding to byte 0 of the stripped payload.
	/// For executable AIF this is ImageBase + 0x80; for non-executable AIF it
	/// is ImageBase.
	///
	KUInt32
	GetPayloadBaseAddress( void ) const
	{
		return mPayloadBaseAddress;
	}

	///
	/// Total byte size of the stripped payload (user RO + RW + zero-init).
	///
	KUInt32
	GetPayloadSize( void ) const
	{
		return mPayloadSize;
	}

	///
	/// Read the stripped payload (user RO + RW, plus zero-initialized tail).
	/// outBuffer must be at least GetPayloadSize() bytes.
	///
	void ReadPayload( void* outBuffer );

	///
	/// Byte offsets within the stripped payload that require relocation.
	///
	const std::vector<KUInt32>&
	GetRelocations( void ) const
	{
		return mRelocations;
	}

private:
	static const KUInt32 kAIFHeaderSize = 0x80;
	static const KUInt32 kARMNop = 0xE1A00000;
	static const KUInt32 kARMBLMask = 0xFF000000;
	static const KUInt32 kARMBLOpcode = 0xEB000000;
	static const KUInt32 kSelfRelocStubSize = 0xB8;
	static const KUInt32 kSelfRelocListAddInstrOffset = 0x98;
	static const KUInt32 kSelfRelocListAddR2PC = 0xE28F2018;

	TDCLPlatformFile mFile;
	bool mExecutable;
	KUInt32 mPayloadBaseAddress;
	KUInt32 mPayloadFileSize;
	KUInt32 mPayloadSize;
	std::vector<KUInt32> mRelocations;
};

#endif
// _TAIFFILE_H
