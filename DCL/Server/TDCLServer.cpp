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
#include <DCL/Interfaces/TDCLEvent.h>
#include <DCL/Interfaces/TDCLApplication.h>

// ------------------------------------------------------------------------- //
//  * TDCLServer( TDCLApplication* )
// ------------------------------------------------------------------------- //
TDCLServer::TDCLServer( TDCLApplication* inApplication )
	:
		TDCLThread( inApplication->GetThreadsIntf() ),
		mApplication( inApplication ),
		mStateMutex( nil ),
		mState( kStopped ),
		mHasStarted( false )
{
	mStateMutex = GetThreadsIntf()->CreateSemaphore();
}

// ------------------------------------------------------------------------- //
//  * ~TDCLServer( void )
// ------------------------------------------------------------------------- //
TDCLServer::~TDCLServer( void )
{	
	if (mStateMutex)
	{
		delete mStateMutex;
	}
}

// ------------------------------------------------------------------------- //
//  * Start( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Start( void )
{
	mStateMutex->Acquire();
	
	if (mState == kStopped)
	{
		mState = kStarting;

		try {
			TDCLThread::Start();
			mHasStarted = true;
		} catch ( ... ) {
			// The state is still kStopped.
			mState = kStopped;
			mStateMutex->Release();

			throw;	// Rethrow.
		}
	} // else: do nothing
	
	mStateMutex->Release();
}

// ------------------------------------------------------------------------- //
//  * Stop( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Stop( void )
{
	mStateMutex->Acquire();
	
	if ((mState == kRunning) || (mState == kStarting))
	{
		PostEvent(
			new TDCLEvent( TDCLEvent::kServerEvent, TDCLEvent::kStopServer ) );
	} // else: do nothing
	
	mStateMutex->Release();
}

// ------------------------------------------------------------------------- //
//  * Kill( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::Kill( void )
{
	mStateMutex->Acquire();
	
	if (mState != kStopped)
	{
		try {
			DoKill();
			
			TDCLThread::Stop();
		} catch ( ... ) {
			// The state is now unknown
			mState = kUnknown;
			
			mStateMutex->Release();

			throw;	// Rethrow.
		}

		mState = kStopped;

	} // else: do nothing
	
	mStateMutex->Release();
}

// ------------------------------------------------------------------------- //
//  * GetState( void )
// ------------------------------------------------------------------------- //
TDCLServer::EState
TDCLServer::GetState( void )
{
	EState theResult;
	
	mStateMutex->Acquire();
	
	theResult = mState;
	
	mStateMutex->Release();
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * RequestPresent( TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLServer::RequestPresent( TDCLCommLayer* inCommLayer )
{
	PostEvent( new TDCLEvent(
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
	PostEvent( new TDCLEvent(
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
	PostEvent( new TDCLEvent(
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
	// Server main loop.
	Boolean keepLooping = true;
	while( keepLooping )
	{
		EState theState;
		EState theNextState;

		mStateMutex->Acquire();

		theState = mState;
		theNextState = theState;
	
		mStateMutex->Release();
		
		switch ( theState )
		{
			case kStarting:
				// Tell subclasses to start listening.
				DoStartListening();
				
				// Next step is to actually wait.
				theNextState = kRunning;
				break;

			case kRunning:
				// Wait for an event.
				{
					TSmartPtr< TDCLEvent > theNextEvent;
					theNextEvent = WaitNextEvent(
										TDCLEvent::kServerEventMask,
										IDCLThreads::IThread::kForever );
					
					if (theNextEvent.Get() == nil)
					{
						// Slept the entire time.
						DoIdle();
					} else {
						Boolean processed = false;
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
			mStateMutex->Acquire();

			// Verify that the state did not change behind our back.
			if (mState == theState)
			{
				// Update
				mState = theNextState;
			}
	
			mStateMutex->Release();
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
//  * ProcessEvent( TDCLEvent*, Boolean* )
// ------------------------------------------------------------------------- //
TDCLServer::EState
TDCLServer::ProcessEvent( TDCLEvent* inEvent, Boolean* outProcessed )
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
	mStateMutex->Acquire();

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
	
	mStateMutex->Release();

	mApplication->HandleServerException( this, inException );
}

// ------------------------------------------------------------------------- //
//  * AcquireConfigurationLock( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::AcquireConfigurationLock( void )
{
	mStateMutex->Acquire();

	if (mHasStarted || (mState != kStopped))
	{
		mStateMutex->Release();
		throw DCLBadStateError;
	}
}

// ------------------------------------------------------------------------- //
//  * ReleaseConfigurationLock( void )
// ------------------------------------------------------------------------- //
void
TDCLServer::ReleaseConfigurationLock( void )
{
	mStateMutex->Release();
}

// ================================================ //
// You will have a head crash on your private pack. //
// ================================================ //
