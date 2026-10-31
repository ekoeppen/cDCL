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

#include "UTestKStreamsAndBuffers.h"

#include <K/Defines/UByteSex.h>
#include <K/Exceptions/IO/TEOFException.h>
#include <K/Misc/TCircleBuffer.h>
#include <K/Streams/TMemoryStream.h>

#include <cassert>
#include <cstdio>
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
		assert(theData[i] == (KUInt8) ioNext);
		ioNext++;
	}
	return theAmount;
}

} // namespace

void UTestKStreamsAndBuffers::Run(void)
{
	std::printf("--- Testing Streams, Buffers, and Endianness ---\n");
	std::printf("Running TCircleBuffer tests...\n");
	TestCircleBufferStartsEmpty();
	TestCircleBufferFillToCapacity();
	TestCircleBufferFillAfterWrap();
	TestCircleBufferProduceIntoWrapped();
	TestCircleBufferConsumeMoreThanAvailable();
	TestCircleBufferPeekAndDiscard();
	TestCircleBufferTraffic();

	std::printf("Running TStream & TMemoryStream tests...\n");
	TestMemoryStreamReadWrite();
	TestStreamEndianness();
	TestStreamEOF();

	std::printf("Running UByteSex tests...\n");
	TestUByteSexSwaps();

	std::printf("--- All stream, buffer, and endianness tests passed! ---\n");
}

void UTestKStreamsAndBuffers::TestCircleBufferStartsEmpty(void)
{
	TCircleBuffer theBuffer(16);
	assert(theBuffer.IsEmpty());
	assert(theBuffer.AvailableBytes() == 0);
	KUInt8 theByte = 0;
	assert(theBuffer.Consume(&theByte, 1) == 0);
}

void UTestKStreamsAndBuffers::TestCircleBufferFillToCapacity(void)
{
	TCircleBuffer theBuffer(16);
	unsigned produced = 0, consumed = 0;
	ProduceStream(theBuffer, 16, produced);
	assert(!theBuffer.IsEmpty());
	assert(theBuffer.AvailableBytes() == 16);
	assert(ConsumeStream(theBuffer, 16, consumed) == 16);
	assert(theBuffer.IsEmpty());
}

void UTestKStreamsAndBuffers::TestCircleBufferFillAfterWrap(void)
{
	TCircleBuffer theBuffer(16);
	unsigned produced = 0, consumed = 0;
	ProduceStream(theBuffer, 10, produced);
	assert(ConsumeStream(theBuffer, 10, consumed) == 10);
	ProduceStream(theBuffer, 16, produced);
	assert(theBuffer.AvailableBytes() == 16);
	assert(ConsumeStream(theBuffer, 16, consumed) == 16);
	assert(theBuffer.IsEmpty());
}

void UTestKStreamsAndBuffers::TestCircleBufferProduceIntoWrapped(void)
{
	TCircleBuffer theBuffer(16);
	unsigned produced = 0, consumed = 0;
	ProduceStream(theBuffer, 12, produced);
	assert(ConsumeStream(theBuffer, 8, consumed) == 8);
	ProduceStream(theBuffer, 8, produced); // wraps around
	ProduceStream(theBuffer, 6, produced); // producer behind consumer
	assert(theBuffer.AvailableBytes() == 18);
	assert(ConsumeStream(theBuffer, 100, consumed) == 18);
	assert(consumed == produced);
}

void UTestKStreamsAndBuffers::TestCircleBufferConsumeMoreThanAvailable(void)
{
	TCircleBuffer theBuffer(16);
	unsigned produced = 0, consumed = 0;
	ProduceStream(theBuffer, 14, produced);
	assert(ConsumeStream(theBuffer, 12, consumed) == 12);
	ProduceStream(theBuffer, 6, produced); // wraps around
	assert(ConsumeStream(theBuffer, 50, consumed) == 8);
	assert(theBuffer.IsEmpty());
	ProduceStream(theBuffer, 5, produced);
	assert(ConsumeStream(theBuffer, 50, consumed) == 5);
}

void UTestKStreamsAndBuffers::TestCircleBufferPeekAndDiscard(void)
{
	TCircleBuffer theBuffer(16);
	const char msg[] = "HelloWorld!";
	theBuffer.Produce(msg, 11);
	assert(theBuffer.AvailableBytes() == 11);

	char peekBuf[12] = { 0 };
	KUIntPtr peeked = theBuffer.Peek(peekBuf, 5);
	assert(peeked == 5);
	assert(std::string(peekBuf, 5) == "Hello");
	assert(theBuffer.AvailableBytes() == 11); // Not consumed!

	theBuffer.Discard(5);
	assert(theBuffer.AvailableBytes() == 6);

	char rest[12] = { 0 };
	KUIntPtr consumed = theBuffer.Consume(rest, 6);
	assert(consumed == 6);
	assert(std::string(rest, 6) == "World!");
	assert(theBuffer.IsEmpty());
}

void UTestKStreamsAndBuffers::TestCircleBufferTraffic(void)
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
		assert(theBuffer.AvailableBytes() == (KUIntPtr) (produced - consumed));
	}
	ConsumeStream(theBuffer, produced - consumed, consumed);
	assert(theBuffer.IsEmpty());
	assert(consumed == produced);
}

void UTestKStreamsAndBuffers::TestMemoryStreamReadWrite(void)
{
	TMemoryStream mem;
	const char testStr[] = "Newton OS Communications";
	KUInt32 count = sizeof(testStr);
	mem.Write(testStr, &count);
	assert(count == sizeof(testStr));
	assert(mem.GetData().size() == sizeof(testStr));

	// Reset cursor to beginning
	mem.SetCursor(0, TRandomAccessStream::kFromStart);
	char outBuf[64] = { 0 };
	KUInt32 readCount = sizeof(testStr);
	mem.Read(outBuf, &readCount);
	assert(readCount == sizeof(testStr));
	assert(std::string(outBuf) == testStr);
}

void UTestKStreamsAndBuffers::TestStreamEndianness(void)
{
	TMemoryStream mem;
	// Write 16-bit and 32-bit big-endian words (Newton format)
	mem.PutInt16BE(0x1234);
	mem.PutInt32BE(0x89ABCDEF);
	mem.PutByte(0x42);

	assert(mem.GetData().size() == 2 + 4 + 1);

	mem.SetCursor(0, TRandomAccessStream::kFromStart);
	assert(mem.GetInt16BE() == 0x1234);
	assert(mem.GetInt32BE() == 0x89ABCDEF);
	assert(mem.GetByte() == 0x42);
}

void UTestKStreamsAndBuffers::TestStreamEOF(void)
{
	TMemoryStream mem;
	mem.PutByte(0xAA);
	mem.SetCursor(0, TRandomAccessStream::kFromStart);
	assert(mem.GetByte() == 0xAA);

	bool threwEOF = false;
	try {
		(void) mem.GetByte();
	} catch (const TEOFException&) {
		threwEOF = true;
	}
	assert(threwEOF);
}

void UTestKStreamsAndBuffers::TestUByteSexSwaps(void)
{
	// 16-bit
	KUInt16 w16 = 0x1234;
	assert(UByteSex::Swap(w16) == 0x3412);
	assert(UByteSex::Swap((KUInt16) 0x3412) == 0x1234);

	// 32-bit
	KUInt32 w32 = 0x12345678;
	assert(UByteSex::Swap(w32) == 0x78563412);
	assert(UByteSex::Swap((KUInt32) 0x78563412) == 0x12345678);

	// 64-bit
	KUInt64 w64 = 0x0123456789ABCDEFULL;
	assert(UByteSex::Swap(w64) == 0xEFCDAB8967452301ULL);
	assert(UByteSex::Swap((KUInt64) 0xEFCDAB8967452301ULL) == 0x0123456789ABCDEFULL);

	// Template Swaps
	uint16_t u16 = 0xAABB;
	uint32_t u32 = 0x11223344;
	uint64_t u64 = 0x1122334455667788ULL;
	assert(UByteSex::Swap(u16) == 0xBBAA);
	assert(UByteSex::Swap(u32) == 0x44332211);
	assert(UByteSex::Swap(u64) == 0x8877665544332211ULL);
}
