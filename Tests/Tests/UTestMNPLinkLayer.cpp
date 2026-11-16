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

#include "UTestMNPLinkLayer.h"
#include "TTestServer.h"

#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Interfaces/TDCLEventBus.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>
#include <DCL/Link/TDCLLink.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;
using Frames = std::vector<Bytes>;
using Clock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;

constexpr ms kTestStall = ms( 100 );

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

const Bytes& kExpectedReply = TMNPProtocol::kLRReply;

Bytes
LA( std::uint8_t inSeq, std::uint8_t inCredit )
{
	return { 0x03, 0x05, inSeq, inCredit };
}

Bytes
LD( std::uint8_t inReason )
{
	return { 0x07, 0x02, 0x01, 0x01, inReason, 0x02, 0x01, 0x00 };
}

bool
IsLT( const Bytes& inBody )
{
	return inBody.size() >= 3 && inBody[0] == 0x02 && inBody[1] == 0x04;
}

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

Frames
Decode( const Bytes& inWire )
{
	TMNPProtocol decoder;
	Frames out;
	for ( std::uint8_t b : inWire )
	{
		const auto r = decoder.PushByte( b );
		CHECK( r != TMNPProtocol::EFrameResult::kError );
		if ( r == TMNPProtocol::EFrameResult::kFrame )
		{
			out.push_back( decoder.GetDecodedFrame() );
		}
	}
	return out;
}

class TApplication : public TDCLLogApplication
{
public:
	TApplication( void ) : TDCLLogApplication( std::tmpfile() )
	{
	}

	std::atomic<bool> mDisconnected{ false };
	std::atomic<bool> mLinkException{ false };
	std::string mExceptionName;
	KSInt32 mExceptionError{ 0 };

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
		mExceptionError = inException ? inException->GetError() : 0;
		mLinkException = true;
	}

	virtual IDCLFiles*
	CreateFilesIntf( void ) override
	{
		return nullptr;
	}
};

class TLink : public TDCLLink
{
public:
	explicit TLink( TDCLApplication* inApplication ) : TDCLLink( inApplication )
	{
		SetMNPStallTimeout( kTestStall );
	}

	using TDCLLink::SetMNPKeepAlive;
	using TDCLLink::SetMNPStallTimeout;

	std::atomic<bool> mDisconnected{ false };
	std::atomic<bool> mInRunning{ false };
	std::atomic<bool> mDoConnectStarted{ false };
	bool mReceiveInDoConnect{ false };
	std::unique_ptr<TDCLDockCommand> mInitialOutbound;

	void
	SendOutboundCommand( TDCLDockCommand* inCommand )
	{
		SendCommand( inCommand );
	}

protected:
	virtual EState
	ProcessDockCommand( TDCLDockCommand*, Boolean* outProcessed ) override
	{
		*outProcessed = false;
		return kRunning;
	}

	virtual void
	DoConnect( void ) override
	{
		mDoConnectStarted = true;
		if ( mReceiveInDoConnect )
		{
			(void)ReceiveCommand();
		}
		NotifyConnected();
	}

	virtual void
	Connected( void ) override
	{
		mInRunning = true;
		if ( mInitialOutbound )
		{
			SendCommand( mInitialOutbound.get() );
			mInitialOutbound.reset();
		}
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
	const auto deadline = Clock::now() + inTimeout;
	while ( !inPredicate() )
	{
		if ( Clock::now() >= deadline )
		{
			return false;
		}
		std::this_thread::sleep_for( ms( 2 ) );
	}
	return true;
}

struct Setup
{
	TApplication app;
	TTestServer server{ &app };
	std::shared_ptr<TDCLSessionBus> bus = server.GetSharedSessionBus();
	TDCLSessionBus::DownstreamGuard downstream;
	TDCLSessionId sessionId;
	TLink link{ &app };

	std::mutex outMutex;
	Bytes written;
	std::atomic<bool> closed{ false };
	std::function<void( const Bytes& )> onWrite;

	void
	CreateSession( void )
	{
		downstream = bus->CreateSession(
		    [this]( Bytes inBytes ) {
			    {
				    std::lock_guard<std::mutex> lock( outMutex );
				    written.insert( written.end(), inBytes.begin(), inBytes.end() );
			    }
			    if ( onWrite )
			    {
				    onWrite( inBytes );
			    }
			    return true;
		    },
		    [this]( void ) {
			    closed.store( true );
		    } );
		sessionId = downstream.GetSessionId();
	}

	Bytes
	TakeWritten( void )
	{
		std::lock_guard<std::mutex> lock( outMutex );
		Bytes copy;
		copy.swap( written );
		return copy;
	}

	Frames
	TakeWrittenFrames( void )
	{
		return Decode( TakeWritten() );
	}

	void
	FeedBytes( const Bytes& inBytes )
	{
		(void)bus->PublishBytes( sessionId, inBytes );
	}

	void
	FeedFrame( const Bytes& inBody )
	{
		FeedBytes( Wire( inBody ) );
	}

	void
	ConnectAndRun( void )
	{
		CreateSession();
		FeedFrame( kHardwareLR );
		FeedFrame( LA( 0, 1 ) );
		CHECK( link.Connect( sessionId, bus, true /* inUseMNP */ ) );

		CHECK( WaitFor( [this] { return link.mInRunning.load(); }, ms( 2000 ) ) );
		const Frames reply = TakeWrittenFrames();
		CHECK( !reply.empty() && reply[0] == kExpectedReply );
	}

	~Setup( void )
	{
		link.Close();
		(void)link.Wait();
		downstream.Reset();
	}
};

void
TestPeerLDEndsIdleLink( void )
{
	Setup s;
	s.ConnectAndRun();

	s.FeedFrame( LD( 0 ) );

	CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( !s.link.IsConnected() );
	CHECK( !s.app.mLinkException );
	CHECK( s.link.PeerDisconnected() );
	CHECK( s.closed.load() );
	CHECK( s.TakeWritten().empty() );
}

void
TestProtocolErrorReachesApplication( void )
{
	Setup s;
	s.ConnectAndRun();

	Bytes bad = Wire( { 0x02, 0x04, 0x01, 0x41 } );
	bad.back() ^= 0x01;
	s.FeedBytes( bad );

	CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
	CHECK( s.app.mExceptionName == "protocol error, link dropped" );
	CHECK( s.app.mExceptionError == TMNPProtocol::kReasonProtocol );
	CHECK( !s.link.PeerDisconnected() );
	CHECK( s.closed.load() );
	const Frames out = s.TakeWrittenFrames();
	CHECK( !out.empty() && out.back() == LD( TMNPProtocol::kReasonProtocol ) );
}

void
TestDataStillDelivered( void )
{
	Setup s;
	s.ConnectAndRun();
	s.FeedFrame( { 0x02, 0x04, 0x01, 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 't', 'e', 's', 't', 0, 0, 0, 0 } );
	CHECK( WaitFor( [&s] { return !s.TakeWritten().empty(); }, ms( 2000 ) ) );
	CHECK( s.link.IsConnected() );
	CHECK( !s.link.mDisconnected );
	CHECK( !s.app.mLinkException );
}

void
TestHandshakeStallBeforeAndAfterLR( void )
{
	// Before LR: sends LD(0x01) and raises TDCLMNPProtocolException(0x01).
	{
		Setup s;
		s.CreateSession();
		CHECK( s.link.Connect( s.sessionId, s.bus, true ) );
		CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
		CHECK( s.link.Wait() );
		CHECK( s.app.mExceptionName == "protocol error, link dropped" );
		CHECK( s.app.mExceptionError == TMNPProtocol::kReasonHandshakeTimeout );
		const Frames out = s.TakeWrittenFrames();
		CHECK( out == Frames{ LD( TMNPProtocol::kReasonHandshakeTimeout ) } );
	}

	// After LR (never receives LA): sends LR reply then LD(0x01).
	{
		Setup s;
		s.CreateSession();
		s.FeedFrame( kHardwareLR );
		CHECK( s.link.Connect( s.sessionId, s.bus, true ) );
		CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
		CHECK( s.link.Wait() );
		CHECK( s.app.mExceptionName == "protocol error, link dropped" );
		CHECK( s.app.mExceptionError == TMNPProtocol::kReasonHandshakeTimeout );
		const Frames out = s.TakeWrittenFrames();
		CHECK( out == ( Frames{ kExpectedReply, LD( TMNPProtocol::kReasonHandshakeTimeout ) } ) );
	}
}

void
TestRejectedLR( void )
{
	Setup s;
	s.CreateSession();
	Bytes badLR = kHardwareLR;
	badLR[12] = 0x05; // unsupported framing mode
	s.FeedFrame( badLR );
	CHECK( s.link.Connect( s.sessionId, s.bus, true ) );

	CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( s.app.mExceptionError == TMNPProtocol::kReasonNegotiation );
	const Frames out = s.TakeWrittenFrames();
	CHECK( out == Frames{ LD( TMNPProtocol::kReasonNegotiation ) } );
}

void
TestPeerLDInHandshake( void )
{
	Setup s;
	s.CreateSession();
	s.FeedFrame( kHardwareLR );
	s.FeedFrame( LD( 0 ) );
	CHECK( s.link.Connect( s.sessionId, s.bus, true ) );

	CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( s.link.PeerDisconnected() );
	CHECK( !s.app.mLinkException );
	const Frames out = s.TakeWrittenFrames();
	CHECK( out == Frames{ kExpectedReply } );
}

void
TestKeepAlive( void )
{
	Setup s;
	s.link.SetMNPKeepAlive( true );
	s.ConnectAndRun();

	// Newton sends keepalive LA(0,2); link must reply with LA(0,1).
	s.FeedFrame( LA( 0, 2 ) );
	Frames out;
	CHECK( WaitFor(
	    [&s, &out] {
		    const Frames f = s.TakeWrittenFrames();
		    out.insert( out.end(), f.begin(), f.end() );
		    return !out.empty();
	    },
	    ms( 2000 ) ) );
	CHECK( out == Frames{ LA( 0, 1 ) } );
}

void
TestKeepAliveCannotStarveInFlightStall( void )
{
	Setup s;
	s.link.SetMNPKeepAlive( true );
	std::vector<std::uint8_t> payload( 16, 0x42 );
	s.link.mInitialOutbound = std::make_unique<TDCLDockCmdGeneric>( 'test', 16, payload.data() );
	s.ConnectAndRun();

	// Keep sending LA(0,1) every 15 ms so the event loop keeps waking up
	// without ACKing LT(1). The stall timer must still expire and send LD(0x04).
	for ( int i = 0; i < 20 && !s.app.mLinkException.load(); ++i )
	{
		s.FeedFrame( LA( 0, 1 ) );
		std::this_thread::sleep_for( ms( 15 ) );
	}
	CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( s.app.mExceptionError == TMNPProtocol::kReasonTimeout );
	const Frames out = s.TakeWrittenFrames();
	CHECK( !out.empty() && out.back() == LD( TMNPProtocol::kReasonTimeout ) );
}

void
TestMultiLTSegmentationAndCreditRestore( void )
{
	Setup s;
	std::mutex ltMutex;
	Frames receivedLTs;
	std::atomic<bool> ackWithZeroOnce{ true };
	s.onWrite = [&s, &ltMutex, &receivedLTs, &ackWithZeroOnce]( const Bytes& inWire )
	{
		for ( const Bytes& frame : Decode( inWire ) )
		{
			if ( IsLT( frame ) )
			{
				{
					std::lock_guard<std::mutex> lock( ltMutex );
					receivedLTs.push_back( frame );
				}
				if ( ackWithZeroOnce.exchange( false ) )
				{
					// Throttle after LT(1) for 20 ms, then restore credit.
					s.FeedFrame( LA( frame[2], 0 ) );
					std::this_thread::sleep_for( ms( 20 ) );
					s.FeedFrame( LA( frame[2], 1 ) );
				}
				else
				{
					s.FeedFrame( LA( frame[2], 1 ) );
				}
			}
		}
	};

	// 600-byte payload + 16-byte header => 3 LTs (253, 253, 110).
	Bytes payload( 600, 0x5A );
	s.link.mInitialOutbound = std::make_unique<TDCLDockCmdGeneric>( 'bigc', 600, payload.data() );
	s.ConnectAndRun();

	CHECK( WaitFor(
	    [&ltMutex, &receivedLTs] {
		    std::lock_guard<std::mutex> lock( ltMutex );
		    return receivedLTs.size() == 3;
	    },
	    ms( 2000 ) ) );

	std::lock_guard<std::mutex> lock( ltMutex );
	CHECK( receivedLTs[0].size() == 3 + 253 );
	CHECK( receivedLTs[1].size() == 3 + 253 );
	CHECK( receivedLTs[2].size() == 3 + 110 );
	CHECK( receivedLTs[0][2] == 1 );
	CHECK( receivedLTs[1][2] == 2 );
	CHECK( receivedLTs[2][2] == 3 );
}

void
TestCreditZeroStall( void )
{
	Setup s;
	s.onWrite = [&s]( const Bytes& inWire )
	{
		for ( const Bytes& frame : Decode( inWire ) )
		{
			if ( IsLT( frame ) )
			{
				s.FeedFrame( LA( frame[2], 0 ) ); // credit stays 0
			}
		}
	};
	Bytes payload( 300, 0x33 );
	s.link.mInitialOutbound = std::make_unique<TDCLDockCmdGeneric>( 'thrt', 300, payload.data() );
	s.ConnectAndRun();

	CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( s.app.mExceptionError == TMNPProtocol::kReasonTimeout );
	const Frames out = s.TakeWrittenFrames();
	CHECK( !out.empty() && out.back() == LD( TMNPProtocol::kReasonTimeout ) );
}

void
TestCleanDisconnectAfterAckAndFallback( void )
{
	// Case 1: Newton ACKs kDDisconnect LT and sends peer LD -> clean exit, no extra LD.
	{
		Setup s;
		s.onWrite = [&s]( const Bytes& inWire )
		{
			for ( const Bytes& frame : Decode( inWire ) )
			{
				if ( IsLT( frame ) )
				{
					s.FeedFrame( LA( frame[2], 1 ) );
					s.FeedFrame( LD( 0 ) );
				}
			}
		};
		s.ConnectAndRun();
		CHECK( s.link.Disconnect() );
		CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
		CHECK( s.link.Wait() );
		CHECK( !s.app.mLinkException );
		const Frames out = s.TakeWrittenFrames();
		CHECK( out.size() == 1 && IsLT( out[0] ) );
	}

	// Case 2: Newton ACKs kDDisconnect LT but never sends LD -> fallback LD(0xff), clean exit.
	{
		Setup s;
		s.onWrite = [&s]( const Bytes& inWire )
		{
			for ( const Bytes& frame : Decode( inWire ) )
			{
				if ( IsLT( frame ) )
				{
					s.FeedFrame( LA( frame[2], 1 ) );
				}
			}
		};
		s.ConnectAndRun();
		CHECK( s.link.Disconnect() );
		CHECK( WaitFor( [&s] { return s.link.mDisconnected.load(); }, ms( 2000 ) ) );
		CHECK( s.link.Wait() );
		CHECK( !s.app.mLinkException );
		const Frames out = s.TakeWrittenFrames();
		CHECK( out.size() == 2 && IsLT( out[0] ) && out[1] == LD( TMNPProtocol::kReasonUser ) );
	}
}

void
TestCancellationUnblocksHandshakeAndReceive( void )
{
	// Cancel during MNP handshake before LR arrives: exits immediately (< stall timeout).
	{
		Setup s;
		s.link.SetMNPStallTimeout( ms( 4000 ) );
		s.CreateSession();
		CHECK( s.link.Connect( s.sessionId, s.bus, true ) );
		std::this_thread::sleep_for( ms( 20 ) );
		const auto t0 = Clock::now();
		CHECK( s.link.RequestStop() );
		CHECK( s.link.Wait() );
		CHECK( Clock::now() - t0 < ms( 500 ) );
	}

	// Cancel while ReceiveCommand() is waiting during DoConnect.
	{
		Setup s;
		s.link.SetMNPStallTimeout( ms( 4000 ) );
		s.link.mReceiveInDoConnect = true;
		s.CreateSession();
		s.FeedFrame( kHardwareLR );
		s.FeedFrame( LA( 0, 1 ) );
		CHECK( s.link.Connect( s.sessionId, s.bus, true ) );
		CHECK( WaitFor( [&s] { return s.link.mDoConnectStarted.load(); }, ms( 2000 ) ) );
		const auto t0 = Clock::now();
		CHECK( s.link.RequestStop() );
		CHECK( s.link.Wait() );
		CHECK( Clock::now() - t0 < ms( 500 ) );
	}
}

void
TestTransportErrorReachesApplication( void )
{
	Setup s;
	s.ConnectAndRun();
	(void)s.bus->PublishError( s.sessionId, std::make_exception_ptr( DCLTimeout ) );
	CHECK( WaitFor( [&s] { return s.app.mLinkException.load(); }, ms( 2000 ) ) );
	CHECK( s.link.Wait() );
	CHECK( s.closed.load() );
}
} // namespace

void
UTestMNPLinkLayer::Run( void )
{
	std::printf( "--- Testing TDCLLink over MNP ---\n" );
	TestPeerLDEndsIdleLink();
	TestProtocolErrorReachesApplication();
	TestDataStillDelivered();
	TestHandshakeStallBeforeAndAfterLR();
	TestRejectedLR();
	TestPeerLDInHandshake();
	TestKeepAlive();
	TestKeepAliveCannotStarveInFlightStall();
	TestMultiLTSegmentationAndCreditRestore();
	TestCreditZeroStall();
	TestCleanDisconnectAfterAckAndFallback();
	TestCancellationUnblocksHandshakeAndReceive();
	TestTransportErrorReachesApplication();
	std::printf( "--- All TDCLLink over MNP tests passed! ---\n" );
}
