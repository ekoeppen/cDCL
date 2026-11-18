// ==============================
// Fichier:			UTestXML.cpp
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

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// Tests
#include "UTestNS.h"

#include <doctest/doctest.h>

#include <string>

TEST_SUITE("XML")
{
	TEST_CASE("Walter Smith frame XML encoding")
	{
		TDCLNSRef theWalterSmithFrame = UTestNS::GetWalterSmith();

		TDCLMemStream theMemStream;
		TDCLXMLEncoder theEncoder( &theMemStream );
		theEncoder.AddObject( theWalterSmithFrame );

		std::string actual(
			static_cast<const char*>( theMemStream.GetBuffer() ),
			theMemStream.GetBufferSize() );

		const std::string expected =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE frame PUBLIC \"-//DCL Group//Canonical DTD 1.0//EN\" \"http://www.kallisys.com/DTDs/newtonscript.dtd\">\n"
			"<frame id=\"n0\">\n"
			"\t<slot symbol=\"nameAgain\">\n"
			"\t\t<string id=\"n1\">\n"
			"\t\t\t<value>Walter Smith</value>\n"
			"\t\t</string>\n"
			"\t</slot>\n"
			"\t<slot symbol=\"uchar\">\n"
			"\t\t<char>\xE2\x80\xA2</char>\n"
			"\t</slot>\n"
			"\t<slot symbol=\"name\">\n"
			"\t\t<precedent idref=\"n1\"/>\n"
			"\t</slot>\n"
			"\t<slot symbol=\"bounds\">\n"
			"\t\t<frame id=\"n2\">\n"
			"\t\t\t<slot symbol=\"right\">\n"
			"\t\t\t\t<int>40</int>\n"
			"\t\t\t</slot>\n"
			"\t\t\t<slot symbol=\"top\">\n"
			"\t\t\t\t<int>14</int>\n"
			"\t\t\t</slot>\n"
			"\t\t\t<slot symbol=\"left\">\n"
			"\t\t\t\t<int>10</int>\n"
			"\t\t\t</slot>\n"
			"\t\t\t<slot symbol=\"bottom\">\n"
			"\t\t\t\t<int>100</int>\n"
			"\t\t\t</slot>\n"
			"\t\t</frame>\n"
			"\t</slot>\n"
			"\t<slot symbol=\"cats\">\n"
			"\t\t<int>2</int>\n"
			"\t</slot>\n"
			"\t<slot symbol=\"phones\">\n"
			"\t\t<array id=\"n3\">\n"
			"\t\t\t<string id=\"n4\">\n"
			"\t\t\t\t<value>408-996-1010</value>\n"
			"\t\t\t</string>\n"
			"\t\t\t<binary id=\"n5\">\n"
			"\t\t\t\t<class><symbol>faxPhone</symbol></class>\n"
			"\t\t\t\t<value>\n"
			"\t\t\t\t\tADQAMAA4AC0AOQA3ADQALQA5ADAAOQA0\n"
			"\t\t\t\t\tAAA=\n"
			"\t\t\t\t</value>\n"
			"\t\t\t</binary>\n"
			"\t\t</array>\n"
			"\t</slot>\n"
			"</frame>";

		CHECK( actual == expected );
	}

	TEST_CASE("Classed string XML encoding")
	{
		TDCLNSFrame theFrame;
		TDCLNSRef theString = TDCLNSRef::MakeString("bar");
		(void) theString.SetClass( TDCLNSRef::MakeSymbol("string.bar") );
		theFrame.Set("foo", theString );

		TDCLMemStream theMemStream;
		TDCLXMLEncoder theEncoder( &theMemStream );
		theEncoder.AddObject( TDCLNSRef( theFrame ) );

		std::string actual(
			static_cast<const char*>( theMemStream.GetBuffer() ),
			theMemStream.GetBufferSize() );

		const std::string expected =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE frame PUBLIC \"-//DCL Group//Canonical DTD 1.0//EN\" \"http://www.kallisys.com/DTDs/newtonscript.dtd\">\n"
			"<frame id=\"n0\">\n"
			"\t<slot symbol=\"foo\">\n"
			"\t\t<string id=\"n1\">\n"
			"\t\t\t<class><symbol>string.bar</symbol></class>\n"
			"\t\t\t<value>bar</value>\n"
			"\t\t</string>\n"
			"\t</slot>\n"
			"</frame>";

		CHECK( actual == expected );
	}
}
