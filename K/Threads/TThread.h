// ==============================
// Fichier:			TThread.h
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
// The Original Code is TThread.h.
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
// $Id: TThread.h,v 1.4 2006/07/07 08:19:16 pguyot Exp $
// ===========

#ifndef _TTHREAD_H
#define _TTHREAD_H

#include <K/Defines/KDefinitions.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

///
/// Class for a thread.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TThread
{
public:
	///
	/// Constants.
	///
	enum {
		kForever = 0,	///< Sleep forever, until WakeUp is called.
	};
	
	///
	/// States of a thread.
	///
	enum EState {
		kStopped,		///< Thread is stopped.
		kRunning,		///< Thread is running.
		kSuspended,		///< Thread is suspended (waiting for Resume).
		kSleeping		///< Thread is sleeping (waiting for WakeUp).
	};

	///
	/// Constructor from a runnable object.
	/// Starts the thread.
	///
	template<class TRunnable>
	TThread( TRunnable* inRunnable );

	///
	/// Destructor.
	/// Detaches the thread if still joinable.
	///
	~TThread( void );

	///
	/// Wait for the thread to finish execution.
	///
	void Join( void );

	///
	/// Determine if the thread is the current thread.
	///
	bool IsCurrentThread( void ) const;

	///
	/// Suspend the thread.
	/// Uses a counter: each suspend call should be balanced by a resume call.
	///
	void Suspend( void );

	///
	/// Resume the thread.
	/// Uses a counter: each suspend call should be balanced by a resume call.
	///
	void Resume( void );

	///
	/// Sleep (for a given time, in milliseconds, or until WakeUp is called).
	/// The thread can actually sleep longer.
	/// This method MUST be called from the current thread.
	/// Decrease the wake up counter or sleep if it's zero.
	///
	/// \param inMillisecs		time to sleep.
	/// \return	\c true if we slept all the time, \c false otherwise.
	///
	bool Sleep( KUInt32 inMilliseconds = kForever );

	///
	/// Wakes the thread up.
	/// Uses a counter with the wake up calls.
	///
	void	WakeUp( void );

private:
	///
	/// Check for cooperative suspension point from the executing thread.
	///
	void	CheckSuspend( void );

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TThread( const TThread& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TThread& operator = ( const TThread& inCopy );

	/// Shared thread synchronization state so detached workers never
	/// touch destroyed TThread members after ~TThread().
	struct SState
	{
		mutable std::mutex		mMutex;
		std::condition_variable	mSleepCond;
		std::condition_variable	mSuspendCond;
		std::thread::id			mThreadId{};
		EState					mState{ kRunning };
		KUInt32					mWakeCount{ 0 };
		KUInt32					mSuspendCount{ 0 };
	};

	/// \name Variables
	std::thread				mThread;		///< Standard thread object.
	std::shared_ptr<SState>	mState;			///< Shared thread state.
};

// -------------------------------------------------------------------------- //
//  * TThread( TRunnable* )
// -------------------------------------------------------------------------- //
template<class TRunnable>
TThread::TThread( TRunnable* inRunnable )
	:
		mState( std::make_shared<SState>() )
{
	auto state = mState;
	mThread = std::thread([state, inRunnable]() {
		{
			std::lock_guard<std::mutex> lock(state->mMutex);
			state->mThreadId = std::this_thread::get_id();
		}
		inRunnable->Run();
		{
			std::lock_guard<std::mutex> lock(state->mMutex);
			state->mState = kStopped;
		}
	});
}

#endif
		// _TTHREAD_H

// ============================================================================= //
//         There was once a programmer who was attached to the court of the      //
// warlord of Wu.  The warlord asked the programmer: "Which is easier to design: //
// an accounting package or an operating system?"                                //
//         "An operating system," replied the programmer.                        //
//         The warlord uttered an exclamation of disbelief.  "Surely an          //
// accounting package is trivial next to the complexity of an operating          //
// system," he said.                                                             //
//         "Not so," said the programmer, "when designing an accounting package, //
// the programmer operates as a mediator between people having different ideas:  //
// how it must operate, how its reports must appear, and how it must conform to  //
// the tax laws.  By contrast, an operating system is not limited my outside     //
// appearances.  When designing an operating system, the programmer seeks the    //
// simplest harmony between machine and ideas.  This is why an operating system  //
// is easier to design."                                                         //
//         The warlord of Wu nodded and smiled.  "That is all good and well, but //
// which is easier to debug?"                                                    //
//         The programmer made no reply.                                         //
//                 -- Geoffrey James, "The Tao of Programming"                   //
// ============================================================================= //
