// =========== Header ===========
// File:                TELFFile.h
// Project:             DCL - ToolchainUtils
// Written by:          Paul Guyot (pguyot@kallisys.net)
//
// Created on:          18/05/2020
// Internal version:    1
//
// Copyright:           © 2020 by Paul Guyot.
//                      All rights reserved worldwide.
// ===========

#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>

#include "elf.h"

// ISO C++
#include <stdexcept>

///
/// Class for compatible ELF files containing code for NewtonOS.
///
class TELFFile : public TDCLPOSIXFile {
public:
    ///
    /// Constructor.
    ///
    TELFFile(TDCLPOSIXFiles* filesIntf, const char* name) : TDCLPOSIXFile(filesIntf, name) {};

    ///
    /// Read file header and check format.
    /// \throw std::runtime_error of format is incompatble
    ///
    void ReadHeader(Elf32_Ehdr* outHeader);

    ///
    /// Get virtual address range of code.
    ///
    void GetVirtualAddressRange(const Elf32_Ehdr& inHeader, KUInt32& outVAddrStart, KUInt32& outSize);

    ///
    /// Read code from a given virtual address.
    ///
    void ReadWithVirtualAddress(const Elf32_Ehdr& inHeader, void* buffer, KUInt32 inSize, KUInt32 inVAddr);

    ///
    /// Read at a given offset.
    /// \throw std::runtime_error if read failed ("Invalid ELF File")
    ///
    void PRead(void* buffer, KUInt32 size, KUInt32 offset);

    ///
    /// Read a null-terminated at a given offset.
    /// String is allocated with malloc.
    ///
    char* PReadCString(KUInt32 offset);

    ///
    /// Read a 32 bits word at a given offset.
    /// Convert to host endianness.
    ///
    KUInt32 PRead32(KUInt32 offset);
};

// =========================================================== //
// May a Misguided Platypus lay its Eggs in your Jockey Shorts //
// =========================================================== //
