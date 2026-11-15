// ==============================
// File:			TDCLWakeableWait.cpp
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
// The Original Code is TDCLWakeableWait.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>

#include <algorithm>
#include <climits>

TDCLWakeableWait::~TDCLWakeableWait( void )
{
	Close();
}

int
TDCLWakeableWait::EnsureOpen( void )
{
	std::lock_guard<std::mutex> lock( mWakeMutex );
	return OpenWakePairLocked();
}

void
TDCLWakeableWait::Close( void )
{
	std::lock_guard<std::mutex> lock( mWakeMutex );
	CloseWakePairLocked();
}

void
TDCLWakeableWait::Trigger( void )
{
	std::lock_guard<std::mutex> lock( mWakeMutex );
	TriggerWakePairLocked();
}

TDCLWakeableWait::EResult
TDCLWakeableWait::WaitReady( dcl_wait_handle_t inHandle, EDirection inDirection, std::chrono::milliseconds inTimeout,
                             const std::stop_token& inStopToken, int* outError )
{
	if ( !IsValidHandle( inHandle, outError ) )
	{
		return EResult::kError;
	}

	if ( inStopToken.stop_requested() )
	{
		return EResult::kCancelled;
	}

	const int openErr = EnsureOpen();
	if ( openErr != 0 )
	{
		if ( outError )
		{
			*outError = openErr;
		}
		return EResult::kError;
	}

	Drain();

	struct TStopCallback
	{
		TDCLWakeableWait* mSelf;
		void
		operator()( void ) const
		{
			mSelf->Trigger();
		}
	};

	std::stop_callback<TStopCallback> onStop( inStopToken, TStopCallback{ this } );

	if ( inStopToken.stop_requested() )
	{
		Drain();
		return EResult::kCancelled;
	}

	const bool hasTimeout = ( inTimeout >= std::chrono::milliseconds::zero() );
	const auto deadline =
	    hasTimeout ? ( std::chrono::steady_clock::now() + inTimeout ) : std::chrono::steady_clock::time_point{};

	bool firstIter = true;
	for ( ;; )
	{
		int pollMs = -1;
		if ( hasTimeout )
		{
			auto remaining =
			    std::chrono::duration_cast<std::chrono::milliseconds>( deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				if ( !firstIter )
				{
					return EResult::kTimeout;
				}
				pollMs = 0;
			}
			else
			{
				pollMs = (int)std::min<long long>( remaining.count(), INT_MAX );
			}
		}
		firstIter = false;

		switch ( PollTwo( inHandle, inDirection, pollMs, outError ) )
		{
			case EPollResult::kInterrupted:
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				continue;

			case EPollResult::kError:
				return EResult::kError;

			case EPollResult::kTimeout:
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				return EResult::kTimeout;

			case EPollResult::kWake:
				Drain();
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				continue;

			case EPollResult::kReadyAndWake:
				Drain();
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				return EResult::kReady;

			case EPollResult::kReady:
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				return EResult::kReady;
		}
	}
}

TDCLWakeableWait::EResult
TDCLWakeableWait::PollNow( dcl_wait_handle_t inHandle, EDirection inDirection, int* outError )
{
	if ( !IsValidHandle( inHandle, outError ) )
	{
		return EResult::kError;
	}

	for ( ;; )
	{
		switch ( PollOne( inHandle, inDirection, outError ) )
		{
			case EPollResult::kInterrupted:
				continue;

			case EPollResult::kReady:
			case EPollResult::kReadyAndWake:
				return EResult::kReady;

			case EPollResult::kTimeout:
			case EPollResult::kWake:
				return EResult::kTimeout;

			case EPollResult::kError:
				return EResult::kError;
		}
	}
}
