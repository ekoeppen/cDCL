// ==============================
// Fichier:			UTestMNPLinkLayer.cpp
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========
//
// A real TDCLLink thread runs over TDCLMNPPipe and TTestMemoryPipe. There is
// no watcher thread: the test calls DataPresent() itself, as TSyncPipe would
// when bytes arrive.

#include "UTestMNPLinkLayer.h"
#include "TMNPPipeStatusTestAccess.h"
#include "TTestMemoryPipe.h"
#include "TTestServer.h"

#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Link/TDCLLink.h>

typedef TDCLStdThreads TDCLPlatformThreads;

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;
using ms = std::chrono::milliseconds;

#define CHECK( cond )                                                         \
	do                                                                        \
	{                                                                         \
		if ( !( cond ) )                                                      \
		{                                                                     \
			std::printf( "FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__ ); \
			std::exit( 1 );                                                   \
		}                                                                     \
	} while ( 0 )

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

	virtual IDCLThreads*
	CreateThreadsIntf( void ) override
	{
		return new TDCLPlatformThreads();
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

	bool
	IsStopped( void )
	{
		return GetThreadState() == IDCLThreads::kStopped;
	}

protected:
	virtual EState
	ProcessDockCommand( TDCLDockCommand*, Boolean* outProcessed ) override
	{
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
		CHECK( link.Connect( &pipe, &server ) );
		// IsConnected() is already true while connecting; wait for the
		// handshake to finish (the LR reply has been written).
		Bytes reply;
		CHECK( WaitFor(
		    [this, &reply]
		    {
			    const Bytes w = sub->TakeWritten();
			    reply.insert( reply.end(), w.begin(), w.end() );
			    return reply.size() >= 31; // SYN DLE STX + 24 + DLE ETX + CRC
		    },
		    ms( 2000 ) ) );
		CHECK( reply.size() == 31 );
		// Let the link reach kRunning and arm data notification.
		std::this_thread::sleep_for( ms( 50 ) );
	}

	~Setup( void )
	{
		// The link thread must be gone before the pipe is destroyed.
		link.Close();
		WaitFor( [this] { return link.IsStopped(); }, ms( 2000 ) );
	}
};

void
TestPeerLDEndsIdleLink( void )
{
	Setup s;
	s.ConnectAndRun();

	// The Newton disconnects while the link is idle. The watcher would see
	// raw bytes and signal the link; BytesAvailable consumes the LD.
	s.sub->Feed( Wire( { 0x07, 0x02, 0x01, 0x01, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00 } ) );
	s.link.DataPresent();

	// Without the fix the link stays running until the 15 s idle period.
	CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
	CHECK( WaitFor( [&s] { return s.link.IsStopped(); }, ms( 2000 ) ) );
	CHECK( !s.app.mLinkException );
	CHECK( s.pipe.PeerDisconnected() );
	CHECK( s.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	CHECK( s.sub->IsDisconnected() );
	CHECK( s.sub->TakeWritten().empty() ); // no LD in reply
}

void
TestProtocolErrorReachesApplication( void )
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

void
TestDataStillDelivered( void )
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
} // namespace

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
UTestMNPLinkLayer::Run( void )
{
	std::printf( "--- Testing TDCLLink over MNP ---\n" );
	TestPeerLDEndsIdleLink();
	TestProtocolErrorReachesApplication();
	TestDataStillDelivered();
	std::printf( "--- All TDCLLink over MNP tests passed! ---\n" );
}
