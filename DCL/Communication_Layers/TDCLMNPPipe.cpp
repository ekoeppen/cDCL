// ==============================
// Fichier:			TDCLMNPPipe.cp
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
// The Original Code is TDCLMNPPipe.cp.
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
// $Id: TDCLMNPPipe.cp,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLMNPPipe.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// ISO C++
#include <algorithm>
#include <chrono>
#include <thread>

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

// MNP frame types (second byte of a decoded frame).
static const KUInt8 kMNPFrameLR = 1;	///< Link request.
static const KUInt8 kMNPFrameLD = 2;	///< Link disconnect.
static const KUInt8 kMNPFrameLT = 4;
static const KUInt8 kMNPFrameLA = 5;
static const KUInt8 kLDReasonUser = 0xff;
static const KUInt8 kLDReasonProtocol = 0xfe;
static const KUInt8 kLDReasonTimeout = 0x04;
static const KUInt8 kLDReasonNegotiation = 0x03;
static const KUInt8 kLDReasonHandshake = 0x01;
static const size_t kMaxInboundBody = 259;

// Commentaire intéressant de Philz:
// Just keep in mind that MNP implementation in lpkg is very primitive.
// I just wanted to have minimal functioning MNP support. It has one
// frame window size, which could harm download performance, especially
// on high speed links :-(

// ------------------------------------------------------------------------- //
//  Constantes
// ------------------------------------------------------------------------- //
const KUInt16 kCRC16Table[256] =
{
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

TDCLMNPPipe::FrameLoggerPtr TDCLMNPPipe::sFrameLogger = nil;
void* TDCLMNPPipe::sFrameLoggerRefCon = nil;

// ------------------------------------------------------------------------- //
//  * TDCLMNPPipe( TDCLPipe* )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::TDCLMNPPipe(
            TDCLPipe* inSubPipe,
            KUInt32 inOutputMaxBufferSize /* = kDefaultOutputMaxBufferSize */ )
    : TDCLPipeProxy(inSubPipe),
      mOutputCapacity(std::max<KUInt32>(1, std::min<KUInt32>(253, inOutputMaxBufferSize)))
{
    mOutput.reserve(mOutputCapacity);
}

void TDCLMNPPipe::SetStallTimeout(std::chrono::milliseconds timeout)
{
    mStallTimeout = timeout;
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMNPPipe( void )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::~TDCLMNPPipe( void )
{
}

// ------------------------------------------------------------------------- //
//  * SetFrameLogger( FrameLoggerPtr, void* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SetFrameLogger( FrameLoggerPtr inLogger, void* inRefCon )
{
	sFrameLoggerRefCon = inRefCon;
	sFrameLogger = inLogger;
}

// ------------------------------------------------------------------------- //
//  * LogFrame( EFrameDirection, const KUInt8*, KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::LogFrame(
			EFrameDirection inDirection,
			const KUInt8* inFrame,
			KUInt32 inSize )
{
	FrameLoggerPtr theLogger = sFrameLogger;
	if (theLogger)
	{
		(*theLogger)( sFrameLoggerRefCon, inDirection, inFrame, inSize );
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void* outBuffer, KUInt32* ioCount )
// ------------------------------------------------------------------------- //
[[noreturn]] void TDCLMNPPipe::Fail(KUInt8 reason)
{
    if (!mClosed) {
        mClosed = true;
        mDisconnectReason = reason;
        try { SendFrame({7, kMNPFrameLD, 1, 1, reason, 2, 1, 0}); } catch (...) {}
        try { GetSubPipe()->Disconnect(); } catch (...) {}
    }
    throw TDCLMNPProtocolError();
}

void TDCLMNPPipe::SendFrame(const Bytes& frame)
{
    Bytes wire{0x16, 0x10, 0x02};
    KUInt16 crc = 0;
    for (auto byte : frame) {
        crc = (crc >> 8) ^ kCRC16Table[(crc ^ byte) & 0xff];
        wire.push_back(byte);
        if (byte == 0x10) wire.push_back(byte);
    }
    wire.push_back(0x10);
    wire.push_back(0x03);
    crc = (crc >> 8) ^ kCRC16Table[(crc ^ 0x03) & 0xff];
    wire.push_back(crc & 0xff);
    wire.push_back(crc >> 8);
    KUInt32 count = wire.size();
    try {
        GetSubPipe()->Write(wire.data(), &count);
        if (count != wire.size()) Fail();
        GetSubPipe()->FlushOutput();
    } catch (const TDCLMNPProtocolError&) { throw; }
      catch (...) { Fail(); }
    LogFrame(kFrameOut, frame.data(), frame.size());
}

TDCLMNPPipe::Bytes TDCLMNPPipe::ReceiveFrame(Clock::time_point deadline,
                                              KUInt8 timeoutReason)
{
    auto readByte = [&]() -> KUInt8 {
        while (!GetSubPipe()->BytesAvailable()) {
            if (mClosed || Clock::now() >= deadline) Fail(timeoutReason);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        KUInt8 byte = 0;
        KUInt32 count = 1;
        try { GetSubPipe()->Read(&byte, &count); }
        catch (...) { Fail(); }
        if (count != 1) Fail();
        return byte;
    };
    KUInt8 first = readByte();
    if (deadline == Clock::time_point::max())
        deadline = Clock::now() + mStallTimeout;
    timeoutReason = kLDReasonProtocol;
    while (first == 0x16) first = readByte();
    if (first != 0x10 || readByte() != 0x02) Fail();
    Bytes frame;
    KUInt16 crc = 0;
    for (;;) {
        KUInt8 byte = readByte();
        if (byte == 0x10) {
            byte = readByte();
            if (byte == 0x03) {
                crc = (crc >> 8) ^ kCRC16Table[(crc ^ byte) & 0xff];
                KUInt16 received = readByte();
                received |= static_cast<KUInt16>(readByte()) << 8;
                if (received != crc || frame.empty()) {
                    LogFrame(kFrameError, frame.data(), frame.size());
                    Fail(kLDReasonProtocol);
                }
                LogFrame(kFrameIn, frame.data(), frame.size());
                return frame;
            }
            if (byte != 0x10) Fail();
        }
        if (frame.size() >= kMaxInboundBody) Fail();
        frame.push_back(byte);
        crc = (crc >> 8) ^ kCRC16Table[(crc ^ byte) & 0xff];
    }
}

void TDCLMNPPipe::Pump(Clock::time_point deadline, KUInt8 timeoutReason)
{
    Bytes frame = ReceiveFrame(deadline, timeoutReason);
    if (frame.size() < 2) Fail();
    switch (frame[1]) {
    case kMNPFrameLD:
        // The wire boundary and CRC delimit the body. Newton OS sends its
        // entire ten-byte LA buffer for an eight-byte LD; only the declared
        // reason and optional user-code parameters are significant.
        if ((frame[0] != 4 && frame[0] != 7) ||
            frame.size() < (frame[0] == 7 ? 8u : 5u) ||
            frame[2] != 1 || frame[3] != 1 ||
            (frame[0] == 7 && (frame[5] != 2 || frame[6] != 1))) Fail();
        mClosed = true;
        mPeerClosed = true;
        mPeerReason = frame[4];
        ++mProgress;
        GetSubPipe()->Disconnect();
        return;
    case kMNPFrameLA:
        if (frame.size() != 4 || frame[0] != 3 || frame[3] > 1) Fail();
        if (mPending) {
            if (frame[2] == mSendSequence) {
                mPending = false;
                mCredit = frame[3];
                ++mProgress;
            } else if (frame[2] != static_cast<KUInt8>(mSendSequence - 1)) {
                Fail();
            }
        } else if (frame[2] == mSendSequence) {
            if (mCredit != frame[3]) {
                mCredit = frame[3];
                ++mProgress;
            }
        } else Fail();
        if (mKeepAlive) SendACK();
        return;
    case kMNPFrameLT:
        if (frame.size() < 4 || frame.size() > kMaxInboundBody || frame[0] != 2) Fail();
        if (frame[2] == static_cast<KUInt8>(mReceiveSequence + 1)) {
            mReceiveSequence = frame[2];
            mHaveInbound = true;
            mInput.insert(mInput.end(), frame.begin() + 3, frame.end());
        } else if (frame[2] != mReceiveSequence || !mHaveInbound) {
            Fail();
        }
        ++mProgress;
        SendACK();
        return;
    default:
        Fail();
    }
}

void TDCLMNPPipe::DrainAvailable()
{
    while (!mClosed && GetSubPipe()->BytesAvailable())
        Pump(Clock::now() + mStallTimeout, kLDReasonProtocol);
}

template <typename Predicate>
void TDCLMNPPipe::WaitFor(Predicate done, KUInt8 timeoutReason)
{
    auto seen = mProgress;
    auto deadline = Clock::now() + mStallTimeout;
    while (!done() && !mClosed) {
        if (mProgress != seen) {
            seen = mProgress;
            deadline = Clock::now() + mStallTimeout;
        }
        Pump(deadline, timeoutReason);
    }
}

void TDCLMNPPipe::SendData()
{
    if (mOutput.empty()) return;
    WaitFor([this] { return !mPending && mCredit != 0; });
    if (mPeerClosed) throw DCLEOF;
    if (mClosed) Fail();
    mSendSequence = static_cast<KUInt8>(mSendSequence + 1);
    Bytes frame{2, kMNPFrameLT, mSendSequence};
    frame.insert(frame.end(), mOutput.begin(), mOutput.end());
    SendFrame(frame);
    mOutput.clear();
    mCredit = 0;
    mPending = true;
    ++mProgress;
}

void
TDCLMNPPipe::Read( void* outBuffer, KUInt32* ioCount )
{
    KUInt32 done = 0, wanted = *ioCount;
    *ioCount = 0;
    try {
        while (done < wanted) {
            if (mInputOffset == mInput.size()) {
                mInput.clear();
                mInputOffset = 0;
                if (mPeerClosed) throw DCLEOF;
                if (mClosed) Fail();
                Pump(Clock::time_point::max());
                continue;
            }
            auto n = std::min<size_t>(wanted - done, mInput.size() - mInputOffset);
            ::memcpy(static_cast<KUInt8*>(outBuffer) + done, mInput.data() + mInputOffset, n);
            mInputOffset += n;
            done += n;
            *ioCount = done;
        }
    } catch (...) { *ioCount = done; throw; }
}

// ------------------------------------------------------------------------- //
//	* Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
    KUInt32 done = 0, wanted = *ioCount;
    *ioCount = 0;
    try {
        if (mPeerClosed) throw DCLEOF;
        if (!mConnected || mClosed) Fail();
        while (done < wanted) {
            if (mOutput.size() == mOutputCapacity) SendData();
            auto n = std::min<size_t>(wanted - done, mOutputCapacity - mOutput.size());
            const auto* bytes = static_cast<const KUInt8*>(inBuffer) + done;
            mOutput.insert(mOutput.end(), bytes, bytes + n);
            done += n;
            *ioCount = done;
            if (mOutput.size() == mOutputCapacity) SendData();
        }
    } catch (...) { *ioCount = done; throw; }
}

// ------------------------------------------------------------------------- //
//	* FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::FlushOutput( void )
{
    if (mPeerClosed) throw DCLEOF;
    if (mClosed) Fail();
    DrainAvailable();
    if (mPeerClosed) throw DCLEOF;
    SendData();
    GetSubPipe()->FlushOutput();
}

// ------------------------------------------------------------------------- //
//	* SendACK( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SendACK( void )
{
    if (mClosed) Fail();
    SendFrame({3, kMNPFrameLA, mReceiveSequence, 1});
}

// ------------------------------------------------------------------------- //
//	* BytesAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLMNPPipe::BytesAvailable( void )
{
    if (!mClosed) DrainAvailable();
    return mInputOffset < mInput.size();
}

// ------------------------------------------------------------------------- //
//  * Disconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Disconnect( void )
{
    if (mClosed) return;
    if (!mConnected) {
        mClosed = true;
        mDisconnectReason = kLDReasonUser;
        try { SendFrame({7, kMNPFrameLD, 1, 1, kLDReasonUser, 2, 1, 0}); } catch (...) {}
        try { GetSubPipe()->Disconnect(); } catch (...) {}
        mCleanDisconnect = true;
        return;
    }
    try {
        // Flush any buffered output and wait for the final LT's LA (D2' step 1).
        SendData();
        WaitFor([this] { return !mPending; });
        if (mPeerClosed) {
            mCleanDisconnect = true;
            return;
        }
        if (mClosed) return;
    } catch (...) {
        if (!mClosed) {
            mClosed = true;
            mDisconnectReason = kLDReasonTimeout;
            try { SendFrame({7, kMNPFrameLD, 1, 1, kLDReasonTimeout, 2, 1, 0}); }
            catch (...) {}
        }
        try { GetSubPipe()->Disconnect(); } catch (...) {}
        return;
    }
    // D2' steps 2-3: wait for the Newton's LD; send LD 0xff only on stall.
    try {
        WaitFor([this] { return mPeerClosed; }, kLDReasonUser);
    } catch (...) {}
    mCleanDisconnect = (mPeerClosed || mDisconnectReason == kLDReasonUser);
}

Boolean
TDCLMNPPipe::PeerDisconnected()
{
    if (!mClosed) DrainAvailable();
    return mPeerClosed;
}

TDCLCommLayer*
TDCLMNPPipe::Connected(TDCLLink* link)
{
    auto lr = ReceiveFrame(Clock::now() + mStallTimeout, kLDReasonHandshake);
    // V.42 Annex A LR: fixed prefix, then type/length/value parameters.
    // The captured Newton LR is 41 bytes with length indicator 0x26, not
    // body-size-minus-one; the frame boundary is supplied by the wire codec.
    static const Bytes prefix{0x02, 0x01, 0x06, 0x01, 0, 0, 0, 0, 0xff};
    if (lr.size() < 11 || lr[1] != kMNPFrameLR ||
        !std::equal(prefix.begin(), prefix.end(), lr.begin() + 2)) Fail(kLDReasonNegotiation);
    bool framing = false, window = false, n401 = false, optimization = false;
    for (size_t i = 11; i < lr.size();) {
        if (lr.size() - i < 2 || lr[i+1] > lr.size() - i - 2) Fail(kLDReasonNegotiation);
        const auto id = lr[i], len = lr[i+1];
        const auto value = i + 2;
        switch (id) {
        case 2: // Octet-oriented framing.
            if (framing || len != 1 || lr[value] != 2) Fail(kLDReasonNegotiation);
            framing = true;
            break;
        case 3: // Newton's offered transmit window; we respond with one.
            if (window || len != 1 || lr[value] == 0) Fail(kLDReasonNegotiation);
            window = true;
            break;
        case 4: // N401 (little-endian), overridden by optimization bit 1.
            if (n401 || len != 2 ||
                (static_cast<unsigned>(lr[value]) |
                 (static_cast<unsigned>(lr[value+1]) << 8)) < 64) Fail(kLDReasonNegotiation);
            n401 = true;
            break;
        case 8: // 256-byte data fields and fixed-field LT/LA.
            if (optimization || len != 1 || lr[value] != 3) Fail(kLDReasonNegotiation);
            optimization = true;
            break;
        default: // Additional Newton parameters are not echoed or interpreted.
            break;
        }
        i += 2 + len;
    }
    if (!framing || !window || !n401 || !optimization) Fail(kLDReasonNegotiation);
    // Reply proven compatible with the observed hardware trace.
    SendFrame({0x17, 1, 2, 1, 6, 1, 0, 0, 0, 0, 0xff,
               2, 1, 2, 3, 1, 1, 4, 2, 64, 0, 8, 1, 3});
    auto la = ReceiveFrame(Clock::now() + mStallTimeout, kLDReasonHandshake);
    if (la != Bytes({3, kMNPFrameLA, 0, 1})) Fail(kLDReasonNegotiation);
    mConnected = true;
    mCredit = 1;
    ++mProgress;
    return TDCLPipeProxy::Connected(link);
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
