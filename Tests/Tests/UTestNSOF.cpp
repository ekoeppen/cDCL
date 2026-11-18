// ==============================
// Fichier:			UTestNSOF.cpp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			21/10/2002
// Tabulation:		4 espaces
//
// Copyright:		© 2002-2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestNSOF.cp,v 1.12 2004/03/13 08:51:22 paul Exp $
// ===========

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// Tests
#include "UTestNS.h"

#include <doctest/doctest.h>

#include <vector>

namespace
{
const std::vector<KUInt8> kExpectedWalterSmithNSOF = {
	0x02, 0x06, 0x06, 0x07, 0x09, 0x6E, 0x61, 0x6D, 0x65, 0x41, 0x67, 0x61, 0x69, 0x6E, 0x07, 0x05,
	0x75, 0x63, 0x68, 0x61, 0x72, 0x07, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0x07, 0x06, 0x62, 0x6F, 0x75,
	0x6E, 0x64, 0x73, 0x07, 0x04, 0x63, 0x61, 0x74, 0x73, 0x07, 0x06, 0x70, 0x68, 0x6F, 0x6E, 0x65,
	0x73, 0x08, 0x1A, 0x00, 0x57, 0x00, 0x61, 0x00, 0x6C, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00,
	0x20, 0x00, 0x53, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x74, 0x00, 0x68, 0x00, 0x00, 0x02, 0x20, 0x22,
	0x09, 0x07, 0x0B, 0x0E, 0x0A, 0x64, 0x28, 0x00, 0x08, 0x05, 0x02, 0x08, 0x1A, 0x00, 0x34, 0x00,
	0x30, 0x00, 0x38, 0x00, 0x2D, 0x00, 0x39, 0x00, 0x39, 0x00, 0x36, 0x00, 0x2D, 0x00, 0x31, 0x00,
	0x30, 0x00, 0x31, 0x00, 0x30, 0x00, 0x00, 0x03, 0x1A, 0x07, 0x08, 0x66, 0x61, 0x78, 0x50, 0x68,
	0x6F, 0x6E, 0x65, 0x00, 0x34, 0x00, 0x30, 0x00, 0x38, 0x00, 0x2D, 0x00, 0x39, 0x00, 0x37, 0x00,
	0x34, 0x00, 0x2D, 0x00, 0x39, 0x00, 0x30, 0x00, 0x39, 0x00, 0x34, 0x00, 0x00
};
} // namespace

TEST_SUITE("NSOF")
{
	TEST_CASE("Walter Smith frame NSOF encode, decode, and re-encode")
	{
		TDCLNSRef theWalterSmithFrameRef = UTestNS::GetWalterSmith();
		TDCLNSFrame& theWalterSmithFrame = theWalterSmithFrameRef.ToFrame();

		TDCLMemStream theMemStream;
		TDCLNSOFEncoder theEncoder( &theMemStream );
		theEncoder.AddObject( theWalterSmithFrameRef );

		const KUInt8* theData = (const KUInt8*) theMemStream.GetBuffer();
		KUInt32 theDataCount = theMemStream.GetBufferSize();
		std::vector<KUInt8> encoded( theData, theData + theDataCount );
		CHECK( encoded == kExpectedWalterSmithNSOF );

		theMemStream.SetCursor( 0, TDCLMemStream::kFromStart );
		TDCLNSOFDecoder theDecoder( &theMemStream );
		TDCLNSRef theFrameDecodedRef = theDecoder.GetNextObject();
		TDCLNSFrame& theFrameDecoded = theFrameDecodedRef.ToFrame();

		KUInt32 nbPairs = theFrameDecoded.GetLength();
		REQUIRE( nbPairs == theWalterSmithFrame.GetLength() );
		for (KUInt32 indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			TDCLNSSymbol& theKey =
				theWalterSmithFrame.GetKey(indexPairs).ToSymbol();
			REQUIRE( theFrameDecoded.HasSlot(theKey) );
			TDCLNSRef theWSValue = theWalterSmithFrame.Get(theKey);
			TDCLNSRef theFDValue = theFrameDecoded.Get(theKey);
			if (theWSValue != theFDValue)
			{
				CHECK( theWSValue.ClassOf() == theFDValue.ClassOf() );
				CHECK( theWSValue.IsPtr() );
			}
		}

		TDCLMemStream anotherMemStream;
		TDCLNSOFEncoder anotherEncoder( &anotherMemStream );
		anotherEncoder.AddObject( theFrameDecodedRef );

		const KUInt8* reData = (const KUInt8*) anotherMemStream.GetBuffer();
		KUInt32 reDataCount = anotherMemStream.GetBufferSize();
		std::vector<KUInt8> reencoded( reData, reData + reDataCount );
		CHECK( reencoded == kExpectedWalterSmithNSOF );
	}
}
