// ==============================
// File:			TMNPProtocol.h
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
// The Original Code is TMNPProtocol.h.
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

#ifndef __TMNPPROTOCOL__
#define __TMNPPROTOCOL__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

class TDCLMNPProtocolException
	:
		public TDCLIOException
{
public:
#if kDCLDebugFILEandLINE
	TDCLMNPProtocolException( const char* inFileName, KUInt32 inLine, KSInt32 inError = 0 )
		: TDCLIOException( exUnknownError, inError, inFileName, inLine )
	{
	}
#else
	TDCLMNPProtocolException( KSInt32 inError = 0 )
		: TDCLIOException( exUnknownError, inError )
	{
	}
#endif

	virtual const char*
	GetExceptionName( void ) const override
	{
		return "protocol error, link dropped";
	}
};

#if kDCLDebugFILEandLINE
	#define DCLMNPProtocolError( inReason ) \
		TDCLMNPProtocolException( __FILE__, __LINE__, inReason )
#else
	#define DCLMNPProtocolError( inReason ) \
		TDCLMNPProtocolException( inReason )
#endif

// ISO C++
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

///
/// Pure non-blocking MNP protocol state machine and wire codec for the Newton
/// serial link. Performs no I/O, locking, or clock access.
///
class TMNPProtocol
{
public:
	using Bytes = std::vector<std::uint8_t>;

	static constexpr std::size_t kDefaultOutputMaxPayloadSize = 253;
	/// Largest LT payload cDCL sends (T1: 1-253 bytes; the Newton rejects more).
	static constexpr std::size_t kMaxOutputPayloadSize = 253;
	/// Largest decoded frame body: a 256-byte LT plus its 3-byte header.
	static constexpr std::size_t kMaxFrameBodySize = 259;
	/// Negotiated window size (enables Newton Inspector keepalive LAs).
	static constexpr std::uint8_t kNegotiatedWindowSize = 2;

	/// PushByte() results. Every framing error is fatal (LD 0xfe).
	enum class EFrameResult : std::uint8_t
	{
		kNone,
		kFrame,
		kError
	};

	enum class EOutcome
	{
		kInProgress,
		kClosed,
		kFailed
	};

	/// Link states, named for what is true while in them.
	enum EState : std::uint8_t
	{
		kListening,              ///< Started; waiting for the Newton's LR.
		kNegotiating,            ///< LR reply sent; waiting for handshake LA(0, credit 1..2).
		kConnected,              ///< No LT outstanding, positive credit; can submit one LT.
		kThrottled,              ///< No LT outstanding, credit 0; output waits for credit.
		kInFlight,               ///< LT(N) sent, waiting for its LA.
		kDisconnecting,          ///< Disconnect requested while LT(N) is in flight (D2').
		kAwaitingPeerDisconnect, ///< All LTs acknowledged; waiting for the Newton's LD.
		kDisconnectedByPeer,     ///< Valid peer LD received (EOF). Terminal.
		kDisconnected,           ///< Clean local disconnect, via peer LD or local LD 0xff. Terminal.
		kFailed,                 ///< Fatal error; LD and closure are best effort. Terminal.
		kStateCount
	};

	enum EEvent : std::uint8_t
	{
		kRequest,        ///< Valid LR (Link Request) from the Newton.
		kHandshakeBad,   ///< LR or handshake LA we cannot accept.
		kHandshakeAck,   ///< LA(0, credit 1..2) completing the handshake.
		kAckWithCredit,  ///< LA acknowledging LT(N), credit > 0.
		kAckNoCredit,    ///< LA acknowledging LT(N), credit 0.
		kRepeatedAck,    ///< LA repeating the previous acknowledgement.
		kPeerDisconnect, ///< Valid LD (Link Disconnect) from the Newton.
		kSubmit,         ///< Caller hands over data to send.
		kDisconnect,     ///< Caller requests a local disconnect.
		kTimeout,        ///< Caller reports a stall.
		kAnomaly         ///< Any other frame or framing error; always fatal.
	};

	enum EAction : std::uint8_t
	{
		kNone,
		kQueueLRReply,
		kQueueLT,
		kQueueLDUser,
		kQueueLDTimeout,
		kQueueLDHandshake,
		kQueueLDNegotiation
	};

	/// LD reason codes (D4').
	enum EReason : std::uint8_t
	{
		kReasonUser = 0xff,
		kReasonProtocol = 0xfe,
		kReasonTimeout = 0x04,
		kReasonNegotiation = 0x03,
		kReasonHandshakeTimeout = 0x01
	};

	struct StateInfo
	{
		EState state;
		bool connected;
		bool disconnecting;
		bool terminal;
	};

	static constexpr StateInfo kStates[kStateCount] = {
	    { kListening, false, false, false },
	    { kNegotiating, false, false, false },
	    { kConnected, true, false, false },
	    { kThrottled, true, false, false },
	    { kInFlight, true, false, false },
	    { kDisconnecting, true, true, false },
	    { kAwaitingPeerDisconnect, true, true, false },
	    { kDisconnectedByPeer, false, false, true },
	    { kDisconnected, false, false, true },
	    { kFailed, false, false, true },
	};

	struct Transition
	{
		EState state;
		EEvent event;
		EState next;
		EAction action;
	};

	static constexpr Transition kTable[] = {
	    { kListening, kRequest, kNegotiating, kQueueLRReply },                       // H1, H2
	    { kListening, kHandshakeBad, kFailed, kQueueLDNegotiation },                 // H3
	    { kListening, kTimeout, kFailed, kQueueLDHandshake },                        // H4'
	    { kListening, kPeerDisconnect, kDisconnectedByPeer, kNone },                 // D1
	    { kListening, kDisconnect, kDisconnected, kQueueLDUser },                    // D2'
	    { kNegotiating, kHandshakeAck, kConnected, kNone },                          // H1
	    { kNegotiating, kHandshakeBad, kFailed, kQueueLDNegotiation },               // H3
	    { kNegotiating, kTimeout, kFailed, kQueueLDHandshake },                      // H4'
	    { kNegotiating, kPeerDisconnect, kDisconnectedByPeer, kNone },               // D1
	    { kNegotiating, kDisconnect, kDisconnected, kQueueLDUser },                  // D2'
	    { kConnected, kSubmit, kInFlight, kQueueLT },                                // T1, T5
	    { kConnected, kAckWithCredit, kConnected, kNone },                           // A5
	    { kConnected, kAckNoCredit, kThrottled, kNone },                             // A1
	    { kConnected, kPeerDisconnect, kDisconnectedByPeer, kNone },                 // D1
	    { kConnected, kDisconnect, kAwaitingPeerDisconnect, kNone },                 // D2'
	    { kThrottled, kAckWithCredit, kConnected, kNone },                           // A1, A5
	    { kThrottled, kAckNoCredit, kThrottled, kNone },                             // A5
	    { kThrottled, kTimeout, kFailed, kQueueLDTimeout },                          // A2'
	    { kThrottled, kPeerDisconnect, kDisconnectedByPeer, kNone },                 // D1
	    { kThrottled, kDisconnect, kAwaitingPeerDisconnect, kNone },                 // D2'
	    { kInFlight, kAckWithCredit, kConnected, kNone },                            // T1, A4
	    { kInFlight, kAckNoCredit, kThrottled, kNone },                              // A1
	    { kInFlight, kRepeatedAck, kInFlight, kNone },                               // A5
	    { kInFlight, kTimeout, kFailed, kQueueLDTimeout },                           // A3'
	    { kInFlight, kPeerDisconnect, kDisconnectedByPeer, kNone },                  // D1
	    { kInFlight, kDisconnect, kDisconnecting, kNone },                           // D2'
	    { kDisconnecting, kAckWithCredit, kAwaitingPeerDisconnect, kNone },          // D2', E1'
	    { kDisconnecting, kAckNoCredit, kAwaitingPeerDisconnect, kNone },            // D2'
	    { kDisconnecting, kRepeatedAck, kDisconnecting, kNone },                     // A5, D2'
	    { kDisconnecting, kTimeout, kFailed, kQueueLDTimeout },                      // D2' unclean
	    { kDisconnecting, kPeerDisconnect, kDisconnectedByPeer, kNone },             // D1
	    { kDisconnecting, kDisconnect, kDisconnecting, kNone },                      // D2'
	    { kAwaitingPeerDisconnect, kPeerDisconnect, kDisconnected, kNone },          // D2' peer closes
	    { kAwaitingPeerDisconnect, kTimeout, kDisconnected, kQueueLDUser },          // D2' fallback
	    { kAwaitingPeerDisconnect, kDisconnect, kAwaitingPeerDisconnect, kNone },    // D2'
	    { kAwaitingPeerDisconnect, kAckWithCredit, kAwaitingPeerDisconnect, kNone }, // A5
	    { kAwaitingPeerDisconnect, kAckNoCredit, kAwaitingPeerDisconnect, kNone },   // A5
	};

	static constexpr std::size_t kRowCount = sizeof( kTable ) / sizeof( kTable[0] );

	static constexpr const Transition* Lookup( EState inState, EEvent inEvent )
	{
		for ( const Transition& row : kTable )
		{
			if ( row.state == inState && row.event == inEvent )
			{
				return &row;
			}
		}
		return nullptr;
	}

	static const Bytes kLRReply;

	explicit TMNPProtocol( std::size_t inOutputMaxPayloadSize = kDefaultOutputMaxPayloadSize );
	~TMNPProtocol( void ) = default;

	void Reset( void );
	void Start( void );

	void SetKeepAlive( bool inKeepAlive ) noexcept { mKeepAlive = inKeepAlive; }
	bool GetKeepAlive( void ) const noexcept { return mKeepAlive; }

	std::size_t GetOutputPayloadLimit( void ) const noexcept { return mOutputPayloadLimit; }

	/// Predicates.
	bool IsStallable( void ) const noexcept { return Lookup( mState, kTimeout ) != nullptr; }
	bool CanSubmit( void ) const noexcept { return mState == kConnected; }
	bool IsConnected( void ) const noexcept { return kStates[mState].connected; }
	bool IsDisconnecting( void ) const noexcept { return kStates[mState].disconnecting; }
	bool IsTerminal( void ) const noexcept { return kStates[mState].terminal; }
	bool PeerDisconnected( void ) const noexcept
	{
		return mState == kDisconnectedByPeer || ( mState == kDisconnected && mLocalReason == 0 );
	}

	EState GetState( void ) const noexcept { return mState; }
	std::uint32_t GetProgress( void ) const noexcept { return mProgress; }
	std::uint8_t GetLocalReason( void ) const noexcept { return mLocalReason; }
	EOutcome GetOutcome( bool inDetached = false ) const noexcept;

	/// Wire codec.
	static Bytes EncodeFrame( const std::uint8_t* inBody, std::size_t inSize );
	static Bytes EncodeFrame( const Bytes& inBody )
	{
		return EncodeFrame( inBody.data(), inBody.size() );
	}
	EFrameResult PushByte( std::uint8_t inByte );
	void PushWireBytes( const std::uint8_t* inData, std::size_t inSize );
	const Bytes& GetDecodedFrame( void ) const noexcept { return mFrameBody; }

	/// Direct frame / event injection (for pure protocol tests and PushByte).
	void OnFrame( const Bytes& inFrame );
	void OnFramingError( void );
	void OnStall( void );
	void RequestDisconnect( void );
	void Fail( std::uint8_t inReason );

	/// Single-LT submission buffer (used by protocol unit tests).
	Bytes& OutputBuffer( void ) noexcept { return mOutputBuffer; }
	const Bytes& OutputBuffer( void ) const noexcept { return mOutputBuffer; }
	void SubmitOutputPayload( void );

	/// Streaming outbound queue (for event-bus callers): queues arbitrary
	/// application bytes and automatically emits LTs up to mOutputPayloadLimit
	/// whenever CanSubmit() becomes true.
	void QueueOutboundBytes( const std::uint8_t* inData, std::size_t inSize );
	bool PumpOutbound( void );
	bool HasPendingOutbound( void ) const noexcept { return !mPendingOutbound.empty(); }

	/// Outbound frames (unencoded frame bodies or wire-encoded bytes).
	bool HasOutboundFrames( void ) const noexcept { return !mOutputFrames.empty(); }
	bool TakeOutboundFrame( Bytes& outFrameBody );
	Bytes TakeAllOutboundWireBytes( void );
	void ClearOutboundFrames( void ) noexcept { mOutputFrames.clear(); }

	/// Inbound delivered payload.
	std::size_t AvailableInput( void ) const noexcept { return mInputBuffer.size(); }
	std::size_t ReadPayload( std::uint8_t* outData, std::size_t inSize );
	Bytes TakeAllInput( void );

	/// Protocol classification / validation helpers.
	EEvent Classify( const Bytes& inFrame ) const;
	static bool IsValidLR( const Bytes& inFrame );
	static bool IsValidLD( const Bytes& inFrame );
	static void CheckTransitionTable( void );

private:
	enum class EFrameState : std::uint8_t
	{
		kIdle,
		kStartDLE,
		kBody,
		kBodyDLE,
		kCRCLow,
		kCRCHigh
	};

	std::uint8_t LastAckedSequence( void ) const;
	static std::uint16_t UpdateCRC( std::uint16_t inCRC, std::uint8_t inByte );
	void HandleLT( const Bytes& inFrame );
	void Dispatch( EEvent inEvent );
	void Enter( EState inState );
	void Perform( EAction inAction );
	void QueueLA( void );
	void QueueLD( std::uint8_t inReason );
	void QueueLT( const Bytes& inPayload );

	EState mState = kListening;
	std::uint32_t mProgress = 0;

	EFrameState mFrameState = EFrameState::kIdle;
	Bytes mFrameBody;
	std::uint16_t mFrameCRC = 0;
	std::uint16_t mReceivedCRC = 0;

	std::uint8_t mSendSeq = 0;
	std::uint8_t mRecvSeq = 0;
	bool mHaveInbound = false;

	std::deque<Bytes> mOutputFrames;
	std::deque<std::uint8_t> mInputBuffer;

	std::uint8_t mLocalReason = 0;
	bool mKeepAlive = false;

	std::size_t mOutputPayloadLimit;
	Bytes mOutputBuffer;
	std::deque<std::uint8_t> mPendingOutbound;
};

#endif
// __TMNPPROTOCOL__
