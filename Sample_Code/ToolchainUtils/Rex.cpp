// ==============================
// Fichier:         Rex.cpp
// Projet:          DCL - Rex
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
/// Compiler ROM Extension from source file.
/// Replaces MPW's Rex.
/// Support a subset of original tool, but handles relocation of packages.
///

// ANSI C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ISO C++
#include <stdexcept>
#include <list>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Streams/TDCLMemStream.h>

// K
#include <K/Defines/UByteSex.h>

struct ConfigEntry {
    uint32_t tag;
    uint32_t offset;
    uint32_t length;
};

struct RExHeader {
    uint32_t signatureA;
    uint32_t signatureB;
    uint32_t checksum;
    uint32_t headerVersion;
    uint32_t manufacturer;
    uint32_t version;
    uint32_t length;
    uint32_t id;
    uint32_t start;
    uint32_t count;
};

enum Token {
    TOKEN_ID,
    TOKEN_VERSION,
    TOKEN_START,
    TOKEN_MANUFACTURER,
    TOKEN_BLOCK,
    TOKEN_PACKAGE
};

// https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
#define CRC16 0x8005

uint16_t ComputeCRC16(const uint8_t* data, size_t size) {
    uint16_t out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if (data == NULL)
        return 0;

    while (size > 0) {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if (bits_read > 7) {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;
    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;    
}

// -------------------------------------------------------------------------- //
//  * entry point
// -------------------------------------------------------------------------- //
int main( int argc, char** argv )
{
    // Check that we did get some parameter.
    if (argc != 4 || strcmp(argv[1], "-o"))
    {
        (void) ::fprintf( stderr,
                    "Create a NewtonOS N2 ROM Extension\n" );
        (void) ::fprintf( stderr,
                    "Syntax: %s -o Output.rex Input.rex.src\n", argv[0] );
        ::exit( -2 );
    }
    
    RExHeader* header = (RExHeader*) ::calloc(1, sizeof(RExHeader));
    header->signatureA = UByteSex_ToBigEndian((KUInt32) 'RExB');
    header->signatureB = UByteSex_ToBigEndian((KUInt32) 'lock');
    header->checksum = UByteSex_ToBigEndian((KUInt32) 0xFFFFFFFF);
    header->headerVersion = UByteSex_ToBigEndian((KUInt32) 1);
    KUInt32 startAddr = 0;
    ConfigEntry* entries = NULL;
    std::list<TDCLPackage*> packageList;
    std::list<std::pair<uint32_t, std::pair<size_t, uint8_t*>>> blockData;
    int theResult = 0;

    try {
        TDCLPOSIXFiles theFilesIntf;
        TDCLPOSIXFile srcFile(&theFilesIntf, argv[3]);
        srcFile.Open(true);

        int line = 1;
        bool fatalError = false;
        while (true) {
            KUInt8* str = srcFile.GetLine();
            if (str == nil) {
                break;
            }
            int lineLen = strlen((char*) str);
            char* brkt;
            char* firstToken = strtok_r((char*) str, " \t", &brkt);
            if (firstToken == NULL) goto next_line;
            if (firstToken[0] == '/' && firstToken[1] == '/') goto next_line;
            if (firstToken[0] == '\n') goto next_line;
            Token firstTokenId;
            if (strcmp(firstToken, "id") == 0) {
                firstTokenId = TOKEN_ID;
            } else if (strcmp(firstToken, "version") == 0) {
                firstTokenId = TOKEN_VERSION;
            } else if (strcmp(firstToken, "start") == 0) {
                firstTokenId = TOKEN_START;
            } else if (strcmp(firstToken, "manufacturer") == 0) {
                firstTokenId = TOKEN_MANUFACTURER;
            } else if (strcmp(firstToken, "block") == 0) {
                firstTokenId = TOKEN_BLOCK;
            } else if (strcmp(firstToken, "package") == 0) {
                firstTokenId = TOKEN_PACKAGE;
            } else {
                fprintf(stderr, "%s:%d: Syntax error: unknown token %s\n", argv[3], line, firstToken);
                fatalError = true;
                goto next_line;
            }
            if (firstTokenId != TOKEN_PACKAGE) {
                char* nextToken = strtok_r(NULL, " \t", &brkt);
                if (nextToken == NULL) {
                    fprintf(stderr, "%s:%d: Syntax error: operand expected for %s\n", argv[3], line, firstToken);
                    fatalError = true;
                    goto next_line;
                }
                uint32_t value;
                if (nextToken[0] >= '0' && nextToken[0] <= '9') {
                    char* endptr;
                    value = strtoul(nextToken, &endptr, 0);
                    if (*endptr != 0 && *endptr != '\n') {
                        fprintf(stderr, "%s:%d: Syntax error: invalid numeric value for %s (%s)\n", argv[3], line, firstToken, nextToken);
                        fatalError = true;
                        goto next_line;
                    }
                } else if (
                    nextToken[0] == '\'' && nextToken[1] != 0
                    && nextToken[2] != 0 && nextToken[3] != 0
                    && nextToken[4] != 0 && nextToken[5] == '\''
                    && (nextToken[6] == 0 || nextToken[6] == '\n')) {
                    value = (nextToken[1] << 24) | (nextToken[2] << 16) | (nextToken[3] << 8) | nextToken[4];
                } else {
                    fprintf(stderr, "%s:%d: Syntax error: invalid value for %s (%s)\n", argv[3], line, firstToken, nextToken);
                    fatalError = true;
                    goto next_line;
                }
                switch (firstTokenId) {
                    case TOKEN_ID:
                        header->id = UByteSex_ToBigEndian((KUInt32) value);
                        break;
                    case TOKEN_VERSION:
                        header->version = UByteSex_ToBigEndian((KUInt32) value);
                        break;
                    case TOKEN_START:
                        startAddr = value;
                        header->start = UByteSex_ToBigEndian((KUInt32) value);
                        break;
                    case TOKEN_MANUFACTURER:
                        header->manufacturer = UByteSex_ToBigEndian((KUInt32) value);
                        break;
                    case TOKEN_BLOCK: {
                            // Skip spaces
                            char* lastToken = nextToken + strlen(nextToken) + 1;
                            while (*lastToken == '\t' || *lastToken == ' ') lastToken++;
                            size_t blockSize;
                            uint8_t* blockPtr;
                            if (lastToken[0] == '<') {
                                char* inlineData = strtok_r(NULL, "\t <>", &brkt);
                                blockSize = strlen(inlineData) / 2;
                                blockPtr = (uint8_t*) ::malloc(blockSize);
                                char byteStr[3];
                                byteStr[2] = 0;
                                for (int i = 0; i < blockSize; i++) {
                                    byteStr[0] = inlineData[i * 2];
                                    byteStr[1] = inlineData[i * 2 + 1];
                                    uint8_t byte = (uint8_t) strtoul(byteStr, NULL, 16);
                                    blockPtr[i] = byte;
                                }
                            } else if (lastToken[0] == '"') {
                                char* filename = strtok_r(NULL, "\"", &brkt);
                                try {
                                    TDCLFile* blockFile = new TDCLPOSIXFile(&theFilesIntf, filename);
                                    blockFile->Open(true);
                                    blockSize = blockFile->GetLength();
                                    blockPtr = (uint8_t*) ::malloc(blockSize);
                                    KUInt32 size = blockSize;
                                    blockFile->Read(blockPtr, &size);
                                    blockFile->Close();
                                    delete blockFile;
                                    if (size != blockSize) {
                                        fprintf(stderr, "%s:%d: Error reading file %s: read %u, expected %u\n", argv[3], line, filename, (unsigned) size, (unsigned) blockSize);
                                        fatalError = true;
                                        goto next_line;
                                    }
                                } catch (TDCLDoesntExistException& anException) {
                                    fprintf(stderr, "%s:%d: file %s does not exist\n", argv[3], line, filename);
                                    fatalError = true;
                                    goto next_line;
                                } catch (TDCLException& anException) {
                                    (void) ::fprintf( stderr, "A DCL exception occurred with package %s (%s, %i, %i)\n",
                                                        filename,
                                                        anException.GetExceptionName(),
                                                        (int) anException.GetExCode(),
                                                        (int) anException.GetError() );
                                    fatalError = true;
                                }
                            } else {
                                fprintf(stderr, "%s:%d: Syntax error: invalid value for %s, expected a string or a value in brackets\n", argv[3], line, firstToken);
                                fatalError = true;
                                goto next_line;
                            }
                            blockData.push_back(std::make_pair(value, std::make_pair(blockSize, blockPtr)));
                        }
                        break;
                    default:
                        // Please compiler.
                        break;
                }
            } else {
                // TOKEN PACKAGE
                int firstTokenLen = strlen(firstToken);
                if (firstTokenLen == lineLen) {
                    fprintf(stderr, "%s:%d: Syntax error: invalid value for %s, expected a filename\n", argv[3], line, firstToken);
                    fatalError = true;
                    goto next_line;
                }
                if (str[firstTokenLen + 1] != '"') {
                    // Skip spaces/tabs/whatever until "
                    if (strtok_r(NULL, "\"", &brkt) == NULL) {
                        fprintf(stderr, "%s:%d: Syntax error: invalid value for %s, expected \"\n", argv[3], line, firstToken);
                        fatalError = true;
                        goto next_line;
                    }
                }
                char* filename = strtok_r(NULL, "\"", &brkt);
                if (filename == NULL) {
                    fprintf(stderr, "%s:%d: Syntax error: invalid value for %s, expected a filename between \"\n", argv[3], line, firstToken);
                    fatalError = true;
                    goto next_line;
                }
                try {
                    TDCLFile* packageFile = new TDCLPOSIXFile(&theFilesIntf, filename);
                    packageFile->Open(true);
                    TDCLPackage* package = new TDCLPackage(packageFile);
                    packageList.push_back(package);
                    packageFile->Close();
                    delete packageFile;
                } catch (TDCLException& anException) {
                    (void) ::fprintf( stderr, "A DCL exception occurred with package %s (%s, %i, %i)\n",
                                        filename,
                                        anException.GetExceptionName(),
                                        (int) anException.GetExCode(),
                                        (int) anException.GetError() );
                    fatalError = true;
                }
            }
        next_line:
            ::free(str);
            line++;
            continue;
        }
        if (!fatalError) {
            TDCLPOSIXFile outputFile(&theFilesIntf, argv[2]);
            outputFile.Create();
            outputFile.Open(false);
            
            if (packageList.size() > 0) {
                blockData.push_back(std::make_pair((uint32_t)'pkgl', std::make_pair((size_t) 0, (uint8_t*)NULL)));
            }
            int nbBlocks = blockData.size();
            header->count = UByteSex_ToBigEndian((KUInt32) blockData.size());
            uint8_t* data = (uint8_t*) ::malloc(0);
            size_t dataSize = 0;
            size_t offset = sizeof(RExHeader) + nbBlocks * sizeof(ConfigEntry);
            entries = (ConfigEntry*) ::calloc(nbBlocks, sizeof(ConfigEntry));
            int entriesIndex = 0;
            for (auto const& block : blockData) {
                uint32_t tag = block.first;
                entries[entriesIndex].tag = UByteSex_ToBigEndian((KUInt32) tag);
                if (tag == 'pkgl') {
                    entries[entriesIndex].offset = UByteSex_ToBigEndian((KUInt32) offset);
                    size_t totalPackageSize = 0;
                    for (TDCLPackage* package : packageList) {
                        TDCLMemStream memStream;
                        package->WriteToStream(&memStream, offset + startAddr);
                        size_t packageSize = memStream.GetCursor();
                        data = (uint8_t*) ::realloc(data, dataSize + packageSize);
                        const void* packagePtr = memStream.GetBuffer();
                        ::memcpy(data + dataSize, (const uint8_t*) packagePtr, packageSize);
                        dataSize += packageSize;
                        offset += packageSize;
                        totalPackageSize += packageSize;
                    }
                    entries[entriesIndex].length = UByteSex_ToBigEndian((KUInt32) totalPackageSize);
                } else {
                    size_t blockSize = block.second.first;
                    data = (uint8_t*) ::realloc(data, dataSize + blockSize);
                    ::memcpy(data + dataSize, (const uint8_t*) block.second.second, blockSize);
                    dataSize += blockSize;
                    entries[entriesIndex].offset = UByteSex_ToBigEndian((KUInt32) offset);
                    offset += blockSize;
                    entries[entriesIndex].length = UByteSex_ToBigEndian((KUInt32) blockSize);
                }
                entriesIndex++;
            }
            header->length = UByteSex_ToBigEndian((KUInt32) offset);
            header->checksum = UByteSex_ToBigEndian((KUInt32) ComputeCRC16(data, dataSize));
            KUInt32 size = sizeof(RExHeader);
            outputFile.Write(header, &size);
            size = sizeof(ConfigEntry) * nbBlocks;
            outputFile.Write(entries, &size);
            size = dataSize;
            outputFile.Write(data, &size);
            ::free(entries);
            entries = NULL;
            ::free(data);
            outputFile.Close();
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
    
    for (auto const& block : blockData) {
        uint8_t* ptr = block.second.second;
        if (ptr) ::free(ptr);
    }
    for (TDCLPackage* package : packageList) {
        delete package;
    }
    if (entries) {
        ::free(entries);
    }
    ::free(header);

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
