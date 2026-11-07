// ==============================
// Fichier:			TDCLMNPPipe.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			27/01/2002
// Tabulation:		4 espaces
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
// The Original Code is TDCLMNPPipe.cpp
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 1998-2004
// the Initial Developers All Rights Reserved.
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLMNPPipe.h>

#undef KERROR_ENABLED
#if defined(error_TDCLMNPPipe) && error_TDCLMNPPipe
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLMNPPipe) && trace_TDCLMNPPipe
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLMNPPipe) && debug_TDCLMNPPipe
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

// ISO C++
#include <algorithm>
#include <chrono>
#include <thread>
#include <utility>

#if KDebugOn && KDEBUG_ENABLED
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#endif

// DCL
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>

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

/// Interval between two BytesAvailable() polls of the sub-pipe while waiting.
constexpr std::chrono::milliseconds kPollInterval( 1 );
/// Bound each receive batch so a continuously readable peer cannot starve
/// WaitFor's stall check (repeated LAs/partial frames are not progress).
constexpr std::size_t kMaxInputBatchBytes = 4096;
} // namespace

const TDCLMNPPipe::Bytes TDCLMNPPipe::kLRReply = { 0x17, 0x01, 0x02, 0x01, 0x06, 0x01, 0x00, 0x00,
                                                   0x00, 0x00, 0xff, 0x02, 0x01, 0x02, 0x03, 0x01,
                                                   0x02, 0x04, 0x02, 0x40, 0x00, 0x08, 0x01, 0x03 };

// ------------------------------------------------------------------------- //
//  Construction and configuration
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * TDCLMNPPipe( TDCLPipe*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::TDCLMNPPipe( TDCLPipe* inSubPipe, KUInt32 inOutputMaxBufferSize /* = kDefaultOutputMaxBufferSize */ )
    : TDCLPipeProxy( inSubPipe ),
      mOutputPayloadLimit( std::max<KUInt32>( 1, std::min<KUInt32>( inOutputMaxBufferSize, kMaxOutputPayloadSize ) ) )
{
	mOutputBuffer.reserve( mOutputPayloadLimit );
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMNPPipe( void )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::~TDCLMNPPipe( void )
{
}

void
TDCLMNPPipe::SetKeepAlive( Boolean inKeepAlive )
{
	mKeepAlive = inKeepAlive;
}

// ------------------------------------------------------------------------- //
//  Public pipe operations
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink* )
// ------------------------------------------------------------------------- //
TDCLCommLayer*
TDCLMNPPipe::Connected( TDCLLink* inLink )
{
	ThrowIfCancelled();
	if ( mPhase != EPhase::kAttached )
	{
		CheckUsable();
		Abort( kReasonProtocol );
	}
	mPhase = EPhase::kRunning;
	WaitFor( [this] { return IsConnected(); } );
	ThrowIfCancelled();
	CheckUsable();

	// Starts the sub-pipe's watcher thread.
	try
	{
		return TDCLPipeProxy::Connected( inLink );
	}
	catch ( const TDCLUserCancelledException& )
	{
		mOutputFrames.clear();
		CloseTransport();
		throw;
	}
	catch ( ... )
	{
		Abort( kReasonProtocol );
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	const KUInt32 theWanted = *ioCount;
	KUInt32 theDone = 0;
	*ioCount = 0;

	long theTimeout = 0;
	if ( !IsTerminal() && mPhase != EPhase::kDetached )
	{
		try
		{
			theTimeout = GetTimeout();
		}
		catch ( ... )
		{
			Abort( kReasonProtocol );
		}
	}
	const TimePoint theDeadline = theTimeout > 0 ? Clock::now() + std::chrono::seconds( theTimeout ) : TimePoint::max();

	for ( ;; )
	{
		ThrowIfCancelled();
		theDone += (KUInt32)ReadPayload( static_cast<KUInt8*>( outBuffer ) + theDone, theWanted - theDone );
		*ioCount = theDone;
		if ( theDone == theWanted )
		{
			return;
		}
		CheckUsable();
		if ( Clock::now() >= theDeadline )
		{
			Abort( kReasonTimeout );
		}
		ServiceTransport( theDeadline );
	}
}

// ------------------------------------------------------------------------- //
//	* Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	const KUInt32 theWanted = *ioCount;
	KUInt32 theDone = 0;
	*ioCount = 0;
	ThrowIfCancelled();
	CheckUsable();

	const KUInt8* theBytes = static_cast<const KUInt8*>( inBuffer );
	while ( theDone < theWanted )
	{
		if ( mOutputBuffer.size() == mOutputPayloadLimit )
		{
			SendOutputPayload();
		}
		const KUInt32 theCount =
		    std::min<KUInt32>( theWanted - theDone, mOutputPayloadLimit - (KUInt32)mOutputBuffer.size() );
		mOutputBuffer.insert( mOutputBuffer.end(), theBytes + theDone, theBytes + theDone + theCount );
		theDone += theCount;
		*ioCount = theDone;
	}
}

// ------------------------------------------------------------------------- //
//	* FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::FlushOutput( void )
{
	ThrowIfCancelled();
	CheckUsable();
	if ( !mOutputBuffer.empty() )
	{
		SendOutputPayload();
	}
}

// ------------------------------------------------------------------------- //
//	* BytesAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLMNPPipe::BytesAvailable( void )
{
	if ( mInputBuffer.empty() && !IsTerminal() && mPhase == EPhase::kRunning )
	{
		ServiceTransport( Clock::now() );
	}
	if ( !mInputBuffer.empty() )
	{
		return true;
	}
	CheckFailed();
	return false;
}

// ------------------------------------------------------------------------- //
//  * Disconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Disconnect( void )
{
	if ( mPhase == EPhase::kDetached )
	{
		return;
	}

	const bool theCancelled = IsStopRequested();
	if ( !theCancelled )
	{
		try
		{
			if ( mPhase == EPhase::kRunning && !IsTerminal() )
			{
				if ( !mOutputBuffer.empty() )
				{
					SendOutputPayload();
				}
				// Wait for the final ACK, then the peer LD; silence after ACK is normal.
				Dispatch( kDisconnect );
				ServiceOutput();
				WaitFor( [this] { return !IsDisconnecting(); } );
			}
			ServiceOutput();
		}
		catch ( const TDCLUserCancelledException& )
		{
			mOutputFrames.clear();
		}
		catch ( ... )
		{
			// Never throw during teardown, but never turn an error into success.
			try
			{
				Fail( kReasonProtocol );
				ServiceOutput();
			}
			catch ( ... )
			{
			}
		}
	}
	else
	{
		mOutputFrames.clear();
	}
	(void) TDCLPipe::RequestStop();
	CloseTransport();
}

// ------------------------------------------------------------------------- //
//  Public session status
// ------------------------------------------------------------------------- //

TDCLMNPPipe::EOutcome
TDCLMNPPipe::GetOutcome( void ) const
{
	if ( mState == kFailed )
	{
		return EOutcome::kFailed;
	}
	return IsTerminal() || mPhase == EPhase::kDetached ? EOutcome::kClosed : EOutcome::kInProgress;
}

// ------------------------------------------------------------------------- //
//  * PeerDisconnected( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLMNPPipe::PeerDisconnected( void )
{
	// A peer LD may end an idle session or complete local teardown. A clean
	// local terminal state with no local LD reason is the latter case.
	return mState == kDisconnectedByPeer || ( mState == kDisconnected && mLocalReason == 0 );
}

// ------------------------------------------------------------------------- //
//  Transport orchestration (I/O and blocking waits)
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//	* SendOutputPayload( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SendOutputPayload( void )
{
	WaitFor( [this] { return CanSubmit(); } );
	CheckUsable();
	SubmitOutputPayload();
	ServiceOutput();
}

// ------------------------------------------------------------------------- //
//  * WaitFor( Done )
// ------------------------------------------------------------------------- //
template<class Done>
void
TDCLMNPPipe::WaitFor( Done inDone )
{
	std::uint32_t theSeen = mProgress;
	TimePoint theDeadline = Clock::now() + mStallTimeout;
	while ( !inDone() && !IsTerminal() )
	{
		ThrowIfCancelled();
		if ( mProgress != theSeen )
		{
			theSeen = mProgress;
			theDeadline = Clock::now() + mStallTimeout;
		}
		if ( Clock::now() >= theDeadline && IsStallable() )
		{
			OnStall();
			ServiceOutput();
			continue;
		}
		ServiceTransport( theDeadline );
	}
}

// ------------------------------------------------------------------------- //
//  * ServiceTransport( TimePoint )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::ServiceTransport( TimePoint inLimit )
{
	for ( ;; )
	{
		ThrowIfCancelled();
		if ( mPhase == EPhase::kDetached )
		{
			break;
		}
		Boolean hasData = false;
		try
		{
			hasData = GetSubPipe()->BytesAvailable();
		}
		catch ( const TDCLUserCancelledException& )
		{
			mOutputFrames.clear();
			CloseTransport();
			throw;
		}
		catch ( ... )
		{
			Abort( kReasonProtocol );
		}
		if ( hasData )
		{
			ProcessAvailableInput();
			break;
		}
		const TimePoint theNow = Clock::now();
		if ( theNow >= inLimit )
		{
			break;
		}
		std::this_thread::sleep_for( std::min<Clock::duration>( kPollInterval, inLimit - theNow ) );
	}

	ServiceOutput();
}

// ------------------------------------------------------------------------- //
//  * ProcessAvailableInput( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::ProcessAvailableInput( void )
{
	try
	{
		for ( std::size_t theBytes = 0;
		      theBytes < kMaxInputBatchBytes && !IsTerminal() && !IsStopRequested() && GetSubPipe()->BytesAvailable();
		      ++theBytes )
		{
			KUInt8 theByte;
			KUInt32 theCount = 1;
			GetSubPipe()->Read( &theByte, &theCount );
			if ( theCount != 1 )
			{
				Abort( kReasonProtocol );
			}

			switch ( PushByte( theByte ) )
			{
			case EFrameResult::kFrame:
			{
				const Bytes& theFrameBody = mFrameBody;
				LogFrame( EFrameDirection::kIn, theFrameBody.data(), (KUInt32)theFrameBody.size() );
				break;
			}
			case EFrameResult::kError:
			{
				const Bytes& thePartial = mFrameBody;
				LogFrame( EFrameDirection::kError, thePartial.data(), (KUInt32)thePartial.size() );
				break;
			}
			case EFrameResult::kNone:
				break;
			}
			// Answer as soon as possible (LA for an LT, LR reply).
			if ( !mOutputFrames.empty() )
			{
				ServiceOutput();
			}
		}
	}
	catch ( const TDCLUserCancelledException& )
	{
		mOutputFrames.clear();
		CloseTransport();
		throw;
	}
	catch ( const TDCLMNPProtocolException& )
	{
		if ( mState != kFailed )
		{
			Abort( kReasonProtocol );
		}
		throw;
	}
	catch ( ... )
	{
		Abort( kReasonProtocol );
	}
}

// ------------------------------------------------------------------------- //
//  * ServiceOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::ServiceOutput( void )
{
	Boolean theSent = false;
	try
	{
		while ( !mOutputFrames.empty() )
		{
			ThrowIfCancelled();
			Bytes theFrameBody = std::move( mOutputFrames.front() );
			mOutputFrames.pop_front();
			if ( mPhase == EPhase::kDetached )
			{
				continue;
			}
			const Bytes theWireBytes = EncodeFrame( theFrameBody );
			KUInt32 theCount = (KUInt32)theWireBytes.size();
			GetSubPipe()->Write( theWireBytes.data(), &theCount );
			if ( theCount != theWireBytes.size() )
			{
				throw DCLMNPProtocolError( kReasonProtocol );
			}
			LogFrame( EFrameDirection::kOut, theFrameBody.data(), (KUInt32)theFrameBody.size() );
			theSent = true;
		}
		if ( theSent )
		{
			GetSubPipe()->FlushOutput();
		}
	}
	catch ( const TDCLUserCancelledException& )
	{
		mOutputFrames.clear();
		CloseTransport();
		throw;
	}
	catch ( ... )
	{
		Fail( kReasonProtocol );
		mOutputFrames.clear();
		CloseTransport();
		throw DCLMNPProtocolError( mLocalReason );
	}
	if ( IsTerminal() )
	{
		CloseTransport();
	}
	CheckFailed();
}

// ------------------------------------------------------------------------- //
//  Failure handling and transport cleanup
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * CheckUsable( void ) const
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::CheckUsable( void ) const
{
	CheckFailed();
	if ( IsTerminal() || mPhase == EPhase::kDetached )
	{
		throw DCLEOF;
	}
}

// ------------------------------------------------------------------------- //
//  * CheckFailed( void ) const
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::CheckFailed( void ) const
{
	if ( mState == kFailed )
	{
		throw DCLMNPProtocolError( mLocalReason );
	}
}

bool
TDCLMNPPipe::IsStopRequested( void )
{
	return StopRequested() || ( GetSubPipe() && GetSubPipe()->StopRequested() );
}

void
TDCLMNPPipe::ThrowIfCancelled( void )
{
	if ( mPhase != EPhase::kDetached && !IsTerminal() && IsStopRequested() )
	{
		mOutputFrames.clear();
		CloseTransport();
		throw DCLUserCancelled;
	}
}

[[noreturn]] void
TDCLMNPPipe::Abort( std::uint8_t inReason )
{
	Fail( inReason );
	ServiceOutput();
	throw DCLMNPProtocolError( mLocalReason );
}

// ------------------------------------------------------------------------- //
//  * CloseTransport( void ) noexcept
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::CloseTransport( void ) noexcept
{
	if ( mPhase == EPhase::kDetached )
	{
		return;
	}
	mPhase = EPhase::kDetached;
	try
	{
		GetSubPipe()->Disconnect();
	}
	catch ( ... )
	{
		// No more I/O is possible. Preserve an earlier failure and its reason.
		if ( mState != kFailed )
		{
			Enter( kFailed );
			mLocalReason = kReasonProtocol;
		}
	}
}

// ------------------------------------------------------------------------- //
//  MNP protocol processing
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * OnFrame( const Bytes& )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::OnFrame( const Bytes& inFrame )
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
}

// ------------------------------------------------------------------------- //
//  * OnFramingError( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::OnFramingError( void )
{
	Dispatch( kAnomaly );
}

// ------------------------------------------------------------------------- //
//  * OnStall( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::OnStall( void )
{
	if ( IsStallable() )
	{
		Dispatch( kTimeout );
	}
}

// ------------------------------------------------------------------------- //
//  * SubmitOutputPayload( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SubmitOutputPayload( void )
{
	if ( mOutputBuffer.empty() || mOutputBuffer.size() > kMaxOutputPayloadSize )
	{
		Dispatch( kAnomaly );
		return;
	}
	Dispatch( kSubmit );
	mOutputBuffer.clear();
}

// ------------------------------------------------------------------------- //
//  * ReadPayload( std::uint8_t*, std::size_t )
// ------------------------------------------------------------------------- //
std::size_t
TDCLMNPPipe::ReadPayload( std::uint8_t* outData, std::size_t inSize )
{
	const std::size_t count = std::min( inSize, mInputBuffer.size() );
	std::copy_n( mInputBuffer.begin(), count, outData );
	mInputBuffer.erase( mInputBuffer.begin(), mInputBuffer.begin() + static_cast<std::ptrdiff_t>( count ) );
	return count;
}

// ------------------------------------------------------------------------- //
//  * HandleLT( const Bytes& )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::HandleLT( const Bytes& inFrame )
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

// ------------------------------------------------------------------------- //
//  * Classify( const Bytes& ) const
// ------------------------------------------------------------------------- //
TDCLMNPPipe::EEvent
TDCLMNPPipe::Classify( const Bytes& inFrame ) const
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

// ------------------------------------------------------------------------- //
//  * IsValidLR( const Bytes& )
// ------------------------------------------------------------------------- //
bool
TDCLMNPPipe::IsValidLR( const Bytes& inFrame )
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

// ------------------------------------------------------------------------- //
//  * IsValidLD( const Bytes& )
// ------------------------------------------------------------------------- //
bool
TDCLMNPPipe::IsValidLD( const Bytes& inFrame )
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

// ------------------------------------------------------------------------- //
//  * LastAckedSequence( void ) const
// ------------------------------------------------------------------------- //
std::uint8_t
TDCLMNPPipe::LastAckedSequence( void ) const
{
	return mState == kInFlight || mState == kDisconnecting ? static_cast<std::uint8_t>( mSendSeq - 1 ) : mSendSeq;
}

// ------------------------------------------------------------------------- //
//  * Dispatch( EEvent )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Dispatch( EEvent inEvent )
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

// ------------------------------------------------------------------------- //
//  * Enter( EState )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Enter( EState inState )
{
	if ( inState != mState )
	{
		mState = inState;
		if ( inState == kFailed )
		{
			mOutputFrames.clear();
			mOutputBuffer.clear();
		}
		++mProgress;
	}
}

// ------------------------------------------------------------------------- //
//  * Perform( EAction )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Perform( EAction inAction )
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

// ------------------------------------------------------------------------- //
//  * Fail( std::uint8_t )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Fail( std::uint8_t inReason )
{
	if ( mState != kFailed )
	{
		Enter( kFailed );
		QueueLD( inReason );
	}
}

// ------------------------------------------------------------------------- //
//  * QueueLT( const Bytes& )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::QueueLT( const Bytes& inPayload )
{
	mSendSeq = static_cast<std::uint8_t>( mSendSeq + 1 );
	Bytes frameBody{ 0x02, kTypeLT, mSendSeq };
	frameBody.insert( frameBody.end(), inPayload.begin(), inPayload.end() );
	mOutputFrames.push_back( std::move( frameBody ) );
}

// ------------------------------------------------------------------------- //
//  * QueueLA( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::QueueLA( void )
{
	mOutputFrames.push_back( Bytes{ 0x03, kTypeLA, mRecvSeq, 0x01 } );
}

// ------------------------------------------------------------------------- //
//  * QueueLD( std::uint8_t )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::QueueLD( std::uint8_t inReason )
{
	mOutputFrames.push_back( Bytes{ 0x07, kTypeLD, 0x01, 0x01, inReason, 0x02, 0x01, 0x00 } );
	mLocalReason = inReason;
}

// ------------------------------------------------------------------------- //
//  Framing
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
//  * PushByte( std::uint8_t )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::EFrameResult
TDCLMNPPipe::PushByte( std::uint8_t inByte )
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

// ------------------------------------------------------------------------- //
//  * EncodeFrame( const std::uint8_t*, std::size_t )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::Bytes
TDCLMNPPipe::EncodeFrame( const std::uint8_t* inBody, std::size_t inSize )
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

// ------------------------------------------------------------------------- //
//  * UpdateCRC( std::uint16_t, std::uint8_t )
// ------------------------------------------------------------------------- //
std::uint16_t
TDCLMNPPipe::UpdateCRC( std::uint16_t inCRC, std::uint8_t inByte )
{
	inCRC = static_cast<std::uint16_t>( inCRC ^ inByte );
	for ( int bit = 0; bit < 8; ++bit )
	{
		inCRC = ( inCRC & 1 ) ? static_cast<std::uint16_t>( ( inCRC >> 1 ) ^ 0xa001 )
		                      : static_cast<std::uint16_t>( inCRC >> 1 );
	}
	return inCRC;
}

// ------------------------------------------------------------------------- //
//  Structural validation and diagnostics
// ------------------------------------------------------------------------- //

void
TDCLMNPPipe::CheckTransitionTable( void )
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

	// Every non-terminal state handles a peer LD and a local Disconnect.
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

// ------------------------------------------------------------------------- //
//  * LogFrame( EFrameDirection, const KUInt8*, KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::LogFrame( EFrameDirection inDirection, const KUInt8* inFrame, KUInt32 inSize )
{
#if KDebugOn && KDEBUG_ENABLED
	static std::mutex sLogMutex;
	std::lock_guard<std::mutex> lock( sLogMutex );

	const char* label = inDirection == EFrameDirection::kIn    ? "in"
	                    : inDirection == EFrameDirection::kOut ? "out"
	                                                           : "ERROR, partial";

	const auto now = std::chrono::system_clock::now();
	const auto seconds = std::chrono::system_clock::to_time_t( now );
	std::tm local{};
#if TARGET_OS_WIN32
	localtime_s( &local, &seconds );
#else
	localtime_r( &seconds, &local );
#endif
	const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>( now.time_since_epoch() ).count() % 1000;

	std::string out;
	out.reserve( 64 + ( ( inSize + 15 ) / 16 ) * 80 );

	char line[128];
	std::snprintf( line, sizeof( line ), "[%02d:%02d:%02d.%03d] [MNP %s] %u bytes\n", local.tm_hour, local.tm_min,
	               local.tm_sec, static_cast<int>( millis ), label, static_cast<unsigned>( inSize ) );
	out += line;

	for ( KUInt32 offset = 0; offset < inSize; offset += 16 )
	{
		int n = std::snprintf( line, sizeof( line ), "%08x  ", static_cast<unsigned>( offset ) );
		for ( KUInt32 i = 0; i < 16; ++i )
		{
			if ( offset + i < inSize )
			{
				n += std::snprintf( line + n, sizeof( line ) - n, "%02x ", static_cast<unsigned>( inFrame[offset + i] ) );
			}
			else
			{
				n += std::snprintf( line + n, sizeof( line ) - n, "   " );
			}
			if ( i == 7 )
			{
				line[n++] = ' ';
			}
		}
		line[n++] = ' ';
		line[n++] = '|';
		for ( KUInt32 i = offset; i < inSize && i < offset + 16; ++i )
		{
			const KUInt8 byte = inFrame[i];
			line[n++] = ( byte >= 0x20 && byte < 0x7f ) ? static_cast<char>( byte ) : '.';
		}
		line[n++] = '|';
		line[n++] = '\n';
		line[n] = '\0';
		out += line;
	}

	KDPRINTF1( "%s", out.c_str() );
#else
	(void) inDirection;
	(void) inFrame;
	(void) inSize;
#endif
}

// ========================================================================== //
// ===  ALL USERS PLEASE NOTE  ========================                       //
//                                                                            //
// Compiler optimizations have been made to macro expand LET into a WITHOUT-  //
// INTERRUPTS special form so that it can PUSH things into a stack in the     //
// LET-OPTIMIZATION area, SETQ the variables and then POP them back when it's //
// done.  Don't worry about this unless you use multiprocessing.              //
// Note that LET *could* have been defined by:                                //
//                                                                            //
//         (LET ((LET '`(LET ((LET ',LET))                                    //
//                         ,LET)))                                            //
//         `(LET ((LET ',LET))                                                //
//                 ,LET))                                                     //
//                                                                            //
// This is believed to speed up execution by as much as a factor of 1.01 or   //
// 3.50 depending on whether you believe our friendly marketing               //
// representatives.  This code was written by a new programmer here (we       //
// snatched him away from Itty Bitti Machines where we was writting COUGHBOL  //
// code) so to give him confidence we trusted his vows of "it works pretty    //
// well" and installed it.                                                    //
// ========================================================================== //
