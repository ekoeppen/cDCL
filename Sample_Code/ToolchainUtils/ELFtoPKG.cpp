// ==============================
// Fichier:         ELFtoPKG.cp
// Projet:          DCL - ELFtoPKG
// Ecrit par:       Paul Guyot (pguyot@kallisys.net)
//
// Créé le:         18/5/2020
//
// Copyright:       © 2020 by Paul Guyot.
//                  Tous droits réservés pour tous pays.
// Licence:         MPL
// ===========

///
/// \file
/// Convert ELF native code to a protocol part embedded in a package.
/// Replaces Packer
///

// ANSI C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ISO C++
#include <stdexcept>

// ELF
#include "elf.h"

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/Package/TDCLPkgRelocatablePart.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

// K
#include <K/Defines/UByteSex.h>

#include "TELFFile.h"

void
CopyRelocations(TDCLPkgRelocatablePart* part, TELFFile& elfFile, KUInt32 minVAddr, KUInt32 relocationTableOffset, KUInt32 relocationTableSize, KUInt32 relocationTableEntrySize) {
    KUInt32 relocationCount = relocationTableSize / relocationTableEntrySize;
    KUInt32 relocationsSize = relocationCount * sizeof(KUInt32);
    KUInt32* relocations = (KUInt32*) ::malloc(relocationsSize);
    for (KUInt32 relocationIx = 0; relocationIx < relocationCount; relocationIx++) {
        Elf32_Rel relocation;
        elfFile.PRead(&relocation, sizeof(relocation), relocationTableOffset + (relocationTableEntrySize * relocationIx));
        KUInt32 offset = UByteSex_FromBigEndian(relocation.r_offset);
        if (UByteSex_FromBigEndian(relocation.r_info) != R_ARM_RELATIVE) {
            ::free(relocations);
            throw std::runtime_error("Unknown relocation info type");
        }
        relocations[relocationIx] = offset - minVAddr;
    }

    part->SetRelocations( relocationCount, relocations );
    ::free(relocations);
}

void
ProcessDynamicSegment(TDCLPkgRelocatablePart* part, TELFFile& elfFile, Elf32_Ehdr& header, KUInt32 baseVAddr) {
    KUInt16 programHeaderCount = UByteSex_FromBigEndian(header.e_phnum);
    KUInt32 programHeadersOffset = UByteSex_FromBigEndian(header.e_phoff);
    for (int n = 0; n < programHeaderCount; n++) {
        Elf32_Phdr programHeader;
        elfFile.PRead(&programHeader, sizeof(programHeader), programHeadersOffset + sizeof(Elf32_Phdr) * n);
        if (UByteSex_FromBigEndian(programHeader.p_type) == PT_DYNAMIC) {
            KUInt32 dynamicEntriesOffset = UByteSex_FromBigEndian(programHeader.p_offset);
            KUInt32 dynamicEntriesVAddr = UByteSex_FromBigEndian(programHeader.p_vaddr);
            KUInt32 dynamicEntriesSize = UByteSex_FromBigEndian(programHeader.p_filesz);
            KUInt32 dynamicEntriesVAddrAdj = dynamicEntriesOffset - dynamicEntriesVAddr;
            Elf32_Dyn* dynamicEntries = (Elf32_Dyn*) ::malloc(dynamicEntriesSize);
            elfFile.PRead(dynamicEntries, dynamicEntriesSize, dynamicEntriesOffset);
            KUInt32 dynamicEntriesCount = dynamicEntriesSize / sizeof(Elf32_Dyn);
            KUInt32 relocationTableOffset = 0;
            KUInt32 relocationTableSize = 0;
            KUInt32 relocationTableEntrySize = sizeof(Elf32_Rel);
            for (int j = 0; j < dynamicEntriesCount; j++) {
                KUInt32 tag = UByteSex_FromBigEndian((KUInt32) dynamicEntries[j].d_tag);
                if (tag == DT_REL) {
                    relocationTableOffset = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_ptr);
                }
                if (tag == DT_RELSZ) {
                    relocationTableSize = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_val);
                }
                if (tag == DT_RELENT) {
                    relocationTableEntrySize = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_val);
                }
                if (tag == DT_RELA || tag == DT_RELASZ || tag == DT_RELAENT) {
                    ::free(dynamicEntries);
                    throw std::runtime_error("RELA entries are currently unsupported (unlike REL entries which we do support)");
                }
            }
            ::free(dynamicEntries);
            if (relocationTableOffset != 0) {
                CopyRelocations(part, elfFile, baseVAddr, dynamicEntriesVAddrAdj + relocationTableOffset, relocationTableSize, relocationTableEntrySize);
            }
        }
    }
}

TDCLPkgRelocatablePart*
ParseELF(TELFFile& elfFile) {
    Elf32_Ehdr header;
    elfFile.ReadHeader(&header);

    if (UByteSex_FromBigEndian(header.e_type) != ET_DYN) {
        throw std::runtime_error("Not a dynamic file");
    }

    KUInt32 baseVAddr;
    KUInt32 imageSize;
    elfFile.GetVirtualAddressRange(header, baseVAddr, imageSize);

    if (baseVAddr != 0) {
        // We probably could handle this by loading relocations first and
        // relocating to 0.
        printf("Invalid base address, expected 0 got %x\n", baseVAddr);
        throw std::runtime_error("Invalid base address");
    }

    KUInt8* data = (KUInt8*) ::malloc(imageSize);
    elfFile.ReadWithVirtualAddress(header, data, imageSize, baseVAddr);

    TDCLPkgRelocatablePart* part = new TDCLPkgRelocatablePart( baseVAddr, data, imageSize );
    ::free(data);

    ProcessDynamicSegment(part, elfFile, header, baseVAddr);

    return part;
}

void usage( const char* name )
{
    (void) ::fprintf( stderr, "Convert ELF to a raw/protocol part in a Newton package\n" );
    (void) ::fprintf( stderr,
                "Syntax: %s -o output.pkg package_name (-protocol|-raw) input.elf \n"
                " [-autoLoad] [-autoRemove] [-autoCopy] [-notify Type HandlerInfo]\n", name );
}

// -------------------------------------------------------------------------- //
//  * entry point
// -------------------------------------------------------------------------- //
int main( int argc, char** argv )
{
    // Check that we did get some parameter.
    if (argc < 6 || strcmp(argv[1], "-o"))
    {
        usage( argv[0] );
        return -2;
    }

    KUInt32 partType = 0;
    KUInt32 flags = 0;
    const char* handlerInfo = NULL;

    if (strcmp(argv[4], "-protocol") == 0) {
        flags = TDCLPackage::kPartProtocolPart;
    } else if (strcmp(argv[4], "-raw") == 0) {
        flags = TDCLPackage::kPartRawPart;
    } else {
        usage( argv[0] );
        return -2;
    }

    int optionIx = 6;
    while (optionIx < argc) {
        if (strcmp(argv[optionIx], "-autoLoad") == 0) {
            flags |= TDCLPackage::kPartAutoLoadFlag;
        } else if (strcmp(argv[optionIx], "-autoRemove") == 0) {
            flags |= TDCLPackage::kPartAutoRemoveFlag;
        } else if (strcmp(argv[optionIx], "-autoCopy") == 0) {
            flags |= TDCLPackage::kPartAutoRemoveFlag;
        } else if (strcmp(argv[optionIx], "-notify") == 0 && optionIx + 2 < argc) {
            flags |= TDCLPackage::kPartNotifyFlag;
            const char* notifyType = argv[optionIx + 1];
            if (::strlen(notifyType) != 4) {
                usage( argv[0] );
                return -2;
            }
            partType = (notifyType[0] << 24)
                     | (notifyType[1] << 16)
                     | (notifyType[2] << 8)
                     | (notifyType[3]);
            handlerInfo = argv[optionIx + 2];
            optionIx += 2;
        } else {
            usage( argv[0] );
            return -2;
        }
        optionIx++;
    }

    int theResult = 0;

    try {
        // Open files early to fail fast.
        TDCLPOSIXFiles theFilesIntf;
        TELFFile elfFile(&theFilesIntf, argv[5]);
        elfFile.Open(true);

        TDCLPOSIXFile outputFile(&theFilesIntf, argv[2]);
        outputFile.Create();
        outputFile.Open(false);

        try {
            TDCLPackage thePackage;

            thePackage.SetNOS1Compatible( false );
            thePackage.SetPackageID( TDCLPackage::kDefaultID );
            thePackage.SetPackageFlags( 0 );
            thePackage.SetPackageVersion( 1 );

            thePackage.SetPackageName(argv[3]);
            thePackage.SetCopyrightString("");

            TDCLPkgRelocatablePart* part = ParseELF(elfFile);

            if (handlerInfo == NULL) {
                (void) thePackage.AddPart(
                            partType,
                            flags,
                            part );
            } else {
                (void) thePackage.AddPart(
                            partType,
                            flags,
                            strlen(handlerInfo),
                            (const KUInt8*) handlerInfo,
                            part);
            }

            thePackage.WriteToStream(&outputFile);
            outputFile.FlushOutput();
        } catch (...) {
            try {
                outputFile.Close();
                outputFile.Delete();
            } catch (...) {
            }
            throw;
        }
    } catch (TDCLException& anException) {
        // Print the exception.
        (void) ::fprintf( stderr, "A DCL exception occurred (%s, %i, %i)\n",
                            anException.GetExceptionName(),
                            (int) anException.GetExCode(),
                            (int) anException.GetError() );
        theResult = -1;
    } catch (std::runtime_error& anErr) {
        (void) ::fprintf( stderr, "%s\n", anErr.what() );
        theResult = -1;
    } catch (std::exception& anException) {
        // Unknown exception.
        (void) ::fprintf( stderr, "A STD exception occurred (%s)\n",
                            anException.what() );
        theResult = -1;
    } catch (...) {
        // Unknown exception.
        (void) ::fprintf( stderr, "An unknown exception occurred\n" );
        theResult = -1;
    }

    return theResult;
}

// ====================================================== //
// When arguments fail, use a blackjack.                  //
//      -- Edward "Spike" O'Donnell, Al Capone associate. //
// ====================================================== //
