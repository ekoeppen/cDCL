// ==============================
// Fichier:			UTestMNPPipe.cpp
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========
//
// Timerless facade scenarios (.topics/mnp-timerless-bdd.md). Feed immediate
// or scheduled Newton frames through a TTestMemoryPipe; decode output with
// an independent wire codec. Stall waits use tens of milliseconds; a watchdog
// bounds the suite. Scheduling happens inside BytesAvailable, without feeder races.

#include "UTestMNPPipe.h"
#include "TMNPPipeStatusTestAccess.h"
#include "TTestMemoryPipe.h"

#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace
{
using Bytes = std::vector<std::uint8_t>;
using Frames = std::vector<Bytes>;
using Clock = std::chrono::steady_clock;
using ms = std::chrono::milliseconds;
constexpr ms kStall( 60 );
static_assert( TDCLMNPPipe::kDefaultStallTimeout == ms( 6000 ), "default stall timeout is 6 s" );

#define CHECK( cond )                                                         \
	do                                                                        \
	{                                                                         \
		if ( !( cond ) )                                                      \
		{                                                                     \
			std::printf( "FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__ ); \
			std::exit( 1 );                                                   \
		}                                                                     \
	} while ( 0 )

// Expects the statement to throw exactly type E.
#define CHECK_THROWS( E, stmt ) \
	do                          \
	{                           \
		bool thrown = false;    \
		try                     \
		{                       \
			stmt;               \
		}                       \
		catch ( const E& )      \
		{                       \
			thrown = true;      \
		}                       \
		CHECK( thrown );        \
	} while ( 0 )

const Bytes kHardwareLR = { 0x26, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02, 0x01, 0x02,
                            0x03, 0x01, 0x08, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03, 0x09, 0x01, 0x01, 0x0e,
                            0x04, 0x03, 0x04, 0x00, 0xfa, 0xc5, 0x06, 0x01, 0x04, 0x00, 0x00, 0xe1, 0x00 };
const Bytes kExpectedReply = { 0x17, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02,
                               0x01, 0x02, 0x03, 0x01, 0x02, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03 };

// ------------------------------------------------------------------------- //
//  Independent wire codec
// ------------------------------------------------------------------------- //

std::uint16_t
CRC( const Bytes& inBody )
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
	for ( auto b : inBody )
	{
		feed( b );
	}
	feed( 0x03 );
	return crc;
}

Bytes
Wire( const Bytes& inBody )
{
	Bytes w = { 0x16, 0x10, 0x02 };
	for ( auto b : inBody )
	{
		w.push_back( b );
		if ( b == 0x10 )
		{
			w.push_back( 0x10 );
		}
	}
	w.push_back( 0x10 );
	w.push_back( 0x03 );
	const std::uint16_t c = CRC( inBody );
	w.push_back( static_cast<std::uint8_t>( c & 0xff ) );
	w.push_back( static_cast<std::uint8_t>( c >> 8 ) );
	return w;
}

// Decodes everything written; fails the test on a malformed frame.
Frames
Decode( const Bytes& inWire )
{
	Frames frames;
	std::size_t i = 0;
	while ( i < inWire.size() )
	{
		while ( i < inWire.size() && inWire[i] == 0x16 )
		{
			++i;
		}
		if ( i == inWire.size() )
		{
			break;
		}
		CHECK( i + 1 < inWire.size() && inWire[i] == 0x10 && inWire[i + 1] == 0x02 );
		i += 2;
		Bytes body;
		for ( ;; )
		{
			CHECK( i < inWire.size() );
			std::uint8_t b = inWire[i++];
			if ( b == 0x10 )
			{
				CHECK( i < inWire.size() );
				b = inWire[i++];
				if ( b == 0x03 )
				{
					break;
				}
				CHECK( b == 0x10 );
			}
			body.push_back( b );
		}
		CHECK( i + 2 <= inWire.size() );
		const std::uint16_t c = static_cast<std::uint16_t>( inWire[i] | ( inWire[i + 1] << 8 ) );
		CHECK( c == CRC( body ) );
		i += 2;
		frames.push_back( body );
	}
	return frames;
}

Bytes
LA( std::uint8_t inSeq, std::uint8_t inCredit )
{
	return { 0x03, 0x05, inSeq, inCredit };
}

Bytes
LT( std::uint8_t inSeq, const Bytes& inData )
{
	Bytes f = { 0x02, 0x04, inSeq };
	f.insert( f.end(), inData.begin(), inData.end() );
	return f;
}

Bytes
LD( std::uint8_t inReason )
{
	return { 0x07, 0x02, 0x01, 0x01, inReason, 0x02, 0x01, 0x00 };
}

bool
IsLT( const Bytes& inFrame )
{
	return inFrame.size() >= 3 && inFrame[1] == 0x04;
}

// ------------------------------------------------------------------------- //
//  Fixture
// ------------------------------------------------------------------------- //

// Supply wire bytes at offsets from Schedule(). The MNP pipe polls
// BytesAvailable in real time, and only that polling thread delivers
// scheduled input once it is due. A broken
// timer restart would consume the entire non-progress script before stalling.
struct Arrival
{
	ms after;
	Bytes wire;
};

class TScriptedPipe : public TTestMemoryPipe
{
public:
	enum class Fault
	{
		kNone,
		kPoll,
		kRead,
		kShortRead,
		kWrite,
		kShortWrite,
		kFlush,
		kConnected,
		kGetTimeout
	};
	Fault fault = Fault::kNone;
	int pollsBeforeFailure = 0;
	bool failDisconnect = false;
	unsigned disconnectCalls = 0;
	unsigned writeCalls = 0;
	std::function<void()> onPoll;

	void
	Read( void* outBuffer, KUInt32* ioCount ) override
	{
		if ( fault == Fault::kRead )
		{
			throw std::runtime_error( "injected read failure" );
		}
		if ( fault == Fault::kShortRead )
		{
			*ioCount = 0;
			return;
		}
		TTestMemoryPipe::Read( outBuffer, ioCount );
	}

	void
	Write( const void* inBuffer, KUInt32* ioCount ) override
	{
		++writeCalls;
		if ( fault == Fault::kWrite )
		{
			throw std::runtime_error( "injected write failure" );
		}
		if ( fault == Fault::kShortWrite )
		{
			--*ioCount;
			return;
		}
		TTestMemoryPipe::Write( inBuffer, ioCount );
	}

	void
	FlushOutput( void ) override
	{
		if ( fault == Fault::kFlush )
		{
			throw std::runtime_error( "injected flush failure" );
		}
		TTestMemoryPipe::FlushOutput();
	}

	void
	Disconnect( void ) override
	{
		++disconnectCalls;
		TTestMemoryPipe::Disconnect();
		if ( failDisconnect )
		{
			throw std::runtime_error( "injected disconnect failure" );
		}
	}

	long
	GetTimeout( void ) override
	{
		if ( fault == Fault::kGetTimeout )
		{
			throw std::runtime_error( "injected timeout-query failure" );
		}
		return TTestMemoryPipe::GetTimeout();
	}

	TDCLCommLayer*
	Connected( TDCLLink* inLink ) override
	{
		if ( fault == Fault::kConnected )
		{
			throw std::runtime_error( "injected attachment failure" );
		}
		return TTestMemoryPipe::Connected( inLink );
	}

	Clock::time_point
	Schedule( std::vector<Arrival> inArrivals )
	{
		mArrivals = std::move( inArrivals );
		mNext = 0;
		mStart = Clock::now();
		return mStart;
	}

	std::size_t
	Delivered( void ) const
	{
		return mNext;
	}

	Boolean
	BytesAvailable( void ) override
	{
		if ( onPoll )
		{
			onPoll();
		}
		if ( fault == Fault::kPoll && pollsBeforeFailure-- == 0 )
		{
			throw std::runtime_error( "injected polling failure" );
		}
		if ( TTestMemoryPipe::BytesAvailable() )
		{
			return true;
		}
		if ( mNext < mArrivals.size() && Clock::now() >= mStart + mArrivals[mNext].after )
		{
			Feed( mArrivals[mNext++].wire );
			return true;
		}
		return false;
	}

private:
	std::vector<Arrival> mArrivals;
	std::size_t mNext = 0;
	Clock::time_point mStart{};
};

// A regression must fail instead of hanging in Read or a broken wait loop.
class Watchdog
{
public:
	Watchdog( void )
	    : mThread(
	          [this]
	          {
		          std::unique_lock<std::mutex> lock( mMutex );
		          if ( !mCond.wait_for( lock, std::chrono::seconds( 20 ), [this] { return mDone; } ) )
		          {
			          std::fputs( "MNP pipe test watchdog expired\n", stderr );
			          std::_Exit( 1 );
		          }
	          } )
	{
	}

	~Watchdog( void )
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mDone = true;
		}
		mCond.notify_all();
		mThread.join();
	}

private:
	std::mutex mMutex;
	std::condition_variable mCond;
	bool mDone = false;
	std::thread mThread;
};

struct Fixture
{
	TScriptedPipe* sub;
	TDCLMNPPipe pipe;

	explicit Fixture( KUInt32 inOutputSize = TDCLMNPPipe::kDefaultOutputMaxBufferSize )
	    : sub( new TScriptedPipe() ), pipe( sub, inOutputSize )
	{
		TMNPPipeStatusTestAccess::SetStallTimeout( pipe, kStall );
	}

	void
	Feed( const Bytes& inBody )
	{
		sub->Feed( Wire( inBody ) );
	}

	// Scripted Newton: for every LT the pipe writes, inReply returns the
	// frames to answer with. Each Write carries exactly one frame.
	void
	OnLT( std::function<Frames( const Bytes& )> inReply )
	{
		sub->SetOnWrite(
		    [this, inReply]( const Bytes& inWire )
		    {
			    for ( const Bytes& frame : Decode( inWire ) )
			    {
				    if ( IsLT( frame ) )
				    {
					    for ( const Bytes& answer : inReply( frame ) )
					    {
						    Feed( answer );
					    }
				    }
			    }
		    } );
	}

	// Acknowledges every LT with credit 1.
	void
	AutoAck( void )
	{
		OnLT( []( const Bytes& inLT ) { return Frames{ LA( inLT[2], 1 ) }; } );
	}

	Frames
	Written( void )
	{
		return Decode( sub->TakeWritten() );
	}

	// H1: the Newton's LR and LA are already waiting.
	void
	Connect( std::uint8_t inCredit = 1 )
	{
		Feed( kHardwareLR );
		Feed( LA( 0, inCredit ) );
		pipe.Connected( nullptr );
		CHECK( Written() == Frames{ kExpectedReply } );
	}
};

Bytes
Pattern( std::size_t inSize, std::uint8_t inSeed )
{
	Bytes data( inSize );
	for ( std::size_t i = 0; i < inSize; ++i )
	{
		data[i] = static_cast<std::uint8_t>( inSeed + i * 13 );
	}
	return data;
}

// Polls the pipe like an idle link thread for inDuration; true as soon as
// payload is available.
bool
PollFor( TDCLMNPPipe& ioPipe, ms inDuration )
{
	const auto until = Clock::now() + inDuration;
	do
	{
		if ( ioPipe.BytesAvailable() )
		{
			return true;
		}
		std::this_thread::sleep_for( ms( 1 ) );
	} while ( Clock::now() < until );
	return false;
}

void
WriteAll( TDCLMNPPipe& ioPipe, const Bytes& inData )
{
	KUInt32 count = static_cast<KUInt32>( inData.size() );
	ioPipe.Write( inData.data(), &count );
	CHECK( count == inData.size() );
}

Bytes
ReadN( TDCLMNPPipe& ioPipe, std::size_t inCount )
{
	Bytes data( inCount );
	KUInt32 count = static_cast<KUInt32>( inCount );
	ioPipe.Read( data.data(), &count );
	CHECK( count == inCount );
	return data;
}

// Failure is sticky and closes the transport before the caller handles it.
void
ExpectFailureState( Fixture& f, std::uint8_t inReason )
{
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kFailed );
	CHECK( !TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == inReason );
	CHECK( !f.pipe.PeerDisconnected() );
	CHECK( f.sub->IsDisconnected() );
	CHECK( f.sub->disconnectCalls == 1 );
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
	CHECK_THROWS( TDCLMNPProtocolException, WriteAll( f.pipe, { 0x01 } ) );
	CHECK_THROWS( TDCLMNPProtocolException, ReadN( f.pipe, 1 ) );
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
	try
	{
		f.pipe.FlushOutput();
		CHECK( false );
	}
	catch ( const TDCLMNPProtocolException& error )
	{
		CHECK( error.GetError() == inReason );
		CHECK( error.GetExCode() == TDCLException::exUnknownError );
	}
	f.pipe.Disconnect(); // never throws, even if cleanup failed
	f.pipe.Disconnect();
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kFailed );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == inReason );
	CHECK( f.sub->disconnectCalls == 1 );
	CHECK( f.Written().empty() );
}

// The last frame is LD(reason), when the transport still permits writing it.
void
ExpectFailed( Fixture& f, std::uint8_t inReason )
{
	const Frames frames = f.Written();
	CHECK( !frames.empty() );
	CHECK( frames.back() == LD( inReason ) );
	ExpectFailureState( f, inReason );
}

// ------------------------------------------------------------------------- //
//  Tests
// ------------------------------------------------------------------------- //

void
TestH1_Handshake( void )
{
	Fixture f;
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kInProgress );
	f.Connect();
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kInProgress );
	CHECK( !f.pipe.PeerDisconnected() );
	CHECK( !f.pipe.BytesAvailable() );
	CHECK( f.Written().empty() );
}

void
TestSingleSessionLifecycle( void )
{
	{
		Fixture f;
		f.Connect();
		WriteAll( f.pipe, { 0x41 } );
		f.pipe.FlushOutput();
		CHECK( f.Written() == Frames{ LT( 1, { 0x41 } ) } );
		// Even a complete second handshake must not reset an active session.
		f.Feed( kHardwareLR );
		f.Feed( LA( 0, 1 ) );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		ExpectFailed( f, 0xfe );
	}
	{
		Fixture f;
		f.pipe.Disconnect();
		CHECK_THROWS( TDCLEOFException, f.pipe.Connected( nullptr ) );
		CHECK( f.Written().empty() );
		CHECK( f.sub->IsDisconnected() );
	}
	{
		Fixture f;
		f.Connect();
		f.pipe.Disconnect();
		CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
		CHECK( f.Written() == Frames{ LD( 0xff ) } );
		CHECK_THROWS( TDCLEOFException, f.pipe.Connected( nullptr ) );
		CHECK( f.Written().empty() );
	}
	{
		Fixture f;
		f.Connect();
		f.Feed( LD( 0x05 ) );
		CHECK( !f.pipe.BytesAvailable() );
		CHECK( f.pipe.PeerDisconnected() );
		CHECK_THROWS( TDCLEOFException, f.pipe.Connected( nullptr ) );
		CHECK( f.pipe.PeerDisconnected() );
		CHECK( f.Written().empty() );
	}
	{
		Fixture f;
		Bytes lr = kHardwareLR;
		lr[13] = 0x01;
		f.Feed( lr );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		ExpectFailed( f, 0x03 ); // Preserve the original failure, do not restart.
	}
}

void
TestH3_RejectedLR( void )
{
	Fixture f;
	Bytes lr = kHardwareLR;
	lr[13] = 0x01; // framing mode 1
	f.Feed( lr );
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
	ExpectFailed( f, 0x03 );
}

void
TestH4_HandshakeStall( void )
{
	for ( bool gotLR : { false, true } )
	{
		Fixture f;
		if ( gotLR )
		{
			f.Feed( kHardwareLR );
		}
		const auto start = Clock::now();
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		CHECK( Clock::now() - start >= kStall );
		ExpectFailed( f, 0x01 );
	}
}

void
TestDisconnectBeforeConnected( void )
{
	// A server may kill an accepted pipe before the link attaches: no MNP
	// frames, but the sub-pipe is still disconnected, exactly once.
	Fixture f;
	f.pipe.Disconnect();
	CHECK( f.Written().empty() );
	CHECK( f.sub->IsDisconnected() );
	f.pipe.Disconnect(); // repeated: no effect, never throws
	CHECK( f.Written().empty() );
}

void
TestHandshakePeerLD( void )
{
	// Decision 1: an LD instead of the handshake is EOF, not an error.
	Fixture f;
	f.Feed( LD( 0x05 ) );
	CHECK_THROWS( TDCLEOFException, f.pipe.Connected( nullptr ) );
	CHECK( f.pipe.PeerDisconnected() );
	CHECK( f.Written().empty() );
}

void
TestH5_Escaping( void )
{
	Fixture f;
	f.Connect();
	const Bytes tricky = { 0x10, 0x10, 0x02, 0x10, 0x03, 0x16, 0x00, 0x10 };
	f.Feed( LT( 1, tricky ) );
	CHECK( ReadN( f.pipe, tricky.size() ) == tricky );
	CHECK( f.Written() == Frames{ LA( 1, 1 ) } );

	WriteAll( f.pipe, tricky );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 1, tricky ) } );
}

void
TestT1_Segmentation( void )
{
	// 600 bytes: 253 + 253 + 94, each LT only after the previous LA.
	Fixture f;
	f.Connect();
	f.AutoAck();
	const Bytes data = Pattern( 600, 7 );
	WriteAll( f.pipe, data );
	f.pipe.FlushOutput();
	const Frames frames = f.Written();
	CHECK( frames.size() == 3 );
	Bytes sent;
	for ( std::size_t i = 0; i < frames.size(); ++i )
	{
		CHECK( IsLT( frames[i] ) && frames[i][2] == i + 1 );
		sent.insert( sent.end(), frames[i].begin() + 3, frames[i].end() );
	}
	CHECK( frames[0].size() == 3 + 253 && frames[2].size() == 3 + 94 );
	CHECK( sent == data );

	// A smaller configured output buffer is respected.
	Fixture small( 100 );
	small.Connect();
	small.AutoAck();
	WriteAll( small.pipe, Pattern( 250, 1 ) );
	small.pipe.FlushOutput();
	const Frames smallFrames = small.Written();
	CHECK( smallFrames.size() == 3 );
	CHECK( smallFrames[0].size() == 103 && smallFrames[2].size() == 53 );
}

void
TestT1_FlushClearsOutput( void )
{
	Fixture f;
	f.Connect();
	const Bytes first = Pattern( 253, 7 );
	WriteAll( f.pipe, first );
	CHECK( f.Written().empty() ); // A full buffer remains buffered until needed.
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 1, first ) } );

	// No LA yet: an empty flush must neither wait nor resend the submitted data.
	f.pipe.FlushOutput();
	CHECK( f.Written().empty() );

	// Reuse the same output buffer while LT(1) is outstanding.
	const Bytes next = { 0x10, 0x42 };
	WriteAll( f.pipe, next );
	CHECK( f.Written().empty() );
	f.Feed( LA( 1, 1 ) );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 2, next ) } );
}

void
TestT1_NoSecondLTBeforeAck( void )
{
	// Even credit 2 cannot permit another LT before LA(1).
	// Write sends the first 253 bytes and buffers the rest; the flush waits.
	for ( std::uint8_t credit : { 1, 2 } )
	{
		Fixture f;
		f.Connect( credit );
		WriteAll( f.pipe, Pattern( 300, 0 ) );
		f.Feed( LA( 0, credit ) ); // Repeated ACK must not release LT(1).
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
		const Frames frames = f.Written();
		CHECK( frames.size() == 2 );
		CHECK( IsLT( frames[0] ) && frames[0][2] == 1 );
		CHECK( frames[1] == LD( 0x04 ) );
	}
}

void
TestT2_InboundDelivery( void )
{
	Fixture f;
	f.Connect();
	const Bytes big = Pattern( 256, 3 );
	f.Feed( LT( 1, big ) );
	f.Feed( LT( 2, { 'x', 'y' } ) );
	Bytes all;
	while ( all.size() < 258 )
	{
		const Bytes part = ReadN( f.pipe, 10 < 258 - all.size() ? 10 : 258 - all.size() );
		all.insert( all.end(), part.begin(), part.end() );
	}
	Bytes expected = big;
	expected.insert( expected.end(), { 'x', 'y' } );
	CHECK( all == expected );
	CHECK( f.Written() == ( Frames{ LA( 1, 1 ), LA( 2, 1 ) } ) );
}

void
TestT3_InboundDuringSendWait( void )
{
	// While waiting for LA(1), an inbound LT is acknowledged and readable.
	Fixture f;
	f.Connect();
	f.OnLT( []( const Bytes& inLT ) { return inLT[2] == 1 ? Frames{ LT( 1, { 0x42 } ), LA( 1, 1 ) } : Frames{}; } );
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	WriteAll( f.pipe, { 0x02 } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == ( Frames{ LT( 1, { 0x01 } ), LA( 1, 1 ), LT( 2, { 0x02 } ) } ) );
	CHECK( ReadN( f.pipe, 1 ) == Bytes{ 0x42 } );
}

void
TestT4_DuplicateLT( void )
{
	Fixture f;
	f.Connect();
	f.Feed( LT( 1, { 0x01 } ) );
	f.Feed( LT( 1, { 0x01 } ) );
	f.Feed( LT( 2, { 0x02 } ) );
	CHECK( ReadN( f.pipe, 2 ) == Bytes( { 0x01, 0x02 } ) );
	CHECK( f.Written() == ( Frames{ LA( 1, 1 ), LA( 1, 1 ), LA( 2, 1 ) } ) );
}

void
TestA1_CreditZeroThenRestored( void )
{
	// LA(1, 0) releases LT(1) but withdraws credit; LA(1, 1) restores it.
	Fixture f;
	f.Connect();
	f.OnLT( []( const Bytes& inLT ) { return inLT[2] == 1 ? Frames{ LA( 1, 0 ), LA( 1, 1 ) } : Frames{}; } );
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	WriteAll( f.pipe, { 0x02 } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == ( Frames{ LT( 1, { 0x01 } ), LT( 2, { 0x02 } ) } ) );
}

void
TestA2_CreditStall( void )
{
	Fixture f;
	f.Connect();
	f.OnLT( []( const Bytes& inLT ) { return inLT[2] == 1 ? Frames{ LA( 1, 0 ) } : Frames{}; } );
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	WriteAll( f.pipe, { 0x02 } );
	// Data stays in the facade; no second LT is submitted without credit.
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
	const Frames frames = f.Written();
	CHECK( frames == ( Frames{ LT( 1, { 0x01 } ), LD( 0x04 ) } ) );
}

void
TestA3_StallStartsAtBlockingCall( void )
{
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	CHECK( !PollFor( f.pipe, 3 * kStall ) );
	WriteAll( f.pipe, { 0x02 } );
	const auto start = Clock::now();
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
	CHECK( Clock::now() - start >= kStall ); // A fresh budget, not the LT's age.
	CHECK( f.Written() == ( Frames{ LT( 1, { 0x01 } ), LD( 0x04 ) } ) );
}

void
TestRead_NoStallWhilePending( void )
{
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	const auto start = f.sub->Schedule( { { 3 * kStall, Wire( LT( 1, { 0x42 } ) ) } } );
	CHECK( ReadN( f.pipe, 1 ) == Bytes{ 0x42 } );
	CHECK( Clock::now() - start >= 3 * kStall );
	CHECK( f.Written() == ( Frames{ LT( 1, { 0x01 } ), LA( 1, 1 ) } ) );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
	// The pending LT was not failed by Read; its LA still releases it.
	f.Feed( LA( 1, 1 ) );
	WriteAll( f.pipe, { 0x02 } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 2, { 0x02 } ) } );
}

void
TestRead_TimeoutIsFatal( void )
{
	Fixture f;
	f.Connect();
	f.pipe.SetTimeout( 1 ); // Pipe timeouts are in whole seconds.
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	f.Written();
	// Already accepted payload contributes to ioCount even on timeout.
	f.Feed( LT( 1, { 0x42 } ) );
	KUInt8 data[2]{};
	KUInt32 count = 2;
	const auto start = Clock::now();
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Read( data, &count ) );
	CHECK( Clock::now() - start >= std::chrono::seconds( 1 ) );
	CHECK( count == 1 && data[0] == 0x42 );
	ExpectFailed( f, 0x04 );
}

void
TestK1_KeepAlive( void )
{
	for ( std::uint8_t credit : { 1, 2 } )
	{
		Fixture f;
		f.Connect( credit );
		f.pipe.SetKeepAlive( true ); // Same ordering as InspectorLink.
		f.Feed( LA( 0, credit ) );
		CHECK( !f.pipe.BytesAvailable() );
		CHECK( f.Written() == Frames{ LA( 0, 1 ) } ); // No application data needed.
		f.Feed( LT( 1, { 0x01 } ) );
		f.Feed( LT( 2, { 0x02 } ) ); // An initial two-LT burst remains acceptable.
		CHECK( ReadN( f.pipe, 2 ) == ( Bytes{ 0x01, 0x02 } ) );
		f.Feed( LA( 0, credit ) );
		CHECK( !f.pipe.BytesAvailable() ); // processes the LA, answers it
		CHECK( f.Written() == ( Frames{ LA( 1, 1 ), LA( 2, 1 ), LA( 2, 1 ) } ) );

		Fixture disabled;
		disabled.Connect( credit );
		disabled.Feed( LA( 0, credit ) );
		CHECK( !disabled.pipe.BytesAvailable() );
		CHECK( disabled.Written().empty() );
	}
}

void
TestX1_BadCRC( void )
{
	Fixture f;
	f.Connect();
	Bytes wire = Wire( LT( 1, { 0x01 } ) );
	wire.back() ^= 0x01;
	f.sub->Feed( wire );
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
	ExpectFailed( f, 0xfe );
}

void
TestX2_UnterminatedFrameHasNoDeadline( void )
{
	Fixture f;
	f.Connect();
	const Bytes wire = Wire( LT( 1, { 0x01 } ) );
	const auto split = wire.end() - 3;
	f.sub->Feed( Bytes( wire.begin(), split ) );
	CHECK( !PollFor( f.pipe, 3 * kStall ) );
	CHECK( f.Written().empty() );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
	f.sub->Feed( Bytes( split, wire.end() ) );
	CHECK( ReadN( f.pipe, 1 ) == Bytes{ 0x01 } );
	CHECK( f.Written() == Frames{ LA( 1, 1 ) } );
}

void
TestX2_MalformedAndOversizedFrames( void )
{
	for ( const Bytes& body : Frames{ { 0x03, 0x05, 0x00 }, LT( 1, Bytes( 257, 0x41 ) ) } )
	{
		Fixture f;
		f.Connect();
		f.Feed( body );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
		ExpectFailed( f, 0xfe );
	}
}

void
TestX6_MidSessionLR( void )
{
	Fixture f;
	f.Connect();
	f.Feed( kHardwareLR );
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
	ExpectFailed( f, 0xfe );
}

// S1 uses legal progress for each phase. Before establishment inbound LTs
// and repeated LAs are H3 negotiation errors, not progress/keepalive frames;
// the handshake analogue is a delayed LR and a split, unfinished frame.
void
TestS1_HandshakeProgress( void )
{
	const ms gap = 2 * kStall / 3;
	{
		Fixture f;
		const auto start = f.sub->Schedule( { { gap, Wire( kHardwareLR ) }, { 2 * gap, Wire( LA( 0, 1 ) ) } } );
		f.pipe.Connected( nullptr );
		CHECK( Clock::now() - start >= 2 * gap ); // Longer than the original budget.
		CHECK( f.Written() == Frames{ kExpectedReply } );
	}
	{
		Fixture f;
		const auto start = f.sub->Schedule( { { gap, Wire( kHardwareLR ) } } );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		CHECK( Clock::now() - start >= gap + kStall );
		ExpectFailed( f, 0x01 );
	}
}

enum class WaitPhase
{
	kFlushAck,
	kFlushCredit,
	kWriteAck,
	kCloseAck,
	kCloseLD
};

constexpr WaitPhase kWaitPhases[] = { WaitPhase::kFlushAck, WaitPhase::kFlushCredit, WaitPhase::kWriteAck,
                                      WaitPhase::kCloseAck, WaitPhase::kCloseLD };

bool
IsClose( WaitPhase inPhase )
{
	return inPhase == WaitPhase::kCloseAck || inPhase == WaitPhase::kCloseLD;
}

void
PrepareWait( Fixture& f, WaitPhase inPhase )
{
	f.Connect();
	if ( inPhase == WaitPhase::kCloseLD )
	{
		return;
	}
	if ( inPhase == WaitPhase::kFlushCredit )
	{
		f.Feed( LA( 0, 0 ) );
		CHECK( !f.pipe.BytesAvailable() );
		WriteAll( f.pipe, { 0x02 } );
		return;
	}
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 1, { 0x01 } ) } );
	if ( inPhase == WaitPhase::kFlushAck )
	{
		WriteAll( f.pipe, { 0x02 } );
	}
	else if ( inPhase == WaitPhase::kWriteAck )
	{
		WriteAll( f.pipe, Bytes( 253, 0x42 ) ); // Next Write must send this buffer.
	}
}

void
RunWait( Fixture& f, WaitPhase inPhase )
{
	if ( IsClose( inPhase ) )
	{
		f.pipe.Disconnect();
	}
	else if ( inPhase == WaitPhase::kWriteAck )
	{
		WriteAll( f.pipe, { 0x43 } );
	}
	else
	{
		f.pipe.FlushOutput();
	}
}

void
TestS1_InboundLTKeepsWaitAlive( void )
{
	const ms gap = 2 * kStall / 3;
	for ( WaitPhase phase : kWaitPhases )
	{
		Fixture f;
		PrepareWait( f, phase );
		Bytes finish;
		if ( phase != WaitPhase::kCloseLD )
		{
			finish = Wire( LA( phase == WaitPhase::kFlushCredit ? 0 : 1, 1 ) );
		}
		if ( IsClose( phase ) )
		{
			const Bytes ld = Wire( LD( 0x00 ) );
			finish.insert( finish.end(), ld.begin(), ld.end() );
		}
		const auto start = f.sub->Schedule( { { gap, Wire( LT( 1, { 0x41 } ) ) },
		                                      { 2 * gap, Wire( LT( 1, { 0x41 } ) ) }, // Duplicate is progress too.
		                                      { 3 * gap, Wire( LT( 2, { 0x42 } ) ) },
		                                      { 4 * gap, finish } } );
		RunWait( f, phase );
		CHECK( Clock::now() - start >= 4 * gap );
		CHECK( f.sub->Delivered() == 4 );
		CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
		Frames expected{ LA( 1, 1 ), LA( 1, 1 ), LA( 2, 1 ) };
		if ( !IsClose( phase ) )
		{
			expected.push_back( phase == WaitPhase::kWriteAck
			                        ? LT( 2, Bytes( 253, 0x42 ) )
			                        : LT( phase == WaitPhase::kFlushCredit ? 1 : 2, { 0x02 } ) );
		}
		else
		{
			CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
			CHECK( f.sub->IsDisconnected() );
		}
		CHECK( f.Written() == expected );
		CHECK( ReadN( f.pipe, 2 ) == Bytes( { 0x41, 0x42 } ) ); // Duplicate delivered only once.
	}
}

// Neither repeated LA/keepalive nor an unfinished frame may reset the wait.
void
TestS1_NonProgressCannotExtendWait( void )
{
	for ( WaitPhase phase : kWaitPhases )
	{
		for ( bool split : { false, true } )
		{
			Fixture f;
			PrepareWait( f, phase );
			f.pipe.SetKeepAlive( true );
			std::vector<Arrival> arrivals;
			const Bytes wire = Wire( LT( 1, Bytes( 32, 0x41 ) ) );
			for ( int i = 0; i < 12; ++i )
			{
				Bytes chunk;
				if ( split )
				{
					chunk = i == 0 ? Bytes( wire.begin(), wire.begin() + 3 ) : Bytes{ wire[i + 2] };
				}
				else
				{
					chunk = Wire( LA( 0, phase == WaitPhase::kFlushCredit ? 0 : i % 2 ) );
				}
				arrivals.push_back( { ( i + 1 ) * kStall / 3, chunk } );
			}
			f.sub->Schedule( arrivals );
			const auto start = Clock::now();
			if ( IsClose( phase ) )
			{
				RunWait( f, phase ); // Disconnect never throws, including an LA stall.
				CHECK( f.sub->IsDisconnected() );
				CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) == ( phase == WaitPhase::kCloseLD ) );
			}
			else
			{
				CHECK_THROWS( TDCLMNPProtocolException, RunWait( f, phase ) );
			}
			const auto elapsed = Clock::now() - start;
			CHECK( elapsed >= kStall && elapsed < 3 * kStall );
			CHECK( f.sub->Delivered() > 0 && f.sub->Delivered() < arrivals.size() );
			const auto reason = phase == WaitPhase::kCloseLD ? 0xff : 0x04;
			CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == reason );
			Frames expected( split ? 0 : f.sub->Delivered(), LA( 0, 1 ) );
			expected.push_back( LD( reason ) );
			CHECK( f.Written() == expected ); // No retransmission and no second LT.
		}
	}
}

void
TestS1_ContinuousLAStreamCannotStarveStall( void )
{
	Fixture f;
	PrepareWait( f, WaitPhase::kFlushAck );
	f.pipe.SetKeepAlive( true );
	f.sub->SetOnWrite(
	    [&f]( const Bytes& wire )
	    {
		    for ( const Bytes& frame : Decode( wire ) )
		    {
			    if ( frame[1] == 0x05 )
			    {
				    f.Feed( LA( 0, 1 ) ); // Every reply immediately produces another LA.
			    }
		    }
	    } );
	f.Feed( LA( 0, 1 ) );
	const auto start = Clock::now();
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
	CHECK( Clock::now() - start >= kStall );
	const Frames frames = f.Written();
	CHECK( frames.size() > 1 && frames.back() == LD( 0x04 ) );
	for ( std::size_t i = 0; i + 1 < frames.size(); ++i )
	{
		CHECK( frames[i] == LA( 0, 1 ) );
	}
}

void
TestS1_PartialHandshakeIsNotProgress( void )
{
	for ( bool gotLR : { false, true } )
	{
		Fixture f;
		if ( gotLR )
		{
			f.Feed( kHardwareLR );
		}
		std::vector<Arrival> arrivals;
		const Bytes wire = Wire( kHardwareLR );
		for ( int i = 0; i < 12; ++i )
		{
			arrivals.push_back(
			    { ( i + 1 ) * kStall / 3, i == 0 ? Bytes( wire.begin(), wire.begin() + 3 ) : Bytes{ wire[i + 2] } } );
		}
		f.sub->Schedule( arrivals );
		const auto start = Clock::now();
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
		CHECK( Clock::now() - start >= kStall && Clock::now() - start < 3 * kStall );
		CHECK( f.sub->Delivered() > 0 && f.sub->Delivered() < arrivals.size() );
		ExpectFailed( f, 0x01 ); // Waiting-state reason, not framing error fe.
	}
}

void
TestS3_NoTimerWhileIdle( void )
{
	for ( std::uint8_t credit : { 0, 1 } )
	{
		Fixture f;
		f.Connect();
		f.Feed( LA( 0, credit ) );
		CHECK( !f.pipe.BytesAvailable() );
		CHECK( !PollFor( f.pipe, 4 * kStall ) );
		CHECK( !f.pipe.BytesAvailable() );
		f.pipe.FlushOutput(); // Empty flush does not start a credit wait.
		CHECK( f.Written().empty() );
		CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
		CHECK( !f.pipe.PeerDisconnected() );
		f.Feed( { 0x07, 0x02, 0x01, 0x01, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00 } );
		CHECK_THROWS( TDCLEOFException, ReadN( f.pipe, 1 ) );
		CHECK( f.pipe.PeerDisconnected() );
		CHECK( f.Written().empty() );
		f.pipe.Disconnect();
		CHECK( f.Written().empty() );
	}
}

void
TestD1_PeerLD( void )
{
	{
		// During a read: accepted data stays readable, then EOF.
		Fixture f;
		f.Connect();
		f.Feed( LT( 1, { 0x11, 0x22 } ) );
		f.Feed( { 0x07, 0x02, 0x01, 0x01, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00 } );
		CHECK( ReadN( f.pipe, 2 ) == Bytes( { 0x11, 0x22 } ) );
		CHECK( f.pipe.PeerDisconnected() );
		CHECK_THROWS( TDCLEOFException, ReadN( f.pipe, 1 ) );
		CHECK_THROWS( TDCLEOFException, WriteAll( f.pipe, { 0x01 } ) );
		CHECK_THROWS( TDCLEOFException, f.pipe.FlushOutput() );
		CHECK( !f.pipe.BytesAvailable() );
		f.pipe.Disconnect();
		CHECK( f.Written() == Frames{ LA( 1, 1 ) } ); // no outbound LD
		CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
	}
	{
		// During a send wait.
		Fixture f;
		f.Connect();
		f.Feed( LD( 0xff ) );
		WriteAll( f.pipe, { 0x01 } );
		f.pipe.FlushOutput();
		WriteAll( f.pipe, { 0x02 } );
		CHECK_THROWS( TDCLEOFException, f.pipe.FlushOutput() );
		CHECK( f.pipe.PeerDisconnected() );
		CHECK( f.Written() == Frames{ LT( 1, { 0x01 } ) } );
	}
}

void
TestD2_DisconnectAfterAck( void )
{
	// The final Dock 'disc' LT is flushed; the LA arrives during Disconnect.
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 'd', 'i', 's', 'c' } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 1, { 'd', 'i', 's', 'c' } ) } );
	const ms gap = 2 * kStall / 3;
	const auto start = f.sub->Schedule( { { gap, Wire( LA( 1, 1 ) ) } } );
	f.pipe.Disconnect();
	CHECK( Clock::now() - start >= gap + kStall ); // LA restarts the LD wait.
	CHECK( f.Written() == Frames{ LD( 0xff ) } );
	CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0xff );
	CHECK( f.sub->IsDisconnected() );
	CHECK( !f.pipe.PeerDisconnected() );
	f.pipe.Disconnect();
	CHECK( f.Written().empty() );
}

void
TestD2_NewtonClosesAfterDisc( void )
{
	// gdcl trace: the Newton acknowledges 'disc' and then sends its own LD
	// (reason 0). cDCL sends no LD of its own.
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 'd', 'i', 's', 'c' } );
	f.pipe.FlushOutput();
	CHECK( f.Written() == Frames{ LT( 1, { 'd', 'i', 's', 'c' } ) } );
	const ms gap = 2 * kStall / 3;
	const auto start =
	    f.sub->Schedule( { { gap, Wire( LA( 1, 1 ) ) },
	                       { 2 * gap, Wire( { 0x07, 0x02, 0x01, 0x01, 0x00, 0x02, 0x01, 0x00, 0x0c, 0xc8 } ) } } );
	f.pipe.Disconnect();
	CHECK( Clock::now() - start >= 2 * gap );
	CHECK( f.Written().empty() ); // no LD
	CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
	CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == 0 );
	CHECK( f.pipe.PeerDisconnected() );
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	CHECK( f.sub->IsDisconnected() );
}

void
TestD2_DisconnectWithoutAck( void )
{
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 0x01 } );
	f.pipe.FlushOutput();
	f.Written();
	f.pipe.Disconnect(); // never throws
	CHECK( f.Written() == Frames{ LD( 0x04 ) } );
	CHECK( !TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
	CHECK( f.sub->IsDisconnected() );
}

void
TestD2_DisconnectFlushesBufferedOutput( void )
{
	Fixture f;
	f.Connect();
	WriteAll( f.pipe, { 0x07 } );
	f.Feed( LA( 1, 1 ) );
	f.pipe.Disconnect();
	CHECK( f.Written() == ( Frames{ LT( 1, { 0x07 } ), LD( 0xff ) } ) );
	CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) );
}

void
TestD2_DisconnectFlushWaitsForCredit( void )
{
	for ( bool restore : { false, true } )
	{
		Fixture f;
		f.Connect();
		f.Feed( LA( 0, 0 ) );
		CHECK( !f.pipe.BytesAvailable() );
		WriteAll( f.pipe, { 0x07 } );
		if ( restore )
		{
			f.sub->Schedule( { { kStall / 2, Wire( LA( 0, 1 ) ) } } );
			f.OnLT( []( const Bytes& lt ) { return Frames{ LA( lt[2], 1 ), LD( 0x00 ) }; } );
		}
		f.pipe.Disconnect();
		CHECK( f.sub->IsDisconnected() );
		CHECK( TMNPPipeStatusTestAccess::IsCleanDisconnect( f.pipe ) == restore );
		CHECK( TMNPPipeStatusTestAccess::GetDisconnectReason( f.pipe ) == ( restore ? 0 : 0x04 ) );
		CHECK( f.Written() == ( restore ? Frames{ LT( 1, { 0x07 } ) } : Frames{ LD( 0x04 ) } ) );
	}
}

void
TestD3_Buffers( void )
{
	Fixture f;
	f.Connect();
	f.pipe.FlushOutput(); // empty flush sends nothing
	CHECK( f.Written().empty() );
	f.Feed( LT( 1, { 0x01, 0x02, 0x03 } ) );
	CHECK( f.pipe.BytesAvailable() );
	ReadN( f.pipe, 2 );
	CHECK( f.pipe.BytesAvailable() );
	ReadN( f.pipe, 1 );
	CHECK( !f.pipe.BytesAvailable() );
}

void
TestTransportReceiveErrors( void )
{
	using Fault = TScriptedPipe::Fault;
	for ( Fault fault : { Fault::kPoll, Fault::kRead, Fault::kShortRead, Fault::kGetTimeout } )
	{
		Fixture f;
		f.Connect();
		f.Feed( LT( 1, { 0x42 } ) );
		f.sub->fault = fault;
		CHECK_THROWS( TDCLMNPProtocolException, ReadN( f.pipe, 1 ) );
		ExpectFailed( f, 0xfe );
	}
	{
		// ServiceTransport's first poll succeeds; ProcessAvailableInput's poll fails.
		Fixture f;
		f.Connect();
		f.Feed( LT( 1, { 0x42 } ) );
		f.sub->fault = Fault::kPoll;
		f.sub->pollsBeforeFailure = 1;
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
		ExpectFailed( f, 0xfe );
	}
	{
		// Even an unlimited idle poll reports a broken transport immediately.
		Fixture f;
		f.Connect();
		f.sub->fault = Fault::kPoll;
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
		ExpectFailed( f, 0xfe );
	}
}

void
TestTransportSendErrors( void )
{
	using Fault = TScriptedPipe::Fault;
	for ( Fault fault : { Fault::kWrite, Fault::kShortWrite, Fault::kFlush } )
	{
		for ( bool cleanupFails : { false, true } )
		{
			Fixture f;
			f.Connect();
			WriteAll( f.pipe, { 0x42 } );
			f.sub->fault = fault;
			f.sub->failDisconnect = cleanupFails;
			const auto writes = f.sub->writeCalls;
			CHECK_THROWS( TDCLMNPProtocolException, f.pipe.FlushOutput() );
			CHECK( f.sub->writeCalls == writes + 1 ); // No retry, not even LD.
			const Frames frames = f.Written();
			CHECK( frames == ( fault == Fault::kFlush ? Frames{ LT( 1, { 0x42 } ) } : Frames{} ) );
			ExpectFailureState( f, 0xfe );
		}
	}
}

void
TestTerminalTransmissionErrors( void )
{
	using Fault = TScriptedPipe::Fault;
	for ( Fault fault : { Fault::kWrite, Fault::kShortWrite, Fault::kFlush } )
	{
		{
			// A failed normal LD must override the optimistic clean terminal state.
			Fixture f;
			f.Connect();
			f.sub->fault = fault;
			f.pipe.Disconnect(); // Never throws, but outcome must be Failed.
			f.Written();
			ExpectFailureState( f, 0xfe );
		}
		{
			// An ACK timeout remains the cause even if sending LD and closing fail.
			Fixture f;
			f.Connect();
			WriteAll( f.pipe, { 0x41 } );
			f.pipe.FlushOutput();
			f.Written();
			f.sub->fault = fault;
			f.sub->failDisconnect = true;
			f.pipe.Disconnect();
			f.Written();
			ExpectFailureState( f, 0x04 );
		}
		{
			// A negotiation failure likewise survives LD transmission failure.
			Fixture f;
			Bytes lr = kHardwareLR;
			lr[13] = 0x01;
			f.Feed( lr );
			f.sub->fault = fault;
			CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
			f.Written();
			ExpectFailureState( f, 0x03 );
		}
	}
}

void
TestCleanupErrors( void )
{
	{
		Fixture f;
		f.sub->failDisconnect = true;
		f.pipe.Disconnect(); // Closing an attached pipe can fail too.
		ExpectFailureState( f, 0xfe );
	}
	{
		Fixture f;
		f.Connect();
		f.sub->failDisconnect = true;
		f.pipe.Disconnect();
		CHECK( f.Written() == Frames{ LD( 0xff ) } );
		ExpectFailureState( f, 0xfe );
	}
	{
		Fixture f;
		f.Connect();
		f.sub->failDisconnect = true;
		f.Feed( LD( 0x05 ) );
		CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
		ExpectFailureState( f, 0xfe );
	}
}

void
TestAttachmentFailure( void )
{
	Fixture f;
	f.Feed( kHardwareLR );
	f.Feed( LA( 0, 1 ) );
	f.sub->fault = TScriptedPipe::Fault::kConnected;
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.Connected( nullptr ) );
	ExpectFailed( f, 0xfe );
}

void
TestNormalOutcomes( void )
{
	for ( bool peerCloses : { false, true } )
	{
		Fixture f;
		f.Connect();
		if ( peerCloses )
		{
			f.Feed( LD( 0x00 ) );
			CHECK( !f.pipe.BytesAvailable() );
			CHECK( f.Written().empty() );
		}
		else
		{
			f.pipe.Disconnect();
			CHECK( f.Written() == Frames{ LD( 0xff ) } );
		}
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
		CHECK( f.sub->IsDisconnected() );
		// Reading a closed session must not query the detached transport.
		f.sub->fault = TScriptedPipe::Fault::kGetTimeout;
		CHECK_THROWS( TDCLEOFException, ReadN( f.pipe, 1 ) );
		f.pipe.Disconnect();
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.Written().empty() );
	}
	Fixture attached;
	attached.pipe.Disconnect();
	CHECK( attached.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
}

void
TestBufferedInputSurvivesFailure( void )
{
	Fixture f;
	f.Connect();
	f.Feed( LT( 1, { 0x42 } ) );
	f.Feed( LA( 99, 1 ) ); // Fatal anomaly after valid payload.
	CHECK_THROWS( TDCLMNPProtocolException, f.pipe.BytesAvailable() );
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kFailed );
	CHECK( f.sub->IsDisconnected() );
	CHECK( f.pipe.BytesAvailable() );
	CHECK( ReadN( f.pipe, 1 ) == Bytes{ 0x42 } );
	ExpectFailed( f, 0xfe );
}

void
RequestStopFromPollingThread( Fixture& f, const std::function<void()>& inAction )
{
	std::mutex m;
	std::condition_variable cv;
	bool polling = false;
	f.sub->onPoll = [&]
	{
		{
			std::lock_guard<std::mutex> lock( m );
			polling = true;
		}
		cv.notify_all();
	};
	std::thread canceller(
	    [&]
	    {
		    std::unique_lock<std::mutex> lock( m );
		    cv.wait( lock, [&] { return polling; } );
		    (void)f.pipe.RequestStop();
	    } );
	inAction();
	canceller.join();
	f.sub->onPoll = nullptr;
}

void
TestCancel_Connected( void )
{
	{
		// Pre-cancelled pipe (or sub-pipe) aborts before sending LR reply.
		Fixture f;
		f.Feed( kHardwareLR );
		f.Feed( LA( 0, 1 ) );
		CHECK( f.pipe.RequestStop() );
		CHECK_THROWS( TDCLUserCancelledException, f.pipe.Connected( nullptr ) );
		CHECK( f.Written().empty() );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
		f.pipe.Disconnect();
		CHECK( f.sub->disconnectCalls == 1 );
	}
	{
		// Sub-pipe stop requested mid-handshake unblocks without waiting for kStall or sending LD.
		Fixture f;
		f.Feed( kHardwareLR );
		f.sub->SetOnWrite( [&]( const Bytes& ) { (void)f.sub->RequestStop(); } );
		const auto t0 = Clock::now();
		CHECK_THROWS( TDCLUserCancelledException, f.pipe.Connected( nullptr ) );
		CHECK( Clock::now() - t0 < kStall );
		CHECK( f.Written() == Frames{ kExpectedReply } );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
}

void
TestCancel_Read( void )
{
	Fixture f;
	f.Connect();
	RequestStopFromPollingThread( f, [&] { CHECK_THROWS( TDCLUserCancelledException, ReadN( f.pipe, 1 ) ); } );
	CHECK( f.Written().empty() );
	CHECK( f.sub->IsDisconnected() );
	CHECK( f.sub->disconnectCalls == 1 );
	CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
}

void
TestCancel_WriteAndFlushOutput( void )
{
	{
		// Cancelled while throttled (credit 0 after LA(1, 0)) waiting to send LT(2).
		Fixture f;
		f.Connect();
		f.OnLT( []( const Bytes& inLT ) { return inLT[2] == 1 ? Frames{ LA( 1, 0 ) } : Frames{}; } );
		WriteAll( f.pipe, { 0x41 } );
		f.pipe.FlushOutput();
		(void)f.Written();
		WriteAll( f.pipe, { 0x42 } );
		RequestStopFromPollingThread( f, [&] { CHECK_THROWS( TDCLUserCancelledException, f.pipe.FlushOutput() ); } );
		CHECK( f.Written().empty() );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
	{
		// Cancelled mid-Write while waiting for the ACK to LT(1) before sending LT(2).
		Fixture f;
		f.Connect();
		f.sub->SetOnWrite( [&]( const Bytes& ) { (void)f.pipe.RequestStop(); } );
		const Bytes twoChunks( 253 * 2 + 1, 0x41 );
		const auto t0 = Clock::now();
		CHECK_THROWS( TDCLUserCancelledException, WriteAll( f.pipe, twoChunks ) );
		CHECK( Clock::now() - t0 < kStall );
		const Frames written = f.Written();
		CHECK( written.size() == 1 );
		CHECK( written[0].size() == 3 + 253 );
		CHECK( written[0][1] == 0x04 );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
	{
		// Write on an already-stopped pipe fails immediately without sending.
		Fixture f;
		f.Connect();
		CHECK( f.pipe.RequestStop() );
		CHECK_THROWS( TDCLUserCancelledException, WriteAll( f.pipe, { 0x41 } ) );
		CHECK( f.Written().empty() );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
}

void
TestCancel_Disconnect( void )
{
	{
		// Pre-cancelled Disconnect skips buffered output and sends no LD.
		Fixture f;
		f.Connect( 1 );
		WriteAll( f.pipe, { 0x41 } );
		CHECK( f.pipe.RequestStop() );
		f.pipe.Disconnect();
		CHECK( f.Written().empty() );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
	{
		// Cancellation arriving during Disconnect's wait exits promptly without sending LD.
		Fixture f;
		f.Connect( 1 );
		WriteAll( f.pipe, { 0x41 } );
		f.sub->SetOnWrite( [&]( const Bytes& ) { (void)f.pipe.RequestStop(); } );
		const auto t0 = Clock::now();
		f.pipe.Disconnect();
		CHECK( Clock::now() - t0 < kStall );
		CHECK( f.Written() == Frames{ LT( 1, { 0x41 } ) } );
		CHECK( f.sub->IsDisconnected() );
		CHECK( f.sub->disconnectCalls == 1 );
		CHECK( f.pipe.GetOutcome() == TDCLMNPPipe::EOutcome::kClosed );
	}
}
} // namespace

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
UTestMNPPipe::Run( void )
{
	Watchdog watchdog;
	std::printf( "--- Testing MNP pipe ---\n" );
	TestH1_Handshake();
	TestSingleSessionLifecycle();
	TestH3_RejectedLR();
	TestH4_HandshakeStall();
	TestDisconnectBeforeConnected();
	TestHandshakePeerLD();
	TestH5_Escaping();
	TestT1_Segmentation();
	TestT1_FlushClearsOutput();
	TestT1_NoSecondLTBeforeAck();
	TestT2_InboundDelivery();
	TestT3_InboundDuringSendWait();
	TestT4_DuplicateLT();
	TestA1_CreditZeroThenRestored();
	TestA2_CreditStall();
	TestA3_StallStartsAtBlockingCall();
	TestRead_NoStallWhilePending();
	TestRead_TimeoutIsFatal();
	TestK1_KeepAlive();
	TestX1_BadCRC();
	TestX2_UnterminatedFrameHasNoDeadline();
	TestX2_MalformedAndOversizedFrames();
	TestX6_MidSessionLR();
	TestS1_HandshakeProgress();
	TestS1_InboundLTKeepsWaitAlive();
	TestS1_NonProgressCannotExtendWait();
	TestS1_ContinuousLAStreamCannotStarveStall();
	TestS1_PartialHandshakeIsNotProgress();
	TestS3_NoTimerWhileIdle();
	TestD1_PeerLD();
	TestD2_DisconnectAfterAck();
	TestD2_NewtonClosesAfterDisc();
	TestD2_DisconnectWithoutAck();
	TestD2_DisconnectFlushesBufferedOutput();
	TestD2_DisconnectFlushWaitsForCredit();
	TestD3_Buffers();
	TestTransportReceiveErrors();
	TestTransportSendErrors();
	TestTerminalTransmissionErrors();
	TestCleanupErrors();
	TestAttachmentFailure();
	TestNormalOutcomes();
	TestBufferedInputSurvivesFailure();
	TestCancel_Connected();
	TestCancel_Read();
	TestCancel_WriteAndFlushOutput();
	TestCancel_Disconnect();
	std::printf( "--- All MNP pipe tests passed! ---\n" );
}
