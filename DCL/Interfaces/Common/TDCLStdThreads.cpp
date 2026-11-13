// ==============================
// File:			TDCLStdThreads.cpp
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
// The Original Code is TDCLStdThreads.cpp.
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
#include <DCL/Interfaces/Common/TDCLStdThreads.h>

#include <chrono>
#include <stop_token>
#include <system_error>
#include <utility>

// DCL
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.h>
#include <DCL/Interfaces/TDCLThread.h>

// ------------------------------------------------------------------------- //
//  * CreateThread( TDCLThread* )
// ------------------------------------------------------------------------- //
IDCLThreads::IThread*
TDCLStdThreads::CreateThread( TDCLThread* inThreadedObject )
{
	return new TThread( inThreadedObject );
}

// ------------------------------------------------------------------------- //
//  * CreateSemaphore( void )
// ------------------------------------------------------------------------- //
IDCLThreads::ISemaphore*
TDCLStdThreads::CreateSemaphore( void )
{
	return new TSemaphore();
}

// ------------------------------------------------------------------------- //
//  * TSemaphore( void )
// ------------------------------------------------------------------------- //
TDCLStdThreads::TSemaphore::TSemaphore( void ) : mCount( 1 )
{
}

// ------------------------------------------------------------------------- //
//  * ~TSemaphore( void )
// ------------------------------------------------------------------------- //
TDCLStdThreads::TSemaphore::~TSemaphore( void ) = default;

// ------------------------------------------------------------------------- //
//  * Acquire( void )
// ------------------------------------------------------------------------- //
void
TDCLStdThreads::TSemaphore::Acquire( void )
{
	std::unique_lock<std::mutex> lock( mMutex );
	mCondVar.wait( lock, [this]( void ) { return mCount > 0; } );
	--mCount;
}

// ------------------------------------------------------------------------- //
//  * Release( void )
// ------------------------------------------------------------------------- //
void
TDCLStdThreads::TSemaphore::Release( void )
{
	{
		std::lock_guard<std::mutex> lock( mMutex );
		++mCount;
	}
	mCondVar.notify_one();
}

// ------------------------------------------------------------------------- //
//  * TThread( TDCLThread* )
// ------------------------------------------------------------------------- //
TDCLStdThreads::TThread::TThread( TDCLThread* inThreadedObject )
    : IThread( inThreadedObject ), mState( kStopped ), mWakeCount( 0 )
{
}

// ------------------------------------------------------------------------- //
//  * ~TThread( void )
// ------------------------------------------------------------------------- //
TDCLStdThreads::TThread::~TThread( void )
{
	std::jthread theThread;
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( mThread.joinable() )
		{
			if ( mThread.get_id() == std::this_thread::get_id() )
			{
				mThread.detach();
			}
			else
			{
				theThread = std::move( mThread );
			}
		}
	}
	if ( theThread.joinable() )
	{
		theThread.join();
	}
}

// ------------------------------------------------------------------------- //
//  * Start( void )
// ------------------------------------------------------------------------- //
void
TDCLStdThreads::TThread::Start( void )
{
	std::unique_lock<std::mutex> lock( mMutex );

	if ( mState != kStopped )
	{
		throw DCLThreadActive;
	}

	if ( mThread.joinable() )
	{
		std::jthread oldThread = std::move( mThread );
		lock.unlock();
		oldThread.join();
		lock.lock();
		if ( mState != kStopped )
		{
			throw DCLThreadActive;
		}
	}

	// Start is permitted: reset the stop source before launching.
	PrepareThreadedObjectForStart();

	mWakeCount = 0;
	mState = kRunning;

	try
	{
		mThread = std::jthread(
		    [this]( std::stop_token )
		    {
			    GetThreadedObject()->DoRun();
			    std::lock_guard<std::mutex> runLock( mMutex );
			    mState = kStopped;
		    } );
	}
	catch ( const std::system_error& err )
	{
		mState = kStopped;
		throw DCLPlatformLimitReachedError( err.code().value() );
	}
}

// ------------------------------------------------------------------------- //
//  * Join( void )
// ------------------------------------------------------------------------- //
void
TDCLStdThreads::TThread::Join( void )
{
	std::jthread theThread;
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( !mThread.joinable() )
		{
			return;
		}
		if ( mThread.get_id() == std::this_thread::get_id() )
		{
			throw DCLThreadDeadLock;
		}
		theThread = std::move( mThread );
	}
	theThread.join();
}

// ------------------------------------------------------------------------- //
//  * Sleep( KUInt32 )
// ------------------------------------------------------------------------- //
Boolean
TDCLStdThreads::TThread::Sleep( KUInt32 inMilliseconds )
{
	std::stop_token theStopToken = GetThreadedObject()->GetStopToken();
	std::unique_lock<std::mutex> lock( mMutex );

	if ( mWakeCount > 0 )
	{
		--mWakeCount;
		return false;
	}

	if ( theStopToken.stop_requested() )
	{
		return false;
	}

	struct TStateResetGuard
	{
		EState& mState;
		~TStateResetGuard( void )
		{
			mState = kRunning;
		}
	};

	mState = kSleeping;
	TStateResetGuard theGuard{ mState };

	Boolean theResult = false;
	if ( inMilliseconds == kForever )
	{
		mSleepCond.wait( lock, theStopToken, [this]( void ) { return mWakeCount > 0; } );
		if ( mWakeCount > 0 )
		{
			--mWakeCount;
		}
		theResult = false;
	}
	else
	{
		bool wokeByWakeUp = mSleepCond.wait_for( lock, theStopToken, std::chrono::milliseconds( inMilliseconds ),
		                                         [this]( void ) { return mWakeCount > 0; } );
		if ( mWakeCount > 0 )
		{
			--mWakeCount;
			theResult = false;
		}
		else if ( theStopToken.stop_requested() )
		{
			theResult = false;
		}
		else
		{
			theResult = !wokeByWakeUp;
		}
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * WakeUp( void )
// ------------------------------------------------------------------------- //
void
TDCLStdThreads::TThread::WakeUp( void )
{
	{
		std::lock_guard<std::mutex> lock( mMutex );
		++mWakeCount;
	}
	mSleepCond.notify_one();
}

// ------------------------------------------------------------------------- //
//  * GetState( void )
// ------------------------------------------------------------------------- //
IDCLThreads::EState
TDCLStdThreads::TThread::GetState( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	return mState;
}
