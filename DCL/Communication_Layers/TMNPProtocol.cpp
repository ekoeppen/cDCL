// ==============================
// File:			TMNPProtocol.cpp
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
// The Original Code is TMNPProtocol.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// Filip Zawadiak and Nicolas Zinovieff. Portions created by the Initial
// Developers are Copyright (C) 1998-2004 the Initial Developers. All Rights
// Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org> (state machine rewrite)
//   Protocol state machine portions Copyright (C) 2026 Eckhart Koeppen.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TMNPProtocol.h>

// ISO C++
#include <algorithm>
#include <iterator>
#include <utility>

namespace
{
constexpr std::uint8_t kTypeLR = 1;
constexpr std::uint8_t kTypeLD = 2;
constexpr std::uint8_t kTypeLT = 4;
constexpr std::uint8_t kTypeLA = 5;
constexpr std::uint8_t kSYN = 0x16;
constexpr std::uint8_t kDLE = 0x10;
constexpr std::uint8_t kSTX = 0x02;
constexpr std::uint8_t kETX = 0x03;
} // namespace

const TMNPProtocol::Bytes TMNPProtocol::kLRReply = {
    0x17, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02,
    0x01, 0x02, 0x03, 0x01, 0x02, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03 };

TMNPProtocol::TMNPProtocol( std::size_t inOutputMaxPayloadSize )
    : mOutputPayloadLimit( std::max<std::size_t>( 1, std::min<std::size_t>( inOutputMaxPayloadSize, kMaxOutputPayloadSize ) ) )
{
	mOutputBuffer.reserve( mOutputPayloadLimit );
}

void
TMNPProtocol::Reset( void )
{
	mState = kListening;
	mProgress = 0;
	mFrameState = EFrameState::kIdle;
	mFrameBody.clear();
	mFrameCRC = 0;
	mReceivedCRC = 0;
	mSendSeq = 0;
	mRecvSeq = 0;
	mHaveInbound = false;
	mOutputFrames.clear();
	mInputBuffer.clear();
	mLocalReason = 0;
	mOutputBuffer.clear();
	mPendingOutbound.clear();
}

void
TMNPProtocol::Start( void )
{
	Enter( kListening );
}

TMNPProtocol::EOutcome
TMNPProtocol::GetOutcome( bool inDetached ) const noexcept
{
	if ( mState == kFailed )
	{
		return EOutcome::kFailed;
	}
	return IsTerminal() || inDetached ? EOutcome::kClosed : EOutcome::kInProgress;
}

void
TMNPProtocol::PushWireBytes( const std::uint8_t* inData, std::size_t inSize )
{
	for ( std::size_t i = 0; i < inSize; ++i )
	{
		(void)PushByte( inData[i] );
	}
}

void
TMNPProtocol::OnFrame( const Bytes& inFrame )
{
	if ( IsTerminal() )
	{
		return; // Ignore frames after shutdown
	}

	if ( IsConnected() && inFrame.size() >= 2 && inFrame[1] == kTypeLT )
	{
		HandleLT( inFrame );
		return;
	}

	const EEvent event = Classify( inFrame );
	Dispatch( event );

	// Answer every valid inbound LA while keepalive is enabled.
	if ( mKeepAlive && IsConnected() && ( event == kAckWithCredit || event == kAckNoCredit || event == kRepeatedAck ) )
	{
		QueueLA();
	}
	PumpOutbound();
}

void
TMNPProtocol::OnFramingError( void )
{
	Dispatch( kAnomaly );
}

void
TMNPProtocol::OnStall( void )
{
	if ( IsStallable() )
	{
		Dispatch( kTimeout );
	}
}

void
TMNPProtocol::RequestDisconnect( void )
{
	PumpOutbound();
	Dispatch( kDisconnect );
}

void
TMNPProtocol::SubmitOutputPayload( void )
{
	if ( mOutputBuffer.empty() || mOutputBuffer.size() > kMaxOutputPayloadSize )
	{
		Dispatch( kAnomaly );
		return;
	}
	Dispatch( kSubmit );
	mOutputBuffer.clear();
}

void
TMNPProtocol::QueueOutboundBytes( const std::uint8_t* inData, std::size_t inSize )
{
	if ( inSize == 0 || IsTerminal() )
	{
		return;
	}
	mPendingOutbound.insert( mPendingOutbound.end(), inData, inData + inSize );
	PumpOutbound();
}

bool
TMNPProtocol::PumpOutbound( void )
{
	if ( !CanSubmit() || mPendingOutbound.empty() )
	{
		return false;
	}
	const std::size_t chunk = std::min( mOutputPayloadLimit, mPendingOutbound.size() );
	mOutputBuffer.assign( mPendingOutbound.begin(), mPendingOutbound.begin() + static_cast<std::ptrdiff_t>( chunk ) );
	mPendingOutbound.erase( mPendingOutbound.begin(), mPendingOutbound.begin() + static_cast<std::ptrdiff_t>( chunk ) );
	SubmitOutputPayload();
	return true;
}

bool
TMNPProtocol::TakeOutboundFrame( Bytes& outFrameBody )
{
	if ( mOutputFrames.empty() )
	{
		return false;
	}
	outFrameBody = std::move( mOutputFrames.front() );
	mOutputFrames.pop_front();
	return true;
}

TMNPProtocol::Bytes
TMNPProtocol::TakeAllOutboundWireBytes( void )
{
	Bytes wire;
	Bytes body;
	while ( TakeOutboundFrame( body ) )
	{
		const Bytes encoded = EncodeFrame( body );
		wire.insert( wire.end(), encoded.begin(), encoded.end() );
	}
	return wire;
}

std::size_t
TMNPProtocol::ReadPayload( std::uint8_t* outData, std::size_t inSize )
{
	const std::size_t count = std::min( inSize, mInputBuffer.size() );
	std::copy_n( mInputBuffer.begin(), count, outData );
	mInputBuffer.erase( mInputBuffer.begin(), mInputBuffer.begin() + static_cast<std::ptrdiff_t>( count ) );
	return count;
}

TMNPProtocol::Bytes
TMNPProtocol::TakeAllInput( void )
{
	Bytes out( mInputBuffer.begin(), mInputBuffer.end() );
	mInputBuffer.clear();
	return out;
}

void
TMNPProtocol::HandleLT( const Bytes& inFrame )
{
	if ( inFrame[0] != 0x02 || inFrame.size() < 4 || inFrame.size() > 3 + 256 )
	{
		Dispatch( kAnomaly );
		return;
	}

	const std::uint8_t seq = inFrame[2];
	if ( seq == static_cast<std::uint8_t>( mRecvSeq + 1 ) )
	{
		mRecvSeq = seq;
		mHaveInbound = true;
		mInputBuffer.insert( mInputBuffer.end(), inFrame.begin() + 3, inFrame.end() );
	}
	else if ( !mHaveInbound || seq != mRecvSeq )
	{
		Dispatch( kAnomaly ); // X4
		return;
	}
	QueueLA();
	++mProgress;
}

TMNPProtocol::EEvent
TMNPProtocol::Classify( const Bytes& inFrame ) const
{
	if ( inFrame.size() < 2 )
	{
		return kAnomaly;
	}
	const std::uint8_t type = inFrame[1];

	if ( mState == kListening )
	{
		if ( type == kTypeLR )
		{
			return IsValidLR( inFrame ) ? kRequest : kHandshakeBad;
		}
		if ( type == kTypeLD && IsValidLD( inFrame ) )
		{
			return kPeerDisconnect;
		}
		return kHandshakeBad;
	}

	if ( mState == kNegotiating )
	{
		if ( inFrame.size() == 4 && inFrame[0] == 0x03 && type == kTypeLA && inFrame[2] == 0 &&
		     inFrame[3] >= 1 && inFrame[3] <= kNegotiatedWindowSize )
		{
			return kHandshakeAck;
		}
		if ( type == kTypeLD && IsValidLD( inFrame ) )
		{
			return kPeerDisconnect;
		}
		return kHandshakeBad;
	}

	if ( type == kTypeLD )
	{
		return IsValidLD( inFrame ) ? kPeerDisconnect : kAnomaly;
	}
	if ( type != kTypeLA || inFrame.size() != 4 || inFrame[0] != 0x03 )
	{
		return kAnomaly;
	}

	const std::uint8_t seq = inFrame[2];
	const std::uint8_t credit = inFrame[3];
	if ( credit > kNegotiatedWindowSize )
	{
		return kAnomaly;
	}
	const EEvent ack = credit ? kAckWithCredit : kAckNoCredit;

	if ( mState == kInFlight || mState == kDisconnecting )
	{
		if ( seq == mSendSeq )
		{
			return ack;
		}
		if ( seq == LastAckedSequence() )
		{
			return kRepeatedAck;
		}
		return kAnomaly;
	}

	return seq == LastAckedSequence() ? ack : kAnomaly;
}

bool
TMNPProtocol::IsValidLR( const Bytes& inFrame )
{
	static const std::uint8_t kPrefix[] = { 0x02, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff };
	if ( inFrame.size() < 11 || inFrame[1] != kTypeLR ||
	     !std::equal( std::begin( kPrefix ), std::end( kPrefix ), inFrame.begin() + 2 ) )
	{
		return false;
	}

	bool framing = false, window = false, n401 = false, optimization = false;
	std::size_t i = 11;
	while ( i < inFrame.size() )
	{
		if ( inFrame.size() - i < 2 )
		{
			return false; // leftover byte
		}
		const std::uint8_t type = inFrame[i];
		const std::uint8_t length = inFrame[i + 1];
		const std::size_t value = i + 2;
		if ( length > inFrame.size() - value )
		{
			return false; // truncated entry
		}
		switch ( type )
		{
		case 0x02: // framing mode: octet-oriented
			if ( framing || length != 1 || inFrame[value] != 0x02 )
			{
				return false;
			}
			framing = true;
			break;
		case 0x03: // k: require support for our fixed negotiated window.
			if ( window || length != 1 || inFrame[value] < kNegotiatedWindowSize )
			{
				return false;
			}
			window = true;
			break;
		case 0x04: // N401, little-endian
			if ( n401 || length != 2 || ( inFrame[value] | ( inFrame[value + 1] << 8 ) ) < 64 )
			{
				return false;
			}
			n401 = true;
			break;
		case 0x08: // data phase optimization
			if ( optimization || length != 1 || inFrame[value] != 0x03 )
			{
				return false;
			}
			optimization = true;
			break;
		default: // well-formed unrecognised parameters are skipped
			break;
		}
		i = value + length;
	}
	return framing && window && n401 && optimization;
}

bool
TMNPProtocol::IsValidLD( const Bytes& inFrame )
{
	if ( inFrame.size() < 5 || inFrame[1] != kTypeLD )
	{
		return false;
	}
	const std::uint8_t indicator = inFrame[0];
	if ( indicator != 4 && indicator != 7 )
	{
		return false;
	}
	if ( inFrame[2] != 0x01 || inFrame[3] != 0x01 )
	{
		return false;
	}
	if ( indicator == 7 && ( inFrame.size() < 8 || inFrame[5] != 0x02 || inFrame[6] != 0x01 ) )
	{
		return false;
	}
	return true;
}

std::uint8_t
TMNPProtocol::LastAckedSequence( void ) const
{
	return mState == kInFlight || mState == kDisconnecting ? static_cast<std::uint8_t>( mSendSeq - 1 ) : mSendSeq;
}

void
TMNPProtocol::Dispatch( EEvent inEvent )
{
	if ( IsTerminal() )
	{
		return;
	}
	const Transition* row = Lookup( mState, inEvent );
	if ( row == nullptr )
	{
		Fail( kReasonProtocol ); // X1-X6: no matching row
		return;
	}
	Enter( row->next );
	Perform( row->action );
}

void
TMNPProtocol::Enter( EState inState )
{
	if ( inState != mState )
	{
		mState = inState;
		if ( inState == kFailed )
		{
			mOutputFrames.clear();
			mOutputBuffer.clear();
			mPendingOutbound.clear();
		}
		++mProgress;
	}
}

void
TMNPProtocol::Perform( EAction inAction )
{
	switch ( inAction )
	{
	case kNone:
		break;
	case kQueueLRReply:
		mOutputFrames.push_back( kLRReply );
		break;
	case kQueueLT:
		QueueLT( mOutputBuffer );
		break;
	case kQueueLDUser:
		QueueLD( kReasonUser );
		break;
	case kQueueLDTimeout:
		QueueLD( kReasonTimeout );
		break;
	case kQueueLDHandshake:
		QueueLD( kReasonHandshakeTimeout );
		break;
	case kQueueLDNegotiation:
		QueueLD( kReasonNegotiation );
		break;
	}
}

void
TMNPProtocol::Fail( std::uint8_t inReason )
{
	if ( mState != kFailed )
	{
		Enter( kFailed );
		QueueLD( inReason );
	}
}

void
TMNPProtocol::QueueLT( const Bytes& inPayload )
{
	mSendSeq = static_cast<std::uint8_t>( mSendSeq + 1 );
	Bytes frameBody{ 0x02, kTypeLT, mSendSeq };
	frameBody.insert( frameBody.end(), inPayload.begin(), inPayload.end() );
	mOutputFrames.push_back( std::move( frameBody ) );
}

void
TMNPProtocol::QueueLA( void )
{
	mOutputFrames.push_back( Bytes{ 0x03, kTypeLA, mRecvSeq, 0x01 } );
}

void
TMNPProtocol::QueueLD( std::uint8_t inReason )
{
	mOutputFrames.push_back( Bytes{ 0x07, kTypeLD, 0x01, 0x01, inReason, 0x02, 0x01, 0x00 } );
	mLocalReason = inReason;
}

TMNPProtocol::EFrameResult
TMNPProtocol::PushByte( std::uint8_t inByte )
{
	auto fail = [this]
	{
		// Retain the partial body for logging; it is cleared at the next STX.
		mFrameState = EFrameState::kIdle;
		OnFramingError();
		return EFrameResult::kError;
	};

	switch ( mFrameState )
	{
	case EFrameState::kIdle:
		if ( inByte == kSYN )
		{
			return EFrameResult::kNone;
		}
		if ( inByte == kDLE )
		{
			mFrameState = EFrameState::kStartDLE;
			return EFrameResult::kNone;
		}
		return fail();

	case EFrameState::kStartDLE:
		if ( inByte != kSTX )
		{
			return fail();
		}
		mFrameBody.clear();
		mFrameCRC = 0;
		mFrameState = EFrameState::kBody;
		return EFrameResult::kNone;

	case EFrameState::kBody:
		if ( inByte == kDLE )
		{
			mFrameState = EFrameState::kBodyDLE;
			return EFrameResult::kNone;
		}
		break;

	case EFrameState::kBodyDLE:
		if ( inByte == kETX )
		{
			if ( mFrameBody.empty() )
			{
				return fail();
			}
			mFrameCRC = UpdateCRC( mFrameCRC, kETX );
			mFrameState = EFrameState::kCRCLow;
			return EFrameResult::kNone;
		}
		if ( inByte != kDLE )
		{
			return fail();
		}
		mFrameState = EFrameState::kBody;
		break;

	case EFrameState::kCRCLow:
		mReceivedCRC = inByte;
		mFrameState = EFrameState::kCRCHigh;
		return EFrameResult::kNone;

	case EFrameState::kCRCHigh:
		mReceivedCRC = static_cast<std::uint16_t>( mReceivedCRC | ( inByte << 8 ) );
		mFrameState = EFrameState::kIdle;
		if ( mReceivedCRC != mFrameCRC )
		{
			return fail();
		}
		OnFrame( mFrameBody );
		return EFrameResult::kFrame;
	}

	if ( mFrameBody.size() >= kMaxFrameBodySize )
	{
		return fail();
	}
	mFrameBody.push_back( inByte );
	mFrameCRC = UpdateCRC( mFrameCRC, inByte );
	return EFrameResult::kNone;
}

TMNPProtocol::Bytes
TMNPProtocol::EncodeFrame( const std::uint8_t* inBody, std::size_t inSize )
{
	Bytes wireBytes;
	wireBytes.reserve( inSize + 8 );
	wireBytes.push_back( kSYN );
	wireBytes.push_back( kDLE );
	wireBytes.push_back( kSTX );

	std::uint16_t crc = 0;
	for ( std::size_t i = 0; i < inSize; ++i )
	{
		const std::uint8_t byte = inBody[i];
		crc = UpdateCRC( crc, byte );
		wireBytes.push_back( byte );
		if ( byte == kDLE )
		{
			wireBytes.push_back( kDLE );
		}
	}

	wireBytes.push_back( kDLE );
	wireBytes.push_back( kETX );
	crc = UpdateCRC( crc, kETX );
	wireBytes.push_back( static_cast<std::uint8_t>( crc & 0xff ) );
	wireBytes.push_back( static_cast<std::uint8_t>( crc >> 8 ) );
	return wireBytes;
}

std::uint16_t
TMNPProtocol::UpdateCRC( std::uint16_t inCRC, std::uint8_t inByte )
{
	inCRC = static_cast<std::uint16_t>( inCRC ^ inByte );
	for ( int bit = 0; bit < 8; ++bit )
	{
		inCRC = ( inCRC & 1 ) ? static_cast<std::uint16_t>( ( inCRC >> 1 ) ^ 0xa001 )
		                      : static_cast<std::uint16_t>( inCRC >> 1 );
	}
	return inCRC;
}

void
TMNPProtocol::CheckTransitionTable( void )
{
	constexpr auto NoDuplicateRows = []( void )
	{
		for ( std::size_t i = 0; i < kRowCount; ++i )
		{
			for ( std::size_t j = i + 1; j < kRowCount; ++j )
			{
				if ( kTable[i].state == kTable[j].state && kTable[i].event == kTable[j].event )
				{
					return false;
				}
			}
		}
		return true;
	};

	constexpr auto NoRowFor = []( EEvent inEvent )
	{
		for ( const Transition& row : kTable )
		{
			if ( row.event == inEvent )
			{
				return false;
			}
		}
		return true;
	};

	constexpr auto NoRowsFromTerminalStates = []( void )
	{
		for ( const Transition& row : kTable )
		{
			if ( kStates[row.state].terminal )
			{
				return false;
			}
		}
		return true;
	};

	static_assert( NoDuplicateRows(), "each (state, event) pair has at most one row" );
	static_assert( NoRowFor( kAnomaly ), "an anomaly is always fatal (no matching row)" );
	static_assert( NoRowsFromTerminalStates(), "terminal states skip the table" );

	constexpr auto StatesIndexed = []( void )
	{
		for ( std::size_t i = 0; i < kStateCount; ++i )
		{
			if ( kStates[i].state != i )
			{
				return false;
			}
		}
		return true;
	};

	static_assert( StatesIndexed(), "state attributes are indexed by EState" );

	constexpr auto HandlesLDAndDisconnect = []( void )
	{
		for ( const StateInfo& state : kStates )
		{
			if ( !state.terminal && ( Lookup( state.state, kPeerDisconnect ) == nullptr ||
			                          Lookup( state.state, kDisconnect ) == nullptr ) )
			{
				return false;
			}
		}
		return true;
	};
	static_assert( HandlesLDAndDisconnect(), "D1, D2 in every non-terminal state" );
}
