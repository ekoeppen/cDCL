// ==============================
// File:			UTestMNPLinkLayer.cpp
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
// The Original Code is UTestMNPLinkLayer.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
//
// A real TDCLLink thread runs over TDCLMNPPipe and TTestMemoryPipe. There is
// no watcher thread: the test calls DataPresent() itself, as TSyncPipe would
// when bytes arrive.

#include "TMNPPipeStatusTestAccess.h"
#include "TTestMemoryPipe.h"
#include "TTestServer.h"

#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Link/TDCLLink.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;
using ms = std::chrono::milliseconds;

const Bytes kHardwareLR = { 0x26, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02, 0x01, 0x02,
                            0x03, 0x01, 0x08, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03, 0x09, 0x01, 0x01, 0x0e,
                            0x04, 0x03, 0x04, 0x00, 0xfa, 0xc5, 0x06, 0x01, 0x04, 0x00, 0x00, 0xe1, 0x00 };

Bytes
Wire( const Bytes& inBody )
{
	std::uint16_t crc = 0;
	auto feed = [&crc]( std::uint8_t b )
	{
		crc ^= b;
		for ( int i = 0; i < 8; ++i )
		{
			crc = ( crc & 1 ) ? static_cast<std::uint16_t>( ( crc >> 1 ) ^ 0xA001 )
			                  : static_cast<std::uint16_t>( crc >> 1 );
		}
	};
	Bytes w = { 0x16, 0x10, 0x02 };
	for ( auto b : inBody )
	{
		feed( b );
		w.push_back( b );
		if ( b == 0x10 )
		{
			w.push_back( 0x10 );
		}
	}
	feed( 0x03 );
	w.push_back( 0x10 );
	w.push_back( 0x03 );
	w.push_back( static_cast<std::uint8_t>( crc & 0xff ) );
	w.push_back( static_cast<std::uint8_t>( crc >> 8 ) );
	return w;
}

///
/// Application that records what happened to the link. Logs go to a
/// temporary file so the test output stays stable.
///
class TApplication : public TDCLLogApplication
{
public:
	TApplication( void ) : TDCLLogApplication( std::tmpfile() )
	{
	}

	std::atomic<bool> mDisconnected{ false };
	std::atomic<bool> mLinkException{ false };
	std::string mExceptionName;

	virtual void
	Disconnected( TDCLLink* inLink ) override
	{
		TDCLLogApplication::Disconnected( inLink );
		mDisconnected = true;
	}

	virtual void
	HandleLinkException( TDCLLink* inLink, TDCLException* inException ) override
	{
		TDCLLogApplication::HandleLinkException( inLink, inException );
		mExceptionName = inException ? inException->GetExceptionName() : "(none)";
		mLinkException = true;
	}

	virtual IDCLFiles*
	CreateFilesIntf( void ) override
	{
		return nullptr;
	}
};

///
/// Link that only runs the MNP handshake on connect (no Dock commands), so
/// that the test exercises the running state.
///
class TLink : public TDCLLink
{
public:
	explicit TLink( TDCLApplication* inApplication ) : TDCLLink( inApplication )
	{
	}

	std::atomic<bool> mDisconnected{ false };

	std::vector<KUInt32>
	GetCommands( void ) const
	{
		std::lock_guard<std::mutex> lock( mCommandsMutex );
		return mCommands;
	}

protected:
	virtual EState
	ProcessDockCommand( TDCLDockCommand* inCommand, Boolean* outProcessed ) override
	{
		if ( inCommand )
		{
			std::lock_guard<std::mutex> lock( mCommandsMutex );
			mCommands.push_back( inCommand->GetCommand() );
		}
		*outProcessed = false; // the link answers kDUnknownCommand
		return kRunning;
	}

	virtual void
	DoConnect( void ) override
	{
		GetApplication()->Connected( this, GetPipe()->Connected( this ) );
	}

	virtual void
	Connected( void ) override
	{
		// No Newton name: skip ConnectedToNewtonDevice.
	}

	virtual void
	Disconnected( void ) override
	{
		TDCLLink::Disconnected();
		mDisconnected = true;
	}

private:
	mutable std::mutex mCommandsMutex;
	std::vector<KUInt32> mCommands;
};

template<typename P>
bool
WaitFor( P inPredicate, ms inTimeout )
{
	const auto deadline = std::chrono::steady_clock::now() + inTimeout;
	while ( !inPredicate() )
	{
		if ( std::chrono::steady_clock::now() >= deadline )
		{
			return false;
		}
		std::this_thread::sleep_for( ms( 5 ) );
	}
	return true;
}

class TBorrowedPipe : public TDCLPipe
{
public:
	explicit TBorrowedPipe( TDCLPipe* inTarget )
	    : TDCLPipe( nil ), mTarget( inTarget )
	{
	}

	Boolean BytesAvailable( void ) override { return mTarget->BytesAvailable(); }
	void FlushOutput( void ) override { mTarget->FlushOutput(); }
	void Disconnect( void ) override { mTarget->Disconnect(); }
	void Idle( void ) override { mTarget->Idle(); }
	long GetTimeout( void ) override { return mTarget->GetTimeout(); }
	void SetTimeout( long inTimeout ) override { mTarget->SetTimeout( inTimeout ); }
	KUInt8 PeekByte( void ) override { return mTarget->PeekByte(); }
	TDCLCommLayer* Connected( TDCLLink* inLink ) override { return mTarget->Connected( inLink ); }
	void ClearDataPresent( void ) override { mTarget->ClearDataPresent(); }
	Boolean PeerDisconnected( void ) override { return mTarget->PeerDisconnected(); }
	void Read( void* outBuffer, KUInt32* ioCount ) override { mTarget->Read( outBuffer, ioCount ); }
	void Write( const void* inBuffer, KUInt32* ioCount ) override { mTarget->Write( inBuffer, ioCount ); }
	bool RequestStop( void ) override
	{
		bool r = TDCLPipe::RequestStop();
		if ( mTarget->RequestStop() )
		{
			r = true;
		}
		return r;
	}

private:
	TDCLPipe* mTarget;
};

///
/// One test setup: application, server (not started; it only receives the
/// link's disconnect event), link, MNP pipe over a memory pipe.
///
struct Setup
{
	TApplication app;
	TTestServer server{ &app };
	TTestMemoryPipe* sub = new TTestMemoryPipe();
	TDCLMNPPipe pipe{ sub };
	TLink link{ &app };

	void
	ConnectAndRun( void )
	{
		sub->Feed( Wire( kHardwareLR ) );
		sub->Feed( Wire( { 0x03, 0x05, 0x00, 0x01 } ) );
		REQUIRE( link.Connect( std::make_unique<TBorrowedPipe>( &pipe ), &server ) );
		// IsConnected() is already true while connecting; wait for the
		// handshake to finish (the LR reply has been written).
		Bytes reply;
		REQUIRE( WaitFor(
		    [this, &reply]
		    {
			    const Bytes w = sub->TakeWritten();
			    reply.insert( reply.end(), w.begin(), w.end() );
			    return reply.size() >= 31; // SYN DLE STX + 24 + DLE ETX + CRC
		    },
		    ms( 2000 ) ) );
		REQUIRE( reply.size() == 31 );
		// Let the link reach kRunning and arm data notification.
		std::this_thread::sleep_for( ms( 50 ) );
	}

	~Setup( void )
	{
		// The link worker must be gone before the pipe is destroyed.
		link.Close();
		(void)link.Wait();
	}
};

} // namespace

TEST_SUITE("MNPLinkLayer")
{
	TEST_CASE("Peer LD ends idle link")
	{
		Setup s;
		s.ConnectAndRun();

		// The Newton disconnects while the link is idle. The watcher would see
		// raw bytes and signal the link; BytesAvailable consumes the LD.
		s.sub->Feed( Wire( { 0x07, 0x02, 0x01, 0x01, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00 } ) );
		s.link.DataPresent();

		// Without the fix the link stays running until the 15 s idle period.
		CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
		CHECK( s.link.Wait() );
		CHECK( !s.link.IsConnected() );
		CHECK( !s.app.mLinkException );
		CHECK( s.pipe.PeerDisconnected() );
		CHECK( s.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
		CHECK( s.sub->IsDisconnected() );
		CHECK( s.sub->TakeWritten().empty() ); // no LD in reply
	}

	TEST_CASE("Protocol error reaches application")
	{
		Setup s;
		s.ConnectAndRun();

		// A frame with a bad CRC: the pipe drops the link and BytesAvailable
		// throws, which the link reports through HandleLinkException.
		Bytes bad = Wire( { 0x02, 0x04, 0x01, 0x41 } );
		bad.back() ^= 0x01;
		s.sub->Feed( bad );
		s.link.DataPresent();

		CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
		CHECK( s.app.mExceptionName == "protocol error, link dropped" );
		CHECK( !s.pipe.PeerDisconnected() );
		CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( s.pipe ) == 0xfe );
		CHECK( s.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kFailed );
		CHECK( s.sub->IsDisconnected() );
	}

	TEST_CASE("Data is still delivered")
	{
		// A complete Dock command keeps the link running and is not mistaken for
		// a close. The base link does not know it and answers kDUnknownCommand.
		Setup s;
		s.ConnectAndRun();
		s.sub->Feed( Wire( { 0x02, 0x04, 0x01, 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 't', 'e', 's', 't', 0, 0, 0, 0 } ) );
		s.link.DataPresent();
		CHECK( WaitFor( [&s] { return !s.sub->TakeWritten().empty(); }, ms( 2000 ) ) );
		CHECK( s.link.IsConnected() );
		CHECK( !s.link.mDisconnected );
		CHECK( !s.app.mLinkException );
	}

	TEST_CASE("Batched commands wake link")
	{
		// Two Dock commands arriving in separate LTs during one ProcessAvailableInput
		// batch leave the second command in mInputBuffer after the link reads the
		// first. ClearDataPresent() must re-notify the link instead of rearming the
		// sub-pipe watcher while raw bytes are already exhausted.
		Setup s;
		s.ConnectAndRun();

		s.sub->Feed( Wire( { 0x02, 0x04, 0x01, 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 'c', 'm', 'd', '1', 0, 0, 0, 0 } ) );
		s.sub->Feed( Wire( { 0x02, 0x04, 0x02, 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 'c', 'm', 'd', '2', 0, 0, 0, 0 } ) );
		s.link.DataPresent();

		Bytes written;
		const Bytes replyTag = { 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 'u', 'n', 'k', 'n' };
		auto countReplies = [&written, &replyTag]( void ) -> std::size_t
		{
			std::size_t count = 0;
			auto it = written.begin();
			while ( ( it = std::search( it, written.end(), replyTag.begin(), replyTag.end() ) ) != written.end() )
			{
				++count;
				it += static_cast<std::ptrdiff_t>( replyTag.size() );
			}
			return count;
		};

		// The negotiated send window is 1, so wait for the first reply and ack it
		// (without posting DataPresent again) so the second reply can be sent.
		REQUIRE( WaitFor(
		    [&s, &written, &countReplies]
		    {
			    const Bytes w = s.sub->TakeWritten();
			    written.insert( written.end(), w.begin(), w.end() );
			    return countReplies() == 1;
		    },
		    ms( 2000 ) ) );

		s.sub->Feed( Wire( { 0x03, 0x05, 0x01, 0x01 } ) );

		REQUIRE( WaitFor(
		    [&s, &written, &countReplies]
		    {
			    const Bytes w = s.sub->TakeWritten();
			    written.insert( written.end(), w.begin(), w.end() );
			    return s.link.GetCommands().size() == 2 && countReplies() == 2;
		    },
		    ms( 2000 ) ) );

		const std::vector<KUInt32> expected = { FOUR_CHAR_CODE( 'cmd1' ), FOUR_CHAR_CODE( 'cmd2' ) };
		CHECK( s.link.GetCommands() == expected );
		CHECK( !s.sub->BytesAvailable() );
		CHECK( s.link.IsConnected() );
		CHECK( !s.link.mDisconnected );
		CHECK( !s.app.mLinkException );
	}
}
