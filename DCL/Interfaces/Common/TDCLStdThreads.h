// ==============================
// File:			TDCLStdThreads.h
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
// The Original Code is TDCLStdThreads.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLSTDTHREADS_H
#define _TDCLSTDTHREADS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLThreads.h>

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>

///
/// Portable C++20 implementation of the IDCLThreads interface backed by
/// std::jthread, std::mutex, and std::condition_variable_any.
///
class TDCLStdThreads : public IDCLThreads
{
public:
	///
	/// Creates an IThread object.
	///
	/// \param inThreadedObject		object associated with the thread.
	/// \return a new IThread object.
	///
	virtual IThread* CreateThread( TDCLThread* inThreadedObject ) override;

	///
	/// Creates a semaphore object.
	///
	/// \return a new semaphore object.
	///
	virtual ISemaphore* CreateSemaphore( void ) override;

private:
	///
	/// Counting semaphore initialized to 1, which can be acquired twice
	/// by the same thread and released by another.
	///
	class TSemaphore : public ISemaphore
	{
	public:
		///
		/// Default constructor.
		///
		TSemaphore( void );

		///
		/// Destructor.
		///
		virtual ~TSemaphore( void );

		///
		/// Waits until the count is positive, then decrements it.
		///
		virtual void Acquire( void ) override;

		///
		/// Increments the count and wakes up one waiting thread.
		///
		virtual void Release( void ) override;

	private:
		TSemaphore( const TSemaphore& ) = delete;
		TSemaphore& operator=( const TSemaphore& ) = delete;

		std::mutex mMutex;                ///< Mutex protecting the count.
		std::condition_variable mCondVar; ///< Condition variable for waiting.
		std::size_t mCount;               ///< Semaphore count.
	};

	///
	/// C++20 thread implementation.
	///
	class TThread : public IThread
	{
	public:
		///
		/// Constructor from a TDCLThread object.
		///
		/// \param inThreadedObject	object associated with the thread.
		///
		TThread( TDCLThread* inThreadedObject );

		///
		/// Destructor.
		///
		virtual ~TThread( void );

		///
		/// Starts the thread.
		///
		/// \throws TDCLThreadActiveException if the thread is not stopped.
		///
		virtual void Start( void ) override;

		///
		/// Waits for the thread to finish executing.
		///
		/// \throws TDCLThreadDeadLockException if called from the thread itself.
		///
		virtual void Join( void ) override;

		///
		/// Sleeps for n milliseconds, until WakeUp(), or until a stop request.
		///
		/// \param inMillisecs	number of milliseconds to sleep (or kForever).
		/// \return	\c true if the full timeout elapsed, \c false otherwise.
		///
		virtual Boolean Sleep( KUInt32 inMillisecs ) override;

		///
		/// Wakes up the thread.
		///
		virtual void WakeUp( void ) override;

		///
		/// Returns the current state of the thread.
		///
		/// \return the thread state.
		///
		virtual EState GetState( void ) override;

	private:
		TThread( const TThread& ) = delete;
		TThread& operator=( const TThread& ) = delete;

		std::jthread mThread;                   ///< Standard thread.
		std::mutex mMutex;                      ///< Mutex protecting the state.
		std::condition_variable_any mSleepCond; ///< Condition variable for Sleep/WakeUp.
		EState mState;                          ///< Thread state.
		KUInt32 mWakeCount;                     ///< Wake-up counter.
	};
};

#endif
// _TDCLSTDTHREADS_H
