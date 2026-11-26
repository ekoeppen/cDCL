// ==============================
// File:			TDCLEventQueue.cpp
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
// The Original Code is TDCLEventQueue.cpp.
//
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2003-2004 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEventQueue.h>

TDCLEventQueue::TDCLEventQueue( void ) : mHead( nullptr ), mTail( nullptr )
{
}

TDCLEventQueue::~TDCLEventQueue( void )
{
	Clear();
}

void
TDCLEventQueue::Clear( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	TDCLEvent* theEvent = mHead;
	mHead = nullptr;
	mTail = nullptr;
	while ( theEvent )
	{
		TDCLEvent* theNext = theEvent->GetNextEvent();
		delete theEvent;
		theEvent = theNext;
	}
}

void
TDCLEventQueue::RemoveEvents( KUInt32 inEventMask, const void* inData )
{
	std::lock_guard<std::mutex> lock( mMutex );
	TDCLEvent* thePrevious = nullptr;
	TDCLEvent* theCurrent = mHead;
	while ( theCurrent )
	{
		TDCLEvent* theNext = theCurrent->GetNextEvent();
		if ( ( ( theCurrent->GetEventKind() & inEventMask ) != 0 )
			&& ( theCurrent->GetEventData() == inData ) )
		{
			if ( thePrevious )
			{
				thePrevious->SetNextEvent( theNext );
			}
			else
			{
				mHead = theNext;
			}
			if ( theNext == nullptr )
			{
				mTail = thePrevious;
			}
			delete theCurrent;
		}
		else
		{
			thePrevious = theCurrent;
		}
		theCurrent = theNext;
	}
}

void
TDCLEventQueue::PostEvent( TDCLEvent* inEvent )
{
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( mTail == nullptr )
		{
			mHead = inEvent;
			mTail = inEvent;
		}
		else
		{
			mTail->SetNextEvent( inEvent );
			mTail = inEvent;
		}
		inEvent->SetNextEvent( nullptr );
	}
	mCV.notify_all();
}

bool
TDCLEventQueue::HasMatchingLocked( KUInt32 inEventMask ) const
{
	const TDCLEvent* theEvent = mHead;
	while ( theEvent )
	{
		if ( ( theEvent->GetEventKind() & inEventMask ) != 0 )
		{
			return true;
		}
		theEvent = theEvent->GetNextEvent();
	}
	return false;
}

TDCLEvent*
TDCLEventQueue::PopMatchingLocked( KUInt32 inEventMask )
{
	TDCLEvent* thePrevious = nullptr;
	TDCLEvent* theCurrent = mHead;
	while ( theCurrent )
	{
		if ( ( theCurrent->GetEventKind() & inEventMask ) != 0 )
		{
			TDCLEvent* theNext = theCurrent->GetNextEvent();
			if ( thePrevious )
			{
				thePrevious->SetNextEvent( theNext );
			}
			else
			{
				mHead = theNext;
			}
			if ( theNext == nullptr )
			{
				mTail = thePrevious;
			}
			theCurrent->SetNextEvent( nullptr );
			return theCurrent;
		}
		thePrevious = theCurrent;
		theCurrent = theCurrent->GetNextEvent();
	}
	return nullptr;
}

TDCLEvent*
TDCLEventQueue::WaitNextEvent( KUInt32 inEventMask, std::chrono::milliseconds inTimeout,
                               const std::stop_token& inStopToken )
{
	std::unique_lock<std::mutex> lock( mMutex );

	if ( inStopToken.stop_requested() )
	{
		return nullptr;
	}

	if ( TDCLEvent* immediate = PopMatchingLocked( inEventMask ) )
	{
		return immediate;
	}

	if ( inTimeout == std::chrono::milliseconds::zero() )
	{
		return nullptr;
	}

	auto pred = [this, inEventMask] { return HasMatchingLocked( inEventMask ); };
	if ( inTimeout < std::chrono::milliseconds::zero() )
	{
		(void)mCV.wait( lock, inStopToken, pred );
	}
	else
	{
		(void)mCV.wait_for( lock, inStopToken, inTimeout, pred );
	}

	if ( inStopToken.stop_requested() )
	{
		return nullptr;
	}

	return PopMatchingLocked( inEventMask );
}
