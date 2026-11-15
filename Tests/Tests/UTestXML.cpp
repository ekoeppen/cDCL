// ==============================
// Fichier:			UTestXML.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			31/01/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003-2004 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestXML.cp,v 1.10 2004/01/02 21:24:28 paul Exp $
// ===========

#include "UTestXML.h"

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// Tests
#include "UTestNS.h"

// ------------------------------------------------------------------------- //
//  * WalterSmith( void )
// ------------------------------------------------------------------------- //
void
UTestXML::WalterSmith( void )
{
	// Récupération de la structure Walter Smith
	TDCLNSRef theWalterSmithFrame = UTestNS::GetWalterSmith();

	// Ensuite, encodage en XML sur stdout.
	TDCLMemStream theMemStream;
	TDCLXMLEncoder theEncoder( &theMemStream );

	theEncoder.AddObject( theWalterSmithFrame );

	// Affichage des données telles quelles.
	const KUInt8* theData = (const KUInt8*) theMemStream.GetBuffer();
	KUInt32 theDataCount = theMemStream.GetBufferSize();
	
//	::fwrite( theData, sizeof( char ), theDataCount, stdout );
//	Ne marche pas avec SIOUX.

	KUInt32 indexData;
	for ( indexData = 0; indexData < theDataCount; indexData++ )
	{
		(void) ::printf( "%c", theData[indexData] );
	}
	(void) ::printf( "\n" );
}

// ------------------------------------------------------------------------- //
//  * Classes( void )
// ------------------------------------------------------------------------- //
void
UTestXML::Classes( void )
{
	TDCLNSFrame theFrame;
	TDCLNSRef theString = TDCLNSRef::MakeString("bar");
	(void) theString.SetClass( TDCLNSRef::MakeSymbol("string.bar") );
	theFrame.Set("foo", theString );
	
	// Ensuite, encodage en XML sur stdout.
	TDCLMemStream theMemStream;
	TDCLXMLEncoder theEncoder( &theMemStream );

	theEncoder.AddObject( TDCLNSRef( theFrame ) );

	// Affichage des données telles quelles.
	const KUInt8* theData = (const KUInt8*) theMemStream.GetBuffer();
	KUInt32 theDataCount = theMemStream.GetBufferSize();
	
//	::fwrite( theData, sizeof( char ), theDataCount, stdout );
//	Ne marche pas avec SIOUX.

	KUInt32 indexData;
	for ( indexData = 0; indexData < theDataCount; indexData++ )
	{
		(void) ::printf( "%c", theData[indexData] );
	}
	(void) ::printf( "\n" );
}

// =============================================== //
// There are never any bugs you haven't found yet. //
// =============================================== //
