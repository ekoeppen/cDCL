// ==============================
// File:			TDCLServer.cp
// Project:			Desktop Connection Library
//
// Created:			14/1/2002
// Tab width:		4 spaces
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
// The Original Code is TDCLServer.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLServer.cp,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

// K
#include <K/Misc/TSmartPtr.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Interfaces/TDCLEvent.h>
#include <DCL/Interfaces/TDCLApplication.h>

// ------------------------------------------------------------------------- //
//  * TDCLServer( TDCLApplication* )
// ------------------------------------------------------------------------- //
TDCLServer::TDCLServer( TDCLApplication* inApplication )
	:
		mApplication( inApplication ),
		mThreadId(),
		mKillThreadId(),
		mState( kStopped ),
		mHasStarted( false )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLServer( void )
// ------------------------------------------------------------------------- //
TDCLServer::~TDCLServer( void )
{
	std::stop_source theStopSource;
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		theStopSource = mThread.get_stop_source();
	}
	(void) theStopSource.request_stop();
	(void) Wait();
}

void
TDCLServer::ThreadEntry( std::stop_token inStopToken )
{
	mThreadId.store( std::this_thread::get_id() );
	mWorkerStopToken = std::move( inStopToken );
	try {
		Run();
	} catch ( TDCLException& inException ) {
		try {
			HandleException( &inException );
		} catch ( ... ) {
		}
	} catch ( ... ) {
		try {
			HandleException( nullptr );
		} catch ( ... ) {
		}
	}
	mWorkerStopToken = std::stop_token();
	mThreadId.store( std::thread::id() );
}

bool
TDCLServer::Wait( void )
{
	if ( std::this_thread::get_id() == mThreadId.load() )
	{
		return false;
	}

	std::lock_guard<std::mutex> joinLock( mJoinMutex );
	if ( mThread.joinable() )
	{
		mThread.join();
	}
	return true;
}

// ------------------------------------------------------------------------- //
//  * Start( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Start( void )
{
	if ( IsShutdownWorkerThread() )
	{
		throw DCLThreadActive;
	}

	std::unique_lock<std::mutex> lock( mStateMutex );
	for ( ;; )
	{
		mKillCV.wait( lock, [ this ] { return mKillThreadId == std::thread::id(); } );

		if ( mState != kStopped )
		{
			return;
		}

		lock.unlock();

		std::unique_lock<std::mutex> joinLock( mJoinMutex );
		lock.lock();
		if ( mKillThreadId != std::thread::id() )
		{
			joinLock.unlock();
			continue;
		}

		if ( mState != kStopped )
		{
			return;
		}

		if ( mThread.joinable() )
		{
			lock.unlock();
			mThread.join();
			lock.lock();

			if ( mKillThreadId != std::thread::id() )
			{
				joinLock.unlock();
				continue;
			}
		}

		if ( mState == kStopped )
		{
			if ( mThreadId.load() != std::thread::id() )
			{
				throw DCLThreadActive;
			}

			mState = kStarting;
			mEventQueue.Clear();

			try {
				mThread = std::jthread(
					[ this ]( std::stop_token inStopToken )
					{
						ThreadEntry( std::move( inStopToken ) );
					} );
				mHasStarted = true;
			} catch ( ... ) {
				mState = kStopped;
				throw;
			}
		}
		return;
	}
}

// ------------------------------------------------------------------------- //
//  * Stop( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Stop( void )
{
	std::lock_guard<std::mutex> lock( mStateMutex );

	if ((mState == kRunning) || (mState == kStarting))
	{
		mEventQueue.PostEvent(
			new TDCLEvent( TDCLEvent::kServerEvent, TDCLEvent::kStopServer ) );
	} // else: do nothing
}

// ------------------------------------------------------------------------- //
//  * Kill( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Kill( void )
{
	std::unique_lock<std::mutex> lock( mStateMutex );
	if ( !IsShutdownWorkerThread() )
	{
		mKillCV.wait( lock, [ this ] { return mKillThreadId == std::thread::id(); } );
	}

	if (mState != kStopped)
	{
		mState = kStopped;
		mKillThreadId = std::this_thread::get_id();
		std::stop_source theStopSource = mThread.get_stop_source();
		lock.unlock();

		try {
			(void) theStopSource.request_stop();
			DoRequestStop();
			(void) Wait();
			DoKill();
		} catch ( ... ) {
			std::lock_guard<std::mutex> errLock( mStateMutex );
			mKillThreadId = std::thread::id();
			mState = kUnknown;
			mKillCV.notify_all();
			throw;	// Rethrow.
		}

		std::lock_guard<std::mutex> doneLock( mStateMutex );
		mKillThreadId = std::thread::id();
		mKillCV.notify_all();
	} else {
		lock.unlock();
		(void) Wait();
	}
}

// ------------------------------------------------------------------------- //
//  * DoRequestStop( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::DoRequestStop( void )
{
}

// ------------------------------------------------------------------------- //
//  * IsShutdownWorkerThread( void ) const
// ------------------------------------------------------------------------- //
bool
TDCLServer::IsShutdownWorkerThread( void ) const
{
	return std::this_thread::get_id() == mThreadId.load();
}

// ------------------------------------------------------------------------- //
//  * GetState( void )
// ------------------------------------------------------------------------- //
TDCLServer::EState
TDCLServer::GetState( void )
{
	std::lock_guard<std::mutex> lock( mStateMutex );
	return mState;
}

// ------------------------------------------------------------------------- //
//  * RequestPresent( TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLServer::RequestPresent( TDCLCommLayer* inCommLayer )
{
	mEventQueue.PostEvent( new TDCLEvent(
						TDCLEvent::kServerEvent,
						TDCLEvent::kIncomingRequest,
						inCommLayer ) );
}

// ------------------------------------------------------------------------- //
//  * WaitingConnection( TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLServer::WaitingConnection( TDCLCommLayer* inCommLayer )
{
	mEventQueue.PostEvent( new TDCLEvent(
						 TDCLEvent::kServerEvent,
						 TDCLEvent::kWaitingConnection,
						 inCommLayer ) );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLServer::Disconnected( TDCLLink* inLink )
{
	mEventQueue.PostEvent( new TDCLEvent(
						TDCLEvent::kServerEvent,
						TDCLEvent::kDisconnected,
						inLink ) );
}

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Run( void )
{
	const std::stop_token theStopToken = mWorkerStopToken;

	// Server main loop.
	bool keepLooping = true;
	while( keepLooping )
	{
		EState theState;
		EState theNextState;

		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			theState = mState;
		}
		theNextState = theState;
		
		switch ( theState )
		{
			case kStarting:
				// Tell subclasses to start listening.
				DoStartListening();
				
				// Next step is to actually wait.
				theNextState = theStopToken.stop_requested() ? kStopped : kRunning;
				break;

			case kRunning:
				if (theStopToken.stop_requested())
				{
					theNextState = kStopped;
				} else {
					// Wait for an event.
					TSmartPtr< TDCLEvent > theNextEvent;
					theNextEvent = mEventQueue.WaitNextEvent(
										TDCLEvent::kServerEventMask,
										TDCLEventQueue::kForever,
										theStopToken );
					
					if (theStopToken.stop_requested())
					{
						theNextState = kStopped;
					} else if (theNextEvent.Get() == nullptr) {
						// Slept the entire time.
						DoIdle();
					} else {
						// Process the event.
						bool processed = false;
						theNextState = ProcessEvent(
										theNextEvent.Get(), &processed );
						
						if (!processed)
						{
							throw DCLBadParamError;
						}
					}
				}
				break;

			case kStopping:
				DoStop();
			
			case kStopped:	
			case kUnknown:	
				theNextState = kStopped;
				keepLooping = false;
				break;
		}
		
		if (theState != theNextState)
		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			if (mState == theState)
			{
				mState = theNextState;
			}
		}
	}

	// Notify the application that the server is down.
	GetApplication()->ServerIsDown( this );
}

// ------------------------------------------------------------------------- //
//  * HandleCommLayerException( TDCLCommLayer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLServer::HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException )
{
	mApplication->HandleCommLayerException( inCommLayer, inException );
}

// ------------------------------------------------------------------------- //
//  * ProcessEvent( TDCLEvent*, bool* )
// ------------------------------------------------------------------------- //
TDCLServer::EState
TDCLServer::ProcessEvent( TDCLEvent* inEvent, bool* outProcessed )
{
	EState theNextState = kRunning;
	
	// What event did we receive?
	TDCLEvent::EEventID theEventID = inEvent->GetEventID();

	switch( theEventID )
	{
		case TDCLEvent::kWaitingConnection:
			// Waiting for the Newton.
			GetApplication()->WaitConnection(
					this,
					(TDCLCommLayer*) (inEvent->GetEventData()) );
			*outProcessed = true;
			break;

		case TDCLEvent::kIncomingRequest:
			// A connection has just arrived.
			theNextState = HandleIncomingConnection(
						(TDCLCommLayer*) (inEvent->GetEventData()) );
			*outProcessed = true;
			break;

		case TDCLEvent::kDisconnected:
		{
			// A link was disconnected.
			TDCLLink* theLink = (TDCLLink*) (inEvent->GetEventData());
			theNextState = HandleLinkDisconnection( theLink );
			
			// Notify the application that the link is disconnected.
			mApplication->Disconnected( theLink );
			*outProcessed = true;
		}
		break;

		case TDCLEvent::kStopServer:
			// Need to shut down.
			theNextState = kStopping;
			*outProcessed = true;
			break;
		
		default:
			*outProcessed = false;
	}
	
	return theNextState;
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLServer::HandleException( TDCLException* inException )
{
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		switch ( mState )
		{
			case kStopped:
			case kStarting:
			case kStopping:
				mState = kStopped;
				break;

			case kRunning:
			default:
				mState = kUnknown;
				break;
		}
	}

	mApplication->HandleServerException( this, inException );
}

// ------------------------------------------------------------------------- //
//  * AcquireConfigurationLock( void )
// ------------------------------------------------------------------------- //
std::unique_lock<std::mutex>
TDCLServer::AcquireConfigurationLock( void )
{
	std::unique_lock<std::mutex> lock( mStateMutex );

	if ( mHasStarted
		|| ( mState != kStopped )
		|| ( mKillThreadId != std::thread::id() )
		|| ( mThreadId.load() != std::thread::id() )
		|| mThread.joinable() )
	{
		throw DCLBadStateError;
	}

	return lock;
}

// ================================================ //
// You will have a head crash on your private pack. //
// ================================================ //
