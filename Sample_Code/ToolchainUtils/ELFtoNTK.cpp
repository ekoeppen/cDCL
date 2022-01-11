// ==============================
// Fichier:         ELFtoNTK.cp
// Projet:          DCL - ELFtoNTK
// Ecrit par:       Paul Guyot (pguyot@kallisys.net)
//
// Créé le:         17/5/2020
//
// Copyright:       © 2020 by Paul Guyot.
//                  Tous droits réservés pour tous pays.
// Licence:         MPL
// ===========

///
/// \file
/// Convert ELF native code to NTKC suitable for NTK.
/// Replaces AIFtoNTK
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
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

// K
#include <K/Defines/UByteSex.h>

#include "TELFFile.h"

void
ParseExpFile(TDCLNSFrame& frame, TDCLStream& expFile) {
    // Parse exp file, which has a syntax a little bit different from AIFtoNTK's exp (we need parameters count)
    bool firstLine = true;
    frame.Set("class", TDCLNSRef::MakeSymbol("NativeModule"));
    frame.Set("version", TDCLNSRef::MakeInt(1));
    frame.Set("CPUType", TDCLNSRef::MakeSymbol("ARM610"));
    TDCLNSArray* entryPoints = new TDCLNSArray();
    frame.Set("entryPoints", entryPoints);

    while (true) {
        KUInt8* str = expFile.GetLine();
        if (str == nil) {
            break;
        }
        if (str[0] != ';' && str[0] != '\n' && str[0] != 0) {
            KUInt8* crsr = str;
            while (*crsr) {
                if (*crsr == '\n') {
                    *crsr = 0;
                    break;
                }
                crsr++;
            }
            if (firstLine) {
                frame.Set("name", TDCLNSRef::MakeSymbol((const char*) str));
                firstLine = false;
            } else {
                crsr = str;
                while (*crsr) {
                    if (*crsr == ':') {
                        *crsr = 0;
                        crsr++;
                        break;
                    }
                    if (*crsr == 0) {
                        throw DCLEOF;
                    }
                    crsr++;
                }
                TDCLNSFrame* entryPoint = new TDCLNSFrame();
                entryPoint->Set("name", TDCLNSRef::MakeSymbol((const char*) str));
                entryPoint->Set("numArgs", TDCLNSRef::MakeInt(atoi((const char*) crsr)));
                entryPoints->Add(entryPoint);
            }
        }
        ::free(str);
    }
}

void
CopyRelocations(TDCLNSFrame& frame, TELFFile& elfFile, KUInt32 minVAddr, KUInt32 relocationTableOffset, KUInt32 relocationTableSize, KUInt32 relocationTableEntrySize) {
    KUInt32 relocationCount = relocationTableSize / relocationTableEntrySize;
    KUInt32 relocationsSize = (relocationCount + 1) * sizeof(KUInt32);
    KUInt32* relocations = (KUInt32*) ::malloc(relocationsSize);
    relocations[0] = UByteSex_ToBigEndian(relocationCount);
    for (KUInt32 relocationIx = 0; relocationIx < relocationCount; relocationIx++) {
        Elf32_Rel relocation;
        elfFile.PRead(&relocation, sizeof(relocation), relocationTableOffset + (relocationTableEntrySize * relocationIx));
        KUInt32 offset = UByteSex_FromBigEndian(relocation.r_offset);
        if (UByteSex_FromBigEndian(relocation.r_info) != R_ARM_RELATIVE) {
            ::free(relocations);
            throw std::runtime_error("Unknown relocation info type");
        }
        relocations[relocationIx + 1] = UByteSex_ToBigEndian(offset - minVAddr);
    }

    frame.Set("relocations", TDCLNSRef::MakeBinary(relocations, relocationsSize, TDCLNSRef::MakeSymbol("relocs")));
    ::free(relocations);
}

unsigned long
elf_hash(const unsigned char *name)
{
    unsigned long h = 0, g;
    while (*name)
    {
        h = (h << 4) + *name++;
        if ((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

KUInt32
GetFunctionOffset(TELFFile& elfFile, const char* inSymbol, KUInt32 hashTableOffset, KUInt32 stringTableOffset, KUInt32 symbolTableOffset, KUInt32 stringTableSize, KUInt32 symbolTableEntrySize) {
    Elf32_Word hashValue = elf_hash((const unsigned char*) inSymbol);
    Elf32_Word nBucket = elfFile.PRead32(hashTableOffset);
    Elf32_Word yIndex = elfFile.PRead32(hashTableOffset + (sizeof(KUInt32) * (2 + (hashValue % nBucket))));
    do {
        Elf32_Sym symbolEntry;
        elfFile.PRead(&symbolEntry, sizeof(symbolEntry), symbolTableOffset + (symbolTableEntrySize * yIndex));
        if (symbolEntry.st_name) {
            KUInt32 symbolOffset = UByteSex_FromBigEndian(symbolEntry.st_name);
            char* symbolName = elfFile.PReadCString(stringTableOffset + symbolOffset);
            int cmp = strcmp(symbolName, inSymbol);
            ::free(symbolName);
            if (cmp == 0) {
                return UByteSex_FromBigEndian(symbolEntry.st_value);
                break;
            }
        }
        yIndex = elfFile.PRead32(hashTableOffset + (sizeof(KUInt32) * (2 + nBucket + yIndex)));
    } while (yIndex);
    printf("Symbol not found: %s\n", inSymbol);
    throw std::runtime_error("Symbol not found");
}

void
SetEntryPointsOffsets(TDCLNSFrame& frame, TELFFile& elfFile, KUInt32 minVAddr, KUInt32 hashTableOffset, KUInt32 stringTableOffset, KUInt32 symbolTableOffset, KUInt32 stringTableSize, KUInt32 symbolTableEntrySize) {
    TDCLNSArray& entryPointsArray = frame.Get("entryPoints").ToArray();
    KUInt32 size = entryPointsArray.GetLength();
    for (KUInt32 i = 0; i < size; i++) {
        TDCLNSFrame& entryPoint = entryPointsArray.Get(i).ToFrame();
        TDCLNSSymbol& functionName = entryPoint.Get("name").ToSymbol();
        const char* functionNameStr = functionName.GetString();
        KUInt32 offset = GetFunctionOffset(elfFile, functionNameStr, hashTableOffset, stringTableOffset, symbolTableOffset, stringTableSize, symbolTableEntrySize);
        entryPoint.Set("offset", TDCLNSRef::MakeInt(offset - minVAddr));
    }
}

void
ProcessDynamicSegment(TDCLNSFrame& frame, TELFFile& elfFile, Elf32_Ehdr& header, KUInt32 minVAddr) {
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
            KUInt32 hashTableOffset = 0;
            KUInt32 stringTableOffset = 0;
            KUInt32 symbolTableOffset = 0;
            KUInt32 stringTableSize = 0;
            KUInt32 symbolTableEntrySize = 0;
            KUInt32 relocationTableOffset = 0;
            KUInt32 relocationTableSize = 0;
            KUInt32 relocationTableEntrySize = sizeof(Elf32_Rel);
            for (int j = 0; j < dynamicEntriesCount; j++) {
                KUInt32 tag = UByteSex_FromBigEndian((KUInt32) dynamicEntries[j].d_tag);
                if (tag == DT_HASH) {
                    hashTableOffset = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_ptr);
                }
                if (tag == DT_STRTAB) {
                    stringTableOffset = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_ptr);
                }
                if (tag == DT_SYMTAB) {
                    symbolTableOffset = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_ptr);
                }
                if (tag == DT_STRSZ) {
                    stringTableSize = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_val);
                }
                if (tag == DT_SYMENT) {
                    symbolTableEntrySize = UByteSex_FromBigEndian(dynamicEntries[j].d_un.d_val);
                }
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
            if (hashTableOffset == 0) {
                throw std::runtime_error("Missing hash table in dynamic segment");
            }
            if (stringTableOffset == 0) {
                throw std::runtime_error("Missing string table in dynamic segment");
            }
            if (symbolTableOffset == 0) {
                throw std::runtime_error("Missing symbol table in dynamic segment");
            }
            if (relocationTableOffset != 0) {
                CopyRelocations(frame, elfFile, minVAddr, dynamicEntriesVAddrAdj + relocationTableOffset, relocationTableSize, relocationTableEntrySize);
            }
            SetEntryPointsOffsets(frame, elfFile, minVAddr, dynamicEntriesVAddrAdj + hashTableOffset, dynamicEntriesVAddrAdj + stringTableOffset, dynamicEntriesVAddrAdj + symbolTableOffset, stringTableSize, symbolTableEntrySize);
        }
    }
}

void
ParseELF(TDCLNSFrame& frame, TELFFile& elfFile) {
    Elf32_Ehdr header;
    elfFile.ReadHeader(&header);

    if (UByteSex_FromBigEndian(header.e_type) != ET_DYN) {
        throw std::runtime_error("Not a shared object file");
    }

    KUInt32 baseVAddr;
    KUInt32 imageSize;
    elfFile.GetVirtualAddressRange(header, baseVAddr, imageSize);
    KUInt8* data = (KUInt8*) ::malloc(imageSize);
    elfFile.ReadWithVirtualAddress(header, data, imageSize, baseVAddr);
    frame.Set("baseAddress", TDCLNSRef::MakeInt(baseVAddr));
    frame.Set("code", TDCLNSRef::MakeBinary(data, imageSize, TDCLNSRef::MakeSymbol("code")));
    ::free(data);

    ProcessDynamicSegment(frame, elfFile, header, baseVAddr);
}

// -------------------------------------------------------------------------- //
//  * entry point
// -------------------------------------------------------------------------- //
int main( int argc, char** argv )
{
    // Check that we did get some parameter.
    if (argc != 6 || strcmp(argv[1], "-via") || strcmp(argv[3], "-o"))
    {
        (void) ::fprintf( stderr,
                    "Convert ELF to NTK native code (.ntkc)\n" );
        (void) ::fprintf( stderr,
                    "Syntax: %s -via entryPointsFile.exp -o output.ntkc input.elf\n", argv[0] );
        ::exit( -2 );
    }

    int theResult = 0;

    try {
        // Open files early to fail fast.
        TDCLPOSIXFiles theFilesIntf;
        TDCLPOSIXFile expFile( &theFilesIntf, argv[2] );
        expFile.Open(true);

        TELFFile elfFile(&theFilesIntf, argv[5]);
        elfFile.Open(true);

        TDCLPOSIXFile outputFile( &theFilesIntf, argv[4] );
        outputFile.Create();
        outputFile.Open(false);

        TDCLNSRef frameRef = TDCLNSRef::MakeFrame();

        try {
            ParseExpFile(frameRef.ToFrame(), expFile);
            frameRef.ToFrame().Set("debugfile", TDCLNSRef::MakeSymbol(argv[5]));
            ParseELF(frameRef.ToFrame(), elfFile);

            // Output structure to outputfile.
            TDCLNSOFEncoder encoder( &outputFile );
            encoder.AddObject(frameRef);
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

// ==================================================================== //
// X windows:                                                           //
//         Accept any substitute.                                       //
//         If it's broke, don't fix it.                                 //
//         If it ain't broke, fix it.                                   //
//         Form follows malfunction.                                    //
//         The Cutting Edge of Obsolescence.                            //
//         The trailing edge of software technology.                    //
//         Armageddon never looked so good.                             //
//         Japan's secret weapon.                                       //
//         You'll envy the dead.                                        //
//         Making the world safe for competing window systems.          //
//         Let it get in YOUR way.                                      //
//         The problem for your problem.                                //
//         If it starts working, we'll fix it.  Pronto.                 //
//         It could be worse, but it'll take time.                      //
//         Simplicity made complex.                                     //
//         The greatest productivity aid since typhoid.                 //
//         Flakey and built to stay that way.                           //
//                                                                      //
// One thousand monkeys.  One thousand MicroVAXes.  One thousand years. //
//         X windows.                                                   //
// ==================================================================== //
