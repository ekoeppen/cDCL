// ==============================
// Fichier:			UTestKStreamsAndBuffers.cpp
// Projet:			cDCL
// Ecrit par:		Eckhart Koeppen (eck@40hz.org)
// 
// Créé le:			31/10/2026
// Tabulation:		4 espaces
// 
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
// 
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
// 
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
// 
// The Original Code is UTestKStreamsAndBuffers.cpp.
// 
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
// 
// ***** END LICENSE BLOCK *****

#include <K/Defines/UByteSex.h>
#include <K/Exceptions/IO/TEOFException.h>
#include <K/Misc/TCircleBuffer.h>
#include <K/Streams/TMemoryStream.h>

#include <doctest/doctest.h>

#include <string>
#include <vector>

namespace {

void ProduceStream(TCircleBuffer& ioBuffer, KUIntPtr inAmount, unsigned& ioNext)
{
	std::vector<KUInt8> theData(inAmount);
	for (auto& b : theData)
		b = (KUInt8) (ioNext++);
	ioBuffer.Produce(theData.data(), inAmount);
}

KUIntPtr ConsumeStream(TCircleBuffer& ioBuffer, KUIntPtr inAmount, unsigned& ioNext)
{
	std::vector<KUInt8> theData(inAmount);
	KUIntPtr theAmount = ioBuffer.Consume(theData.data(), inAmount);
	for (KUIntPtr i = 0; i < theAmount; i++)
	{
		CHECK(theData[i] == (KUInt8) ioNext);
		ioNext++;
	}
	return theAmount;
}

} // namespace

TEST_SUITE("KStreamsAndBuffers")
{
	TEST_CASE("TCircleBuffer starts empty")
	{
		TCircleBuffer theBuffer(16);
		CHECK(theBuffer.IsEmpty());
		CHECK(theBuffer.AvailableBytes() == 0);
		KUInt8 theByte = 0;
		CHECK(theBuffer.Consume(&theByte, 1) == 0);
	}

	TEST_CASE("TCircleBuffer fills to capacity")
	{
		TCircleBuffer theBuffer(16);
		unsigned produced = 0, consumed = 0;
		ProduceStream(theBuffer, 16, produced);
		CHECK_FALSE(theBuffer.IsEmpty());
		CHECK(theBuffer.AvailableBytes() == 16);
		CHECK(ConsumeStream(theBuffer, 16, consumed) == 16);
		CHECK(theBuffer.IsEmpty());
	}

	TEST_CASE("TCircleBuffer fills after wrap")
	{
		TCircleBuffer theBuffer(16);
		unsigned produced = 0, consumed = 0;
		ProduceStream(theBuffer, 10, produced);
		REQUIRE(ConsumeStream(theBuffer, 10, consumed) == 10);
		ProduceStream(theBuffer, 16, produced);
		CHECK(theBuffer.AvailableBytes() == 16);
		CHECK(ConsumeStream(theBuffer, 16, consumed) == 16);
		CHECK(theBuffer.IsEmpty());
	}

	TEST_CASE("TCircleBuffer produces into wrapped buffer")
	{
		TCircleBuffer theBuffer(16);
		unsigned produced = 0, consumed = 0;
		ProduceStream(theBuffer, 12, produced);
		REQUIRE(ConsumeStream(theBuffer, 8, consumed) == 8);
		ProduceStream(theBuffer, 8, produced); // wraps around
		ProduceStream(theBuffer, 6, produced); // producer behind consumer
		CHECK(theBuffer.AvailableBytes() == 18);
		CHECK(ConsumeStream(theBuffer, 100, consumed) == 18);
		CHECK(consumed == produced);
	}

	TEST_CASE("TCircleBuffer consumes more than available")
	{
		TCircleBuffer theBuffer(16);
		unsigned produced = 0, consumed = 0;
		ProduceStream(theBuffer, 14, produced);
		REQUIRE(ConsumeStream(theBuffer, 12, consumed) == 12);
		ProduceStream(theBuffer, 6, produced); // wraps around
		CHECK(ConsumeStream(theBuffer, 50, consumed) == 8);
		CHECK(theBuffer.IsEmpty());
		ProduceStream(theBuffer, 5, produced);
		CHECK(ConsumeStream(theBuffer, 50, consumed) == 5);
	}

	TEST_CASE("TCircleBuffer peek and discard")
	{
		TCircleBuffer theBuffer(16);
		const char msg[] = "HelloWorld!";
		theBuffer.Produce(msg, 11);
		CHECK(theBuffer.AvailableBytes() == 11);

		char peekBuf[12] = { 0 };
		KUIntPtr peeked = theBuffer.Peek(peekBuf, 5);
		CHECK(peeked == 5);
		CHECK(std::string(peekBuf, 5) == "Hello");
		CHECK(theBuffer.AvailableBytes() == 11); // Not consumed!

		theBuffer.Discard(5);
		CHECK(theBuffer.AvailableBytes() == 6);

		char rest[12] = { 0 };
		KUIntPtr consumed = theBuffer.Consume(rest, 6);
		CHECK(consumed == 6);
		CHECK(std::string(rest, 6) == "World!");
		CHECK(theBuffer.IsEmpty());
	}

	TEST_CASE("TCircleBuffer randomized traffic")
	{
		TCircleBuffer theBuffer(0xEA0 * 4);
		unsigned produced = 0, consumed = 0;
		for (int round = 0; round < 200; round++)
		{
			int buffers = 1 + (round * 7) % 5;
			for (int i = 0; i < buffers; i++)
				ProduceStream(theBuffer, 0xEA0, produced);
			KUIntPtr piece = 512 + (round * 131) % 1500;
			while (theBuffer.AvailableBytes() > (KUIntPtr) (round % 3) * 0xEA0)
				ConsumeStream(theBuffer, piece, consumed);
			REQUIRE(theBuffer.AvailableBytes() == (KUIntPtr) (produced - consumed));
		}
		ConsumeStream(theBuffer, produced - consumed, consumed);
		CHECK(theBuffer.IsEmpty());
		CHECK(consumed == produced);
	}

	TEST_CASE("TMemoryStream read and write")
	{
		TMemoryStream mem;
		const char testStr[] = "Newton OS Communications";
		KUInt32 count = sizeof(testStr);
		mem.Write(testStr, &count);
		CHECK(count == sizeof(testStr));
		CHECK(mem.GetData().size() == sizeof(testStr));

		// Reset cursor to beginning
		mem.SetCursor(0, TRandomAccessStream::kFromStart);
		char outBuf[64] = { 0 };
		KUInt32 readCount = sizeof(testStr);
		mem.Read(outBuf, &readCount);
		CHECK(readCount == sizeof(testStr));
		CHECK(std::string(outBuf) == testStr);
	}

	TEST_CASE("TMemoryStream big-endian primitives")
	{
		TMemoryStream mem;
		// Write 16-bit and 32-bit big-endian words (Newton format)
		mem.PutInt16BE(0x1234);
		mem.PutInt32BE(0x89ABCDEF);
		mem.PutByte(0x42);

		REQUIRE(mem.GetData().size() == 2 + 4 + 1);

		mem.SetCursor(0, TRandomAccessStream::kFromStart);
		CHECK(mem.GetInt16BE() == 0x1234);
		CHECK(mem.GetInt32BE() == 0x89ABCDEF);
		CHECK(mem.GetByte() == 0x42);
	}

	TEST_CASE("TMemoryStream throws TEOFException at EOF")
	{
		TMemoryStream mem;
		mem.PutByte(0xAA);
		mem.SetCursor(0, TRandomAccessStream::kFromStart);
		CHECK(mem.GetByte() == 0xAA);
		CHECK_THROWS_AS((void) mem.GetByte(), TEOFException);
	}

	TEST_CASE("UByteSex swaps 16/32/64-bit integers")
	{
		// 16-bit
		KUInt16 w16 = 0x1234;
		CHECK(UByteSex::Swap(w16) == 0x3412);
		CHECK(UByteSex::Swap((KUInt16) 0x3412) == 0x1234);

		// 32-bit
		KUInt32 w32 = 0x12345678;
		CHECK(UByteSex::Swap(w32) == 0x78563412);
		CHECK(UByteSex::Swap((KUInt32) 0x78563412) == 0x12345678);

		// 64-bit
		KUInt64 w64 = 0x0123456789ABCDEFULL;
		CHECK(UByteSex::Swap(w64) == 0xEFCDAB8967452301ULL);
		CHECK(UByteSex::Swap((KUInt64) 0xEFCDAB8967452301ULL) == 0x0123456789ABCDEFULL);

		// Template Swaps
		uint16_t u16 = 0xAABB;
		uint32_t u32 = 0x11223344;
		uint64_t u64 = 0x1122334455667788ULL;
		CHECK(UByteSex::Swap(u16) == 0xBBAA);
		CHECK(UByteSex::Swap(u32) == 0x44332211);
		CHECK(UByteSex::Swap(u64) == 0x8877665544332211ULL);
	}
}
