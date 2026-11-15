// ==============================
// Fichier:         WatsonEnabler.cpp
// Projet:          DCL - PackageUtils
// Ecrit par:       Paul Guyot (pguyot@kallisys.net)
//
// Créé le:         17/6/2020
// Tabulation:      4 espaces
//
// Copyright:       © 2020 by Paul Guyot.
//                  Tous droits réservés pour tous pays.
// Licence:         Licence Réflexive Kallisys
//                  http://www.kallisys.org/reflexive/
// ===========

#include <DCL/Headers/DCLDefinitions.h>

// ANSI C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if TARGET_OS_WIN32
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <DCL/Interfaces/POSIX/Compat/libgen.h>
typedef TDCLWin32File TDCLPlatformFile;
typedef TDCLWin32Files TDCLPlatformFiles;
#else
#include <libgen.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
typedef TDCLPOSIXFile TDCLPlatformFile;
typedef TDCLPOSIXFiles TDCLPlatformFiles;
#endif

// ISO C++
#include <stdexcept>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgPart.h>
#include <DCL/Streams/TDCLStdStream.h>
#include <DCL/Streams/TDCLStream.h>

// -------------------------------------------------------------------------- //
// Prototypes
// -------------------------------------------------------------------------- //
void usage( char* inToolName );
void help( char* inToolName );
int main( int inArgc, char** inArgv );

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * usage( void )
// -------------------------------------------------------------------------- //
void
usage( char* inToolName )
{
    (void) ::fprintf( stderr, "Syntax: %s [options] [--] [input output]\n", inToolName );
    (void) ::fprintf( stderr,
                        "Try %s --help for more information\n", inToolName );
}

// -------------------------------------------------------------------------- //
//  * help( char* inToolName )
// -------------------------------------------------------------------------- //
void
help( char* inToolName )
{
    (void) ::fprintf( stderr, "%s - 'signs' package for Watson (file or stdin).\n", inToolName );
    (void) ::fprintf( stderr, "%s [options] [--] [input output]\n", inToolName );
    (void) ::fprintf( stderr, "   --help      display this help.\n" );
}

// -------------------------------------------------------------------------- //
//  * main( int, char** )
// -------------------------------------------------------------------------- //
int
main( int inArgc, char** inArgv )
{
    int theResult = 0;

    char* inputFilePath = nil;
    char* outputFilePath = nil;
    if (inArgc >= 2)
    {
        // Ou bien inArgv[1] est --help ou bien c'est le nom du fichier, ou bien
        // c'est --.
        if (::strcmp( inArgv[1], "--help" ) == 0)
        {
            ::help( ::basename( inArgv[0] ) );
            ::exit( 0 );
        }

        if (::strcmp( inArgv[1], "--" ) == 0)
        {
            if (inArgc == 4)
            {
                inputFilePath = inArgv[2];
                outputFilePath = inArgv[3];
            } else {
                (void) ::fprintf( stderr, "Incorrect number of arguments.\n" );
                ::usage( ::basename( inArgv[0] ) );
                ::exit( 1 );
            }
        } else if (inArgc == 3) {
            inputFilePath = inArgv[1];
            outputFilePath = inArgv[2];
        } else {
            // Trop d'arguments.
            (void) ::fprintf( stderr, "Incorrect number of arguments.\n" );
            ::usage( ::basename( inArgv[0] ) );
            ::exit( 1 );
        }
    }

    TDCLPlatformFiles theFilesIntf;
    TDCLStream* inputStream = nil;
    TDCLStream* outputStream = nil;
    TDCLFile* inputFile = nil;
    TDCLFile* outputFile = nil;

    if (inputFilePath)
    {
        inputFile = new TDCLPlatformFile( &theFilesIntf, inputFilePath );
        inputFile->Open( true );
        inputStream = inputFile;
    } else {
        inputStream = new TDCLStdStream();
    }
    if (outputFilePath)
    {
        try {
            outputFile = new TDCLPlatformFile( &theFilesIntf, outputFilePath );
            outputFile->Create();
            outputFile->Open(false);
            outputStream = outputFile;
        } catch (TDCLExistsAlreadyException& anException) {
            fprintf(stderr, "File %s already exists\n", outputFilePath);
            return 1;
        }
    } else {
        outputStream = new TDCLStdStream();
    }

    try {
        // Lecture du paquet.
        TDCLPackage thePackage( inputStream );
        if (thePackage.GetPackageFlags() & TDCLPackage::kWatsonSignaturePresentFlag) {
            fprintf(stderr, "Package is already prepared for Watson\n");
            return 1;
        }
        if (thePackage.GetRelocation()) {
            fprintf(stderr, "Package contains relocations, which is incompatible with Watson's signature system\n");
            return 1;
        }
        TDCLPkgPart* theSignature = new TDCLPkgPart(0, (const KUInt8*) "xxxxSLB0Schlumberger Industries", 32);
        thePackage.AddPart(0, TDCLPackage::kPartRawPart, theSignature);
        thePackage.SetPackageFlags(thePackage.GetPackageFlags() | TDCLPackage::kWatsonSignaturePresentFlag);
		thePackage.WriteToStream( outputStream );
    } catch (TDCLException& anException) {
        // On affiche l'exception.
        (void) ::fprintf( stderr, "A DCL exception occurred (%s, %i, %i)\n",
                            anException.GetExceptionName(),
                            (int) anException.GetExCode(),
                            (int) anException.GetError() );
        theResult = -1;
    } catch (std::exception& anException) {
        // Exception inconnue.
        (void) ::fprintf( stderr, "A STD exception occurred (%s)\n",
                            anException.what() );
        theResult = -1;
    } catch (...) {
        // Exception inconnue.
        (void) ::fprintf( stderr, "An unknown exception occurred\n" );
        theResult = -1;
    }

    // Ménage.
    if (inputFile)
    {
        inputFile->Close();
    }
    if (outputFile)
    {
        outputFile->Close();
    }
    if (inputStream)
    {
        delete inputStream;
    }
    if (outputStream)
    {
        delete outputStream;
    }

    return theResult;
}

// ======================================================================== //
// There was, it appeared, a mysterious rite of initiation through which,   //
// in one way or another, almost every member of the team passed.  The term //
// that the old hands used for this rite -- West invented the term, not the //
// practice -- was `signing up.'  By signing up for the project you agreed  //
// to do whatever was necessary for success.  You agreed to forsake, if     //
// necessary, family, hobbies, and friends -- if you had any of these left  //
// (and you might not, if you had signed up too many times before).         //
//                 -- Tracy Kidder, "The Soul of a New Machine"             //
// ======================================================================== //
