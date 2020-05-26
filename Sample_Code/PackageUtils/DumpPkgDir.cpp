// ==============================
// Fichier:         DumpPkgDir.cp
// Projet:          DCL - PackageUtils
// Ecrit par:       Paul Guyot (pguyot@kallisys.net)
//
// Créé le:         26/4/2000
// Tabulation:      4 espaces
//
// Copyright:       © 2000-2004 by Paul Guyot.
//                  Tous droits réservés pour tous pays.
// Licence:         Licence Réflexive Kallisys
//                  http://www.kallisys.org/reflexive/
// ===========
// $Id: DumpPkgDir.cp,v 1.6 2004/04/15 15:53:37 paul Exp $
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
    (void) ::fprintf( stderr, "Syntaxe: %s [options] [--] [chemin]\n", inToolName );
    (void) ::fprintf( stderr,
                        "Essayez %s --help pour plus d'information\n", inToolName );
}

// -------------------------------------------------------------------------- //
//  * help( char* inToolName )
// -------------------------------------------------------------------------- //
void
help( char* inToolName )
{
    (void) ::fprintf( stderr, "%s - affiche le catalogue d'un paquet Newton (fichier ou stdin).\n", inToolName );
    (void) ::fprintf( stderr, "%s [options] [--] [chemin]\n", inToolName );
    (void) ::fprintf( stderr, "   --help      afficher cette aide.\n" );
}

// -------------------------------------------------------------------------- //
//  * main( int, char** )
// -------------------------------------------------------------------------- //
int
main( int inArgc, char** inArgv )
{
    int theResult = 0;

    char* theFilePath = nil;
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
            if (inArgc == 3)
            {
                theFilePath = inArgv[2];
            } else if (inArgc != 2) {
                // Trop d'arguments.
                (void) ::fprintf( stderr, "Trop d'arguments.\n" );
                ::usage( ::basename( inArgv[0] ) );
                ::exit( 1 );
            }
        } else if (inArgc == 2) {
            theFilePath = inArgv[1];
        } else {
            // Trop d'arguments.
            (void) ::fprintf( stderr, "Trop d'arguments.\n" );
            ::usage( ::basename( inArgv[0] ) );
            ::exit( 1 );
        }
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

    try {
        // Lecture du paquet.
        TDCLPackage thePackage( theStream );

        // Affichage des informations.
        char someCString[512];
        UUTF16CStr::ToISO88591( thePackage.GetPackageName(), (KUInt8*) someCString, 512 );
        (void) ::printf( "Name:           %s\n", someCString );

        const KUInt16* theCopyrightStr = thePackage.GetCopyrightString();
        if (theCopyrightStr)
        {
            UUTF16CStr::ToISO88591( thePackage.GetCopyrightString(), (KUInt8*) someCString, 512 );
            (void) ::printf( "Copyright:      %s\n", someCString );
        } else {
            (void) ::printf( "Copyright:      (null)\n" );
        }

        KUInt32 theID = thePackage.GetPackageID();
        (void) ::printf( "ID:             %.8X (%c%c%c%c)\n",
                    (unsigned int) theID,
                    (char) ((theID >> 24) & 0xFF),
                    (char) ((theID >> 16) & 0xFF),
                    (char) ((theID >> 8) & 0xFF),
                    (char) (theID & 0xFF) );
        (void) ::printf( "Version:        %u\n", (unsigned int) thePackage.GetPackageVersion() );
        KUInt32 theFlags = thePackage.GetPackageFlags();
        (void) ::printf( "Flags:          %.8X", (unsigned int) theFlags );
        Boolean hasSomeFlag = false;
        if (theFlags & TDCLPackage::kDirAutoRemoveFlag)
        {
            hasSomeFlag = true;
            (void) ::printf( " ( AutoRemove" );
        }
        if (theFlags & TDCLPackage::kDirCopyProtectFlag)
        {
            if (hasSomeFlag)
            {
                (void) ::printf( " + CopyProtected" );
            } else {
                hasSomeFlag = true;
                (void) ::printf( " ( CopyProtected" );
            }
        }
        if (theFlags & TDCLPackage::kDirNoCompressionFlag)
        {
            if (hasSomeFlag)
            {
                (void) ::printf( " + NoCompression" );
            } else {
                hasSomeFlag = true;
                (void) ::printf( " ( NoCompression" );
            }
        }
        if (theFlags & TDCLPackage::kDirRelocationFlag)
        {
            if (hasSomeFlag)
            {
                (void) ::printf( " + Relocation" );
            } else {
                hasSomeFlag = true;
                (void) ::printf( " ( Relocation" );
            }
        }
        if (theFlags & TDCLPackage::kDirUseFasterCompressionFlag)
        {
            if (hasSomeFlag)
            {
                (void) ::printf( " + UseFasterCompression" );
            } else {
                hasSomeFlag = true;
                (void) ::printf( " ( UseFasterCompression" );
            }
        }
        if (hasSomeFlag)
        {
            (void) ::printf( " )\n" );
        } else {
            (void) ::printf( "\n" );
        }
        (void) ::printf( "Creation date:  %.8X\n", (unsigned int) thePackage.GetCreationDate() );

        if (!thePackage.IsNOS1Compatible())
        {
            (void) ::printf( "NewtonOS 2.x only\n" );
        }

        // Affichage des données de relocation
        if (thePackage.GetRelocation())
        {
            (void) ::printf( "---- Relocation data ----\n" );
            const TDCLPackage::SPackageRelocationData* relocationData = thePackage.GetRelocationData();
            (void) ::printf( "Relocation size:   %.8X\n", relocationData->fRelocationSize );
            KUInt32 pageSize = relocationData->fPageSize;
            (void) ::printf( "Page size:         %u\n", relocationData->fPageSize );
            (void) ::printf( "Number of entries: %d\n", relocationData->fNumEntries );
            (void) ::printf( "Base address:      %.8X\n", relocationData->fBaseAddress );
            for ( KUInt32 entriesIx = 0; entriesIx < relocationData->fNumEntries; entriesIx++ )
            {
                (void) ::printf( "  ---- relocation set #%u ----\n", entriesIx );
                KUInt16 pageNumber = relocationData->fRelocationSets[entriesIx].fPageNumber;
                (void) ::printf( "  Page number : %.4X --\n", pageNumber );
                KUInt16 offsetCount = relocationData->fRelocationSets[entriesIx].fOffsetCount;
                (void) ::printf( "  Offset count: %u --\n", offsetCount );
                for ( KUInt16 offsetIx = 0; offsetIx < offsetCount; offsetIx++ )
                {
                    KUInt16 offset = relocationData->fRelocationSets[entriesIx].fOffsets[offsetIx];
                    KUInt32 offsetInPackage = (pageNumber * pageSize) + (offset << 2);
                    (void) ::printf( "  %.2X (%.8X)\n", offset, offsetInPackage );
                }
            }
        }

        // Itération sur les parties.
        KUInt32 nbParts = thePackage.GetNumParts();
        KUInt32 indexParts;
        for (indexParts = 0; indexParts < nbParts; indexParts++)
        {
            (void) ::printf( "  ---- part #%u/%u ----\n",
                                    (unsigned int) indexParts + 1,
                                    (unsigned int) nbParts );
            KUInt32 theType = thePackage.GetPartType( indexParts );
            (void) ::printf( "  Type:           %.8X (%c%c%c%c)\n",
                        (unsigned int) theType,
                        (char) ((theType >> 24) & 0xFF),
                        (char) ((theType >> 16) & 0xFF),
                        (char) ((theType >> 8) & 0xFF),
                        (char) (theType & 0xFF) );
            TDCLPkgPart* thePart = thePackage.GetPart( indexParts );
            unsigned int theSize = (unsigned int) thePart->GetSize();
            (void) ::printf( "  Size:           %u (0x%X)\n", theSize, theSize );
            unsigned int theOffset = (unsigned int) thePart->GetOffset();
            (void) ::printf( "  Offset:         %u (0x%X)\n", theOffset, theOffset );

            theFlags = thePackage.GetPartFlags( indexParts );
            (void) ::printf( "  Flags:          %.8X", (unsigned int) theFlags );

            KUInt32 thePartType = theFlags & TDCLPackage::kPartTypeMask;
            if (thePartType == TDCLPackage::kPartProtocolPart)
            {
                (void) ::printf( " ( ProtocolPart" );
            } else if (thePartType == TDCLPackage::kPartNOSPart) {
                (void) ::printf( " ( NOSPart" );
            } else if (thePartType == TDCLPackage::kPartRawPart) {
                (void) ::printf( " ( RawPart" );
            } else {
                (void) ::printf( " ( ?" );
            }
            if (theFlags & TDCLPackage::kPartAutoLoadFlag)
            {
                (void) ::printf( " + AutoLoad" );
            }
            if (theFlags & TDCLPackage::kPartAutoRemoveFlag)
            {
                (void) ::printf( " + AutoRemove" );
            }
            if (theFlags & TDCLPackage::kPartNotifyFlag)
            {
                (void) ::printf( " + Notify" );
            }
            if (theFlags & TDCLPackage::kPartAutoCopyFlag)
            {
                (void) ::printf( " + AutoCopy" );
            }
            (void) ::printf( " )\n" );

            // On affiche les données pour le gestionnaire.
            KUInt32 theHandlerInfoSize;
            const KUInt8* theHandlerInfo =
                thePackage.GetPartHandlerInfo( indexParts, &theHandlerInfoSize );

            char theASCIILine[17];
            theASCIILine[16] = 0;
            KUInt32 theIndexMod16 = 0;

            KUInt32 handlerInfoIndex;
            for (
                    handlerInfoIndex = 0;
                    handlerInfoIndex < theHandlerInfoSize;
                    handlerInfoIndex++ )
            {
                theIndexMod16 = handlerInfoIndex % 16;
                if (theIndexMod16 == 0)
                {
                    if (handlerInfoIndex == 0)
                    {
                        (void) ::printf( "  Handler Info:   " );
                    } else {
                        (void) ::printf( " %s\n                  ", theASCIILine );
                    }
                }
                KUInt8 theByte = theHandlerInfo[handlerInfoIndex];
                if ((theByte >= 0x20) && (theByte < 0x7F))
                {
                    theASCIILine[theIndexMod16] = (char) theByte;
                } else {
                    theASCIILine[theIndexMod16] = '.';
                }

                (void) ::printf( "%.2X", (unsigned int) theByte );
            }
            if (theHandlerInfoSize > 0)
            {
                KUInt32 fillerIndex = theIndexMod16 + 1;
                theASCIILine[fillerIndex] = '\0';
                for (; fillerIndex < 16; fillerIndex++)
                {
                    (void) ::printf( "  " );
                }
                (void) ::printf( " %s\n", theASCIILine );
            } else {
                (void) ::printf( "\n" );
            }
        }

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
