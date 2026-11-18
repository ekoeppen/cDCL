// ==============================
// Fichier:			UTestNS.cpp
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

#include <doctest/doctest.h>

#include <string>
#include <vector>

namespace
{
struct ExpectedSlot
{
	KUInt32 hash;
	const char* key;
	KSInt32 value;
};

void
CheckFrameSlots( const TDCLNSFrame& inFrame, const std::vector<ExpectedSlot>& inExpected )
{
	REQUIRE( inFrame.GetLength() == inExpected.size() );
	for ( KUInt32 i = 0; i < inExpected.size(); ++i )
	{
		TDCLNSSymbol& keySym = inFrame.GetKey( i ).ToSymbol();
		CHECK( keySym.GetHashCode() == inExpected[i].hash );
		CHECK( std::string( keySym.GetString() ) == inExpected[i].key );
		CHECK( inFrame.GetValue( i ).ToInt() == inExpected[i].value );
	}
}

void
CheckArrayInts( const TDCLNSArray& inArray, const std::vector<KSInt32>& inExpected )
{
	REQUIRE( inArray.GetLength() == inExpected.size() );
	for ( KUInt32 i = 0; i < inExpected.size(); ++i )
	{
		CHECK( inArray.Get( i ).ToInt() == inExpected[i] );
	}
}
} // namespace

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

	TDCLNSRef theFrameRef = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theFrame = theFrameRef.ToFrame();

	TDCLNSRef theName = TDCLNSRef::MakeString( "Walter Smith" );
	theFrame.Set( "name", theName );
	theFrame.Set( "cats", TDCLNSRef::MakeInt( 2 ) );

	TDCLNSRef theBoundsRef = TDCLNSRef::MakeFrame();
	TDCLNSFrame& theBounds = theBoundsRef.ToFrame();
	theBounds.Set( "left", TDCLNSRef::MakeInt( 10 ) );
	theBounds.Set( "top", TDCLNSRef::MakeInt( 14 ) );
	theBounds.Set( "right", TDCLNSRef::MakeInt( 40 ) );
	theBounds.Set( "bottom", TDCLNSRef::MakeInt( 100 ) );
	theFrame.Set( "bounds", theBoundsRef );

	theFrame.Set( "uchar", TDCLNSRef::MakeChar( (KUInt16) 0x2022 ) );

	TDCLNSRef thePhonesRef = TDCLNSRef::MakeArray();
	TDCLNSArray& thePhones = thePhonesRef.ToArray();
	thePhones.Add( TDCLNSRef::MakeString( "408-996-1010" ) );
	thePhones.Add( TDCLNSRef::kNILREF );
	thePhones.Set( 1, TDCLNSRef::MakeString( "408-974-9094" ).SetClass( "faxPhone" ) );
	theFrame.Set( "phones", thePhonesRef );

	theFrame.Set( "nameAgain", theName );

	return theFrameRef;
}

TEST_SUITE("NS")
{
	TEST_CASE("FrameKeysSort: on-the-fly sort and insertion sort after NSOF round-trip")
	{
		// 'onze and 'sept share hash code 0x4C7A405C.
		TDCLNSFrame theFrame;
		theFrame.Set( "un",				TDCLNSRef::MakeInt( 1 ) );
		theFrame.Set( "deux",			TDCLNSRef::MakeInt( 2 ) );
		theFrame.Set( "trois",			TDCLNSRef::MakeInt( 3 ) );
		theFrame.Set( "quatre",			TDCLNSRef::MakeInt( 4 ) );
		theFrame.Set( "cinq",			TDCLNSRef::MakeInt( 5 ) );
		theFrame.Set( "six",				TDCLNSRef::MakeInt( 6 ) );
		theFrame.Set( "sept",			TDCLNSRef::MakeInt( 7 ) );
		theFrame.Set( "huit",			TDCLNSRef::MakeInt( 8 ) );
		theFrame.Set( "neuf",			TDCLNSRef::MakeInt( 9 ) );
		theFrame.Set( "dix",				TDCLNSRef::MakeInt( 10 ) );
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

		CheckFrameSlots( theFrame, {
			{ 0x00FB92C2, "quatre", 4 },
			{ 0x100B4CEA, "huit", 8 },
			{ 0x2F2653FC, "quinze", 15 },
			{ 0x30ED6EAE, "vingt-et-un", 21 },
			{ 0x44F26348, "vingt", 20 },
			{ 0x4C7A405C, "onze", 11 },
			{ 0x4C7A405C, "sept", 7 },
			{ 0x53369580, "seize", 16 },
			{ 0x676B94EC, "dix-huit", 18 },
			{ 0x739AEDE3, "quatorze", 14 },
			{ 0x879FE27D, "dix", 10 },
			{ 0x972D6606, "deux", 2 },
			{ 0x9F330C7B, "treize", 13 },
			{ 0xA3DA885E, "dix-sept", 17 },
			{ 0xA571983E, "neuf", 9 },
			{ 0xA6BAE98F, "douze", 12 },
			{ 0xBD5280CB, "un", 1 },
			{ 0xCACB2B13, "cinq", 5 },
			{ 0xCCE00454, "six", 6 },
			{ 0xD4E5AAC9, "trois", 3 },
			{ 0xFCD1E040, "dix-neuf", 19 },
		} );

		TDCLMemStream theMemStream;
		TDCLNSOFEncoder theEncoder( &theMemStream );
		theEncoder.AddObject( TDCLNSRef( theFrame ) );

		theMemStream.SetCursor( 0, TDCLMemStream::kFromStart );
		TDCLNSOFDecoder theDecoder( &theMemStream );
		TDCLNSRef theFrameAgainRef = theDecoder.GetNextObject();
		TDCLNSFrame& theFrameAgain = theFrameAgainRef.ToFrame();

		theFrameAgain.Set( "vingt-deux", TDCLNSRef::MakeInt( 22 ) );
		theFrameAgain.Set( "quartante-deux", TDCLNSRef::MakeInt( 42 ) );

		CheckFrameSlots( theFrameAgain, {
			{ 0x00FB92C2, "quatre", 4 },
			{ 0x100B4CEA, "huit", 8 },
			{ 0x2F2653FC, "quinze", 15 },
			{ 0x30ED6EAE, "vingt-et-un", 21 },
			{ 0x44F26348, "vingt", 20 },
			{ 0x4C7A405C, "onze", 11 },
			{ 0x4C7A405C, "sept", 7 },
			{ 0x53369580, "seize", 16 },
			{ 0x676B94EC, "dix-huit", 18 },
			{ 0x739AEDE3, "quatorze", 14 },
			{ 0x879FE27D, "dix", 10 },
			{ 0x972D6606, "deux", 2 },
			{ 0x9F330C7B, "treize", 13 },
			{ 0xA3DA885E, "dix-sept", 17 },
			{ 0xA571983E, "neuf", 9 },
			{ 0xA6BAE98F, "douze", 12 },
			{ 0xABE02ED3, "vingt-deux", 22 },
			{ 0xB31A4D58, "quartante-deux", 42 },
			{ 0xBD5280CB, "un", 1 },
			{ 0xCACB2B13, "cinq", 5 },
			{ 0xCCE00454, "six", 6 },
			{ 0xD4E5AAC9, "trois", 3 },
			{ 0xFCD1E040, "dix-neuf", 19 },
		} );
	}

	TEST_CASE("CatenateStrings: & and && operators")
	{
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

		TDCLMemStream theMemStream;
		TDCLXMLEncoder theEncoder( &theMemStream );
		theEncoder.AddObject( TDCLNSRef( theArray ) );

		std::string actual(
			static_cast<const char*>( theMemStream.GetBuffer() ),
			theMemStream.GetBufferSize() );

		const std::string expected =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE frame PUBLIC \"-//DCL Group//Canonical DTD 1.0//EN\" \"http://www.kallisys.com/DTDs/newtonscript.dtd\">\n"
			"<array id=\"n0\">\n"
			"\t<string id=\"n1\">\n\t\t<value>HelloWorld</value>\n\t</string>\n"
			"\t<string id=\"n2\">\n\t\t<value>HelloWorld!</value>\n\t</string>\n"
			"\t<string id=\"n3\">\n\t\t<value>HelloWorld!</value>\n\t</string>\n"
			"\t<string id=\"n4\">\n\t\t<value>HelloWorld !</value>\n\t</string>\n"
			"\t<string id=\"n5\">\n\t\t<value>HelloWorld !</value>\n\t</string>\n"
			"\t<string id=\"n6\">\n\t\t<value>HelloWorld</value>\n\t</string>\n"
			"\t<string id=\"n7\">\n\t\t<value>HelloWorld!</value>\n\t</string>\n"
			"\t<string id=\"n8\">\n\t\t<value>HelloWorld!</value>\n\t</string>\n"
			"\t<string id=\"n9\">\n\t\t<value>HelloWorld !</value>\n\t</string>\n"
			"\t<string id=\"n10\">\n\t\t<value>HelloWorld !</value>\n\t</string>\n"
			"\t<string id=\"n11\">\n\t\t<value>Hello World</value>\n\t</string>\n"
			"\t<string id=\"n12\">\n\t\t<value>Hello World!</value>\n\t</string>\n"
			"\t<string id=\"n13\">\n\t\t<value>Hello World!</value>\n\t</string>\n"
			"\t<string id=\"n14\">\n\t\t<value>Hello World !</value>\n\t</string>\n"
			"\t<string id=\"n15\">\n\t\t<value>Hello World !</value>\n\t</string>\n"
			"\t<string id=\"n16\">\n\t\t<value>Hello World</value>\n\t</string>\n"
			"\t<string id=\"n17\">\n\t\t<value>Hello World!</value>\n\t</string>\n"
			"\t<string id=\"n18\">\n\t\t<value>Hello World!</value>\n\t</string>\n"
			"\t<string id=\"n19\">\n\t\t<value>Hello World !</value>\n\t</string>\n"
			"\t<string id=\"n20\">\n\t\t<value>Hello World !</value>\n\t</string>\n"
			"</array>";

		CHECK( actual == expected );
	}

	TEST_CASE("ArraysAndFrames: slot creation, lookup, removal, and insertion")
	{
		TDCLNSFrame theFrame;
		theFrame.Set( "un",		TDCLNSRef::MakeInt( 1 ) );
		theFrame.Set( "deux",		TDCLNSRef::MakeInt( 2 ) );
		theFrame.Set( "trois",		TDCLNSRef::MakeInt( 3 ) );
		theFrame.Set( "quatre",	TDCLNSRef::MakeInt( 4 ) );

		CheckFrameSlots( theFrame, {
			{ 0x00FB92C2, "quatre", 4 },
			{ 0x972D6606, "deux", 2 },
			{ 0xBD5280CB, "un", 1 },
			{ 0xD4E5AAC9, "trois", 3 },
		} );
		CHECK( theFrame.HasSlot( "un" ) );

		(void) theFrame.RemoveSlot( "un" );
		CHECK_FALSE( theFrame.HasSlot( "un" ) );
		CheckFrameSlots( theFrame, {
			{ 0x00FB92C2, "quatre", 4 },
			{ 0x972D6606, "deux", 2 },
			{ 0xD4E5AAC9, "trois", 3 },
		} );

		TDCLNSArray theArray;
		theArray.Add( TDCLNSRef::MakeInt( 1 ) );
		theArray.Add( TDCLNSRef::MakeInt( 2 ) );
		theArray.Add( TDCLNSRef::MakeInt( 3 ) );
		theArray.Add( TDCLNSRef::MakeInt( 4 ) );
		theArray.Add( TDCLNSRef::MakeInt( 5 ) );
		theArray.Add( TDCLNSRef::MakeInt( 6 ) );
		CheckArrayInts( theArray, { 1, 2, 3, 4, 5, 6 } );

		theArray.RemoveSlot( 0 );
		CheckArrayInts( theArray, { 2, 3, 4, 5, 6 } );

		theArray.RemoveSlot( 2 );
		CheckArrayInts( theArray, { 2, 3, 5, 6 } );

		theArray.RemoveSlot( theArray.GetLength() - 1 );
		CheckArrayInts( theArray, { 2, 3, 5 } );

		theArray.Insert( 0, TDCLNSRef::MakeInt( 10 ) );
		CheckArrayInts( theArray, { 10, 2, 3, 5 } );

		theArray.Insert( 3, TDCLNSRef::MakeInt( 11 ) );
		CheckArrayInts( theArray, { 10, 2, 3, 11, 5 } );

		theArray.Insert( theArray.GetLength(), TDCLNSRef::MakeInt( 12 ) );
		CheckArrayInts( theArray, { 10, 2, 3, 11, 5, 12 } );
	}
}
