// ==============================
// Fichier:			NSOFtoXML.cp
// Projet:			DCL - NSOFtoXML
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			2/1/2004
// Tabulation:		4 espaces
// 
// Copyright:		© 2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: NSOFtoXML.cp,v 1.3 2004/01/07 06:49:33 paul Exp $
// ===========

///
/// \file
/// This file includes a very simple console-based NSOF to XML converter.
///

// ANSI C
#include <stdio.h>

// ISO C++
#include <stdexcept>

// DCL
#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStdStream.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/Exceptions/TDCLException.h>

#define kNWTPreambleSize		16

// -------------------------------------------------------------------------- //
//  * entry point
// -------------------------------------------------------------------------- //
int main( int argc, char** argv )
{
	// Check that we did get some parameter.
	if (argc > 1)
	{
		(void) ::fprintf( stderr,
					"Convert NSOF (or .nwt) data to XML\n" );
		(void) ::fprintf( stderr,
					"Syntax: %s < NSOF input > XML output\n", argv[0] );
		::exit( -2 );
	}
	
	int theResult = 0;
	
	// Just suck stdin and convert to stdout.
	try {
		do {
			TDCLStdStream theStdIOStream;
			
			// Check that this looks like NSOF (skip .nwt header if any)
			if (theStdIOStream.PeekByte() != KDCLNSOF::kNSOFVersion)
			{
				// Skip first kNWTPreambleSize bytes for .nwt files.
				KUInt32 thePreambleSize = kNWTPreambleSize;
				KUInt8 thePreamble[ kNWTPreambleSize ];
				theStdIOStream.Read( thePreamble, &thePreambleSize );
				
				if ((thePreambleSize != kNWTPreambleSize) ||
					(theStdIOStream.PeekByte() != KDCLNSOF::kNSOFVersion))
				{
					(void) ::fprintf( stderr,
							"Input doesn't seem to be in NSOF or .nwt format\n" );
					break;
				}
			}
			
			// Create the NSOF decoder.
			TDCLNSOFDecoder theDecoder( &theStdIOStream );
			
			// Create the XML encoder.
			TDCLXMLEncoder theEncoder( &theStdIOStream );
			
			// Suck the next object.
			TDCLNSRef theObject = theDecoder.GetNextObject();
			
			// Send it to the encoder.
			theEncoder.AddObject( theObject );
			
			// Flush the output.
			theStdIOStream.FlushOutput();
			
			// That's all folks.
		} while (false);
	} catch (TDCLException& anException) {
		// Print the exception.
		// (this could happen if input wasn't in NSOF format for example).
		(void) ::fprintf( stderr, "A DCL exception occurred (%s, %i, %i)\n",
							anException.GetExceptionName(),
							(int) anException.GetExCode(),
							(int) anException.GetError() );
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
