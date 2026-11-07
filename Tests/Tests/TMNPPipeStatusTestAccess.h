// ==============================
// File:			TMNPPipeStatusTestAccess.h
// Project:			Desktop Connection Library
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
// The Original Code is TMNPPipeStatusTestAccess.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef TMNPPIPESTATUSTESTACCESS_H
#define TMNPPIPESTATUSTESTACCESS_H

#include <DCL/Communication_Layers/TDCLMNPPipe.h>

// Test-only access to timing and teardown diagnostics, not application API.
class TMNPPipeStatusTestAccess
{
public:
	static void
	SetStallTimeout( TDCLMNPPipe& inPipe, std::chrono::milliseconds inTimeout )
	{
		inPipe.mStallTimeout = inTimeout;
	}

	static bool
	IsCleanDisconnect( const TDCLMNPPipe& inPipe )
	{
		return inPipe.mState == TDCLMNPPipe::kDisconnected;
	}

	static std::uint8_t
	GetDisconnectReason( const TDCLMNPPipe& inPipe )
	{
		return inPipe.mLocalReason;
	}
};

#endif
