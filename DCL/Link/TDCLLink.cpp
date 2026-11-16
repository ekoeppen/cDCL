// ==============================
// File:			TDCLLink.cpp
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
// The Original Code is TDCLLink.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLink.h>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C++
#include <algorithm>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>

// K
#include <K/Misc/TSmartPtr.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TMNPProtocol.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadAppCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Server/TDCLServer.h>

#undef KERROR_ENABLED
#define KERROR_ENABLED 0
#undef KTRACE_ENABLED
#define KTRACE_ENABLED 0
#undef KDEBUG_ENABLED
#define KDEBUG_ENABLED 0

namespace
{
constexpr std::chrono::milliseconds kMNPStallTimeout{ 6000 };
} // namespace

TDCLLink::TDCLLink(
	TDCLApplication* inApplication,
	KUInt32 inType /* = TDCLDockCommand::kNone_SType */,
	KUInt32 inIdlePeriod /* = kDefaultIdlePeriod */ )
	: mServer( nil ),
	  mCommLayer( nil ),
	  mApplication( inApplication ),
	  mSessionBus(),
	  mSessionId(),
	  mUpstreamGuard(),
	  mUseMNP( false ),
	  mMNPKeepAlive( false ),
	  mPeerDisconnected( false ),
	  mCloseOnExit( false ),
	  mTimeout( TDCLCommLayer::kDefaultTimeout ),
	  mMNPStallTimeout( kMNPStallTimeout ),
	  mMNPStallActive( false ),
	  mMNPStallProgress( 0 ),
	  mMNPStallDeadline(),
	  mMNPProtocol(),
	  mCommandFramer( TDCLCommandFramer::EMode::kDock ),
	  mOutputStream( [this]( std::vector<std::uint8_t>&& inChunk ) { SendRawBytes( std::move( inChunk ) ); } ),
	  mThreadId(),
	  mStopSource(),
	  mState( kDisconnected ),
	  mType( inType ),
	  mIdlePeriod( inIdlePeriod ),
	  mIsBusy( false ),
	  mNewtonName( nil ),
	  mNewtonID( 0 ),
	  mManufacturer( 0 ),
	  mMachineType( 0 ),
	  mROMVersion( 0 ),
	  mROMStage( 0 ),
	  mRAMSize( 0 ),
	  mScreenHeight( 0 ),
	  mScreenWidth( 0 ),
	  mSystemUpdate( 0 ),
	  mNOSVersion( 0 ),
	  mInternalStoreSig( 0 ),
	  mVertScreenRes( 0 ),
	  mHoriScreenRes( 0 ),
	  mScreenDepth( 0 )
{
}

TDCLLink::~TDCLLink( void )
{
	(void)RequestStop();
	(void)Wait();
	mUpstreamGuard.Reset();
	CloseSessionTransport();
	if ( mNewtonName )
	{
		::free( (void*)mNewtonName );
		mNewtonName = nil;
	}
}

void
TDCLLink::ThreadEntry( std::stop_token inStopToken )
{
	mThreadId.store( std::this_thread::get_id() );
	mWorkerStopToken = std::move( inStopToken );
	try
	{
		Run();
	}
	catch ( TDCLException& inException )
	{
		try
		{
			HandleException( &inException );
		}
		catch ( ... )
		{
		}
	}
	catch ( ... )
	{
		try
		{
			HandleException( nil );
		}
		catch ( ... )
		{
		}
	}
	mWorkerStopToken = std::stop_token();
	mThreadId.store( std::thread::id() );
}

bool
TDCLLink::RequestStop( void )
{
	return mStopSource.request_stop();
}

bool
TDCLLink::StopRequested( void ) const noexcept
{
	return mStopSource.stop_requested();
}

bool
TDCLLink::Wait( void )
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

Boolean
TDCLLink::Connect( TDCLSessionId inSessionId, TDCLServer* inServer, TDCLCommLayer* inCommLayer )
{
	mServer = inServer;
	mCommLayer = inCommLayer;
	auto bus = inServer ? inServer->GetSharedSessionBus() : nullptr;
	const bool useMNP = inCommLayer ? inCommLayer->UsesMNP() : false;
	const long timeout = inCommLayer ? inCommLayer->GetTimeout() : TDCLCommLayer::kDefaultTimeout;
	return Connect( inSessionId, std::move( bus ), useMNP, timeout );
}

Boolean
TDCLLink::Connect(
	TDCLSessionId inSessionId,
	std::shared_ptr<TDCLSessionBus> inBus,
	bool inUseMNP /* = false */,
	long inTimeout /* = TDCLCommLayer::kDefaultTimeout */ )
{
	Boolean theResult = false;

	{
		std::unique_lock<std::mutex> lock( mStateMutex );

		if ( mState == kDisconnected )
		{
			if ( std::this_thread::get_id() == mThreadId.load() )
			{
				throw DCLThreadActive;
			}

			lock.unlock();

			std::unique_lock<std::mutex> joinLock( mJoinMutex );
			lock.lock();

			if ( mState == kDisconnected && mThread.joinable() )
			{
				lock.unlock();
				mThread.join();
				lock.lock();
			}

			if ( mState == kDisconnected )
			{
				if ( mThreadId.load() != std::thread::id() )
				{
					throw DCLThreadActive;
				}

				theResult = true;
				mState = kConnecting;
				mSessionBus = std::move( inBus );
				mSessionId = inSessionId;
				mUseMNP = inUseMNP;
				mTimeout = inTimeout;
				mPeerDisconnected = false;
				mCloseOnExit = false;
				mCommandFramer.Reset();
				mEventQueue.Clear();

				ResetMNPStallTracking();
				if ( mUseMNP )
				{
					mMNPProtocol = std::make_unique<TMNPProtocol>();
					mMNPProtocol->SetKeepAlive( mMNPKeepAlive );
					mMNPProtocol->Start();
				}
				else
				{
					mMNPProtocol.reset();
				}

				if ( mSessionBus && mSessionId )
				{
					mUpstreamGuard = mSessionBus->BindUpstream( mSessionId, &mEventQueue );
				}

				mStopSource = std::stop_source{};
				const std::stop_token token = mStopSource.get_token();

				try
				{
					mThread = std::jthread(
					    [this, token]( std::stop_token ) { ThreadEntry( token ); } );
				}
				catch ( ... )
				{
					mState = kDisconnected;
					mUpstreamGuard.Reset();
					CloseSessionTransport();
					mServer = nil;
					mCommLayer = nil;
					throw;
				}
			}
		}
	}

	if ( !theResult && inBus && inSessionId )
	{
		inBus->CloseSession( inSessionId );
	}

	return theResult;
}

Boolean
TDCLLink::Disconnect( void )
{
	Boolean theResult = false;
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		if ( ( mState != kDisconnected ) && ( mState != kDisconnecting ) )
		{
			theResult = true;
		}
	}
	if ( theResult )
	{
		PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kDisconnectLink ) );
	}
	return theResult;
}

Boolean
TDCLLink::PostAppCommand( TDCLAppCommand* inCommand )
{
	Boolean theResult = false;
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		if ( ( mState != kDisconnected ) && ( mState != kDisconnecting ) )
		{
			theResult = true;
		}
	}
	if ( theResult )
	{
		PostEvent( inCommand );
	}
	return theResult;
}

Boolean
TDCLLink::Close( void )
{
	Boolean theResult = IsConnected();
	if ( theResult )
	{
		PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kCloseLink ) );
	}
	return theResult;
}

Boolean
TDCLLink::IsConnected( void )
{
	std::lock_guard<std::mutex> lock( mStateMutex );
	return ( mState != kDisconnected );
}

void
TDCLLink::SetCommandMode( TDCLCommandFramer::EMode inMode ) noexcept
{
	mCommandFramer.SetMode( inMode );
}

void
TDCLLink::SetMNPKeepAlive( bool inKeepAlive ) noexcept
{
	mMNPKeepAlive = inKeepAlive;
	if ( mMNPProtocol )
	{
		mMNPProtocol->SetKeepAlive( inKeepAlive );
	}
}

void
TDCLLink::SetMNPStallTimeout( std::chrono::milliseconds inTimeout ) noexcept
{
	mMNPStallTimeout = ( inTimeout.count() > 0 ) ? inTimeout : std::chrono::milliseconds( 1 );
}

void
TDCLLink::ResetMNPStallTracking( void ) noexcept
{
	mMNPStallActive = false;
	mMNPStallProgress = 0;
	mMNPStallDeadline = std::chrono::steady_clock::time_point{};
}

void
TDCLLink::RefreshMNPStallDeadline( void ) noexcept
{
	if ( !mUseMNP || !mMNPProtocol || !mMNPProtocol->IsStallable() )
	{
		mMNPStallActive = false;
		return;
	}
	const std::uint32_t progress = mMNPProtocol->GetProgress();
	if ( !mMNPStallActive || progress != mMNPStallProgress )
	{
		mMNPStallActive = true;
		mMNPStallProgress = progress;
		mMNPStallDeadline = std::chrono::steady_clock::now() + mMNPStallTimeout;
	}
}

void
TDCLLink::HandleMNPStall( void )
{
	if ( mUseMNP && mMNPProtocol && mMNPProtocol->IsStallable() )
	{
		mMNPProtocol->OnStall();
		FlushMNPOutbound();
		RefreshMNPStallDeadline();
		if ( mMNPProtocol->GetState() == TMNPProtocol::kFailed )
		{
			throw DCLMNPProtocolError( mMNPProtocol->GetLocalReason() );
		}
		if ( mMNPProtocol->PeerDisconnected() || mMNPProtocol->IsTerminal() )
		{
			mPeerDisconnected = true;
		}
	}
}

void
TDCLLink::SendCommand(
	TDCLDockCommand* inCommand,
	ProgressFuncPtr inProgressFuncPtr /* = nil */,
	void* inRefCon /* = nil */ )
{
	if ( inCommand )
	{
		inCommand->SendCommand( &mOutputStream, inProgressFuncPtr, inRefCon );
	}
}

void
TDCLLink::FlushOutput( void )
{
	mOutputStream.FlushOutput();
}

void
TDCLLink::SendRawBytes( std::vector<std::uint8_t>&& inBytes )
{
	if ( inBytes.empty() )
	{
		return;
	}
	if ( mUseMNP && mMNPProtocol )
	{
		mMNPProtocol->QueueOutboundBytes( inBytes.data(), inBytes.size() );
		FlushMNPOutbound();
	}
	else if ( mSessionBus && mSessionId )
	{
		(void)mSessionBus->SendBytes( mSessionId, std::move( inBytes ) );
	}
}

void
TDCLLink::FlushMNPOutbound( void )
{
	if ( !mMNPProtocol || !mSessionBus || !mSessionId )
	{
		return;
	}
	auto wire = mMNPProtocol->TakeAllOutboundWireBytes();
	if ( !wire.empty() )
	{
		(void)mSessionBus->SendBytes( mSessionId, std::move( wire ) );
	}
}

void
TDCLLink::ProcessInboundBytes( const std::vector<std::uint8_t>& inBytes )
{
	if ( inBytes.empty() )
	{
		return;
	}

	if ( mUseMNP && mMNPProtocol )
	{
		mMNPProtocol->PushWireBytes( inBytes.data(), inBytes.size() );
		FlushMNPOutbound();

		auto payload = mMNPProtocol->TakeAllInput();
		if ( !payload.empty() )
		{
			(void)mCommandFramer.Feed( payload.data(), payload.size() );
		}

		if ( mMNPProtocol->GetState() == TMNPProtocol::kFailed )
		{
			throw DCLMNPProtocolError( mMNPProtocol->GetLocalReason() );
		}
		if ( mMNPProtocol->PeerDisconnected() )
		{
			mPeerDisconnected = true;
		}
	}
	else
	{
		(void)mCommandFramer.Feed( inBytes.data(), inBytes.size() );
	}
}

void
TDCLLink::EnsureMNPConnected( void )
{
	if ( !mUseMNP || !mMNPProtocol )
	{
		return;
	}

	using Clock = std::chrono::steady_clock;
	std::uint32_t seenProgress = mMNPProtocol->GetProgress();
	Clock::time_point deadline = Clock::now() + mMNPStallTimeout;

	while ( !mMNPProtocol->IsConnected() && !mMNPProtocol->IsTerminal() )
	{
		if ( mWorkerStopToken.stop_requested() )
		{
			throw DCLUserCancelled;
		}
		if ( mMNPProtocol->GetProgress() != seenProgress )
		{
			seenProgress = mMNPProtocol->GetProgress();
			deadline = Clock::now() + mMNPStallTimeout;
		}

		const auto now = Clock::now();
		if ( now >= deadline )
		{
			if ( mMNPProtocol->IsStallable() )
			{
				mMNPProtocol->OnStall();
				FlushMNPOutbound();
				continue;
			}
			break;
		}

		const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>( deadline - now );
		std::unique_ptr<TDCLEvent> ev(
		    mEventQueue.WaitNextEvent( TDCLEvent::kLinkEvent, remaining, mWorkerStopToken ) );

		if ( mWorkerStopToken.stop_requested() )
		{
			throw DCLUserCancelled;
		}
		if ( !ev )
		{
			continue;
		}

		switch ( ev->GetEventID() )
		{
		case TDCLEvent::kBytesReceived:
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( ev.get() );
			ProcessInboundBytes( typed->GetPayload().bytes );
			break;
		}
		case TDCLEvent::kSessionClosed:
			mPeerDisconnected = true;
			throw DCLEOF;
		case TDCLEvent::kTransportError:
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLTransportErrorPayload>*>( ev.get() );
			if ( typed->GetPayload().error )
			{
				std::rethrow_exception( typed->GetPayload().error );
			}
			throw DCLEOF;
		}
		case TDCLEvent::kCloseLink:
			mCloseOnExit = true;
			throw DCLEOF;
		case TDCLEvent::kDisconnectLink:
			throw DCLEOF;
		default:
			break;
		}
	}

	if ( mMNPProtocol->GetState() == TMNPProtocol::kFailed )
	{
		throw DCLMNPProtocolError( mMNPProtocol->GetLocalReason() );
	}
	if ( mMNPProtocol->PeerDisconnected() || mMNPProtocol->IsTerminal() )
	{
		mPeerDisconnected = true;
		throw DCLEOF;
	}
}

void
TDCLLink::DrainMNPAndDisconnect( void )
{
	if ( !mUseMNP || !mMNPProtocol || mMNPProtocol->IsTerminal() || mPeerDisconnected )
	{
		return;
	}

	using Clock = std::chrono::steady_clock;
	std::uint32_t seenProgress = mMNPProtocol->GetProgress();
	Clock::time_point deadline = Clock::now() + mMNPStallTimeout;

	// First flush any pending queued LTs before sending LD.
	while ( mMNPProtocol->HasPendingOutbound() && !mMNPProtocol->IsTerminal() && !mPeerDisconnected )
	{
		if ( mWorkerStopToken.stop_requested() )
		{
			return;
		}
		if ( mMNPProtocol->GetProgress() != seenProgress )
		{
			seenProgress = mMNPProtocol->GetProgress();
			deadline = Clock::now() + mMNPStallTimeout;
		}
		const auto now = Clock::now();
		if ( now >= deadline )
		{
			if ( mMNPProtocol->IsStallable() )
			{
				mMNPProtocol->OnStall();
				FlushMNPOutbound();
			}
			break;
		}
		const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>( deadline - now );
		std::unique_ptr<TDCLEvent> ev(
		    mEventQueue.WaitNextEvent( TDCLEvent::kLinkEvent, remaining, mWorkerStopToken ) );
		if ( mWorkerStopToken.stop_requested() )
		{
			return;
		}
		if ( !ev )
		{
			continue;
		}
		if ( ev->GetEventID() == TDCLEvent::kBytesReceived )
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( ev.get() );
			ProcessInboundBytes( typed->GetPayload().bytes );
		}
		else if ( ev->GetEventID() == TDCLEvent::kSessionClosed || ev->GetEventID() == TDCLEvent::kCloseLink )
		{
			mPeerDisconnected = true;
			return;
		}
	}

	mMNPProtocol->RequestDisconnect();
	FlushMNPOutbound();

	deadline = Clock::now() + mMNPStallTimeout;
	seenProgress = mMNPProtocol->GetProgress();
	while ( mMNPProtocol->IsDisconnecting() && !mMNPProtocol->IsTerminal() && !mPeerDisconnected )
	{
		if ( mWorkerStopToken.stop_requested() )
		{
			return;
		}
		if ( mMNPProtocol->GetProgress() != seenProgress )
		{
			seenProgress = mMNPProtocol->GetProgress();
			deadline = Clock::now() + mMNPStallTimeout;
		}
		const auto now = Clock::now();
		if ( now >= deadline )
		{
			if ( mMNPProtocol->IsStallable() )
			{
				mMNPProtocol->OnStall();
				FlushMNPOutbound();
			}
			continue;
		}
		const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>( deadline - now );
		std::unique_ptr<TDCLEvent> ev(
		    mEventQueue.WaitNextEvent( TDCLEvent::kLinkEvent, remaining, mWorkerStopToken ) );
		if ( mWorkerStopToken.stop_requested() )
		{
			return;
		}
		if ( !ev )
		{
			continue;
		}
		if ( ev->GetEventID() == TDCLEvent::kBytesReceived )
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( ev.get() );
			ProcessInboundBytes( typed->GetPayload().bytes );
		}
		else if ( ev->GetEventID() == TDCLEvent::kSessionClosed || ev->GetEventID() == TDCLEvent::kCloseLink )
		{
			mPeerDisconnected = true;
			return;
		}
	}
}

void
TDCLLink::CloseSessionTransport( void ) noexcept
{
	if ( mSessionBus && mSessionId )
	{
		mSessionBus->CloseSession( mSessionId );
		mSessionId = TDCLSessionId{};
	}
}

void
TDCLLink::NotifyConnected( void )
{
	GetApplication()->Connected( this, mCommLayer );
}

void
TDCLLink::Disconnected( void )
{
	mUpstreamGuard.Reset();
	CloseSessionTransport();

	if ( mServer )
	{
		mServer->Disconnected( this );
	}

	if ( mNewtonName )
	{
		::free( (void*)mNewtonName );
		mNewtonName = nil;
	}
}

void
TDCLLink::Connected( void )
{
	mApplication->ConnectedToNewtonDevice( this, mNewtonName );
}

void
TDCLLink::DoConnect( void )
{
	NotifyConnected();

	if ( StopRequested() )
	{
		throw DCLUserCancelled;
	}

	// Read kDRequestToDock
	{
		auto theNewtMessage = ReceiveCommand();
		if ( theNewtMessage->GetCommand() != TDCLDockCommand::kDRequestToDock )
		{
			throw DCLUnexpDockCmd;
		}
	}

	// Send kDInitiateDocking
	{
		TDCLDockCmdSingleLong theInitiateDockingMessage( TDCLDockCommand::kDInitiateDocking, mType );
		SendCommand( &theInitiateDockingMessage );
	}

	// Read kDNewtonName
	{
		auto theNewtMessage = ReceiveCommand();
		if ( theNewtMessage->GetCommand() != TDCLDockCommand::kDNewtonName )
		{
			throw DCLUnexpDockCmd;
		}

		auto* theNewtonName = static_cast<TDCLDockCmdNewtonName*>( theNewtMessage.get() );
		mNewtonID = theNewtonName->GetNewtonUniqueID();
		mManufacturer = theNewtonName->GetManufacturer();
		mMachineType = theNewtonName->GetMachineType();
		mROMVersion = theNewtonName->GetROMVersion();
		mROMStage = theNewtonName->GetROMStage();
		mRAMSize = theNewtonName->GetRAMSize();
		mScreenHeight = theNewtonName->GetScreenHeight();
		mScreenWidth = theNewtonName->GetScreenWidth();
		mSystemUpdate = theNewtonName->GetSystemUpdate();
		mNOSVersion = theNewtonName->GetNOSVersion();
		mInternalStoreSig = theNewtonName->GetInternalStoreSig();
		mVertScreenRes = theNewtonName->GetVertScreenRes();
		mHoriScreenRes = theNewtonName->GetHoriScreenRes();
		mScreenDepth = theNewtonName->GetScreenDepth();

		KUInt32 theSize = ( theNewtonName->GetNameLength() + 1 ) * sizeof( KUInt16 );
		KUInt16* copiedName = static_cast<KUInt16*>( ::malloc( theSize ) );
		if ( copiedName == nil )
		{
			throw DCLMemError;
		}
		(void)::memcpy( copiedName, theNewtonName->GetNewtonName(), theSize );
		mNewtonName = copiedName;
	}
}

void
TDCLLink::DoDisconnect( void )
{
	mApplication->Disconnecting( this );

	if ( StopRequested() || mPeerDisconnected )
	{
		return;
	}

	{
		TDCLDockCmdNoData theDisconnectMessage( TDCLDockCommand::kDDisconnect );
		SendCommand( &theDisconnectMessage );
	}

	try
	{
		if ( mUseMNP && mMNPProtocol )
		{
			DrainMNPAndDisconnect();
		}
		else
		{
			// Consume any already-buffered result/hello commands.
			while ( mCommandFramer.HasCommand() )
			{
				auto cmd = mCommandFramer.TakeCommand();
				if ( cmd->GetCommand() == TDCLDockCommand::kDResult )
				{
					auto* res = static_cast<TDCLDockCmdSingleLong*>( cmd.get() );
					const KSInt32 err = static_cast<KSInt32>( res->GetLong() );
					if ( err )
					{
						throw DCLNewton( err );
					}
					break;
				}
				else if ( cmd->GetCommand() != TDCLDockCommand::kDHello )
				{
					throw DCLUnexpDockCmd;
				}
			}
		}
	}
	catch ( TDCLEOFException& )
	{
	}
}

void
TDCLLink::Run( void )
{
	const std::stop_token theStopToken = mWorkerStopToken;
	Boolean keepLooping = true;

	while ( keepLooping )
	{
		EState theState;
		EState theNextState;

		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			if ( theStopToken.stop_requested() )
			{
				mState = kDisconnected;
			}
			theState = mState;
		}
		theNextState = theState;

		try
		{
			switch ( theState )
			{
			case kConnecting:
				if ( mUseMNP )
				{
					EnsureMNPConnected();
				}
				DoConnect();
				theNextState = kConnected;
				break;

			case kConnected:
				Connected();
				theNextState = kRunning;
				break;

			case kRunning:
				theNextState = TransitionFromRunning();
				break;

			case kDisconnecting:
				if ( !theStopToken.stop_requested() && !mCloseOnExit )
				{
					DoDisconnect();
				}
				theNextState = kDisconnected;
				break;

			default: // kDisconnected, kUnknown
				Disconnected();
				keepLooping = false;
				theNextState = kDisconnected;
				break;
			}
		}
		catch ( TDCLEOFException& )
		{
			Disconnected();
			keepLooping = false;
			theNextState = kDisconnected;
		}
		catch ( TDCLUserCancelledException& )
		{
			Disconnected();
			keepLooping = false;
			theNextState = kDisconnected;
		}

		if ( theState != theNextState )
		{
			std::lock_guard<std::mutex> lock( mStateMutex );
			if ( mState == theState )
			{
				mState = theNextState;
			}
		}
	}
}

TDCLLink::EState
TDCLLink::TransitionFromRunning( void )
{
	if ( StopRequested() )
	{
		return kDisconnected;
	}

	auto dispatchCommand = [this]( TDCLDockCommand* inCmd ) -> EState {
		Boolean outProcessed = false;
		EState next = ProcessDockCommand( inCmd, &outProcessed );
		if ( !outProcessed )
		{
			TDCLDockCmdSingleLong unknownCmd( TDCLDockCommand::kDUnknownCommand, inCmd->GetCommand() );
			SendCommand( &unknownCmd );
		}
		return next;
	};

	// Drain any commands already buffered by the framer from an earlier chunk.
	while ( mCommandFramer.HasCommand() )
	{
		mIsBusy = true;
		try
		{
			auto cmd = mCommandFramer.TakeCommand();
			EState next = dispatchCommand( cmd.get() );
			mIsBusy = false;
			if ( next != kRunning )
			{
				return next;
			}
		}
		catch ( ... )
		{
			mIsBusy = false;
			throw;
		}
	}

	if ( mCommandFramer.HasError() )
	{
		std::rethrow_exception( mCommandFramer.GetError() );
	}
	if ( mPeerDisconnected || mCommandFramer.IsEOF() )
	{
		return kDisconnected;
	}
	if ( mUseMNP && mMNPProtocol && mMNPProtocol->IsTerminal() )
	{
		if ( mMNPProtocol->GetState() == TMNPProtocol::kFailed )
		{
			throw DCLMNPProtocolError( mMNPProtocol->GetLocalReason() );
		}
		return kDisconnected;
	}

	KUInt32 theEventMask = TDCLEvent::kLinkEvent | ( GetAppCommandMask() & TDCLEvent::kAnyAppCommandMask );
	std::chrono::milliseconds waitTimeout =
	    ( mIdlePeriod == 0 ) ? TDCLEventQueue::kForever : std::chrono::milliseconds( mIdlePeriod );

	using Clock = std::chrono::steady_clock;
	RefreshMNPStallDeadline();
	bool mnpStallBounded = false;
	if ( mMNPStallActive )
	{
		const auto now = Clock::now();
		if ( now >= mMNPStallDeadline )
		{
			HandleMNPStall();
			if ( mPeerDisconnected )
			{
				return kDisconnected;
			}
			RefreshMNPStallDeadline();
		}
		if ( mMNPStallActive )
		{
			const auto mnpRemaining =
			    std::chrono::duration_cast<std::chrono::milliseconds>( mMNPStallDeadline - Clock::now() );
			const auto clamped = ( mnpRemaining.count() > 0 ) ? mnpRemaining : std::chrono::milliseconds( 0 );
			if ( waitTimeout == TDCLEventQueue::kForever || clamped <= waitTimeout )
			{
				waitTimeout = clamped;
				mnpStallBounded = true;
			}
		}
	}

	std::unique_ptr<TDCLEvent> theNextEvent(
	    mEventQueue.WaitNextEvent( theEventMask, waitTimeout, mWorkerStopToken ) );

	if ( StopRequested() )
	{
		return kDisconnected;
	}

	if ( !theNextEvent )
	{
		if ( mnpStallBounded && mMNPStallActive && Clock::now() >= mMNPStallDeadline )
		{
			HandleMNPStall();
			if ( mPeerDisconnected )
			{
				return kDisconnected;
			}
		}
		return Idle();
	}

	EState theNextState = kRunning;
	mIsBusy = true;
	try
	{
		if ( theNextEvent->GetEventKind() & TDCLEvent::kAnyAppCommandMask )
		{
			auto* appCmd = static_cast<TDCLAppCommand*>( theNextEvent.get() );
			try
			{
				Boolean outProcessed = false;
				mApplication->AppCommandBeingProcessed( this, appCmd );
				theNextState = ProcessAppCommand( appCmd, &outProcessed );
				if ( !outProcessed )
				{
					throw DCLBadAppCmd;
				}
				mApplication->AppCommandProcessed( this, appCmd, nil );
			}
			catch ( TDCLException& inException )
			{
				mApplication->AppCommandProcessed( this, appCmd, &inException );
			}
		}
		else
		{
			switch ( theNextEvent->GetEventID() )
			{
			case TDCLEvent::kBytesReceived:
			{
				auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( theNextEvent.get() );
				ProcessInboundBytes( typed->GetPayload().bytes );
				while ( mCommandFramer.HasCommand() )
				{
					auto cmd = mCommandFramer.TakeCommand();
					theNextState = dispatchCommand( cmd.get() );
					if ( theNextState != kRunning )
					{
						break;
					}
				}
				if ( theNextState == kRunning && mCommandFramer.HasError() )
				{
					std::rethrow_exception( mCommandFramer.GetError() );
				}
				if ( theNextState == kRunning && ( mPeerDisconnected || mCommandFramer.IsEOF() ) )
				{
					theNextState = kDisconnected;
				}
				break;
			}
			case TDCLEvent::kSessionClosed:
				mPeerDisconnected = true;
				theNextState = kDisconnected;
				break;
			case TDCLEvent::kTransportError:
			{
				auto* typed = static_cast<TDCLPayloadEvent<TDCLTransportErrorPayload>*>( theNextEvent.get() );
				if ( typed->GetPayload().error )
				{
					std::rethrow_exception( typed->GetPayload().error );
				}
				theNextState = kDisconnected;
				break;
			}
			case TDCLEvent::kDisconnectLink:
				theNextState = kDisconnecting;
				break;
			case TDCLEvent::kCloseLink:
				theNextState = kDisconnected;
				break;
			default:
				break;
			}
		}
	}
	catch ( ... )
	{
		mIsBusy = false;
		throw;
	}

	mIsBusy = false;
	return theNextState;
}

void
TDCLLink::HandleException( TDCLException* inException )
{
	EState theFinalState;
	{
		std::lock_guard<std::mutex> lock( mStateMutex );
		switch ( mState )
		{
		case kDisconnected:
		case kConnecting:
		case kDisconnecting:
			mState = kDisconnecting;
			theFinalState = kDisconnected;
			break;
		case kConnected:
		case kRunning:
		default:
			mState = kUnknown;
			theFinalState = kUnknown;
			break;
		}
	}

	auto cleanup = [this, theFinalState]() {
		mUpstreamGuard.Reset();
		CloseSessionTransport();
		if ( mNewtonName )
		{
			::free( (void*)mNewtonName );
			mNewtonName = nil;
		}
		std::lock_guard<std::mutex> lock( mStateMutex );
		if ( mState == kDisconnecting )
		{
			mState = theFinalState;
		}
	};

	try
	{
		mApplication->HandleLinkException( this, inException );
	}
	catch ( ... )
	{
		cleanup();
		throw;
	}

	cleanup();
}

TDCLLink::EState
TDCLLink::Idle( void )
{
	return kRunning;
}

TDCLLink::EState
TDCLLink::ProcessAppCommand( TDCLAppCommand* /* inCommand */, Boolean* outProcessed )
{
	*outProcessed = false;
	return kRunning;
}

KUInt32
TDCLLink::GetAppCommandMask( void )
{
	return static_cast<KUInt32>( TDCLEvent::kAnyAppCommandMask );
}

std::unique_ptr<TDCLDockCommand>
TDCLLink::ReceiveCommand( void )
{
	using Clock = std::chrono::steady_clock;
	const bool hasOverallTimeout = ( mTimeout > 0 );
	const Clock::time_point overallDeadline =
	    hasOverallTimeout ? ( Clock::now() + std::chrono::seconds( mTimeout ) ) : Clock::time_point{};

	for ( ;; )
	{
		if ( auto cmd = mCommandFramer.TakeCommand() )
		{
			return cmd;
		}
		if ( mCommandFramer.HasError() )
		{
			std::rethrow_exception( mCommandFramer.GetError() );
		}
		if ( mPeerDisconnected || mCommandFramer.IsEOF() )
		{
			throw DCLEOF;
		}
		if ( mWorkerStopToken.stop_requested() )
		{
			throw DCLUserCancelled;
		}

		RefreshMNPStallDeadline();
		std::chrono::milliseconds waitTimeout = TDCLEventQueue::kForever;
		bool mnpStallBounded = false;

		if ( hasOverallTimeout )
		{
			const auto now = Clock::now();
			if ( now >= overallDeadline )
			{
				throw DCLTimeout;
			}
			waitTimeout = std::chrono::duration_cast<std::chrono::milliseconds>( overallDeadline - now );
		}

		if ( mMNPStallActive )
		{
			const auto now = Clock::now();
			if ( now >= mMNPStallDeadline )
			{
				HandleMNPStall();
				if ( mPeerDisconnected )
				{
					throw DCLEOF;
				}
				continue;
			}
			const auto mnpRemaining =
			    std::chrono::duration_cast<std::chrono::milliseconds>( mMNPStallDeadline - now );
			const auto clamped = ( mnpRemaining.count() > 0 ) ? mnpRemaining : std::chrono::milliseconds( 0 );
			if ( waitTimeout == TDCLEventQueue::kForever || clamped <= waitTimeout )
			{
				waitTimeout = clamped;
				mnpStallBounded = true;
			}
		}

		std::unique_ptr<TDCLEvent> ev(
		    mEventQueue.WaitNextEvent( TDCLEvent::kLinkEvent, waitTimeout, mWorkerStopToken ) );

		if ( mWorkerStopToken.stop_requested() )
		{
			throw DCLUserCancelled;
		}
		if ( !ev )
		{
			if ( mnpStallBounded && mMNPProtocol && mMNPProtocol->IsStallable() && Clock::now() >= mMNPStallDeadline )
			{
				HandleMNPStall();
				if ( mPeerDisconnected )
				{
					throw DCLEOF;
				}
				continue;
			}
			throw DCLTimeout;
		}

		switch ( ev->GetEventID() )
		{
		case TDCLEvent::kBytesReceived:
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( ev.get() );
			ProcessInboundBytes( typed->GetPayload().bytes );
			break;
		}
		case TDCLEvent::kSessionClosed:
			mPeerDisconnected = true;
			throw DCLEOF;
		case TDCLEvent::kTransportError:
		{
			auto* typed = static_cast<TDCLPayloadEvent<TDCLTransportErrorPayload>*>( ev.get() );
			if ( typed->GetPayload().error )
			{
				std::rethrow_exception( typed->GetPayload().error );
			}
			throw DCLEOF;
		}
		case TDCLEvent::kCloseLink:
		case TDCLEvent::kDisconnectLink:
			throw DCLEOF;
		default:
			break;
		}
	}
}

KSInt32
TDCLLink::GetResultCmd( void )
{
	KSInt32 theResult = 0;

	for ( ;; )
	{
		auto msg = ReceiveCommand();
		if ( msg->GetCommand() == TDCLDockCommand::kDHello )
		{
			// Skip keepalive hello; continue if another command is already buffered.
			if ( !mCommandFramer.HasCommand() )
			{
				// Wait for the actual kDResult command.
				continue;
			}
			continue;
		}
		if ( msg->GetCommand() == TDCLDockCommand::kDResult )
		{
			auto* res = static_cast<TDCLDockCmdSingleLong*>( msg.get() );
			theResult = static_cast<KSInt32>( res->GetLong() );
			break;
		}
		throw DCLUnexpDockCmd;
	}

	return theResult;
}
