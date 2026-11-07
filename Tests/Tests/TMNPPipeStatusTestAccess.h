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
