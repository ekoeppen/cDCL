// ==============================
// Fichier:			TThread.cp
// Projet:			K
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Crée le:			26/5/2005
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
// The Original Code is TThread.cp.
// 
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2005 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org>
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TThread.cp,v 1.3 2006/07/07 08:19:16 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TThread.h"

#include <chrono>
#include <cassert>

// -------------------------------------------------------------------------- //
//  * ~TThread( void )
// -------------------------------------------------------------------------- //
TThread::~TThread( void )
{
	if (mThread.joinable())
	{
		mThread.detach();
	}
}

// -------------------------------------------------------------------------- //
//  * IsCurrentThread( void ) const
// -------------------------------------------------------------------------- //
Boolean
TThread::IsCurrentThread( void ) const
{
	return (std::this_thread::get_id() == mThreadId);
}

// -------------------------------------------------------------------------- //
//  * CheckSuspend( void )
// -------------------------------------------------------------------------- //
void
TThread::CheckSuspend( void )
{
	std::unique_lock<std::mutex> lock(mMutex);
	while (mSuspendCount > 0)
	{
		mState = kSuspended;
		mSuspendCond.wait(lock);
	}
	if (mState == kSuspended)
	{
		mState = kRunning;
	}
}

// -------------------------------------------------------------------------- //
//  * Suspend( void )
// -------------------------------------------------------------------------- //
void
TThread::Suspend( void )
{
	std::lock_guard<std::mutex> lock(mMutex);
	mSuspendCount++;
	if (mState == kRunning)
	{
		mState = kSuspended;
	}
}

// -------------------------------------------------------------------------- //
//  * Resume( void )
// -------------------------------------------------------------------------- //
void
TThread::Resume( void )
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (mSuspendCount > 0)
	{
		mSuspendCount--;
		if (mSuspendCount == 0)
		{
			if (mState == kSuspended)
			{
				mState = kRunning;
			}
			mSuspendCond.notify_all();
		}
	}
}

// ------------------------------------------------------------------------- //
//  * Sleep( KUInt32 )
// ------------------------------------------------------------------------- //
Boolean
TThread::Sleep( KUInt32 inMilliseconds /* = kForever */ )
{
	assert(IsCurrentThread());
	Boolean theResult = false;
	Boolean forever = (inMilliseconds == kForever);

	std::unique_lock<std::mutex> lock(mMutex);

	if (mWakeCount > 0)
	{
		mWakeCount--;
		theResult = false;
	} else {
		mState = kSleeping;

		if (!forever)
		{
			auto duration = std::chrono::milliseconds(inMilliseconds);
			std::cv_status status = mSleepCond.wait_for(lock, duration);
			theResult = (status == std::cv_status::timeout);
		} else {
			mSleepCond.wait(lock);
			theResult = false;
		}

		if (mSuspendCount > 0)
		{
			mState = kSuspended;
		} else {
			mState = kRunning;
		}
	}

	while (mSuspendCount > 0)
	{
		mState = kSuspended;
		mSuspendCond.wait(lock);
	}
	if (mState == kSuspended)
	{
		mState = kRunning;
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * WakeUp( void )
// ------------------------------------------------------------------------- //
void
TThread::WakeUp( void )
{
	std::lock_guard<std::mutex> lock(mMutex);

	if (mState == kSleeping)
	{
		mSleepCond.notify_one();
	} else {
		mWakeCount++;
	}
}

// ============================================================================= //
// `Lasu' Releases SAG 0.3 -- Freeware Book Takes Paves For New World Order      //
// by staff writers                                                              //
//                                                                               //
//         ...                                                                   //
//         The central Superhighway site called ``sunsite.unc.edu''              //
// collapsed in the morning before the release.  News about the release had      //
// been leaked by a German hacker group, Harmonious Hardware Hackers, who        //
// had cracked into the author's computer earlier in the week.  They had         //
// got the release date wrong by one day, and caused dozens of eager fans        //
// to connect to the sunsite computer at the wrong time.  ``No computer can      //
// handle that kind of stress,'' explained the mourning sunsite manager,         //
// Erik Troan.  ``The spinning disks made the whole computer jump, and           //
// finally it crashed through the floor to the basement.''  Luckily,             //
// repairs were swift and the computer was working again the same evening.       //
// ``Thank God we were able to buy enough needles and thread and patch it        //
// together without major problems.''  The site has also installed a new         //
// throttle on the network pipe, allowing at most four clients at the same       //
// time, thus making a new crash less likely.  ``The book is now in our          //
// Incoming folder'', says Troan, ``and you're all welcome to come and get it.'' //
//                 -- Lars Wirzenius <wirzeniu@cs.helsinki.fi>                   //
//                    [comp.os.linux.announce]                                   //
// ============================================================================= //
