// ==============================
// Fichier:			tests.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			02/02/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: tests.cp,v 1.22 2004/04/15 16:00:16 paul Exp $
// ===========

// ANSI C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

// Tests
#include "tests.h"
#include "UTestCommLayers.h"
#include "UTestNS.h"
#include "UTestNSOF.h"
#include "UTestPkg.h"
#include "UTestThreads.h"
#include "UTestTime.h"
#include "UTestXML.h"

// ------------------------------------------------------------------------- //
//  * main
// ------------------------------------------------------------------------- //
// Point d'entrée des tests pour une interface cli.
// Syntaxe:
//		DCLTests	<nom du test>
int
main( int argc, char* argv[] )
{
	const char* argument = nil;
	if (argc == 3)
	{
		argument = argv[2];
	} else if (argc != 2)
	{
		(void) ::printf( "Erreur de syntaxe.\n" );
		(void) ::printf( "%s <nom du test> [argument]\n", argv[0] );
		::exit(1);
	}
	
	test( argv[1], argument );
		
	return 0;
}

// ------------------------------------------------------------------------- //
//  * test( const char*, const char* )
// ------------------------------------------------------------------------- //
void
test( const char* inTestName, const char* inArgument )
{
	try {
		if (::strcmp(inTestName, "cl-cad-bsd") == 0) {
			UTestCommLayers::ConnectAndDisconnectBSDSocket();
		} else if (::strcmp(inTestName, "cl-cad-fdserial") == 0) {
			UTestCommLayers::ConnectAndDisconnectFDSerialPort( inArgument );
		} else if (::strcmp(inTestName, "cl-cad-xadsp") == 0) {
			UTestCommLayers::ConnectAndDisconnectXADSP();
		} else if (::strcmp(inTestName, "cl-sast-bsd") == 0) {
			UTestCommLayers::StartAndStopTwiceBSDSocket();
		} else if (::strcmp(inTestName, "cl-sast-fdserial") == 0) {
			UTestCommLayers::StartAndStopTwiceFDSerialPort( inArgument );
		} else if (::strcmp(inTestName, "cl-sast-xadsp") == 0) {
			UTestCommLayers::StartAndStopTwiceXADSP();
		} else if (::strcmp(inTestName, "ns-arraysandframes") == 0) {
			UTestNS::ArraysAndFrames();
		} else if (::strcmp(inTestName, "ns-framekeyssort") == 0) {
			UTestNS::FrameKeysSort();
		} else if (::strcmp(inTestName, "ns-catenatestrings") == 0) {
			UTestNS::CatenateStrings();
		} else if (::strcmp(inTestName, "ns-references-debugstr") == 0) {
			// Sur Mac, on appelle DebugStr ce qui a un certain résultat ...
#if !TARGET_OS_MAC
	(void) ::printf( "Test non disponible pour cette plateforme\n" );
#else
			UTestNS::References();
#endif
		} else if (::strcmp(inTestName, "ns-references-kill") == 0) {
			// ... différent de celui qu'on a si on appelle kill avec SIGINT.
#if TARGET_OS_MAC
	(void) ::printf( "Test non disponible pour cette plateforme\n" );
#else
			UTestNS::References();
#endif
		} else if (::strcmp(inTestName, "nsof-waltersmith") == 0) {
			UTestNSOF::WalterSmith();
		} else if (::strcmp(inTestName, "pkg-waltersmith") == 0) {
			UTestPkg::WalterSmith();
		} else if (::strcmp(inTestName, "pkg-idempotent") == 0) {
			UTestPkg::IdemPotent();
		} else if (::strcmp(inTestName, "threads-posix") == 0) {
			UTestThreads::Posix();
		} else if (::strcmp(inTestName, "time-values") == 0) {
			UTestTime::CheckReturnValues();
		} else if (::strcmp(inTestName, "xml-waltersmith") == 0) {
			UTestXML::WalterSmith();
		} else if (::strcmp(inTestName, "xml-classes") == 0) {
			UTestXML::Classes();
		} else {
			(void) ::printf( "%s est un test inconnu.\n", inTestName );
		}
	} catch ( TDCLException& anException ) {
		(void) ::printf( "Une exception TDCLException est survenue.\n" );
#if kDCLDebugFILEandLINE
		(void) ::printf( ">> %s (code: %u, erreur: %i) [%s:%u]\n",
						anException.GetExceptionName(),
						(unsigned int) anException.GetExCode(),
						(int) anException.GetError(),
						anException.GetFileName(),
						(unsigned int) anException.GetLine() );
#else
		(void) ::printf( ">> %s (code: %u, erreur: %i)\n",
						anException.GetExceptionName(),
						(unsigned int) anException.GetExCode(),
						(int) anException.GetError() );
#endif
	} catch (...) {
		(void) ::printf( "Une exception inconnue est survenue.\n" );
	}
}

// =============================================================== //
// SCCS, the source motel!  Programs check in and never check out! //
//                 -- Ken Thompson                                 //
// =============================================================== //
