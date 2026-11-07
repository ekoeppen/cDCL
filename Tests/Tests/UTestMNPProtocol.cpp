// ==============================
// Fichier:			UTestMNPProtocol.cpp
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========
//
// Focused pure-protocol tests for TDCLMNPPipe's wire codec, parameter/frame
// validation matrices, 8-bit sequence wraparound, and hardware trace replay.
// Blocking transport, timeout/stall, keepalive, and disconnect facade behavior
// are covered through the public API in UTestMNPPipe and UTestMNPLinkLayer.

#include "UTestMNPProtocol.h"

#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include "TTestMemoryPipe.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

// Minimal test-only access for pure event/codec verification without timers or threads.
class TMNPPipeTestAccess : private TDCLMNPPipe
{
public:
	using TDCLMNPPipe::Bytes;
	using TDCLMNPPipe::CanSubmit;
	using TDCLMNPPipe::EFrameResult;
	using TDCLMNPPipe::EncodeFrame;
	using TDCLMNPPipe::EState;
	using TDCLMNPPipe::kAwaitingPeerDisconnect;
	using TDCLMNPPipe::kConnected;
	using TDCLMNPPipe::kDisconnected;
	using TDCLMNPPipe::kFailed;
	using TDCLMNPPipe::kInFlight;
	using TDCLMNPPipe::kListening;
	using TDCLMNPPipe::kMaxFrameBodySize;
	using TDCLMNPPipe::kReasonNegotiation;
	using TDCLMNPPipe::kReasonProtocol;
	using TDCLMNPPipe::kThrottled;
	using TDCLMNPPipe::OnFrame;
	using TDCLMNPPipe::PushByte;

	TMNPPipeTestAccess( void ) : TDCLMNPPipe( new TUnusedPipe )
	{
	}

	void
	Start( void )
	{
		Enter( kListening );
	}
	void
	Disconnect( void ) override
	{
		Dispatch( kDisconnect );
	}
	void
	Submit( const std::uint8_t* inData, std::size_t inSize )
	{
		mOutputBuffer.clear();
		if ( inSize != 0 )
		{
			mOutputBuffer.assign( inData, inData + inSize );
		}
		SubmitOutputPayload();
	}
	std::size_t
	Read( std::uint8_t* outData, std::size_t inSize )
	{
		return ReadPayload( outData, inSize );
	}
	bool
	TakeOutbound( Bytes& outFrame )
	{
		if ( mOutputFrames.empty() )
		{
			return false;
		}
		outFrame = std::move( mOutputFrames.front() );
		mOutputFrames.pop_front();
		return true;
	}
	EState
	State( void ) const
	{
		return mState;
	}
	std::size_t
	Available( void ) const
	{
		return mInputBuffer.size();
	}
	std::uint8_t
	LocalReason( void ) const
	{
		return mLocalReason;
	}
	const Bytes&
	DecodedFrame( void ) const
	{
		return mFrameBody;
	}

private:
	// Any accidental transport access makes the deterministic suite fail.
	class TUnusedPipe : public TTestMemoryPipe
	{
	public:
		void
		Read( void*, KUInt32* ) override
		{
			std::abort();
		}
		void
		Write( const void*, KUInt32* ) override
		{
			std::abort();
		}
		void
		FlushOutput( void ) override
		{
			std::abort();
		}
		Boolean
		BytesAvailable( void ) override
		{
			std::abort();
		}
		void
		Disconnect( void ) override
		{
			std::abort();
		}
		long
		GetTimeout( void ) override
		{
			std::abort();
		}
		TDCLCommLayer*
		Connected( TDCLLink* ) override
		{
			std::abort();
		}
	};
};

namespace
{
using Bytes = std::vector<std::uint8_t>;
using L = TMNPPipeTestAccess;

#define CHECK( cond )                                                         \
	do                                                                        \
	{                                                                         \
		if ( !( cond ) )                                                      \
		{                                                                     \
			std::printf( "FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__ ); \
			std::exit( 1 );                                                   \
		}                                                                     \
	} while ( 0 )

// Observed hardware LR (decoded body, 41 bytes).
const Bytes kHardwareLR = { 0x26, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02, 0x01, 0x02,
                            0x03, 0x01, 0x08, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03, 0x09, 0x01, 0x01, 0x0e,
                            0x04, 0x03, 0x04, 0x00, 0xfa, 0xc5, 0x06, 0x01, 0x04, 0x00, 0x00, 0xe1, 0x00 };

// Expected LR reply, written out independently of the pipe's reply constant.
const Bytes kExpectedReply = { 0x17, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02,
                               0x01, 0x02, 0x03, 0x01, 0x02, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03 };

Bytes
LA( std::uint8_t inSeq, std::uint8_t inCredit )
{
	return { 0x03, 0x05, inSeq, inCredit };
}

Bytes
LT( std::uint8_t inSeq, const Bytes& inData )
{
	Bytes frame = { 0x02, 0x04, inSeq };
	frame.insert( frame.end(), inData.begin(), inData.end() );
	return frame;
}

Bytes
LD( std::uint8_t inReason )
{
	return { 0x07, 0x02, 0x01, 0x01, inReason, 0x02, 0x01, 0x00 };
}

// Removes the TLV with the given type from an LR at offset >= 11.
Bytes
WithoutParam( const Bytes& inLR, std::uint8_t inType )
{
	Bytes out( inLR.begin(), inLR.begin() + 11 );
	for ( std::size_t i = 11; i < inLR.size(); )
	{
		const std::size_t next = i + 2 + inLR[i + 1];
		if ( inLR[i] != inType )
		{
			out.insert( out.end(), inLR.begin() + i, inLR.begin() + next );
		}
		i = next;
	}
	return out;
}

Bytes
WithParam( Bytes inLR, const Bytes& inTLV )
{
	inLR.insert( inLR.end(), inTLV.begin(), inTLV.end() );
	return inLR;
}

// Replaces the value of a one-byte parameter.
Bytes
WithValue( Bytes inLR, std::uint8_t inType, std::uint8_t inValue )
{
	for ( std::size_t i = 11; i < inLR.size(); i += 2 + inLR[i + 1] )
	{
		if ( inLR[i] == inType )
		{
			inLR[i + 2] = inValue;
		}
	}
	return inLR;
}

struct Harness
{
	L link;
	std::vector<Bytes> out;

	Harness( void )
	{
		link.Start();
	}

	void
	Feed( const Bytes& inFrame )
	{
		link.OnFrame( inFrame );
		Drain();
	}

	void
	Submit( const Bytes& inData )
	{
		link.Submit( inData.data(), inData.size() );
		Drain();
	}

	void
	Disconnect( void )
	{
		link.Disconnect();
		Drain();
	}

	void
	Drain( void )
	{
		Bytes frame;
		while ( link.TakeOutbound( frame ) )
		{
			out.push_back( frame );
		}
	}

	std::vector<Bytes>
	Take( void )
	{
		std::vector<Bytes> result;
		result.swap( out );
		return result;
	}

	Bytes
	ReadAll( void )
	{
		Bytes data( link.Available() );
		link.Read( data.data(), data.size() );
		return data;
	}

	void
	Establish( std::uint8_t inCredit = 1 )
	{
		Feed( kHardwareLR );
		CHECK( Take() == std::vector<Bytes>{ kExpectedReply } );
		Feed( LA( 0, inCredit ) );
		CHECK( out.empty() );
		CHECK( link.State() == L::kConnected );
	}

	void
	SendAcked( const Bytes& inData )
	{
		Submit( inData );
		const std::vector<Bytes> frames = Take();
		CHECK( frames.size() == 1 );
		Feed( LA( frames[0][2], 1 ) );
		CHECK( link.State() == L::kConnected );
	}
};

void
ExpectFatal( Harness& h, std::uint8_t inReason )
{
	CHECK( h.link.State() == L::kFailed );
	CHECK( h.link.LocalReason() == inReason );
	const std::vector<Bytes> frames = h.Take();
	CHECK( !frames.empty() );
	CHECK( frames.back() == LD( inReason ) );
	for ( const Bytes& f : frames )
	{
		CHECK( f[1] != 0x04 ); // no LT follows an anomaly
	}
	// Nothing further is emitted, whatever arrives.
	h.Feed( LA( 0, 1 ) );
	CHECK( h.out.empty() );
}

// ------------------------------------------------------------------------- //
//  Negotiation parameter matrices (H2, H3)
// ------------------------------------------------------------------------- //

void
TestH2_AdditionalParameters( void )
{
	const Bytes variants[] = {
	    WithoutParam( WithoutParam( WithoutParam( kHardwareLR, 0x09 ), 0x0e ), 0xc5 ),
	    WithParam( WithoutParam( kHardwareLR, 0x0e ), { 0x0e, 0x04, 0x03, 0x04, 0x00, 0xfa } ), // reordered
	    WithParam( kHardwareLR, { 0x7f, 0x02, 0xaa, 0xbb } ),                                   // unknown type
	    WithParam( WithoutParam( kHardwareLR, 0x02 ), { 0x02, 0x01, 0x02 } ),                   // mandatory reordered
	    WithValue( kHardwareLR, 0x03, 0x02 ),                                                 // minimum window
	};
	for ( const Bytes& lr : variants )
	{
		Harness h;
		h.Feed( lr );
		CHECK( h.Take() == std::vector<Bytes>{ kExpectedReply } );
		h.Feed( LA( 0, 1 ) );
		CHECK( h.link.State() == L::kConnected );
	}
}

void
TestH3_RejectedNegotiation( void )
{
	Bytes truncated = kHardwareLR;
	truncated.pop_back();
	Bytes badPrefix = kHardwareLR;
	badPrefix[5] = 0x07;
	const Bytes rejected[] = {
	    WithValue( kHardwareLR, 0x02, 0x01 ),                                       // framing mode
	    WithValue( kHardwareLR, 0x08, 0x02 ),                                       // optimization
	    WithValue( kHardwareLR, 0x03, 0x00 ),                                       // k = 0
	    WithValue( kHardwareLR, 0x03, 0x01 ),                                       // k < fixed window 2
	    WithoutParam( kHardwareLR, 0x04 ),                                          // missing N401
	    WithParam( kHardwareLR, { 0x02, 0x01, 0x02 } ),                             // duplicate
	    WithParam( WithoutParam( kHardwareLR, 0x04 ), { 0x04, 0x02, 0x20, 0x00 } ), // N401 < 64
	    truncated,                                                                  // truncated entry
	    WithParam( kHardwareLR, { 0x7f } ),                                         // leftover byte
	    badPrefix,                                                                  // constant parameter
	    Bytes( kHardwareLR.begin(), kHardwareLR.begin() + 10 ),                     // header too short
	};
	for ( const Bytes& lr : rejected )
	{
		Harness h;
		h.Feed( lr );
		ExpectFatal( h, L::kReasonNegotiation );
	}

	// Only LA(0, credit 1..2) may confirm the reply.
	const Bytes badConfirm[] = { LA( 1, 1 ), LA( 0, 0 ), LA( 0, 3 ), kHardwareLR, LT( 1, { 0x41 } ) };
	for ( const Bytes& f : badConfirm )
	{
		Harness h;
		h.Feed( kHardwareLR );
		h.Take();
		h.Feed( f );
		ExpectFatal( h, L::kReasonNegotiation );
	}
}

// ------------------------------------------------------------------------- //
//  Windowing & 8-bit sequence wraparound (A4, A5)
// ------------------------------------------------------------------------- //

void
TestWindowTwoUsesOneOutstandingLT( void )
{
	Harness h;
	h.Establish( 2 );
	h.Submit( { 0x41 } );
	CHECK( h.Take() == std::vector<Bytes>{ LT( 1, { 0x41 } ) } );
	CHECK( !h.link.CanSubmit() );
	h.Feed( LA( 0, 2 ) ); // Credit 2 on a stale ACK cannot release LT(1).
	CHECK( !h.link.CanSubmit() );
	CHECK( h.Take().empty() );
	h.Feed( LA( 1, 2 ) );
	CHECK( h.link.CanSubmit() );
	h.Submit( { 0x42 } );
	CHECK( h.Take() == std::vector<Bytes>{ LT( 2, { 0x42 } ) } );
	CHECK( !h.link.CanSubmit() );
	h.Feed( LA( 2, 0 ) );
	CHECK( h.link.State() == L::kThrottled );
	h.Feed( LA( 2, 2 ) );
	CHECK( h.link.CanSubmit() );
	CHECK( h.Take().empty() );
}

void
TestA4_SequenceWrap( void )
{
	Harness h;
	h.Establish();
	for ( int i = 1; i <= 255; ++i )
	{
		h.SendAcked( { static_cast<std::uint8_t>( i ) } );
		h.Feed( LT( static_cast<std::uint8_t>( i ), { 0x00 } ) );
		CHECK( h.Take() == std::vector<Bytes>{ LA( static_cast<std::uint8_t>( i ), 1 ) } );
	}
	h.ReadAll();

	// Outbound LT 0 follows LT 255 and is acknowledged by LA(0).
	h.Submit( { 0xaa } );
	const std::vector<Bytes> frames = h.Take();
	CHECK( frames.size() == 1 && frames[0][2] == 0 );
	h.Feed( LA( 0, 1 ) );
	CHECK( h.link.State() == L::kConnected );

	// Inbound LT 0 follows 255; a repeated LT 255 before it is a duplicate.
	h.Feed( LT( 255, { 0x01 } ) );
	CHECK( h.Take() == std::vector<Bytes>{ LA( 255, 1 ) } );
	h.Feed( LT( 0, { 0xbb } ) );
	CHECK( h.Take() == std::vector<Bytes>{ LA( 0, 1 ) } );
	CHECK( h.ReadAll() == Bytes{ 0xbb } );
}

void
TestA5_AcknowledgementsAcrossWrap( void )
{
	for ( int sequence : { 255, 0, 1 } )
	{
		for ( bool closing : { false, true } )
		{
			for ( std::uint8_t credit : { 0, 1, 2 } )
			{
				Harness h;
				h.Establish();
				const int preceding = sequence == 0 ? 255 : sequence - 1;
				for ( int i = 0; i < preceding; ++i )
				{
					h.SendAcked( { 0x41 } );
				}
				h.Submit( { 0x42 } );
				CHECK( h.Take() == std::vector<Bytes>{ LT( sequence, { 0x42 } ) } );
				if ( closing )
				{
					h.Disconnect();
				}
				const auto pending = h.link.State();
				const auto stale = static_cast<std::uint8_t>( sequence - 1 );
				h.Feed( LA( stale, 0 ) );
				h.Feed( LA( stale, 1 ) );
				h.Feed( LA( stale, 2 ) );
				CHECK( h.link.State() == pending );
				CHECK( h.Take().empty() );

				h.Feed( LA( sequence, credit ) );
				const auto acknowledged = closing ? L::kAwaitingPeerDisconnect
				                                 : ( credit ? L::kConnected : L::kThrottled );
				CHECK( h.link.State() == acknowledged );
				h.Feed( LA( sequence, credit ) ); // Repeated current ACK remains valid.
				CHECK( h.link.State() == acknowledged );
				CHECK( h.Take().empty() );
				h.Feed( LA( stale, 1 ) ); // Previous sequence is no longer acceptable.
				ExpectFatal( h, L::kReasonProtocol );
			}
		}
	}
}

// ------------------------------------------------------------------------- //
//  Sequence / credit / LD validation (X3, X4, X5, D1)
// ------------------------------------------------------------------------- //

void
TestX3_UnexpectedSequenceLA( void )
{
	{
		// Future sequence while pending.
		Harness h;
		h.Establish();
		h.Submit( { 0x01 } );
		h.Take();
		h.Feed( LA( 2, 1 ) );
		ExpectFatal( h, L::kReasonProtocol );
	}
	{
		// Nothing pending: any sequence other than the last acknowledged.
		Harness h;
		h.Establish();
		h.SendAcked( { 0x01 } );
		h.Feed( LA( 0, 1 ) );
		ExpectFatal( h, L::kReasonProtocol );
	}
}

void
TestX4_OutOfSequenceLT( void )
{
	{
		Harness h;
		h.Establish();
		h.Feed( LT( 2, { 0x01 } ) );
		ExpectFatal( h, L::kReasonProtocol );
		CHECK( h.link.Available() == 0 );
	}
	{
		// LT(0) before any LT is neither next nor a duplicate.
		Harness h;
		h.Establish();
		h.Feed( LT( 0, { 0x01 } ) );
		ExpectFatal( h, L::kReasonProtocol );
	}
	{
		Harness h;
		h.Establish();
		h.Feed( LT( 1, { 0x01 } ) );
		h.Take();
		h.ReadAll();
		h.Feed( LT( 3, { 0x02 } ) );
		ExpectFatal( h, L::kReasonProtocol );
		CHECK( h.link.Available() == 0 );
	}
}

void
TestX5_CreditOutOfWindow( void )
{
	Harness h;
	h.Establish();
	h.Submit( { 0x01 } );
	h.Take();
	h.Feed( LA( 1, 3 ) );
	ExpectFatal( h, L::kReasonProtocol );
}

void
TestD1_InvalidPeerLD( void )
{
	const Bytes invalid[] = {
	    { 0x04, 0x02, 0x01, 0x01 },                   // truncated reason
	    { 0x05, 0x02, 0x01, 0x01, 0x05 },             // bad indicator
	    { 0x04, 0x02, 0x02, 0x01, 0x05 },             // wrong parameter type
	    { 0x07, 0x02, 0x01, 0x01, 0x05, 0x02, 0x01 }, // truncated user code
	    { 0x07, 0x02, 0x01, 0x01, 0x05, 0x03, 0x01, 0x00 },
	};
	for ( const Bytes& ld : invalid )
	{
		Harness h;
		h.Establish();
		h.Feed( ld );
		ExpectFatal( h, L::kReasonProtocol );
	}
}

// ------------------------------------------------------------------------- //
//  End-to-end trace replay (E1)
// ------------------------------------------------------------------------- //

void
TestE1_TraceReplay( void )
{
	// Shape of the hardware install trace: a 14,812-byte package stream sent
	// in 59 LTs (58 of 253 bytes), three small Dock commands before it and the
	// final Dock 'disc' LT (sequence 0x3f): 63 LTs in total. The Newton sends
	// 4 LTs, each acknowledged once. After the final LT is acknowledged the
	// Newton closes with its own LD (gdcl trace). Payload contents are synthetic.
	Harness h;
	h.Establish();

	int ltCount = 0, fullCount = 0, laOut = 0, inboundLT = 0;
	Bytes sent, expected;
	auto sendChunk = [&]( const Bytes& inChunk )
	{
		h.Submit( inChunk );
		const std::vector<Bytes> frames = h.Take();
		CHECK( frames.size() == 1 && frames[0][1] == 0x04 );
		++ltCount;
		CHECK( frames[0][2] == static_cast<std::uint8_t>( ltCount ) );
		fullCount += ( inChunk.size() == 253 );
		sent.insert( sent.end(), frames[0].begin() + 3, frames[0].end() );
		expected.insert( expected.end(), inChunk.begin(), inChunk.end() );
		// The Newton answers the three Dock commands and the package with an LT.
		if ( ltCount <= 3 || ltCount == 62 )
		{
			h.Feed( LT( static_cast<std::uint8_t>( ++inboundLT ), { 0x12, 0x34 } ) );
			for ( const Bytes& f : h.Take() )
			{
				CHECK( f == LA( static_cast<std::uint8_t>( inboundLT ), 1 ) );
				++laOut;
			}
		}
		h.Feed( LA( static_cast<std::uint8_t>( ltCount ), 1 ) );
		CHECK( h.link.State() == L::kConnected );
	};

	for ( int i = 0; i < 3; ++i )
	{
		sendChunk( Bytes( 20, static_cast<std::uint8_t>( 'a' + i ) ) );
	}
	Bytes package( 14812 );
	for ( std::size_t i = 0; i < package.size(); ++i )
	{
		package[i] = static_cast<std::uint8_t>( i * 7 );
	}
	for ( std::size_t offset = 0; offset < package.size(); offset += 253 )
	{
		const std::size_t n = std::min<std::size_t>( 253, package.size() - offset );
		sendChunk( Bytes( package.begin() + offset, package.begin() + offset + n ) );
	}

	// Final Dock 'disc' LT: Disconnect() while its LA is in flight.
	const Bytes disc = { 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 'd', 'i', 's', 'c', 0, 0, 0, 0 };
	h.Submit( disc );
	std::vector<Bytes> frames = h.Take();
	CHECK( frames.size() == 1 && frames[0] == LT( 0x3f, disc ) );
	++ltCount;
	h.Disconnect();
	CHECK( h.Take().empty() );
	h.Feed( LA( 0x3f, 1 ) );
	CHECK( h.Take().empty() );
	h.Feed( { 0x07, 0x02, 0x01, 0x01, 0x00, 0x02, 0x01, 0x00, 0x0c, 0xc8 } );
	CHECK( h.Take().empty() );

	CHECK( ltCount == 63 );
	CHECK( fullCount == 58 );
	CHECK( laOut == 4 );
	CHECK( sent == expected );
	CHECK( h.ReadAll().size() == 8 );
	CHECK( h.link.State() == L::kDisconnected );
}

// ------------------------------------------------------------------------- //
//  Wire framing (H5, X1, X2)
// ------------------------------------------------------------------------- //

// Independent CRC-16/ARC reference encoder following the spec.
Bytes
ReferenceEncode( const Bytes& inBody )
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
	Bytes wire = { 0x16, 0x10, 0x02 };
	for ( auto b : inBody )
	{
		feed( b );
		wire.push_back( b );
		if ( b == 0x10 )
		{
			wire.push_back( 0x10 );
		}
	}
	feed( 0x03 );
	wire.push_back( 0x10 );
	wire.push_back( 0x03 );
	wire.push_back( static_cast<std::uint8_t>( crc & 0xff ) );
	wire.push_back( static_cast<std::uint8_t>( crc >> 8 ) );
	return wire;
}

struct Pushed
{
	std::vector<Bytes> frames;
	std::size_t errors = 0;
};

Pushed
PushAll( L& ioLink, const Bytes& inWire )
{
	Pushed result;
	for ( auto b : inWire )
	{
		switch ( ioLink.PushByte( b ) )
		{
		case L::EFrameResult::kFrame:
			result.frames.push_back( ioLink.DecodedFrame() );
			break;
		case L::EFrameResult::kError:
			++result.errors;
			break;
		case L::EFrameResult::kNone:
			break;
		}
	}
	return result;
}

void
ExpectFramingError( const Bytes& inWire )
{
	L link;
	link.Start();
	const Pushed p = PushAll( link, inWire );
	// Trailing bytes of a broken frame may cause further framing errors.
	CHECK( p.frames.empty() );
	CHECK( p.errors >= 1 );
	CHECK( link.State() == L::kFailed );
	CHECK( link.LocalReason() == L::kReasonProtocol );
}

void
TestFraming_Encode( void )
{
	// The reference encoder is checked against a known LA frame on the wire.
	CHECK( ReferenceEncode( { 0x03, 0x05, 0x00, 0x01 } ).size() == 4 + 7 );
	const Bytes bodies[] = {
	    { 0x03, 0x05, 0x00, 0x01 },
	    kHardwareLR,
	    { 0x02, 0x04, 0x01, 0x10, 0x10, 0x02, 0x10, 0x03, 0x16, 0x10, 0x00, 0x10 }, // H5
	};
	for ( const Bytes& body : bodies )
	{
		CHECK( L::EncodeFrame( body ) == ReferenceEncode( body ) );
	}
}

void
TestFraming_Decode( void )
{
	// H5 escaping, the largest LT (T2), leading SYN fill and frames split
	// across byte boundaries back to back.
	Bytes maxLT = { 0x02, 0x04, 0x01 };
	for ( int i = 0; i < 256; ++i )
	{
		maxLT.push_back( static_cast<std::uint8_t>( i ) );
	}
	CHECK( maxLT.size() == L::kMaxFrameBodySize );
	const Bytes escaped = { 0x02, 0x04, 0x01, 0x10, 0x10, 0x02, 0x10, 0x03, 0x16, 0x10, 0x00, 0x10 };
	Bytes wire = { 0x16, 0x16 };
	for ( const Bytes* body : { &escaped, static_cast<const Bytes*>( &maxLT ) } )
	{
		const Bytes w = ReferenceEncode( *body );
		wire.insert( wire.end(), w.begin(), w.end() );
	}
	L link;
	const Pushed p = PushAll( link, wire );
	CHECK( p.errors == 0 );
	CHECK( p.frames == ( std::vector<Bytes>{ escaped, maxLT } ) );
}

void
TestFraming_Errors( void )
{
	// X1: bad CRC and a corrupted body. X2: oversize, bad start, bad escape,
	// empty body. Every framing error is fatal (LD 0xfe).
	Bytes badCRC = ReferenceEncode( kHardwareLR );
	badCRC.back() ^= 0x01;
	Bytes corrupted = ReferenceEncode( { 0x03, 0x05, 0x00, 0x01 } );
	corrupted[5] ^= 0x40;
	const Bytes errors[] = {
	    badCRC,
	    corrupted,
	    ReferenceEncode( Bytes( L::kMaxFrameBodySize + 1, 0x41 ) ),
	    { 0x41 },
	    { 0x10, 0x41 },
	    { 0x16, 0x10, 0x02, 0x03, 0x10, 0x41 },
	    { 0x16, 0x10, 0x02, 0x10, 0x03 },
	};
	for ( const Bytes& wire : errors )
	{
		ExpectFramingError( wire );
	}
}

void
TestFraming_ResynchronizesAfterFatalError( void )
{
	// The decoder still finds the next frame, but the link stays failed.
	const Bytes la = { 0x03, 0x05, 0x01, 0x01 };
	Bytes wire = ReferenceEncode( la );
	wire.back() ^= 0x01;
	const Bytes good = ReferenceEncode( la );
	wire.insert( wire.end(), good.begin(), good.end() );
	L link;
	link.Start();
	const Pushed p = PushAll( link, wire );
	CHECK( p.errors == 1 );
	CHECK( p.frames == std::vector<Bytes>{ la } );
	CHECK( link.State() == L::kFailed );
	CHECK( link.LocalReason() == L::kReasonProtocol );
}
} // namespace

// ------------------------------------------------------------------------- //
//  * Run( void )
// ------------------------------------------------------------------------- //
void
UTestMNPProtocol::Run( void )
{
	std::printf( "--- Testing MNP protocol ---\n" );
	TestH2_AdditionalParameters();
	TestH3_RejectedNegotiation();
	TestWindowTwoUsesOneOutstandingLT();
	TestA4_SequenceWrap();
	TestA5_AcknowledgementsAcrossWrap();
	TestX3_UnexpectedSequenceLA();
	TestX4_OutOfSequenceLT();
	TestX5_CreditOutOfWindow();
	TestD1_InvalidPeerLD();
	TestE1_TraceReplay();
	TestFraming_Encode();
	TestFraming_Decode();
	TestFraming_Errors();
	TestFraming_ResynchronizesAfterFatalError();
	std::printf( "--- All MNP protocol tests passed! ---\n" );
}
