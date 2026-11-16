// ==============================
// File:			UTestEventBus.cpp
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
// The Original Code is UTestEventBus.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include "UTestEventBus.h"

// DCL
#include <DCL/Communication_Layers/TMNPProtocol.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Interfaces/TDCLEventBus.h>
#include <DCL/Interfaces/TDCLEventQueue.h>
#include <DCL/Link/TDCLCommandFramer.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h>
#include <DCL/Streams/TDCLChunkStream.h>
#include <K/Defines/UByteSex.h>

// ISO C++
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stop_token>
#include <thread>
#include <vector>

namespace
{
#define CHECK( cond )                                                                 \
	do                                                                                \
	{                                                                                 \
		if ( !( cond ) )                                                              \
		{                                                                             \
			(void) ::printf( "UTestEventBus failed at line %d: %s\n", __LINE__, #cond ); \
			::abort();                                                                \
		}                                                                             \
	} while ( 0 )

void
TestSessionBusPreBindAndLifetime( void )
{
	TDCLSessionBus bus;
	std::atomic<int> closeCount{ 0 };
	std::vector<std::vector<std::uint8_t>> sent;

	auto downstream = bus.CreateSession(
	    [&sent]( std::vector<std::uint8_t> bytes ) {
		    sent.push_back( std::move( bytes ) );
		    return true;
	    },
	    [&closeCount]( void ) {
		    closeCount.fetch_add( 1 );
	    } );

	const TDCLSessionId id = downstream.GetSessionId();
	CHECK( id.IsValid() );

	// Publish before BindUpstream; events must buffer in exact order.
	CHECK( bus.PublishBytes( id, { 1, 2, 3 } ) );
	CHECK( bus.PublishError( id, std::make_exception_ptr( DCLEOF ) ) );
	CHECK( bus.PublishClosed( id ) );

	TDCLEventQueue queue;
	auto upstream = bus.BindUpstream( id, &queue );
	CHECK( static_cast<bool>( upstream ) );

	std::stop_source ss;
	auto popEvent = [&]( TDCLEvent::EEventID expectedId ) -> std::unique_ptr<TDCLEvent> {
		TDCLEvent* raw = queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds( 0 ), ss.get_token() );
		CHECK( raw != nullptr );
		std::unique_ptr<TDCLEvent> ev( raw );
		CHECK( ev->GetEventID() == expectedId );
		return ev;
	};

	{
		auto ev = popEvent( TDCLEvent::kBytesReceived );
		auto* typed = static_cast<TDCLPayloadEvent<TDCLBytesPayload>*>( ev.get() );
		CHECK( typed->GetPayload().sessionId == id );
		CHECK( typed->GetPayload().bytes == ( std::vector<std::uint8_t>{ 1, 2, 3 } ) );
	}
	{
		auto ev = popEvent( TDCLEvent::kTransportError );
		auto* typed = static_cast<TDCLPayloadEvent<TDCLTransportErrorPayload>*>( ev.get() );
		bool caughtEOF = false;
		try
		{
			std::rethrow_exception( typed->GetPayload().error );
		}
		catch ( const TDCLEOFException& )
		{
			caughtEOF = true;
		}
		CHECK( caughtEOF );
	}
	{
		auto ev = popEvent( TDCLEvent::kSessionClosed );
		auto* typed = static_cast<TDCLPayloadEvent<TDCLSessionClosedPayload>*>( ev.get() );
		CHECK( typed->GetPayload().sessionId == id );
	}

	// Downstream SendBytes and idempotent CloseSession.
	CHECK( bus.SendBytes( id, { 0xaa, 0xbb } ) );
	CHECK( sent.size() == 1 );
	CHECK( sent[0] == ( std::vector<std::uint8_t>{ 0xaa, 0xbb } ) );

	bus.CloseSession( id );
	bus.CloseSession( id );
	CHECK( closeCount.load() == 1 );
	CHECK( !bus.SendBytes( id, { 0xcc } ) );

	// Unbind upstream before queue goes out of scope.
	upstream.Reset();
	CHECK( !bus.PublishBytes( id, { 4, 5 } ) );

	// Reset downstream guard and verify late sends are no-ops.
	downstream.Reset();
	CHECK( !bus.SendBytes( id, { 6 } ) );
	CHECK( !bus.PublishClosed( id ) );
}

void
TestSessionBusConcurrentTeardown( void )
{
	for ( int iter = 0; iter < 50; ++iter )
	{
		TDCLSessionBus bus;
		std::atomic<bool> destroyed{ false };

		auto downstream = bus.CreateSession(
		    [&destroyed]( std::vector<std::uint8_t> ) {
			    CHECK( !destroyed.load() );
			    return true;
		    },
		    [&destroyed]( void ) {
			    CHECK( !destroyed.load() );
		    } );
		const TDCLSessionId id = downstream.GetSessionId();

		auto queue = std::make_unique<TDCLEventQueue>();
		auto upstream = bus.BindUpstream( id, queue.get() );

		std::jthread pub( [&]( std::stop_token st ) {
			while ( !st.stop_requested() )
			{
				bus.PublishBytes( id, { 0x42 } );
			}
		} );
		std::jthread sender( [&]( std::stop_token st ) {
			while ( !st.stop_requested() )
			{
				bus.SendBytes( id, { 0x24 } );
			}
		} );

		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		// Detach upstream then destroy queue while publisher still runs.
		upstream.Reset();
		queue.reset();

		// Detach downstream then mark destroyed while sender still runs.
		downstream.Reset();
		destroyed.store( true );

		pub.request_stop();
		sender.request_stop();
	}
}

void
TestDockCommandFramerAndChunkStream( void )
{
	std::vector<std::uint8_t> wire;
	TDCLChunkStream stream( [&wire]( std::vector<std::uint8_t>&& chunk ) {
		wire.insert( wire.end(), chunk.begin(), chunk.end() );
	} );

	TDCLDockCmdNoData hello( TDCLDockCommand::kDHello );
	hello.SendCommand( &stream );

	TDCLDockCmdSingleLong timeoutCmd( TDCLDockCommand::kDSetTimeout, 42 );
	timeoutCmd.SendCommand( &stream );

	const char oddPayload[] = "abcde"; // 5 bytes -> padded to 8
	TDCLDockCmdGeneric genericCmd( TDCLDockCommand::kDSetDrive, 5, oddPayload );
	genericCmd.SendCommand( &stream );

	// Feed byte-by-byte to verify arbitrary chunk boundary handling.
	TDCLCommandFramer framer( TDCLCommandFramer::EMode::kDock );
	std::vector<std::unique_ptr<TDCLDockCommand>> cmds;
	for ( std::uint8_t b : wire )
	{
		const auto st = framer.Feed( &b, 1 );
		CHECK( st != TDCLCommandFramer::EStatus::kError );
		CHECK( st != TDCLCommandFramer::EStatus::kEOF );
		while ( auto c = framer.TakeCommand() )
		{
			cmds.push_back( std::move( c ) );
		}
	}

	CHECK( cmds.size() == 3 );
	CHECK( cmds[0]->GetCommand() == TDCLDockCommand::kDHello );
	CHECK( cmds[1]->GetCommand() == TDCLDockCommand::kDSetTimeout );
	CHECK( static_cast<TDCLDockCmdSingleLong*>( cmds[1].get() )->GetLong() == 42 );
	CHECK( cmds[2]->GetCommand() == TDCLDockCommand::kDSetDrive );
	auto* g2 = static_cast<TDCLDockCmdGeneric*>( cmds[2].get() );
	CHECK( g2->GetLength() == 5 );
	CHECK( std::memcmp( g2->GetDataPtr(), oddPayload, 5 ) == 0 );

	// Test kDBackupIDs (0xFFFFFFFF length, odd number of shorts -> 2-byte alignment) + trailing kDHello.
	std::vector<std::uint8_t> backupWire;
	auto pushBE32 = [&]( KUInt32 v ) {
		const KUInt32 be = UByteSex_ToBigEndian( v );
		const auto* p = reinterpret_cast<const std::uint8_t*>( &be );
		backupWire.insert( backupWire.end(), p, p + 4 );
	};
	auto pushBE16 = [&]( KUInt16 v ) {
		const KUInt16 be = UByteSex_ToBigEndian( v );
		const auto* p = reinterpret_cast<const std::uint8_t*>( &be );
		backupWire.insert( backupWire.end(), p, p + 2 );
	};
	pushBE32( 'newt' );
	pushBE32( 'dock' );
	pushBE32( TDCLDockCommand::kDBackupIDs );
	pushBE32( 0xFFFFFFFFu );
	pushBE16( 1 );
	pushBE16( 2 );
	pushBE16( 0x8000 ); // 3 shorts -> odd -> 2-byte pad follows
	pushBE16( 0 );      // pad
	// Trailing kDHello
	pushBE32( 'newt' );
	pushBE32( 'dock' );
	pushBE32( TDCLDockCommand::kDHello );
	pushBE32( 0 );

	framer.Reset();
	CHECK( framer.Feed( backupWire ) == TDCLCommandFramer::EStatus::kCommandReady );
	auto c0 = framer.TakeCommand();
	auto c1 = framer.TakeCommand();
	CHECK( c0 != nullptr && c0->GetCommand() == TDCLDockCommand::kDBackupIDs );
	CHECK( c1 != nullptr && c1->GetCommand() == TDCLDockCommand::kDHello );
	CHECK( !framer.HasCommand() );

	// EOF header (zeros)
	std::vector<std::uint8_t> eofBytes( 16, 0 );
	CHECK( framer.Feed( eofBytes ) == TDCLCommandFramer::EStatus::kEOF );
	CHECK( framer.IsEOF() );

	// Bad magic -> error
	framer.Reset();
	std::vector<std::uint8_t> badBytes( 16, 0x11 );
	CHECK( framer.Feed( badBytes ) == TDCLCommandFramer::EStatus::kError );
	CHECK( framer.HasError() );
}

void
TestInspectorCommandFramer( void )
{
	std::vector<std::uint8_t> wire;
	TDCLChunkStream stream( [&wire]( std::vector<std::uint8_t>&& chunk ) {
		wire.insert( wire.end(), chunk.begin(), chunk.end() );
	} );

	// Unpadded 5-byte inspector text command followed by 0-byte result.
	const char text[] = "hello";
	TDCLInspectorCmdText txtCmd( 5, text );
	txtCmd.SendCommand( &stream );
	// Verify unpadded length: 16 header + 5 payload = 21 bytes.
	CHECK( wire.size() == 21 );

	TDCLInspectorCommand noopCmd( TDCLInspectorCommand::kDResult, 0, nullptr );
	noopCmd.SendCommand( &stream );

	TDCLCommandFramer framer( TDCLCommandFramer::EMode::kInspector );
	CHECK( framer.Feed( wire ) == TDCLCommandFramer::EStatus::kCommandReady );
	auto c0 = framer.TakeCommand();
	auto c1 = framer.TakeCommand();
	CHECK( c0 != nullptr && c0->GetCommand() == TDCLInspectorCommand::kDText );
	auto* t0 = static_cast<TDCLInspectorCmdText*>( c0.get() );
	CHECK( t0->GetLength() == 5 );
	CHECK( std::memcmp( t0->GetDataPtr(), text, 5 ) == 0 );
	CHECK( c1 != nullptr && c1->GetCommand() == TDCLInspectorCommand::kDResult );
}

void
TestMNPProtocolNonBlockingStream( void )
{
	TMNPProtocol proto;
	proto.Start();

	// Valid Newton LR frame body.
	const TMNPProtocol::Bytes lr = {
	    0x17, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff,
	    0x02, 0x01, 0x02, 0x03, 0x01, 0x02, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03 };
	const auto lrWire = TMNPProtocol::EncodeFrame( lr );
	proto.PushWireBytes( lrWire.data(), lrWire.size() );
	CHECK( proto.GetState() == TMNPProtocol::kNegotiating );
	CHECK( !proto.TakeAllOutboundWireBytes().empty() );

	const TMNPProtocol::Bytes handshakeLA = { 0x03, 0x05, 0x00, 0x01 };
	const auto laWire = TMNPProtocol::EncodeFrame( handshakeLA );
	proto.PushWireBytes( laWire.data(), laWire.size() );
	CHECK( proto.IsConnected() );

	// Queue > 253 bytes so it chunks across 2 LTs automatically as ACKs arrive.
	std::vector<std::uint8_t> payload( 300, 0x5a );
	proto.QueueOutboundBytes( payload.data(), payload.size() );
	CHECK( proto.GetState() == TMNPProtocol::kInFlight );
	CHECK( proto.HasPendingOutbound() );
	CHECK( !proto.TakeAllOutboundWireBytes().empty() );

	// Ack LT(1) -> automatically pumps LT(2) (47 bytes).
	const TMNPProtocol::Bytes ack1 = { 0x03, 0x05, 0x01, 0x01 };
	const auto ack1Wire = TMNPProtocol::EncodeFrame( ack1 );
	proto.PushWireBytes( ack1Wire.data(), ack1Wire.size() );
	CHECK( proto.GetState() == TMNPProtocol::kInFlight );
	CHECK( !proto.HasPendingOutbound() );
	CHECK( !proto.TakeAllOutboundWireBytes().empty() );
}
} // namespace

void
UTestEventBus::Run( void )
{
	TestSessionBusPreBindAndLifetime();
	TestSessionBusConcurrentTeardown();
	TestDockCommandFramerAndChunkStream();
	TestInspectorCommandFramer();
	TestMNPProtocolNonBlockingStream();
}
