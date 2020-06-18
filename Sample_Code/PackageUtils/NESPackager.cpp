// ==============================
// Fichier:			NESPackager.cp
// Projet:			(DCL - PackageUtils)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			4/4/2004
// Tabulation:		4 espaces
// 
// Copyright:		© 2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: NESPackager.cp,v 1.13 2004/10/20 13:29:29 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>

// ANSI C & POSIX
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/param.h>

// ISO C++
#include <stdexcept>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSBinary.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgNOSPart.h>
#include <DCL/Streams/TDCLStdStream.h>
#include <DCL/Streams/TDCLStream.h>

///
/// NESPackager utilise le fichier nespkgtemplate.nsof comme modèle.
/// Celui-ci doit être ou bien:
/// - à côté de NESPackager (c'est là qu'il le cherche d'abord)
/// - dans ../share/DCL/
/// (le chemin est relatif au chemin de NESPackager).
/// - dans ${install_prefix}/share/DCL/nespkgtemplate.nsof
/// - ou bien fourni par l'option -m de la ligne de commande.
///

#define pkgTemplatePathSuffix1	"/nespkgtemplate.nsof"
#define pkgTemplatePathSuffix2	"/../share/DCL/nespkgtemplate.nsof"
#define pkgTemplateInstallPath  INSTALL_PREFIX "/share/DCL/nespkgtemplate.nsof"

// -------------------------------------------------------------------------- //
// Prototypes
// -------------------------------------------------------------------------- //
///
/// Affiche la syntaxe.
///
/// \param inToolName	nom de la commande.
///
void usage( const char* inToolName );

///
/// Affiche l'aide.
///
/// \param inToolName	nom de la commande.
///
void help( const char* inToolName );

///
/// Récupère le modèle à partir du fichier.
///
/// \param inFilesIntf		interface pour les fichiers.
/// \param inTemplatePath	chemin (option -m) ou \c nil
/// \return le modèle pour le paquet.
///
TDCLNSRef GetTemplate(
			TDCLPOSIXFiles* inFilesIntf,
			const char* inTemplatePath,
			char* inToolName );

///
/// Construit la signature du paquet.
///
/// \param inPackageName	nom du paquet.
/// \return la signature du paquet (symbole)
///
TDCLNSRef BuildPackageSym( const char* inPackageName );

///
/// Construit la structure du paquet.
///
/// \param inROMStream		flux vers la ROM
/// \param inPackageName	nom du paquet.
/// \param inPackageSym		signature (de BuildPackageSym)
/// \param inTemplateFrame	modèle de paquet.
/// \return la structure du paquet.
///
void BuildPackageFrame(
			TDCLStream* inROMStream,
			const char* inPackageName,
			const TDCLNSRef& inPackageSym,
			const TDCLNSRef& inTemplateFrame );

///
/// Construit le paquet.
///
/// \param inFilesIntf		interface pour les fichiers.
/// \param inPackageFrame	structure du paquet (de BuildPackageFrame)
/// \param outPkgFilePath	chemin vers le fichier de sortie ou \c nil pour
///							stdout.
/// \param inPackageSym		signature (de BuildPackageSym)
///
void BuildPackage(
			TDCLPOSIXFiles* inFilesIntf,
			const TDCLNSRef& inPackageFrame,
			const char* outPkgFilePath,
			const TDCLNSRef& inPackageSym );

///
/// Point d'entrée.
///
/// \param inArgc		nombre d'arguments
/// \param inArgv		les arguments
/// \return un code d'erreur ou 0
///
int main( int inArgc, char** inArgv );

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //
static const char* const kPackageNameSuffix = ":NESPackager";
static const char* const kPackageCopyrightString =
	"(c) Nintendo. DO NOT USE WITHOUT LEGAL COPY OF THIS GAME!!!!!!";

// -------------------------------------------------------------------------- //
//  * usage( void )
// -------------------------------------------------------------------------- //
void
usage( const char* inToolName )
{
	(void) ::fprintf( stderr, "Syntax: %s [options] [--] [path]\n", inToolName );
	(void) ::fprintf( stderr,
						"Try %s --help for more information\n", inToolName );
}

// -------------------------------------------------------------------------- //
//  * help( const char* inToolName )
// -------------------------------------------------------------------------- //
void
help( const char* inToolName )
{
	(void) ::fprintf( stderr,
		"%s - compile a Newtendo package from a ROM file (or stdin).\n", inToolName );
	(void) ::fprintf( stderr,
		"%s [options] [--] [path]\n", inToolName );
	(void) ::fprintf( stderr,
		"   -h|-?       show this help.\n" );
	(void) ::fprintf( stderr,
		"   -n name     package name.\n" );
	(void) ::fprintf( stderr,
		"   -o path     path of the resulting package [guessed].\n" );
	(void) ::fprintf( stderr,
		"   -o -        send the result to stdout.\n" );
	(void) ::fprintf( stderr,
		"   -m path     path to the package template (NSOF stream).\n" );
}

// -------------------------------------------------------------------------- //
//  * GetTemplate( TDCLPOSIXFiles*, const char*, const char* )
// -------------------------------------------------------------------------- //
TDCLNSRef
GetTemplate(
		TDCLPOSIXFiles* inFilesIntf,
		const char* inTemplatePath,
		char* inToolName )
{
	TDCLNSRef theResult;
	TDCLFile* theTemplateFile = nil;
	char* theTemplatePath = nil;
	try {
		if (inTemplatePath)
		{
			theTemplateFile = new TDCLPOSIXFile( inFilesIntf, inTemplatePath );
			theTemplateFile->Open( true );
		} else {
			// Recherche à côté de NESPackager.
			size_t theBaseLength = ::strlen( ::dirname( inToolName ) );
			size_t theSuffix1Length = ::strlen( pkgTemplatePathSuffix1 );
			size_t theSuffix2Length = ::strlen( pkgTemplatePathSuffix2 );
			size_t theMaxLength =
				theSuffix1Length > theSuffix2Length
				? theSuffix1Length : theSuffix2Length;
			theTemplatePath = (char*) ::malloc( theBaseLength + theMaxLength + 1 );
			(void) ::memcpy(
						(void*) theTemplatePath,
						(const void*) ::dirname( inToolName ),
						theBaseLength );
			(void) ::memcpy(
						(void*) &theTemplatePath[theBaseLength],
						(const void*) pkgTemplatePathSuffix1,
						theSuffix1Length + 1 );	// terminateur.
			
			try {
				theTemplateFile = new TDCLPOSIXFile( inFilesIntf, theTemplatePath );
				theTemplateFile->Open( true );
			} catch (TDCLDoesntExistException& anException) {
			    try {
                    // Recherche en ../share/DCL/
                    (void) ::memcpy(
                            (void*) &theTemplatePath[theBaseLength],
                            (const void*) pkgTemplatePathSuffix2,
                            theSuffix2Length + 1 );	// terminateur.
                    theTemplateFile = new TDCLPOSIXFile( inFilesIntf, theTemplatePath );
                    theTemplateFile->Open( true );
                } catch (TDCLDoesntExistException& anException) {
                    theTemplateFile = new TDCLPOSIXFile( inFilesIntf, pkgTemplateInstallPath );
                    theTemplateFile->Open( true );
                }
			}
			
			::free( theTemplatePath );
			theTemplatePath = nil;
		} // if (inTemplatePath)

		{
			TDCLNSOFDecoder theDecoder( theTemplateFile );
			theResult = theDecoder.GetNextObject();
		}
		
		theTemplateFile->Close();
		delete theTemplateFile;
		theTemplateFile = nil;
	} catch (TDCLDoesntExistException& anException) {
		if (inTemplatePath)
		{
			(void) ::fprintf(
						stderr,
						"Could not find template file %s\n",
						inTemplatePath );
		} else {
			(void) ::fprintf(
						stderr,
						"Could not find template file. "
						"Provide -m option or put it next to NESPackager.\n" );
		}

		// Ménage.
		if (theTemplatePath)
		{
			::free( theTemplatePath );
		}
		
		throw;
	} catch (...) {
		(void) ::fprintf(
					stderr,
					"We have a problem with the template file.\n" );

		// Ménage.
		if (theTemplatePath)
		{
			::free( theTemplatePath );
		}
		if (theTemplateFile)
		{
			theTemplateFile->Close();
			delete theTemplateFile;
		}

		throw;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * BuildPackageSym( const char* )
// -------------------------------------------------------------------------- //
TDCLNSRef
BuildPackageSym( const char* inPackageName )
{
	// Taille de la chaîne.
	KUInt32 theNameSize = ::strlen( inPackageName );
	KUInt32 theSuffixSize = ::strlen( kPackageNameSuffix );
	
	KUInt32 theSize = theNameSize + theSuffixSize;
	
	// On tronque.
	if (theSize > TDCLPackage::kPackageNameMaxLength)
	{
		theNameSize = TDCLPackage::kPackageNameMaxLength - theSuffixSize;
		theSize = TDCLPackage::kPackageNameMaxLength;
	}
	
	// Chaîne avec le suffixe.
	char* thePackageNameWithSig = (char*) ::malloc( theSize + 1 );
	(void) ::memcpy(
				(void*) thePackageNameWithSig,
				(const void*) inPackageName,
				theNameSize );
	// Copie avec le terminateur.
	(void) ::memcpy(
				(void*) &thePackageNameWithSig[theNameSize],
				(const void*) kPackageNameSuffix,
				theSuffixSize + 1 );
	
	TDCLNSRef theResult = TDCLNSRef::MakeSymbol( thePackageNameWithSig );

	// Ménage.
	::free( thePackageNameWithSig );
	
	return theResult;
}


// -------------------------------------------------------------------------- //
//  * BuildPackageFrame( TDCLStream*, const char*, const TDCLNSRef&, ... )
// -------------------------------------------------------------------------- //
void
BuildPackageFrame(
			TDCLStream* inROMStream,
			const char* inPackageName,
			const TDCLNSRef& inPackageSym,
			const TDCLNSRef& inTemplateFrame )
{
	TDCLNSFrame& thePackageFrame = inTemplateFrame.ToFrame();
	
	TDCLNSRef thePackageName = TDCLNSRef::MakeString( inPackageName );
	thePackageFrame.Set( "text", thePackageName );
	thePackageFrame.Set( "app", inPackageSym );

	TDCLNSFrame& theFormFrame = thePackageFrame.Get( "theForm" ).ToFrame();
	theFormFrame.Set( "appSymbol", inPackageSym );

	// Lecture de la ROM.
	size_t ROMPtrSize = 131072;
	size_t ROMSize = 0;
	void* ROMBuffer = ::malloc( ROMPtrSize );
	while (true)
	{
		if (ROMBuffer == nil)
		{
			throw DCLMemError;
		}
		
		KUInt32 toRead = 131072;
		inROMStream->Read( &((char*) ROMBuffer)[ROMSize], &toRead );
		
		ROMSize += toRead;
		
		if (toRead < 131072)
		{
			break;
		}
		
		ROMPtrSize += 131072;
		ROMBuffer = ::realloc( ROMBuffer, ROMPtrSize );
	}
	
	theFormFrame.Set( "rom", TDCLNSRef::MakeBinary(
					ROMBuffer,
					ROMSize,
					TDCLNSRef::MakeSymbol( "newtyESROM" ) ) );
	
	::free( ROMBuffer );

}

// -------------------------------------------------------------------------- //
//  * BuildPackage( TDCLPOSIXFiles*, const TDCLNSRef&, const char*, ... )
// -------------------------------------------------------------------------- //
void
BuildPackage(
		TDCLPOSIXFiles* inFilesIntf,
		const TDCLNSRef& inPackageFrame,
		const char* outPkgFilePath,
		const TDCLNSRef& inPackageSym )
{
	TDCLStream* theOutputStream = nil;
	TDCLFile* theOutputFile = nil;
	
	if (outPkgFilePath)
	{
		theOutputFile = new TDCLPOSIXFile( inFilesIntf, outPkgFilePath );
		theOutputFile->Create();
		theOutputFile->Open( false );
		theOutputStream = theOutputFile;
	} else {
		theOutputStream = new TDCLStdStream();
	}

	try {
		// Création du paquet.
		TDCLPackage thePackage;
		
		thePackage.SetNOS1Compatible( false );
		thePackage.SetPackageID( TDCLPackage::kDefaultID );
		thePackage.SetPackageFlags( 0 );
		thePackage.SetPackageVersion( 1 );

		thePackage.SetPackageName( inPackageSym.ToSymbol().GetString() );

		thePackage.SetCopyrightString( kPackageCopyrightString );

		(void) thePackage.AddPart(
					TDCLPackage::kFormPart,
					TDCLPackage::kPartNOSPart | TDCLPackage::kPartNotifyFlag,
					new TDCLPkgNOSPart( inPackageFrame ) );
		
		thePackage.WriteToStream( theOutputFile );
	} catch ( ... ) {
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
	Boolean wantStdout = false;
	char* theOutputPath = nil;
	Boolean theOutputPathWasAllocated = false;
	char* thePackageName = nil;
	Boolean thePackageNameWasAllocated = false;
	char* theTemplatePath = nil;

	int theOptionChar;
	while ((theOptionChar = getopt( inArgc, inArgv, "h?n:o:m:")) != -1)
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
				thePackageName = optarg ;
			}
			break;
			
			case 'o':
			{
				if (::strcmp( optarg, "-" ) == 0)
				{
					wantStdout = true;
				} else {
					theOutputPath = optarg;
				}
			}
			break;
			
			case 'm':
			{
				theTemplatePath = optarg;
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
		(void) ::fprintf( stderr, "Too many arguments.\n" );
		::usage( ::basename( theToolName ) );
		::exit( 1 );
	}
	
	if ((!wantStdout) && (theOutputPath == nil) && (theFilePath != nil))
	{
		// On détermine le nom du fichier de sortie en ajoutant .pkg.
		size_t theFilePathLength = ::strlen( theFilePath );
		theOutputPath = (char*) ::malloc( theFilePathLength + 5 );
		theOutputPathWasAllocated = true;
		
		(void) ::memcpy(
						(void*) theOutputPath,
						(const void*) theFilePath,
						theFilePathLength );
		(void) ::memcpy(
						(void*) &theOutputPath[theFilePathLength],
						(const void*) ".pkg",
						5 );		
	}
	
	if (thePackageName == nil)
	{
		// On devine le nom du paquet.
		if (theFilePath == nil)
		{
			// Aléatoire (enfin, presque)
			thePackageName = (char*) ::malloc( 64 );
			thePackageNameWasAllocated = true;
			(void) ::sprintf(
						thePackageName,
						"Untitled ROM File %u",
						(unsigned int) time(NULL) );
		} else {
			// À partir du nom du fichier ROM.
			char* theBaseName = ::basename( theFilePath );
			size_t theBaseNameLength = ::strlen( theBaseName );
			thePackageName = (char*) ::malloc( theBaseNameLength + 1 );
			thePackageNameWasAllocated = true;
			
			(void) ::memcpy(
						(void*) thePackageName,
						(const void*) theBaseName,
						theBaseNameLength + 1 );
		}
	}
	
	// Interface pour les fichiers.
	TDCLPOSIXFiles theFilesIntf;
	
	// Flux d'entrée.
	TDCLStream* theROMStream = nil;
	
	// Fichier d'entrée.
	TDCLFile* theFile = nil;

	if (theFilePath)
	{
		theFile = new TDCLPOSIXFile( &theFilesIntf, theFilePath );
		theFile->Open( true );
		theROMStream = theFile;
	} else {
		theROMStream = new TDCLStdStream();
	}
	
	try {
		TDCLNSRef thePkgFrame = GetTemplate(
					&theFilesIntf,
					theTemplatePath,
					theToolName );

		TDCLNSRef theSym = BuildPackageSym( thePackageName );

		BuildPackageFrame( theROMStream, thePackageName, theSym, thePkgFrame );
		
		BuildPackage(
				&theFilesIntf,
				thePkgFrame,
				theOutputPath,
				theSym );
	} catch (TDCLException& anException) {
		// On affiche l'exception.
		(void) ::fprintf( stderr, "A DCL exception has occurred (%s, %i, %i)\n",
							anException.GetExceptionName(),
							(int) anException.GetExCode(),
							(int) anException.GetError() );
		theResult = -1;
	} catch (std::exception& anException) {
		// Exception inconnue.
		(void) ::fprintf( stderr, "An STD exception has occurred (%s)\n",
							anException.what() );
		theResult = -1;
	} catch (...) {
		// Exception inconnue.
		(void) ::fprintf( stderr, "An unknown exception has occurred\n" );
		theResult = -1;
	}
	
	// Ménage.
	if (theOutputPathWasAllocated)
	{
		::free( theOutputPath );
	}
	if (thePackageNameWasAllocated)
	{
		::free( thePackageName );
	}
	if (theFile)
	{
		theFile->Close();
	}
	if (theROMStream)
	{
		delete theROMStream;
	}
	
	return theResult;
}

// ====================================== //
// Ignorance is bliss.                    //
//                 -- Thomas Gray         //
//                                        //
// Fortune updates the great quotes, #42: //
//         BLISS is ignorance.            //
// ====================================== //
