// ==============================
// Fichier:			UTestNS.cp
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
// $Id: UTestNS.cp,v 1.17 2004/03/28 15:59:07 paul Exp $
// ===========

#include "UTestNS.h"

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/Streams/TDCLMemStream.h>

// ANSI C
#include <stdio.h>

// ------------------------------------------------------------------------- //
//  * GetWalterSmith( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
UTestNS::GetWalterSmith( void )
{
	/*
	x := {name: "Walter Smith",
		cats: 2,
		bounds: {left: 10, top: 14, right: 40, bottom: 100},
		uChar: $\u2022,
		phones: ["408-996-1010", nil]};
	x.phones[1] := SetClass("408-974-9094", 'faxPhone);
	x.nameAgain := x.name;
	*/
	
	// Création de la structure.
	TDCLNSRef theFrameRef = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theFrame = theFrameRef.ToFrame();
	
	// Ajout du nom
	TDCLNSRef theName = TDCLNSRef::MakeString( "Walter Smith" );
	theFrame.Set( "name", theName );

	// Les chats.
	theFrame.Set( "cats", TDCLNSRef::MakeInt( 2 ) );
	
	// Ajout des coordonnées
	TDCLNSRef theBoundsRef = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theBounds = theBoundsRef.ToFrame();
	theBounds.Set( "left", TDCLNSRef::MakeInt( 10 ) );
	theBounds.Set( "top", TDCLNSRef::MakeInt( 14 ) );
	theBounds.Set( "right", TDCLNSRef::MakeInt( 40 ) );
	theBounds.Set( "bottom", TDCLNSRef::MakeInt( 100 ) );
	
	theFrame.Set( "bounds", theBoundsRef );

	theFrame.Set( "uchar", TDCLNSRef::MakeChar( (KUInt16) 0x2022 ) );

	// Numéros de téléphone
	TDCLNSRef thePhonesRef = TDCLNSRef::MakeArray();
	TDCLNSArray& thePhones = thePhonesRef.ToArray();
	thePhones.Add( TDCLNSRef::MakeString( "408-996-1010" ) );
	thePhones.Add( TDCLNSRef::kNILREF );

	thePhones.Set( 1, TDCLNSRef::MakeString( "408-974-9094" )
						.SetClass( "faxPhone" ) );
	
	theFrame.Set( "phones", thePhonesRef );

	// Copie de la référence.
	theFrame.Set( "nameAgain", theName );
	
	return theFrameRef;
}

// ------------------------------------------------------------------------- //
//  * FrameKeysSort( void )
// ------------------------------------------------------------------------- //
void
UTestNS::FrameKeysSort( void )
{
	// 1 - test du tri au vol.
	(void) ::printf( "1- Test du tri au vol\n" );
	// 'onze et 'sept on même valeur de hachage (si si).
	TDCLNSFrame theFrame;
	theFrame.Set( "un",				TDCLNSRef::MakeInt( 1 ) );
	theFrame.Set( "deux",			TDCLNSRef::MakeInt( 2 ) );
	theFrame.Set( "trois",			TDCLNSRef::MakeInt( 3 ) );
	theFrame.Set( "quatre",			TDCLNSRef::MakeInt( 4 ) );
	theFrame.Set( "cinq",			TDCLNSRef::MakeInt( 5 ) );
	theFrame.Set( "six",			TDCLNSRef::MakeInt( 6 ) );
	theFrame.Set( "sept",			TDCLNSRef::MakeInt( 7 ) );
	theFrame.Set( "huit",			TDCLNSRef::MakeInt( 8 ) );
	theFrame.Set( "neuf",			TDCLNSRef::MakeInt( 9 ) );
	theFrame.Set( "dix",			TDCLNSRef::MakeInt( 10 ) );
	theFrame.Set( "onze",			TDCLNSRef::MakeInt( 11 ) );
	theFrame.Set( "douze",			TDCLNSRef::MakeInt( 12 ) );
	theFrame.Set( "treize",			TDCLNSRef::MakeInt( 13 ) );
	theFrame.Set( "quatorze",		TDCLNSRef::MakeInt( 14 ) );
	theFrame.Set( "quinze",			TDCLNSRef::MakeInt( 15 ) );
	theFrame.Set( "seize",			TDCLNSRef::MakeInt( 16 ) );
	theFrame.Set( "dix-sept",		TDCLNSRef::MakeInt( 17 ) );
	theFrame.Set( "dix-huit",		TDCLNSRef::MakeInt( 18 ) );
	theFrame.Set( "dix-neuf",		TDCLNSRef::MakeInt( 19 ) );
	theFrame.Set( "vingt",			TDCLNSRef::MakeInt( 20 ) );
	theFrame.Set( "vingt-et-un",	TDCLNSRef::MakeInt( 21 ) );
	
	// On affiche toutes les valeurs dans l'ordre
	KUInt32 indexPairs;
	KUInt32 nbPairs = theFrame.GetLength();
	(void) ::printf( "%u paires\n", (unsigned int) nbPairs );
	for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
	{
		TDCLNSRef theKey = theFrame.GetKey(indexPairs);
		TDCLNSRef theValue = theFrame.GetValue(indexPairs);
		TDCLNSSymbol& theKeyAsSymbol = theKey.ToSymbol();
		(void) ::printf( "%.8X - %s - %i\n",
			(unsigned int) theKeyAsSymbol.GetHashCode(),
			theKeyAsSymbol.GetString(),
			(int) theValue.ToInt());
	}

	(void) ::printf( "\n2- Ajout de deux paires et tri par insertion\n" );
	
	// Conversion en flux.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( TDCLNSRef( theFrame ) );

	// Conversion depuis le flux.
	theMemStream.SetCursor( 0, TDCLMemStream::kFromStart );
	TDCLNSOFDecoder theDecoder( &theMemStream );
	TDCLNSRef theFrameAgainRef = theDecoder.GetNextObject();
	TDCLNSFrame& theFrameAgain = theFrameAgainRef.ToFrame();
	
	// Ajout d'une valeur (force le tri)
	// -- je crois que dans le format NSOF, on ne peut pas marquer
	// une structure comme triée et pour le moment, la structure n'est
	// pas triée lorsque lue à partir d'un flux.
	// Il faudrait quand même forcer les clés à être dans le désordre
	// en créant un flux avec une structure désordonnée.
	theFrameAgain.Set( "vingt-deux",		TDCLNSRef::MakeInt( 22 ) );
	theFrameAgain.Set( "quartante-deux",	TDCLNSRef::MakeInt( 42 ) );
	
	// Affichage à nouveau.
	nbPairs = theFrameAgain.GetLength();
	(void) ::printf( "%u paires\n", (unsigned int) nbPairs );
	for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
	{
		TDCLNSRef theKey = theFrameAgain.GetKey(indexPairs);
		TDCLNSRef theValue = theFrameAgain.GetValue(indexPairs);
		TDCLNSSymbol& theKeyAsSymbol = theKey.ToSymbol();
		(void) ::printf( "%.8X - %s - %i\n",
			(unsigned int) theKeyAsSymbol.GetHashCode(),
			theKeyAsSymbol.GetString(),
			(int) theValue.ToInt());
	}
}

// ------------------------------------------------------------------------- //
//  * CatenateStrings( void )
// ------------------------------------------------------------------------- //
void
UTestNS::CatenateStrings( void )
{
	// Je vais passer par un encodeur XML pour afficher les chaînes
	// concaténées.

	TDCLNSArray theArray;
	TDCLNSRef helloString = TDCLNSRef::MakeString("Hello");
	TDCLNSRef worldString = TDCLNSRef::MakeString("World");
	TDCLNSRef exclString = TDCLNSRef::MakeString("!");
	
	theArray.Add(helloString & worldString);
	theArray.Add(helloString & worldString & exclString);
	theArray.Add(helloString & worldString & "!");
	theArray.Add(helloString & worldString && exclString);
	theArray.Add(helloString & worldString && "!");
	theArray.Add(helloString & "World");
	theArray.Add(helloString & "World" & exclString);
	theArray.Add(helloString & "World" & "!");
	theArray.Add(helloString & "World" && exclString);
	theArray.Add(helloString & "World" && "!");
	theArray.Add(helloString && worldString);
	theArray.Add(helloString && worldString & exclString);
	theArray.Add(helloString && worldString & "!");
	theArray.Add(helloString && worldString && exclString);
	theArray.Add(helloString && worldString && "!");
	theArray.Add(helloString && "World");
	theArray.Add((helloString && "World") & exclString);
	theArray.Add((helloString && "World") & "!");
	theArray.Add((helloString && "World") && exclString);
	theArray.Add((helloString && "World") && "!");
	
	// Encodage XML.
	TDCLMemStream theMemStream;
	TDCLXMLEncoder theEncoder( &theMemStream );
	theEncoder.AddObject( TDCLNSRef( theArray ) );
	
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
//  * References( void )
// ------------------------------------------------------------------------- //
void
UTestNS::References( void )
{
	// Attention: le code ci-dessous est illégal. Désormais,
	// lorsque le deuxième cas se présente, une assertion est levée
	// et la macro DEBUGSTR appelée.
	
	{
		TDCLNSFrame someFrame;
		{
			TDCLNSArray someArray;
			someFrame.Set("foo", someArray.Clone());
		} // <-- pas d'assertion
	}

	{
		TDCLNSFrame someFrame;
		{
			TDCLNSArray someArray;
			someFrame.Set("foo", TDCLNSRef( someArray ));
		} // <-- assertion
	}
}

// ------------------------------------------------------------------------- //
//  * ArraysAndFrames( void )
// ------------------------------------------------------------------------- //
void
UTestNS::ArraysAndFrames( void )
{
	(void) ::printf( "1- création d'une structure\n" );
	TDCLNSFrame theFrame;
	theFrame.Set( "un",				TDCLNSRef::MakeInt( 1 ) );
	theFrame.Set( "deux",			TDCLNSRef::MakeInt( 2 ) );
	theFrame.Set( "trois",			TDCLNSRef::MakeInt( 3 ) );
	theFrame.Set( "quatre",			TDCLNSRef::MakeInt( 4 ) );

	// Affichage.
	KUInt32 indexSlots;
	KUInt32 nbSlots = theFrame.GetLength();
	(void) ::printf( "%u paires\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theKey = theFrame.GetKey(indexSlots);
		TDCLNSRef theValue = theFrame.GetValue(indexSlots);
		TDCLNSSymbol& theKeyAsSymbol = theKey.ToSymbol();
		(void) ::printf( "%.8X - %s - %i\n",
			(unsigned int) theKeyAsSymbol.GetHashCode(),
			theKeyAsSymbol.GetString(),
			(int) theValue.ToInt());
	}
	
	// La clé est-elle présente?
	if (!theFrame.HasSlot( "un" ))
	{
		(void) ::printf( "'un' devrait être dans la structure.\n" );
	}

	// Suppression d'une clé.
	(void) ::printf( "2- suppression d'une clé\n" );
	(void) theFrame.RemoveSlot( "un" );

	// La clé est-elle présente?
	if (theFrame.HasSlot( "un" ))
	{
		(void) ::printf( "'un' ne devrait plus être dans la structure.\n" );
	}
	
	// Affichage à nouveau.
	nbSlots = theFrame.GetLength();
	(void) ::printf( "%u paires\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theKey = theFrame.GetKey(indexSlots);
		TDCLNSRef theValue = theFrame.GetValue(indexSlots);
		TDCLNSSymbol& theKeyAsSymbol = theKey.ToSymbol();
		(void) ::printf( "%.8X - %s - %i\n",
			(unsigned int) theKeyAsSymbol.GetHashCode(),
			theKeyAsSymbol.GetString(),
			(int) theValue.ToInt());
	}

	(void) ::printf( "3- création d'un tableau\n" );
	TDCLNSArray theArray;
	theArray.Add( TDCLNSRef::MakeInt( 1 ) );
	theArray.Add( TDCLNSRef::MakeInt( 2 ) );
	theArray.Add( TDCLNSRef::MakeInt( 3 ) );
	theArray.Add( TDCLNSRef::MakeInt( 4 ) );
	theArray.Add( TDCLNSRef::MakeInt( 5 ) );
	theArray.Add( TDCLNSRef::MakeInt( 6 ) );

	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}
	
	(void) ::printf( "4- suppression au début\n" );
	theArray.RemoveSlot( 0 );

	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}

	(void) ::printf( "5- suppression au milieu\n" );
	theArray.RemoveSlot( 2 );
	
	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}

	(void) ::printf( "6- suppression à la fin\n" );
	theArray.RemoveSlot( nbSlots - 1 );
	
	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}

	(void) ::printf( "7- insertion au début\n" );
	theArray.Insert( 0, TDCLNSRef::MakeInt( 10 ) );
	
	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}

	(void) ::printf( "8- insertion au milieu\n" );
	theArray.Insert( 3, TDCLNSRef::MakeInt( 11 ) );
	
	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}

	(void) ::printf( "8- insertion à la fin\n" );
	theArray.Insert( nbSlots, TDCLNSRef::MakeInt( 12 ) );
	
	nbSlots = theArray.GetLength();
	(void) ::printf( "%u éléments\n", (unsigned int) nbSlots );
	for (indexSlots = 0; indexSlots < nbSlots; indexSlots++)
	{
		TDCLNSRef theElement = theArray.Get( indexSlots );
		(void) ::printf( "%i - %i\n",
			(int) indexSlots,
			(int) theElement.ToInt());
	}
}

// ====================================================================== //
// Real software engineers don't debug programs, they verify correctness. //
// This process doesn't necessarily involve execution of anything on a    //
// computer, except perhaps a Correctness Verification Aid package.       //
// ====================================================================== //
