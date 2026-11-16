// ==============================
// File:			TDCLEventBus.cpp
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
// The Original Code is TDCLEventBus.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEventBus.h>

class TDCLSessionBus::DownstreamTarget
{
public:
	DownstreamTarget( SendBytesFn inSend, CloseSessionFn inClose )
		: mSend( std::move( inSend ) ),
		  mClose( std::move( inClose ) )
	{
	}

	bool SendBytes( std::vector<std::uint8_t>&& inBytes )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( !mActive || !mSend )
		{
			return false;
		}
		return mSend( std::move( inBytes ) );
	}

	void Close( void )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( !mActive || !mClose )
		{
			return;
		}
		mClose();
	}

	void Deactivate( void ) noexcept
	{
		std::lock_guard<std::mutex> lock( mMutex );
		mActive = false;
		mSend = nullptr;
		mClose = nullptr;
	}

private:
	std::mutex mMutex;
	bool mActive = true;
	SendBytesFn mSend;
	CloseSessionFn mClose;
};

struct TDCLSessionBus::SessionSlot
{
	std::mutex mMutex;
	TDCLEventQueue* mUpstreamQueue = nullptr;
	bool mUpstreamDetached = false;
	bool mClosed = false;
	std::vector<std::unique_ptr<TDCLEvent>> mBufferedEvents;
	std::shared_ptr<DownstreamTarget> mDownstream;

	bool PostOrBuffer( std::unique_ptr<TDCLEvent> inEvent )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( mUpstreamDetached )
		{
			return false;
		}
		if ( mUpstreamQueue != nullptr )
		{
			mUpstreamQueue->PostEvent( inEvent.release() );
			return true;
		}
		mBufferedEvents.push_back( std::move( inEvent ) );
		return true;
	}
};

// ------------------------------------------------------------------------- //
//  DownstreamGuard
// ------------------------------------------------------------------------- //

TDCLSessionBus::DownstreamGuard::DownstreamGuard(
	TDCLSessionBus* inBus,
	TDCLSessionId inId,
	std::shared_ptr<SessionSlot> inSlot,
	std::shared_ptr<DownstreamTarget> inTarget )
	: mBus( inBus ),
	  mSessionId( inId ),
	  mSlot( std::move( inSlot ) ),
	  mTarget( std::move( inTarget ) )
{
}

TDCLSessionBus::DownstreamGuard::DownstreamGuard( DownstreamGuard&& inOther ) noexcept
	: mBus( std::exchange( inOther.mBus, nullptr ) ),
	  mSessionId( std::exchange( inOther.mSessionId, TDCLSessionId{} ) ),
	  mSlot( std::move( inOther.mSlot ) ),
	  mTarget( std::move( inOther.mTarget ) )
{
}

TDCLSessionBus::DownstreamGuard&
TDCLSessionBus::DownstreamGuard::operator=( DownstreamGuard&& inOther ) noexcept
{
	if ( this != &inOther )
	{
		Reset();
		mBus = std::exchange( inOther.mBus, nullptr );
		mSessionId = std::exchange( inOther.mSessionId, TDCLSessionId{} );
		mSlot = std::move( inOther.mSlot );
		mTarget = std::move( inOther.mTarget );
	}
	return *this;
}

TDCLSessionBus::DownstreamGuard::~DownstreamGuard( void )
{
	Reset();
}

void
TDCLSessionBus::DownstreamGuard::Reset( void ) noexcept
{
	if ( mTarget )
	{
		mTarget->Deactivate();
		mTarget.reset();
	}
	if ( mSlot )
	{
		std::lock_guard<std::mutex> lock( mSlot->mMutex );
		mSlot->mDownstream.reset();
		mSlot->mClosed = true;
		mSlot.reset();
	}
	if ( mBus != nullptr && mSessionId )
	{
		mBus->RemoveSlot( mSessionId );
	}
	mBus = nullptr;
	mSessionId = TDCLSessionId{};
}

// ------------------------------------------------------------------------- //
//  UpstreamGuard
// ------------------------------------------------------------------------- //

TDCLSessionBus::UpstreamGuard::UpstreamGuard(
	TDCLSessionId inId,
	std::shared_ptr<SessionSlot> inSlot,
	TDCLEventQueue* inQueue )
	: mSessionId( inId ),
	  mSlot( std::move( inSlot ) ),
	  mQueue( inQueue )
{
}

TDCLSessionBus::UpstreamGuard::UpstreamGuard( UpstreamGuard&& inOther ) noexcept
	: mSessionId( std::exchange( inOther.mSessionId, TDCLSessionId{} ) ),
	  mSlot( std::move( inOther.mSlot ) ),
	  mQueue( std::exchange( inOther.mQueue, nullptr ) )
{
}

TDCLSessionBus::UpstreamGuard&
TDCLSessionBus::UpstreamGuard::operator=( UpstreamGuard&& inOther ) noexcept
{
	if ( this != &inOther )
	{
		Reset();
		mSessionId = std::exchange( inOther.mSessionId, TDCLSessionId{} );
		mSlot = std::move( inOther.mSlot );
		mQueue = std::exchange( inOther.mQueue, nullptr );
	}
	return *this;
}

TDCLSessionBus::UpstreamGuard::~UpstreamGuard( void )
{
	Reset();
}

void
TDCLSessionBus::UpstreamGuard::Reset( void ) noexcept
{
	if ( mSlot )
	{
		std::lock_guard<std::mutex> lock( mSlot->mMutex );
		if ( mSlot->mUpstreamQueue == mQueue )
		{
			mSlot->mUpstreamQueue = nullptr;
			mSlot->mUpstreamDetached = true;
			mSlot->mBufferedEvents.clear();
		}
		mSlot.reset();
	}
	mQueue = nullptr;
	mSessionId = TDCLSessionId{};
}

// ------------------------------------------------------------------------- //
//  TDCLSessionBus
// ------------------------------------------------------------------------- //

TDCLSessionBus::DownstreamGuard
TDCLSessionBus::CreateSession( SendBytesFn inSendBytes, CloseSessionFn inClose )
{
	const TDCLSessionId id = TDCLSessionId::Next();
	auto target = std::make_shared<DownstreamTarget>( std::move( inSendBytes ), std::move( inClose ) );
	auto slot = std::make_shared<SessionSlot>();
	slot->mDownstream = target;

	{
		std::lock_guard<std::mutex> lock( mMapMutex );
		mSessions.emplace( id, slot );
	}

	return DownstreamGuard( this, id, std::move( slot ), std::move( target ) );
}

TDCLSessionBus::UpstreamGuard
TDCLSessionBus::BindUpstream( TDCLSessionId inSessionId, TDCLEventQueue* inQueue )
{
	if ( !inSessionId || inQueue == nullptr )
	{
		return {};
	}

	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return {};
	}

	std::lock_guard<std::mutex> lock( slot->mMutex );
	if ( slot->mUpstreamDetached || slot->mUpstreamQueue != nullptr )
	{
		return {};
	}

	slot->mUpstreamQueue = inQueue;
	for ( auto& ev : slot->mBufferedEvents )
	{
		inQueue->PostEvent( ev.release() );
	}
	slot->mBufferedEvents.clear();

	return UpstreamGuard( inSessionId, std::move( slot ), inQueue );
}

bool
TDCLSessionBus::PublishBytes( TDCLSessionId inSessionId, std::vector<std::uint8_t> inBytes )
{
	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return false;
	}
	auto event = std::make_unique<TDCLPayloadEvent<TDCLBytesPayload>>(
		TDCLEvent::kLinkEvent,
		TDCLEvent::kBytesReceived,
		TDCLBytesPayload{ inSessionId, std::move( inBytes ) } );
	return slot->PostOrBuffer( std::move( event ) );
}

bool
TDCLSessionBus::PublishClosed( TDCLSessionId inSessionId )
{
	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return false;
	}
	auto event = std::make_unique<TDCLPayloadEvent<TDCLSessionClosedPayload>>(
		TDCLEvent::kLinkEvent,
		TDCLEvent::kSessionClosed,
		TDCLSessionClosedPayload{ inSessionId } );
	return slot->PostOrBuffer( std::move( event ) );
}

bool
TDCLSessionBus::PublishError( TDCLSessionId inSessionId, std::exception_ptr inError )
{
	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return false;
	}
	auto event = std::make_unique<TDCLPayloadEvent<TDCLTransportErrorPayload>>(
		TDCLEvent::kLinkEvent,
		TDCLEvent::kTransportError,
		TDCLTransportErrorPayload{ inSessionId, std::move( inError ) } );
	return slot->PostOrBuffer( std::move( event ) );
}

bool
TDCLSessionBus::SendBytes( TDCLSessionId inSessionId, std::vector<std::uint8_t> inBytes )
{
	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return false;
	}
	std::shared_ptr<DownstreamTarget> target;
	{
		std::lock_guard<std::mutex> lock( slot->mMutex );
		if ( slot->mClosed )
		{
			return false;
		}
		target = slot->mDownstream;
	}
	if ( !target )
	{
		return false;
	}
	return target->SendBytes( std::move( inBytes ) );
}

void
TDCLSessionBus::CloseSession( TDCLSessionId inSessionId )
{
	auto slot = FindSlot( inSessionId );
	if ( !slot )
	{
		return;
	}
	std::shared_ptr<DownstreamTarget> target;
	{
		std::lock_guard<std::mutex> lock( slot->mMutex );
		if ( slot->mClosed )
		{
			return;
		}
		slot->mClosed = true;
		target = slot->mDownstream;
	}
	if ( target )
	{
		target->Close();
	}
}

void
TDCLSessionBus::PostSessionAccepted(
	TDCLEventQueue& inServerQueue,
	TDCLSessionId inSessionId,
	TDCLCommLayer* inCommLayer )
{
	auto* event = new TDCLPayloadEvent<TDCLSessionAcceptedPayload>(
		TDCLEvent::kServerEvent,
		TDCLEvent::kSessionAccepted,
		TDCLSessionAcceptedPayload{ inSessionId, inCommLayer } );
	inServerQueue.PostEvent( event );
}

void
TDCLSessionBus::PostLinkDisconnected(
	TDCLEventQueue& inServerQueue,
	TDCLSessionId inSessionId,
	TDCLLink* inLink )
{
	auto* event = new TDCLPayloadEvent<TDCLLinkDisconnectedPayload>(
		TDCLEvent::kServerEvent,
		TDCLEvent::kLinkDisconnected,
		TDCLLinkDisconnectedPayload{ inSessionId, inLink } );
	inServerQueue.PostEvent( event );
}

std::shared_ptr<TDCLSessionBus::SessionSlot>
TDCLSessionBus::FindSlot( TDCLSessionId inSessionId ) const
{
	if ( !inSessionId )
	{
		return nullptr;
	}
	std::lock_guard<std::mutex> lock( mMapMutex );
	auto it = mSessions.find( inSessionId );
	return it != mSessions.end() ? it->second : nullptr;
}

void
TDCLSessionBus::RemoveSlot( TDCLSessionId inSessionId )
{
	std::lock_guard<std::mutex> lock( mMapMutex );
	mSessions.erase( inSessionId );
}
