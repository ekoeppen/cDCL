// ==============================
// Fichier:         DumpPkgPart.cp
// Projet:          DCL - PackageUtils
// Ecrit par:       Paul Guyot (pguyot@kallisys.net)
//
// Créé le:         12/1/2004
// Tabulation:      4 espaces
//
// Copyright:       © 2000-2004 by Paul Guyot.
//                  Tous droits réservés pour tous pays.
// Licence:         Licence Réflexive Kallisys
//                  http://www.kallisys.org/reflexive/
// ===========
// $Id: DumpPkgPart.cp,v 1.9 2004/04/29 08:57:17 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>

// ANSI C & POSIX
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

// ISO C++
#include <stdexcept>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgNOSPart.h>
#include <DCL/Package/TDCLPkgPart.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <DCL/Streams/TDCLStdStream.h>
#include <DCL/Streams/TDCLStream.h>

// -------------------------------------------------------------------------- //
// Prototypes
// -------------------------------------------------------------------------- //
///
/// Affiche la syntaxe.
///
/// \param inToolName   nom de la commande.
///
void usage( const char* inToolName );

///
/// Affiche l'aide.
///
/// \param inToolName   nom de la commande.
///
void help( const char* inToolName );

///
/// Extrait une partie.
///
/// \param inFilesIntf      interface pour les fichiers.
/// \param inPackage        paquet dont on veut une partie.
/// \param inPartIndex      indice de la partie que l'on souhaite extraire.
/// \param outPartFilePath  chemin pour le fichier de sortie ou \c nil pour
///                         stdout.
/// \param inXMLize         si le fichier de sortie doit être converti en XML
///                         (lorsque c'est possible).
/// \param inNSOFize        si le fichier de sortie doit être converti en NSOF
///                         (lorsque c'est possible).
///
void DumpPart(
                TDCLPOSIXFiles* inFilesIntf,
                TDCLPackage* inPackage,
                KUInt32 inPartIndex,
                const char* outPartFilePath,
                Boolean inXMLize,
                Boolean inNSOFize );

///
/// Point d'entrée.
///
/// \param inArgc       nombre d'arguments
/// \param inArgv       les arguments
/// \return un code d'erreur ou 0
///
int main( int inArgc, char** inArgv );

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * usage( const char* )
// -------------------------------------------------------------------------- //
void
usage( const char* inToolName )
{
    (void) ::fprintf( stderr, "Syntaxe: %s [options] [--] [chemin]\n", inToolName );
    (void) ::fprintf( stderr,
                        "Essayez %s --help pour plus d'information\n", inToolName );
}

// -------------------------------------------------------------------------- //
//  * help( const char* inToolName )
// -------------------------------------------------------------------------- //
void
help( const char* inToolName )
{
    (void) ::fprintf( stderr, "%s - extrait une ou les parties d'un paquet Newton (fichier ou stdin).\n", inToolName );
    (void) ::fprintf( stderr, "%s [options] [--] [chemin]\n", inToolName );
    (void) ::fprintf( stderr, "   -h|-?       afficher cette aide.\n" );
    (void) ::fprintf( stderr, "   -n <x>      extraire seulement la partie x.\n" );
    (void) ::fprintf( stderr, "   -n -<x>     extraire seulement la partie x\n" );
    (void) ::fprintf( stderr, "               en partant de la fin.\n" );
    (void) ::fprintf( stderr, "   -x          convertir en XML lorsque c'est possible.\n" );
    (void) ::fprintf( stderr, "   -s          convertir en NSOF lorsque c'est possible.\n" );
    (void) ::fprintf( stderr, "   -o -        extraire la/les parties sur stdout.\n" );
    (void) ::fprintf( stderr, "   -o prefixe  extraire la/les parties dans prefixe.n\n" );
    (void) ::fprintf( stderr, "               (ou juste prefixe si utilisé avec -n)\n" );
    (void) ::fprintf( stderr, "   -V addr     adresse virtuelle (paquets en ROM/chargés)\n" );
}

// -------------------------------------------------------------------------- //
//  * DumpPart( TDCLPOSIXFiles*, TDCLPackage*, KUInt32, const char*, ... )
// -------------------------------------------------------------------------- //
void
DumpPart(
        TDCLPOSIXFiles* inFilesIntf,
        TDCLPackage* inPackage,
        KUInt32 inPartIndex,
        const char* outPartFilePath,
        Boolean inXMLize,
        Boolean inNSOFize )
{
    TDCLStream* theOutputStream = nil;
    TDCLFile* theOutputFile = nil;

    if (outPartFilePath)
    {
        theOutputFile = new TDCLPOSIXFile( inFilesIntf, outPartFilePath );
        theOutputFile->Create();
        theOutputFile->Open( false );
        theOutputStream = theOutputFile;
    } else {
        theOutputStream = new TDCLStdStream();
    }

    try {
        TDCLPkgPart* thePart = inPackage->GetPart( inPartIndex );

        if (inXMLize && thePart->IsNOSPart())
        {
            // Encodage en XML.
            TDCLXMLEncoder xmlEncoder( theOutputStream );
            xmlEncoder.AddObject( ((TDCLPkgNOSPart*) thePart)->GetObject() );
        } else if (inNSOFize && thePart->IsNOSPart()) {
            // Encodage en NSOF.
            TDCLNSOFEncoder nsofEncoder( theOutputStream );
            nsofEncoder.AddObject( ((TDCLPkgNOSPart*) thePart)->GetObject() );
        } else {
            KUInt32 thePartSize = thePart->GetSize();
            theOutputStream->Write( thePart->GetBuffer(), &thePartSize );
        }
    } catch (...) {
        // Ménage.
        if (theOutputFile)
        {
            theOutputFile->Close();
        }
        if (theOutputStream)
        {
            delete theOutputStream;
        }

        throw;
    }

    // Ménage.
    if (theOutputFile)
    {
        theOutputFile->Close();
    }
    if (theOutputStream)
    {
        delete theOutputStream;
    }
}

// -------------------------------------------------------------------------- //
//  * main( int, char** )
// -------------------------------------------------------------------------- //
int
main( int inArgc, char** inArgv )
{
    char* theToolName = inArgv[0];

    // On accepte --help
    if ((inArgc > 1) && (::strcmp( inArgv[1], "--help" ) == 0))
    {
        ::help( ::basename( theToolName ) );
        ::exit( 0 );
    }

    int theResult = 0;

    char* theFilePath = nil;
    Boolean wantOnePartOnly = false;
    int partIndex = 0;
    Boolean wantStdout = false;
    char* theOutputPrefix = nil;
    Boolean wantXML = false;
    Boolean wantNSOF = false;
    KUInt32 theVAddr = 0;

    int theOptionChar;
    while ((theOptionChar = getopt( inArgc, inArgv, "hn:o:xsV:")) != -1)
        switch (theOptionChar) {
            case 'h':
            case '?':
            {
                ::help( ::basename( theToolName ) );
                ::exit( 0 );
            }
            break;

            case 'n':
            {
                wantOnePartOnly = true;
                partIndex = ::atoi( optarg );
            }
            break;

            case 'o':
            {
                if (::strcmp( optarg, "-" ) == 0)
                {
                    wantStdout = true;
                } else {
                    theOutputPrefix = optarg;
                }
            }
            break;

            case 'x':
            {
                wantXML = true;
            }
            break;

            case 's':
            {
                wantNSOF = true;
            }
            break;

            case 'V':
            {
                theVAddr = strtoul(optarg, NULL, 0);
            }
            break;

            default:
            {
                ::usage( ::basename( theToolName ) );
                ::exit( 1 );
            }
    }

    inArgc -= optind;
    inArgv += optind;

    if (inArgc == 1)
    {
        theFilePath = inArgv[0];
    } else if (inArgc > 1) {
        // Trop d'arguments.
        (void) ::fprintf( stderr, "Trop d'arguments.\n" );
        ::usage( ::basename( theToolName ) );
        ::exit( 1 );
    }

    if (wantXML && wantNSOF)
    {
        (void) ::fprintf( stderr, "-x and -s are incompatible options.\n" );
        ::usage( ::basename( theToolName ) );
        ::exit( 1 );
    }

    if ((!wantStdout) && (theOutputPrefix == nil))
    {
        // Si theFilePath vaut nil, on aura toujours theOutputPrefix
        // égal à nil.
        theOutputPrefix = theFilePath;
    }

    TDCLPOSIXFiles theFilesIntf;
    TDCLStream* theStream = nil;
    TDCLFile* theFile = nil;

    if (theFilePath)
    {
        theFile = new TDCLPOSIXFile( &theFilesIntf, theFilePath );
        theFile->Open( true );
        theStream = theFile;
    } else {
        theStream = new TDCLStdStream();
    }

    char* thePartFilePath = nil;

    try {
        // Lecture du paquet.
        TDCLPackage thePackage( theStream, theVAddr );

        // Itération sur les parties.
        KUInt32 nbParts = thePackage.GetNumParts();
        if (wantOnePartOnly)
        {
            KUInt32 theActualPartIndex = (KUInt32) partIndex;
            if (partIndex < 0)
            {
                theActualPartIndex = (KUInt32) (nbParts + partIndex);
            }

            if (theActualPartIndex >= nbParts)
            {
                (void) ::fprintf( stderr, "Aucune partie d'indice %i\n", partIndex );
                ::exit( 2 );
            }

            DumpPart( &theFilesIntf, &thePackage, theActualPartIndex, theOutputPrefix, wantXML, wantNSOF );
        } else {
            char* thePartFilePathIndexPart = nil;
            if (theOutputPrefix)
            {
                size_t thePrefixSize = ::strlen( theOutputPrefix );
                thePartFilePath = (char*) ::malloc( thePrefixSize + 9 );
                (void) ::memcpy(
                                (void*) thePartFilePath,
                                (const void*) theOutputPrefix,
                                thePrefixSize );
                thePartFilePath[thePrefixSize] = '.';
                thePartFilePathIndexPart = &thePartFilePath[thePrefixSize + 1];
            }

            KUInt32 indexParts;
            for (indexParts = 0; indexParts < nbParts; indexParts++)
            {
                if (thePartFilePath)
                {
                    (void) ::sprintf( thePartFilePathIndexPart, "%u", (unsigned int) indexParts );
                }

                DumpPart( &theFilesIntf, &thePackage, indexParts, thePartFilePath, wantXML, wantNSOF );
            }
        }
    } catch (TDCLDoesntExistException& anException) {
        // Le fichier n'existe pas.
        (void) ::fprintf( stderr, "Le paquet est introuvable\n" );
    } catch (TDCLExistsAlreadyException& anException) {
        // Le fichier existe déjà.
        (void) ::fprintf( stderr, "Un fichier cible existe déjà\n" );
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
    if (thePartFilePath)
    {
        ::free( thePartFilePath );
    }
    if (theFile)
    {
        theFile->Close();
    }
    if (theStream)
    {
        delete theStream;
    }

    return theResult;
}

// ========================================================================= //
// `Lasu' Releases SAG 0.3 -- Freeware Book Takes Paves For New World Order  //
// by staff writers                                                          //
//                                                                           //
// Helsinki, Finland, August 6, 1995 -- In a surprise movement, Lars         //
// ``Lasu'' Wirzenius today released the 0.3 edition of the ``Linux System   //
// Administrators' Guide''.  Already an industry non-classic, the new        //
// version sports such overwhelming features as an overview of a Linux       //
// system, a completely new climbing session in a tree, and a list of        //
// acknowledgements in the introduction.                                     //
//         The SAG, as the book is affectionately called, is one of the      //
// corner stones of the Linux Documentation Project.  ``We at the LDP feel   //
// that we wouldn't be able to produce anything at all, that all our work    //
// would be futile, if it weren't for the SAG,'' says Matt Welsh, director   //
// of LDP, Inc.                                                              //
//         The new version is still distributed freely, now even with a      //
// copyright that allows modification.  ``More dough,'' explains the author. //
// Despite insistent rumors about blatant commercialization, the SAG will    //
// probably remain free.  ``Even more dough,'' promises the author.          //
//         The author refuses to comment on Windows NT and Windows 96        //
// versions, claiming not to understand what the question is about.          //
// Industry gossip, however, tells that Bill Gates, co-founder and CEO of    //
// Microsoft, producer of the Windows series of video games, has visited     //
// Helsinki several times this year.  Despite of this, Linus Torvalds,       //
// author of the word processor Linux with which the SAG was written, is     //
// not worried.  ``We'll have world domination real soon now, anyway,'' he   //
// explains, ``for 1.4 at the lastest.''                                     //
//         ...                                                               //
//                 -- Lars Wirzenius <wirzeniu@cs.helsinki.fi>               //
//                    [comp.os.linux.announce]                               //
// ========================================================================= //
