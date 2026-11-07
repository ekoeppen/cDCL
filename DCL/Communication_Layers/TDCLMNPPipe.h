// ==============================
// Fichier:			TDCLMNPPipe.h
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
// The Original Code is TDCLMNPPipe.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// Filip Zawadiak and Nicolas Zinovieff. Portions created by the Initial
// Developers are Copyright (C) 1998-2004 the Initial Developers. All Rights
// Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Filip Zawadiak <philz@vyx.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org> (protocol state machine)
//   Protocol state machine portions Copyright (C) 2026 Eckhart Koeppen.
//
// ***** END LICENSE BLOCK *****
//
// Copyright (c) 2001 Filip Zawadiak <philz@vyx.net>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ===========
// $Id: TDCLMNPPipe.h,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#ifndef __TDCLMNPPIPE__
#define __TDCLMNPPIPE__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipeProxy.h>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

// ISO C++
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

///
/// The single error reported for an MNP failure: a protocol anomaly, a
/// timeout or a transport error. The session has failed and the transport
/// has been closed, best effort; LD transmission is not guaranteed.
///
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

///
/// MNP pipe for the Newton serial link: a one-frame, stop-and-wait transport
/// for the simplified profile in .topics/mnp-timerless-bdd.md.
/// Negotiates window 2 to enable the Newton's Inspector keepalive LAs, but
/// sends only one LT at a time and advertises receive credit 1.
///
/// Each pipe supports a single session; Connected() may only be called once.
/// The wire codec and table-driven protocol state machine are private parts of
/// this pipe. Protocol transitions queue frames without doing I/O or reading a
/// clock. Transport driving runs on the calling thread (the link thread):
/// it polls the sub-pipe's BytesAvailable() every
/// millisecond until data arrives or the caller's wait limit passes, feeds
/// wire bytes to the protocol, and writes the frames it produces. One
/// stall timer bounds blocking waits for handshake, acknowledgement, credit
/// and peer LD. It restarts on protocol
/// progress (state changes or accepted inbound LTs), never on partial frames
/// or repeated LAs. No timer runs while idle; Read uses only GetTimeout().
///
/// Errors: a protocol failure throws TDCLMNPProtocolException from Read,
/// Write, FlushOutput and BytesAvailable. A valid LD from the Newton ends the
/// stream: buffered data stays readable, then Read, Write and FlushOutput
/// throw TDCLEOFException, and PeerDisconnected() returns true.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \author	Filip Zawadiak <philz@vyx.net> (?)
/// \author	Eckhart Koeppen <eck@40hz.org> (state machine rewrite)
/// \test	UTestMNPPipe (public API), UTestMNPProtocol (private protocol/codec).
///
class TDCLMNPPipe
	:
		public TDCLPipeProxy
{
public:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kDefaultOutputMaxBufferSize	= 253,	///< 256 - 3.
		///< Le code de microcom donne 64+5. Philz donne 2K
		///< cependant il n'a pas de paquets plus gros que 256 (il tronçonne
		///< le paquet Newton à installer en morceaux de 128 octets). Le Newton
		///< n'aime pas mes paquets de 299 octets, en revanche, ça marche
		///< avec 253+3.
	};

	///
	/// Constructeur à partir d'une couche de communication et d'une taille
	/// maximale de mémoire tampon de sortie.
	///
	/// \param inSubPipe				couche de communication sur laquelle
	///									faire de la compression MNP.
	/// \param inOutputMaxBufferSize	maximum LT payload, clamped to 1..253.
	///
	TDCLMNPPipe(
			TDCLPipe* inSubPipe,
			KUInt32 inOutputMaxBufferSize = kDefaultOutputMaxBufferSize );

	///
	/// Destructeur.
	///
	virtual ~TDCLMNPPipe( void );

	/// \name interface entrée/sortie

	///
	/// Reads delivered payload. Waits until enough data has arrived, or for
	/// GetTimeout() seconds if positive.
	///
	/// \throws TDCLEOFException			after a peer LD, once the data is read.
	/// \throws TDCLMNPProtocolException	if the link failed.
	/// \note The ordinary pipe timeout is also fatal (LD 04, protocol exception).
	///
	virtual	void		Read( void* outBuffer, KUInt32* ioCount );

	///
	/// Buffers application bytes and sends them in LTs of the configured size,
	/// one at a time, honouring the Newton's credit. ioCount reports bytes
	/// accepted into buffering, not bytes transmitted or acknowledged.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Sends buffered output as an LT. Does not wait for its acknowledgement;
	/// the next send or Disconnect does. Read has only the pipe timeout.
	///
	virtual void		FlushOutput( void );

	///
	/// Processes frames that have arrived and reports whether delivered
	/// payload is waiting. Never blocks.
	///
	virtual	Boolean		BytesAvailable( void );

	///
	/// Local disconnect (D2'): flushes buffered output, waits for the pending
	/// LT's LA, then the Newton's LD, under the stall timeout. An LA stall
	/// sends LD 04 (unclean); an LD stall sends LD ff (clean). A peer LD
	/// receives no reply. Disconnects the sub-pipe and never throws.
	///
	virtual	void		Disconnect( void );

	///
	/// Whether the Newton closed the MNP link with a valid LD frame.
	/// False after a local protocol failure.
	///
	virtual	Boolean		PeerDisconnected( void );

	///
	/// Runs the MNP handshake (LR, LR reply, LA) once. A repeated call fails
	/// an active session; a detached or terminal pipe cannot be restarted.
	///
	/// \throws TDCLMNPProtocolException	if the handshake fails or times out.
	/// \throws TDCLEOFException			if the Newton sends LD instead.
	///
	virtual	TDCLCommLayer*	Connected( TDCLLink* inLink );

	///
	/// Answer every inbound LA with LA(last received, 1) (K1). Used by
	/// TDCLInspectorLink.
	///
	void				SetKeepAlive( Boolean inKeepAlive );

	/// \name Stall timeout
	static constexpr std::chrono::milliseconds kDefaultStallTimeout{ 6000 };

	/// Completed sessions have exactly two outcomes for applications: normal
	/// closure or failure (retry). This status survives cleanup; LD reasons
	/// are diagnostic details, not success/error return codes.
	enum class EOutcome
	{
		kInProgress,
		kClosed,
		kFailed
	};
	EOutcome GetOutcome( void ) const;

private:
	friend class TMNPPipeTestAccess;
	friend class TMNPPipeStatusTestAccess;

	// Protocol types and transition table.
	using Bytes = std::vector<std::uint8_t>;

	/// PushByte() results. Every framing error is fatal (LD 0xfe).
	enum class EFrameResult : std::uint8_t
	{
		kNone,
		kFrame,
		kError
	};

	/// Largest decoded frame body: a 256-byte LT plus its 3-byte header.
	static constexpr std::size_t kMaxFrameBodySize = 259;

	/// Link states, named for what is true while in them. Order:
	/// Listening -> Negotiating -> Connected (Throttled, InFlight) ->
	/// Disconnecting -> AwaitingPeerDisconnect -> Disconnected.
	enum EState : std::uint8_t
	{
		kListening,              ///< Started; waiting for the Newton's LR.
		kNegotiating,            ///< LR reply sent; waiting for handshake LA(0, credit 1..2).
		kConnected,              ///< No LT outstanding, positive credit; can submit one LT.
		kThrottled,              ///< No LT outstanding, credit 0; output waits for credit.
		kInFlight,               ///< LT(N) sent, waiting for its LA.
		kDisconnecting,          ///< Disconnect() requested while LT(N) is in flight (D2').
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

	// Timerless transition table in docs/wip/timerless.md (37 rows).
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

	/// Finds the row for (state, event), or nullptr if there is none.
	static constexpr const Transition*
	Lookup( EState inState, EEvent inEvent )
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

	/// Largest LT payload cDCL sends (T1: 1-253 bytes; the Newton rejects more).
	static constexpr std::size_t kMaxOutputPayloadSize = 253;

	/// Window > 1 enables the Newton's Inspector keepalive LAs. This is a
	/// negotiated maximum, not our transmit concurrency or receive credit:
	/// the state machine keeps one LT pending and QueueLA advertises credit 1.
	static constexpr std::uint8_t kNegotiatedWindowSize = 2;

	/// The exact 24-byte LR reply (window 2).
	static const Bytes kLRReply;

	/// Protocol predicates. Only WaitFor owns the stall deadline.
	bool
	IsStallable( void ) const
	{
		return Lookup( mState, kTimeout ) != nullptr;
	}
	bool
	CanSubmit( void ) const
	{
		return mState == kConnected;
	}
	bool
	IsConnected( void ) const
	{
		return kStates[mState].connected;
	}
	bool
	IsDisconnecting( void ) const
	{
		return kStates[mState].disconnecting;
	}
	bool
	IsTerminal( void ) const
	{
		return kStates[mState].terminal;
	}

	/// Wire codec: decoding dispatches complete frames, never performs I/O.
	static Bytes EncodeFrame( const std::uint8_t* inBody, std::size_t inSize );
	static Bytes
	EncodeFrame( const Bytes& inBody )
	{
		return EncodeFrame( inBody.data(), inBody.size() );
	}
	EFrameResult PushByte( std::uint8_t inByte );

	/// Protocol events and payload handling. No clock or transport access.
	void OnFrame( const Bytes& inFrame );
	void OnFramingError( void );
	void OnStall( void );
	/// Dispatches the buffered payload as one LT, then clears it. Requires credit.
	void SubmitOutputPayload( void );
	std::size_t ReadPayload( std::uint8_t* outData, std::size_t inSize );
	EEvent Classify( const Bytes& inFrame ) const;
	static bool IsValidLR( const Bytes& inFrame );
	static bool IsValidLD( const Bytes& inFrame );
	/// Contains compile-time assertions; no runtime validation is needed.
	static void CheckTransitionTable( void );

	enum class EFrameState : std::uint8_t
	{
		kIdle,
		kStartDLE,
		kBody,
		kBodyDLE,
		kCRCLow,
		kCRCHigh
	};

	/// One outstanding LT: the previous sequence is acknowledged while it is pending.
	std::uint8_t LastAckedSequence( void ) const;
	static std::uint16_t UpdateCRC( std::uint16_t inCRC, std::uint8_t inByte );
	void HandleLT( const Bytes& inFrame );
	void Dispatch( EEvent inEvent );
	void Enter( EState inState );
	void Perform( EAction inAction );
	void Fail( std::uint8_t inReason );
	void QueueLA( void );
	void QueueLD( std::uint8_t inReason );
	void QueueLT( const Bytes& inPayload );

	// Transport driving and blocking waits.
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	///
	/// Constructeur par copie volontairement indisponible.
	///
	TDCLMNPPipe( const TDCLMNPPipe& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	TDCLMNPPipe& operator = ( const TDCLMNPPipe& inCopy );

	/// Direction of a logged MNP frame.
	enum class EFrameDirection
	{
		kIn,
		kOut,
		kError
	};

	///
	/// Log a decoded MNP frame via KDebug when debug_TDCLMNPPipe is enabled.
	///
	static	void		LogFrame( EFrameDirection inDirection,
							const KUInt8* inFrame, KUInt32 inSize );

	/// Waits for the predicate or a terminal state, restarting the one stall
	/// deadline on progress. Callers interpret EOF/failure after returning.
	template<class Done> void WaitFor( Done inDone );
	/// Polls the sub-pipe every millisecond until it reports data or inLimit
	/// passes, processes a batch of input and writes protocol frames. Does not run
	/// any timer itself (Read and idle polling use it too).
	void				ServiceTransport( TimePoint inLimit );
	/// Feeds a bounded batch of received bytes into the protocol, returning control
	/// to WaitFor even if the peer is continuously readable.
	void				ProcessAvailableInput( void );
	/// Writes queued frames, closes terminal sessions, and reports stored failure.
	void				ServiceOutput( void );
	/// Submits the buffered output as one LT, waiting for credit.
	void				SendOutputPayload( void );
	/// Throws if the link cannot carry data (EOF or protocol error).
	void				CheckUsable( void ) const;
	/// Throws the stored protocol error if the link failed.
	void				CheckFailed( void ) const;
	/// Returns whether cooperative stop has been requested on this pipe or sub-pipe.
	bool				IsStopRequested( void );
	/// Closes transport without sending LD and throws TDCLUserCancelledException if stopped.
	void				ThrowIfCancelled( void );
	/// Detaches once; a cleanup error records failure without replacing its cause.
	void CloseTransport( void ) noexcept;
	/// Fatal adapter error: fail, attempt LD, close, and report the stored reason.
	[[noreturn]] void Abort( std::uint8_t inReason );

	/// Pipe lifecycle around the MNP protocol. The sub-pipe is already open when
	/// the pipe is created; this tracks whether MNP runs on it.
	enum class EPhase
	{
		kAttached,	///< Sub-pipe open, MNP not started (before Connected()).
		kRunning,	///< Connected() called; MNP runs on the sub-pipe.
		kDetached	///< Sub-pipe closure attempted; the session cannot continue.
	};

	// Protocol state, codec state and queued frames/payload.
	EState mState = kListening;
	/// Counts state changes and accepted inbound LTs, including duplicates.
	/// Compare for inequality: this unsigned counter may wrap.
	std::uint32_t mProgress = 0;

	EFrameState mFrameState = EFrameState::kIdle;
	Bytes mFrameBody;
	std::uint16_t mFrameCRC = 0;
	std::uint16_t mReceivedCRC = 0;

	std::uint8_t mSendSeq = 0;   ///< N: sequence of the last LT queued for transmission.
	std::uint8_t mRecvSeq = 0;   ///< R: sequence of the last accepted LT.
	bool mHaveInbound = false;   ///< Whether an LT has been accepted.

	std::deque<Bytes> mOutputFrames; ///< Unencoded MNP frame bodies, including control frames.
	std::deque<std::uint8_t> mInputBuffer; ///< Accepted LT payload; no MNP headers or frame boundaries.

	std::uint8_t mLocalReason = 0;
	bool mKeepAlive = false;

	// Transport lifecycle and output not yet submitted.
	KUInt32				mOutputPayloadLimit; ///< Configured maximum payload per outgoing LT.
	std::vector<KUInt8>	mOutputBuffer;		///< Application bytes awaiting LT construction.
	EPhase				mPhase = EPhase::kAttached;
	std::chrono::milliseconds mStallTimeout{ kDefaultStallTimeout };
};

#endif
		// __TDCLMNPPIPE__

// ====================================================================== //
// C makes it easy for you to shoot yourself in the foot.  C++ makes that //
// harder, but when you do, it blows away your whole leg.                 //
//                 -- Bjarne Stroustrup                                   //
// ====================================================================== //
