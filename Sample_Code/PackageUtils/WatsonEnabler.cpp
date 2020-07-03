// ==============================
// Fichier:         WatsonEnabler.cp
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
#include <libgen.h>

// ISO C++
#include <stdexcept>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
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
    (void) ::fprintf( stderr, "Syntaxe: %s [options] [--] [input output]\n", inToolName );
    (void) ::fprintf( stderr,
                        "Essayez %s --help pour plus d'information\n", inToolName );
}

// -------------------------------------------------------------------------- //
//  * help( char* inToolName )
// -------------------------------------------------------------------------- //
void
help( char* inToolName )
{
    (void) ::fprintf( stderr, "%s - 'signe' le paquet pour Watson (fichier ou stdin).\n", inToolName );
    (void) ::fprintf( stderr, "%s [options] [--] [input output]\n", inToolName );
    (void) ::fprintf( stderr, "   --help      afficher cette aide.\n" );
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
                (void) ::fprintf( stderr, "Nombre incorrect d'arguments.\n" );
                ::usage( ::basename( inArgv[0] ) );
                ::exit( 1 );
            }
        } else if (inArgc == 3) {
            inputFilePath = inArgv[1];
            outputFilePath = inArgv[2];
        } else {
            // Trop d'arguments.
            (void) ::fprintf( stderr, "Nombre incorrect d'arguments.\n" );
            ::usage( ::basename( inArgv[0] ) );
            ::exit( 1 );
        }
    }

    TDCLPOSIXFiles theFilesIntf;
    TDCLStream* inputStream = nil;
    TDCLStream* outputStream = nil;
    TDCLFile* inputFile = nil;
    TDCLFile* outputFile = nil;

    if (inputFilePath)
    {
        inputFile = new TDCLPOSIXFile( &theFilesIntf, inputFilePath );
        inputFile->Open( true );
        inputStream = inputFile;
    } else {
        inputStream = new TDCLStdStream();
    }
    if (outputFilePath)
    {
        outputFile = new TDCLPOSIXFile( &theFilesIntf, outputFilePath );
        outputFile->Create();
        outputFile->Open(false);
        outputStream = outputFile;
    } else {
        outputStream = new TDCLStdStream();
    }

    try {
        // Lecture du paquet.
        TDCLPackage thePackage( inputStream );
        TDCLPkgPart* theSignature = new TDCLPkgPart(0, "xxxxSLB0Schlumberger Industries", 32);
        thePackage.AddPart(0, TDCLPackage::kPartRawPart, theSignature);
        thePackage.SetPackageFlags(thePackage.GetPackageFlags() | TDCLPackage::kWatsonSignaturePresentFlag);
		thePackage.WriteToStream( outputStream );
    } catch (TDCLException& anException) {
        // On affiche l'exception.
        (void) ::fprintf( stderr, "Une exception DCL est survenue (%s, %i, %i)\n",
                            anException.GetExceptionName(),
                            (int) anException.GetExCode(),
                            (int) anException.GetError() );
        theResult = -1;
    } catch (std::exception& anException) {
        // Exception inconnue.
        (void) ::fprintf( stderr, "Une exception STD est survenue (%s)\n",
                            anException.what() );
        theResult = -1;
    } catch (...) {
        // Exception inconnue.
        (void) ::fprintf( stderr, "Une exception inconnue est survenue\n" );
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
