// ==============================
// File:			TDCLEventQueue.h
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
// The Original Code is TDCLEventQueue.h.
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

#ifndef _TDCLEVENTQUEUE_H
#define _TDCLEVENTQUEUE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEvent.h>

// C++
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stop_token>

///
/// Thread-safe FIFO queue of TDCLEvent objects with bitmask filtering
/// and std::stop_token wakeup support.
///
class TDCLEventQueue
{
public:
	static constexpr std::chrono::milliseconds kForever{ -1 };

	TDCLEventQueue( void );
	~TDCLEventQueue( void );

	TDCLEventQueue( const TDCLEventQueue& ) = delete;
	TDCLEventQueue& operator=( const TDCLEventQueue& ) = delete;

	///
	/// Appends \p inEvent to the queue and wakes a waiting consumer.
	/// Ownership of \p inEvent transfers to the queue until popped.
	///
	void PostEvent( TDCLEvent* inEvent );

	///
	/// Waits for and pops the first queued event matching \p inEventMask.
	///
	/// \param inEventMask	bitmask of TDCLEvent::EEventKind to match.
	/// \param inTimeout	maximum wait (< 0 waits forever, 0 polls without blocking).
	/// \param inStopToken	optional cooperative stop token that aborts waiting.
	/// \return matching event (caller owns), or \c nil on timeout or stop.
	///
	TDCLEvent* WaitNextEvent( KUInt32 inEventMask = TDCLEvent::kAnyEventMask,
	                          std::chrono::milliseconds inTimeout = kForever, const std::stop_token& inStopToken = {} );

	///
	/// Deletes all currently queued events.
	///
	void Clear( void );

	///
	/// Deletes all currently queued events matching the mask and data pointer.
	///
	void RemoveEvents( KUInt32 inEventMask, const void* inData );

private:
	bool HasMatchingLocked( KUInt32 inEventMask ) const;
	TDCLEvent* PopMatchingLocked( KUInt32 inEventMask );

	std::mutex mMutex;
	std::condition_variable_any mCV;
	TDCLEvent* mHead;
	TDCLEvent* mTail;
};

#endif
// _TDCLEVENTQUEUE_H
