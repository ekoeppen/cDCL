// ==============================
// Fichier:			PBBookMaker.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			9/4/2004
// Tabulation:		4 espaces
// 
// Copyright:		© 2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: PBBookMaker.cp,v 1.17 2004/10/19 06:50:09 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>

// ANSI C & POSIX
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ISO C++
#include <stdexcept>

// DCL
#include <DCL/Package/UDCLPaperback.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/IO Exceptions/TDCLDoesntExistException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/NS Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS Objects/Objects/TDCLNSRef.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgNOSPart.h>
#include <DCL/Streams/TDCLStdStream.h>
#include <DCL/Streams/TDCLStream.h>

using namespace std;

///
/// PBBookMaker utilise le fichier pbbooktemplate.nsof comme modèle.
/// Celui-ci doit être ou bien:
/// - à côté de PBBookMaker (c'est là qu'il le cherche d'abord)
/// - dans ../share/dcl/
/// (le chemin est relatif au chemin de PBBookMaker).
/// - ou bien fourni par l'option -m de la ligne de commande.
///

#define bookTemplatePathSuffix1	"/pbbooktemplate.nsof"
#define bookTemplatePathSuffix2	"/../share/dcl/pbbooktemplate.nsof"

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
/// \param inBook			livre (de BuildBook)
/// \param inBookTitle		titre du livre.
/// \param inPackageSym		signature (de BuildPackageSym)
/// \param inTemplateFrame	modèle de bouquin.
/// \return la structure du paquet.
///
void BuildPackageFrame(
			const TDCLNSRef& inBook,
			const char* inBookTitle,
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
/// \param inNOS1Compatible	si le paquet est compatible NOS 1.x
///
void BuildPackage(
			TDCLPOSIXFiles* inFilesIntf,
			const TDCLNSRef& inPackageFrame,
			const char* outPkgFilePath,
			const TDCLNSRef& inPackageSym,
			Boolean inNOS1Compatible );

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
static const char* const kPackageNameSuffix = ":PBBookMaker";

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
		"%s - compile a Paperback book from a text file \n", inToolName );
	(void) ::fprintf( stderr,
		"     (or stdin).\n" );
	(void) ::fprintf( stderr,
		"%s [options] [--] [path]\n", inToolName );
	(void) ::fprintf( stderr,
		"   -h|-?       show this help\n" );
	(void) ::fprintf( stderr,
		"   -n name     package name\n" );
	(void) ::fprintf( stderr,
		"   -t title    book title\n" );
	(void) ::fprintf( stderr,
		"   -f family   default font family [geneva]\n" );
	(void) ::fprintf( stderr,
		"               (espy|newYork|geneva|handwriting|...)\n" );
	(void) ::fprintf( stderr,
		"   -s size     default font size [9]\n" );
	(void) ::fprintf( stderr,
		"   -e encoding text file encoding [iso-8859-1]\n" );
	(void) ::fprintf( stderr,
		"               (iso-8859-1|iso-8859-2|macintosh|\n" );
	(void) ::fprintf( stderr,
		"               utf-8|utf-16) \n" );
	(void) ::fprintf( stderr,
		"   -i prefix   prefix for table of content (in iso-8859-1).\n" );
	(void) ::fprintf( stderr,
		"   -d          delete table of content lines.\n" );
	(void) ::fprintf( stderr,
		"   -o path     path of the resulting package [guessed].\n" );
	(void) ::fprintf( stderr,
		"   -o -        send the resulting package to stdout.\n" );
	(void) ::fprintf( stderr,
		"   -m path     path to the book template (NSOF stream).\n" );
	(void) ::fprintf( stderr,
		"   -1          generate a NewtonOS 1.x book\n" );
	(void) ::fprintf( stderr,
		"               (template must be 1.x compatible)\n" );
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
			// Recherche à côté de PBBookMaker.
			size_t theBaseLength = ::strlen( ::dirname( inToolName ) );
			size_t theSuffix1Length = ::strlen( bookTemplatePathSuffix1 );
			size_t theSuffix2Length = ::strlen( bookTemplatePathSuffix2 );
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
						(const void*) bookTemplatePathSuffix1,
						theSuffix1Length + 1 );	// terminateur.
			
			try {
				theTemplateFile = new TDCLPOSIXFile( inFilesIntf, theTemplatePath );
				theTemplateFile->Open( true );
			} catch (TDCLDoesntExistException& anException) {
				// Recherche en ../share/dcl/
				(void) ::memcpy(
						(void*) &theTemplatePath[theBaseLength],
						(const void*) bookTemplatePathSuffix2,
						theSuffix2Length + 1 );	// terminateur.
				theTemplateFile = new TDCLPOSIXFile( inFilesIntf, theTemplatePath );
				theTemplateFile->Open( true );
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
						"Provide -m option or put it next to PBBookMaker.\n" );
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
//  * BuildPackageFrame( const TDCLNSRef&, const char*, const TDCLNSRef&, ... )
// -------------------------------------------------------------------------- //
void
BuildPackageFrame(
			const TDCLNSRef& inBook,
			const char* inBookTitle,
			const TDCLNSRef& inPackageSym,
			const TDCLNSRef& inTemplateFrame )
{
	TDCLNSFrame& thePackageFrame = inTemplateFrame.ToFrame();
	
	TDCLNSRef theBookTitle = TDCLNSRef::MakeString( inBookTitle );
	thePackageFrame.Set( "text", theBookTitle );
	thePackageFrame.Set( "app", inPackageSym );

	TDCLNSFrame& theFormFrame = thePackageFrame.Get( "theForm" ).ToFrame();
	theFormFrame.Set( "book", inBook );
	theFormFrame.Set( "appSymbol", inPackageSym );
}

// -------------------------------------------------------------------------- //
//  * BuildPackage( TDCLPOSIXFiles*, const TDCLNSRef&, const char*, ... )
// -------------------------------------------------------------------------- //
void
BuildPackage(
		TDCLPOSIXFiles* inFilesIntf,
		const TDCLNSRef& inPackageFrame,
		const char* outPkgFilePath,
		const TDCLNSRef& inPackageSym,
		Boolean inNOS1Compatible )
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
		
		thePackage.SetNOS1Compatible( inNOS1Compatible );
		thePackage.SetPackageID( TDCLPackage::kDefaultID );
		thePackage.SetPackageFlags( 0 );
		thePackage.SetPackageVersion( 1 );
		thePackage.SetPackageName( inPackageSym.ToSymbol().GetString() );

		(void) thePackage.AddPart(
					TDCLPackage::kFormPart,
					TDCLPackage::kPartNOSPart | TDCLPackage::kPartNotifyFlag,
					new TDCLPkgNOSPart( inPackageFrame ) );
		
		thePackage.WriteToStream( theOutputStream );
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
	char* theBookTitle = nil;
	char* theFontFamily = nil;
	int theFontSize = 9;
	UDCLPaperback::EEncoding theEncoding = UDCLPaperback::kISO88591;
	char* theTemplatePath = nil;
	char* tocLinesPrefix = nil;
	Boolean suppressTocLines = false;
	Boolean newtonOS1xCompatible = false;

	int theOptionChar;
	while ((theOptionChar =
					getopt( inArgc, inArgv, "h?n:t:f:s:e:o:m:i:d1")) != -1)
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
				thePackageName = optarg;
			}
			break;
			
			case 't':
			{
				theBookTitle = optarg;
			}
			break;
			
			case 'f':
			{
				theFontFamily = optarg;
			}
			break;
			
			case 's':
			{
				theFontSize = ::atoi( optarg );
			}
			break;
			
			case 'e':
			{
				if (::strcmp(optarg, "iso-8859-1") == 0) {
					theEncoding = UDCLPaperback::kISO88591;
				} else if (::strcmp(optarg, "iso-8859-2") == 0) {
					theEncoding = UDCLPaperback::kISO88592;
				} else if (::strcmp(optarg, "macintosh") == 0) {
					theEncoding = UDCLPaperback::kMacRoman;
				} else if (::strcmp(optarg, "utf-8") == 0) {
					theEncoding = UDCLPaperback::kUTF8;
				} else if (::strcmp(optarg, "utf-16") == 0) {
					theEncoding = UDCLPaperback::kUTF16;
				} else {
					(void) ::fprintf( stderr, "Unknown encoding %s\n", optarg );
					::usage( ::basename( theToolName ) );
					::exit( 1 );
				}
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

			case 'i':
			{
				tocLinesPrefix = optarg;
			}
			break;

			case 'd':
			{
				suppressTocLines = true;
			}
			break;

			case '1':
			{
				newtonOS1xCompatible = true;
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

	if (suppressTocLines && (tocLinesPrefix == nil))
	{
		(void) ::fprintf( stderr, "-d option requires -i option.\n" );
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
		if (theBookTitle == nil)
		{
			// On devine le nom du paquet.
			if (theFilePath == nil)
			{
				if (theOutputPath == nil)
				{
					// Aléatoire (enfin, presque)
					thePackageName = (char*) ::malloc( 64 );
					thePackageNameWasAllocated = true;
					(void) ::sprintf(
								thePackageName,
								"Book %u",
								(unsigned int) time(NULL) );
				} else {
					// À partir du nom du paquet.
					char* theBaseName = ::basename( theOutputPath );
					size_t theBaseNameLength = ::strlen( theBaseName );
					thePackageName = (char*) ::malloc( theBaseNameLength + 1 );
					thePackageNameWasAllocated = true;
					
					(void) ::memcpy(
								(void*) thePackageName,
								(const void*) theBaseName,
								theBaseNameLength + 1 );
				}
			} else {
				// À partir du nom du fichier texte.
				char* theBaseName = ::basename( theFilePath );
				size_t theBaseNameLength = ::strlen( theBaseName );
				thePackageName = (char*) ::malloc( theBaseNameLength + 1 );
				thePackageNameWasAllocated = true;
				
				(void) ::memcpy(
							(void*) thePackageName,
							(const void*) theBaseName,
							theBaseNameLength + 1 );
			}
		} else {
			thePackageName = theBookTitle;
		}
	}
	
	if (theBookTitle == nil)
	{
		theBookTitle = thePackageName;
	}
	
	// Interface pour les fichiers.
	TDCLPOSIXFiles theFilesIntf;
	
	// Flux d'entrée.
	TDCLStream* theTextStream = nil;
	
	// Fichier d'entrée.
	TDCLFile* theFile = nil;

	if (theFilePath)
	{
		theFile = new TDCLPOSIXFile( &theFilesIntf, theFilePath );
		theFile->Open( true );
		theTextStream = theFile;
	} else {
		theTextStream = new TDCLStdStream();
	}
	
	// Fichier du modèle.
	TDCLFile* theTemplateFile = nil;
	
	try {
		TDCLNSRef thePkgFrame = GetTemplate(
					&theFilesIntf,
					theTemplatePath,
					theToolName );

		TDCLNSRef theBook = UDCLPaperback::BuildBook(
								theTextStream,
								theEncoding,
								theBookTitle,
								tocLinesPrefix,
								suppressTocLines,
								theFontFamily,
								theFontSize );
		TDCLNSRef theSym = BuildPackageSym( thePackageName );

		BuildPackageFrame( theBook, theBookTitle, theSym, thePkgFrame );
		
		BuildPackage(
				&theFilesIntf,
				thePkgFrame,
				theOutputPath,
				theSym,
				newtonOS1xCompatible );
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
	if (theTemplateFile)
	{
		theTemplateFile->Close();
		delete theTemplateFile;
	}
	if (theFile)
	{
		theFile->Close();
	}
	if (theTextStream)
	{
		delete theTextStream;
	}
	
	return theResult;
}

// ============================================================================= //
//         There once was a man who went to a computer trade show.  Each day as  //
// he entered, the man told the guard at the door:                               //
//         "I am a great thief, renowned for my feats of shoplifting.  Be        //
// forewarned, for this trade show shall not escape unplundered."                //
//         This speech disturbed the guard greatly, because there were millions  //
// of dollars of computer equipment inside, so he watched the man carefully.     //
// But the man merely wandered from booth to booth, humming quietly to himself.  //
//         When the man left, the guard took him aside and searched his clothes, //
// but nothing was to be found.                                                  //
//         On the next day of the trade show, the man returned and chided the    //
// guard saying: "I escaped with a vast booty yesterday, but today will be even  //
// better."  So the guard watched him ever more closely, but to no avail.        //
//         On the final day of the trade show, the guard could restrain his      //
// curiosity no longer. "Sir Thief," he said, "I am so perplexed, I cannot live  //
// in peace.  Please enlighten me.  What is it that you are stealing?"           //
//         The man smiled.  "I am stealing ideas," he said.                      //
//                 -- Geoffrey James, "The Tao of Programming"                   //
// ============================================================================= //
