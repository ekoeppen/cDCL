// =========== Header ===========
// File:                TELFFile.cpp
// Project:             DCL - ToolchainUtils
// Written by:          Paul Guyot (pguyot@kallisys.net)
//
// Created on:          18/05/2020
// Internal version:    1
//
// Copyright:           © 2020 by Paul Guyot.
//                      All rights reserved worldwide.
// ===========

#include "TELFFile.h"

// K
#include <K/Defines/UByteSex.h>

// ANSI C
#include <string.h>

void
TELFFile::ReadHeader(Elf32_Ehdr* header) {
    try {
        if (GetByte() != ELFMAG0
        || GetByte() != ELFMAG1
        || GetByte() != ELFMAG2
        || GetByte() != ELFMAG3
        ) {
            throw std::runtime_error("Not an ELF file");
        }
    } catch (TDCLIOException& ioEx) {
        throw std::runtime_error("Not an ELF file");
    }

    KUInt32 readSize;
    PRead(header, sizeof(*header), 0);
    if (header->e_ident[EI_CLASS] != ELFCLASS32
        || header->e_ident[EI_DATA] != ELFDATA2MSB
        || header->e_ident[EI_VERSION] != EV_CURRENT) {
        throw std::runtime_error("Incompatible ELF file");
    }

    if (header->e_ident[EI_CLASS] != ELFCLASS32
        || header->e_ident[EI_DATA] != ELFDATA2MSB
        || header->e_ident[EI_VERSION] != EV_CURRENT) {
        throw std::runtime_error("Incompatible ELF file");
    }

    if (UByteSex_FromBigEndian(header->e_machine) != EM_ARM) {
        throw std::runtime_error("Invalid architecture (expected EM_ARM)");
    }
}

void
TELFFile::GetVirtualAddressRange(const Elf32_Ehdr& header, KUInt32& outVAddrStart, KUInt32& outSize) {
    // Work on sections to identify the minimum range of data to copy
    KUInt32 sectionsOffset = UByteSex_FromBigEndian(header.e_shoff);
    KUInt16 sectionsCount = UByteSex_FromBigEndian(header.e_shnum);
    KUInt16 stringTableSectionIx = UByteSex_FromBigEndian(header.e_shstrndx);
    Elf32_Shdr stringTableSectionHeader;
    PRead(&stringTableSectionHeader, sizeof(stringTableSectionHeader), sectionsOffset + sizeof(Elf32_Shdr) * stringTableSectionIx);
    KUInt32 stringTableOffset = UByteSex_FromBigEndian(stringTableSectionHeader.sh_offset);

    KUInt32 minVAddr = (KUInt32) -1;
    KUInt32 maxVAddr = 0;
    for (int n = 0; n < sectionsCount; n++) {
        Elf32_Shdr sectionHeader;
        PRead(&sectionHeader, sizeof(sectionHeader), sectionsOffset + sizeof(Elf32_Shdr) * n);
        if (UByteSex_FromBigEndian(sectionHeader.sh_type) != SHT_NULL) {
            KUInt32 sectionNameOffset = UByteSex_FromBigEndian(sectionHeader.sh_name);
            char sectionName[64];
            if (sectionNameOffset) {
                char* sectionName = PReadCString(stringTableOffset + sectionNameOffset);
                if (strcmp(sectionName, ".ARM.attributes") == 0
                    || strcmp(sectionName, ".comment") == 0
                    || strcmp(sectionName, ".dynamic") == 0
                    || strcmp(sectionName, ".dynstr") == 0
                    || strcmp(sectionName, ".dynsym") == 0
                    || strcmp(sectionName, ".gnu.version_d") == 0
                    || strcmp(sectionName, ".gnu.version") == 0
                    || strcmp(sectionName, ".hash") == 0
                    || strcmp(sectionName, ".rel.dyn") == 0
                    || strcmp(sectionName, ".shstrtab") == 0
                    || strcmp(sectionName, ".strtab") == 0
                    || strcmp(sectionName, ".symtab") == 0
                    ) {
                    // Ignore these.
                } else if (strcmp(sectionName, ".interp") == 0
                    || strcmp(sectionName, ".text") == 0
                    || strcmp(sectionName, ".rodata") == 0
                    || strcmp(sectionName, ".data.rel.ro") == 0
                    || strcmp(sectionName, ".got") == 0
                    || strcmp(sectionName, ".plt") == 0) {
                    KUInt32 sectionVAddr = UByteSex_FromBigEndian(sectionHeader.sh_addr);
                    KUInt32 sectionSize = UByteSex_FromBigEndian(sectionHeader.sh_size);
                    minVAddr = minVAddr < sectionVAddr ? minVAddr : sectionVAddr;
                    maxVAddr = maxVAddr > sectionVAddr + sectionSize ? maxVAddr : sectionVAddr + sectionSize;
                } else {
                    printf("Warning: unknown section %s!\n", sectionName);
                    if (UByteSex_FromBigEndian(sectionHeader.sh_type) == SHT_PROGBITS) {
                        ::free(sectionName);
                        throw std::runtime_error("Unknown section of type PROGBITS");
                    }
                }
                ::free(sectionName);
            } else {
                printf("Warning: unnamed section\n");
            }
        }
    }

    outVAddrStart = minVAddr;
    outSize = maxVAddr - minVAddr;
}

void
TELFFile::ReadWithVirtualAddress(const Elf32_Ehdr& header, void* buffer, KUInt32 inSize, KUInt32 inVAddr) {
    // Work on program headers to actually copy data.
    KUInt8* data = (KUInt8*) buffer;
    KUInt32 endVAddr = inVAddr + inSize;
    KUInt16 programHeaderCount = UByteSex_FromBigEndian(header.e_phnum);
    KUInt32 programHeadersOffset = UByteSex_FromBigEndian(header.e_phoff);
    for (int n = 0; n < programHeaderCount; n++) {
        Elf32_Phdr programHeader;
        PRead(&programHeader, sizeof(programHeader), programHeadersOffset + sizeof(Elf32_Phdr) * n);
        if (UByteSex_FromBigEndian(programHeader.p_type) == PT_LOAD) {
            KUInt32 segmentAddr = UByteSex_FromBigEndian(programHeader.p_vaddr);
            KUInt32 segmentSize = UByteSex_FromBigEndian(programHeader.p_filesz);
            KUInt32 segmentOffset = UByteSex_FromBigEndian(programHeader.p_offset);
            if (inVAddr <= segmentAddr + segmentSize && endVAddr >= segmentAddr) {
                KUInt32 startCopyVAddr = inVAddr < segmentAddr ? segmentAddr : inVAddr;
                KUInt32 endCopyVAddr = endVAddr > (segmentAddr + segmentSize) ? (segmentAddr + segmentSize) : endVAddr;
                PRead(data + startCopyVAddr - inVAddr, endCopyVAddr - startCopyVAddr, segmentOffset + startCopyVAddr - segmentAddr);
            }
        }
    }
}

void
TELFFile::PRead(void* buffer, KUInt32 size, KUInt32 offset) {
    KUInt32 count = size;

    try {
        SetCursor(offset, TDCLRandomAccessStream::kFromStart);
    } catch (TDCLIOException& ioEx) {
        printf("I/O, reading at offset %d for %u\n", offset, size);
        throw std::runtime_error("Invalid ELF file");
    }

    Read(buffer, &count);
    if (count != size) {
        printf("I/O, reading at offset %d for %u, got %d\n", offset, size, count);
        throw std::runtime_error("Invalid ELF file");
    }
}

char*
TELFFile::PReadCString(KUInt32 offset) {
    try {
        SetCursor(offset, TDCLRandomAccessStream::kFromStart);
    } catch (TDCLIOException& ioEx) {
        printf("I/O, reading at offset %d\n", offset);
        throw std::runtime_error("Invalid ELF file");
    }
    return (char*) GetCString();
}

KUInt32
TELFFile::PRead32(KUInt32 offset) {
    KUInt32 value;
    PRead(&value, sizeof(value), offset);
    return UByteSex_FromBigEndian(value);
}

// =================== //
// Hail to the sun god //
// He's such a fun god //
// Ra! Ra! Ra!         //
// =================== //

