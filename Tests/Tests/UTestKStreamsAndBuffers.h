// ==============================
// Fichier:			UTestKStreamsAndBuffers.h
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
// The Original Code is UTestKStreamsAndBuffers.h.
// 
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
// 
// ***** END LICENSE BLOCK *****

#ifndef _UTESTKSTREAMSANDBUFFERS_H
#define _UTESTKSTREAMSANDBUFFERS_H

#include <K/Defines/KDefinitions.h>

class UTestKStreamsAndBuffers
{
public:
	/// Run all stream, circular buffer, and endianness tests.
	static void Run(void);

private:
	// TCircleBuffer test cases (aligned with Einstein)
	static void TestCircleBufferStartsEmpty(void);
	static void TestCircleBufferFillToCapacity(void);
	static void TestCircleBufferFillAfterWrap(void);
	static void TestCircleBufferProduceIntoWrapped(void);
	static void TestCircleBufferConsumeMoreThanAvailable(void);
	static void TestCircleBufferPeekAndDiscard(void);
	static void TestCircleBufferTraffic(void);

	// TStream & TMemoryStream test cases
	static void TestMemoryStreamReadWrite(void);
	static void TestStreamEndianness(void);
	static void TestStreamEOF(void);

	// UByteSex test cases
	static void TestUByteSexSwaps(void);
};

#endif
		// _UTESTKSTREAMSANDBUFFERS_H
