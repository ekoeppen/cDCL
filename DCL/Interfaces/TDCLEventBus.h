// ==============================
// File:			TDCLEventBus.h
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
// The Original Code is TDCLEventBus.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLEVENTBUS_H
#define _TDCLEVENTBUS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEvent.h>
#include <DCL/Interfaces/TDCLEventQueue.h>
#include <DCL/Interfaces/TDCLSessionId.h>

// ISO C++
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

class TDCLCommLayer;
class TDCLLink;

///
/// Typed payload event delivered via TDCLEventQueue.
/// Owned and deleted by TDCLEventQueue::Clear() or the event consumer.
///
template <typename TPayload>
class TDCLPayloadEvent
	:
		public TDCLEvent
{
public:
	TDCLPayloadEvent( EEventKind inKind, EEventID inID, TPayload inPayload )
		: TDCLEvent( inKind, inID, this ),
		  mPayload( std::move( inPayload ) )
	{
	}

	const TPayload& GetPayload( void ) const noexcept
	{
		return mPayload;
	}

	TPayload& GetPayload( void ) noexcept
	{
		return mPayload;
	}

	TPayload TakePayload( void ) noexcept
	{
		return std::move( mPayload );
	}

private:
	TPayload mPayload;
};

struct TDCLSessionAcceptedPayload
{
	TDCLSessionId sessionId;
	TDCLCommLayer* commLayer = nullptr;
};

struct TDCLBytesPayload
{
	TDCLSessionId sessionId;
	std::vector<std::uint8_t> bytes;
};

struct TDCLSessionClosedPayload
{
	TDCLSessionId sessionId;
};

struct TDCLTransportErrorPayload
{
	TDCLSessionId sessionId;
	std::exception_ptr error;
};

struct TDCLLinkDisconnectedPayload
{
	TDCLSessionId sessionId;
	TDCLLink* link = nullptr;
};

///
/// Internal session-addressed event bus bridging transport sessions and
/// server/link event queues without cross-object raw backpointers.
///
/// Lock hierarchy (must never be inverted):
///   1. TDCLSessionBus::mMapMutex (never held while locking a slot or posting)
///   2. SessionSlot::mMutex       (may lock TDCLEventQueue::mMutex; never held
///                                 while calling downstream transport callbacks)
///   3. DownstreamTarget::mMutex  (serializes SendBytes/Close vs Deactivate)
///   4. TDCLEventQueue::mMutex
///
class TDCLSessionBus
{
public:
	using SendBytesFn = std::function<bool( std::vector<std::uint8_t> )>;
	using CloseSessionFn = std::function<void( void )>;

	class DownstreamTarget;
	struct SessionSlot;

	///
	/// RAII guard held by the transport owning a session's OS socket/FD.
	/// Resetting/destroying it deactivates downstream callbacks synchronously
	/// (waiting for any in-flight SendBytes/Close call to return) so the
	/// transport can safely destroy its members immediately afterward.
	///
	class DownstreamGuard
	{
	public:
		DownstreamGuard( void ) = default;
		DownstreamGuard( const DownstreamGuard& ) = delete;
		DownstreamGuard& operator=( const DownstreamGuard& ) = delete;
		DownstreamGuard( DownstreamGuard&& inOther ) noexcept;
		DownstreamGuard& operator=( DownstreamGuard&& inOther ) noexcept;
		~DownstreamGuard( void );

		TDCLSessionId GetSessionId( void ) const noexcept { return mSessionId; }
		explicit operator bool( void ) const noexcept { return static_cast<bool>( mSessionId ); }

		void Reset( void ) noexcept;

	private:
		friend class TDCLSessionBus;
		DownstreamGuard(
			TDCLSessionBus* inBus,
			TDCLSessionId inId,
			std::shared_ptr<SessionSlot> inSlot,
			std::shared_ptr<DownstreamTarget> inTarget );

		TDCLSessionBus* mBus = nullptr;
		TDCLSessionId mSessionId;
		std::shared_ptr<SessionSlot> mSlot;
		std::shared_ptr<DownstreamTarget> mTarget;
	};

	///
	/// RAII guard held by the link (or upstream consumer) owning a TDCLEventQueue.
	/// Resetting/destroying it atomically detaches the queue from the session
	/// before the queue itself is destroyed.
	///
	class UpstreamGuard
	{
	public:
		UpstreamGuard( void ) = default;
		UpstreamGuard( const UpstreamGuard& ) = delete;
		UpstreamGuard& operator=( const UpstreamGuard& ) = delete;
		UpstreamGuard( UpstreamGuard&& inOther ) noexcept;
		UpstreamGuard& operator=( UpstreamGuard&& inOther ) noexcept;
		~UpstreamGuard( void );

		TDCLSessionId GetSessionId( void ) const noexcept { return mSessionId; }
		explicit operator bool( void ) const noexcept { return static_cast<bool>( mSessionId ); }

		void Reset( void ) noexcept;

	private:
		friend class TDCLSessionBus;
		UpstreamGuard( TDCLSessionId inId, std::shared_ptr<SessionSlot> inSlot, TDCLEventQueue* inQueue );

		TDCLSessionId mSessionId;
		std::shared_ptr<SessionSlot> mSlot;
		TDCLEventQueue* mQueue = nullptr;
	};

	TDCLSessionBus( void ) = default;
	~TDCLSessionBus( void ) = default;

	TDCLSessionBus( const TDCLSessionBus& ) = delete;
	TDCLSessionBus& operator=( const TDCLSessionBus& ) = delete;

	///
	/// Allocates a new session and registers the transport's downstream handlers.
	/// Any upstream events emitted before BindUpstream() are buffered in-order.
	///
	[[nodiscard]] DownstreamGuard CreateSession( SendBytesFn inSendBytes, CloseSessionFn inClose );

	///
	/// Binds an upstream TDCLEventQueue to receive kBytesReceived, kSessionClosed,
	/// and kTransportError events for \p inSessionId, flushing any pre-bind
	/// buffered events immediately in arrival order.
	/// Returns an invalid guard if the session does not exist or already detached.
	///
	[[nodiscard]] UpstreamGuard BindUpstream( TDCLSessionId inSessionId, TDCLEventQueue* inQueue );

	/// Upstream publications (Transport -> Link). No-op if session unknown/detached.
	bool PublishBytes( TDCLSessionId inSessionId, std::vector<std::uint8_t> inBytes );
	bool PublishClosed( TDCLSessionId inSessionId );
	bool PublishError( TDCLSessionId inSessionId, std::exception_ptr inError );

	/// Downstream commands (Link/Server -> Transport). Safe no-op/false if closed.
	bool SendBytes( TDCLSessionId inSessionId, std::vector<std::uint8_t> inBytes );
	void CloseSession( TDCLSessionId inSessionId );

	/// Helper to post kSessionAccepted to a server queue.
	static void PostSessionAccepted(
		TDCLEventQueue& inServerQueue,
		TDCLSessionId inSessionId,
		TDCLCommLayer* inCommLayer );

	/// Helper to post kLinkDisconnected to a server queue.
	static void PostLinkDisconnected(
		TDCLEventQueue& inServerQueue,
		TDCLSessionId inSessionId,
		TDCLLink* inLink );

private:
	std::shared_ptr<SessionSlot> FindSlot( TDCLSessionId inSessionId ) const;
	void RemoveSlot( TDCLSessionId inSessionId );

	mutable std::mutex mMapMutex;
	std::unordered_map<TDCLSessionId, std::shared_ptr<SessionSlot>> mSessions;
};

#endif
// _TDCLEVENTBUS_H
