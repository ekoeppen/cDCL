#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <deque>
#include <functional>
#include <stdexcept>
#include <vector>

using Bytes = std::vector<KUInt8>;

static void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

// Bitwise CRC, intentionally independent of the production lookup table.
static KUInt16 CRC(const Bytes& data)
{
    KUInt16 crc = 0;
    for (auto byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? (crc >> 1) ^ 0xa001 : crc >> 1;
    }
    return crc;
}

static Bytes Encode(const Bytes& frame)
{
    Bytes wire{0x16, 0x10, 0x02};
    Bytes checksum = frame;
    checksum.push_back(0x03);
    for (auto byte : frame) {
        wire.push_back(byte);
        if (byte == 0x10) wire.push_back(byte);
    }
    wire.push_back(0x10);
    wire.push_back(0x03);
    auto crc = CRC(checksum);
    wire.push_back(crc & 0xff);
    wire.push_back(crc >> 8);
    return wire;
}

static Bytes Decode(const Bytes& wire)
{
    Check(wire.size() >= 7 && wire[0] == 0x16 && wire[1] == 0x10 &&
          wire[2] == 0x02, "invalid frame prefix");
    Bytes frame;
    size_t i = 3;
    for (; i < wire.size() - 2; ++i) {
        if (wire[i] == 0x10) {
            Check(++i < wire.size() - 2, "truncated escape");
            if (wire[i] == 0x03) break;
            Check(wire[i] == 0x10, "invalid escape");
        }
        frame.push_back(wire[i]);
    }
    Check(i + 2 == wire.size() - 1 && wire[i] == 0x03, "missing terminator");
    Bytes checksum = frame;
    checksum.push_back(0x03);
    auto crc = CRC(checksum);
    Check(wire[i+1] == (crc & 0xff) && wire[i+2] == (crc >> 8), "invalid CRC");
    return frame;
}

// Decoded LR from the successful hardware install in /tmp/cdcl-install-hw.log.
static Bytes PeerLR()
{
    return {0x26, 1, 2, 1, 6, 1, 0, 0, 0, 0, 0xff, 2, 1, 2, 3, 1,
            8, 4, 2, 64, 0, 8, 1, 3, 9, 1, 1, 0x0e, 4, 3, 4, 0,
            0xfa, 0xc5, 6, 1, 4, 0, 0, 0xe1, 0};
}

// Decoded response from the same successful hardware install.
static Bytes HostLR()
{
    return {0x17, 1, 2, 1, 6, 1, 0, 0, 0, 0, 0xff, 2, 1, 2, 3, 1,
            1, 4, 2, 64, 0, 8, 1, 3};
}

static bool ValidLD(const Bytes& frame, KUInt8 reason)
{
    return frame == Bytes({7, 2, 1, 1, reason, 2, 1, 0});
}

static Bytes UserLD()
{
    return {7, 2, 1, 1, 0xff, 2, 1, 0};
}

class Peer final : public TDCLPipe {
public:
    Peer() : TDCLPipe(nullptr) {}
    void Feed(const Bytes& frame) {
        auto wire = Encode(frame);
        incoming.insert(incoming.end(), wire.begin(), wire.end());
    }
    void Read(void* dst, KUInt32* count) override {
        Check(*count && !incoming.empty(), "unexpected peer read");
        auto* bytes = static_cast<KUInt8*>(dst);
        auto n = std::min<size_t>(*count, incoming.size());
        for (size_t i = 0; i < n; ++i) {
            bytes[i] = incoming.front();
            incoming.pop_front();
        }
        *count = n;
    }
    void Write(const void* src, KUInt32* count) override {
        const auto* bytes = static_cast<const KUInt8*>(src);
        outgoing.insert(outgoing.end(), bytes, bytes + *count);
        if (autoAck) {
            auto frame = Decode(Bytes(bytes, bytes + *count));
            if (frame.size() >= 3 && frame[1] == 4)
                Feed({3, 5, frame[2], ackCredit});
        }
    }
    Boolean BytesAvailable() override {
        if (onPoll) onPoll(*this);
        if (pollsUntilAck > 0) {
            Check(outgoing.empty(), "LD sent before final LT was ACKed");
            observedNoPrematureLD = true;
            if (--pollsUntilAck == 0) Feed({3, 5, pendingAckSequence, 1});
        }
        return !incoming.empty();
    }
    void FlushOutput() override {}
    void Disconnect() override { closed = true; }
    long GetTimeout() override { return 0; }
    void SetTimeout(long) override {}
    TDCLCommLayer* Connected(TDCLLink*) override { return nullptr; }
    void ClearDataPresent() override {}
    Bytes TakeFrame() {
        Check(outgoing.size() >= 7, "no outbound frame");
        size_t end = 3;
        while (end + 3 < outgoing.size()) {
            if (outgoing[end] == 0x10) {
                ++end;
                if (outgoing[end] == 0x03) break;
            }
            ++end;
        }
        Check(end + 2 < outgoing.size(), "unterminated outbound frame");
        Bytes wire(outgoing.begin(), outgoing.begin() + end + 3);
        outgoing.erase(outgoing.begin(), outgoing.begin() + end + 3);
        return Decode(wire);
    }
    std::deque<KUInt8> incoming;
    Bytes outgoing;
    bool closed = false;
    bool autoAck = false;
    KUInt8 ackCredit = 1;
    int pollsUntilAck = 0;
    KUInt8 pendingAckSequence = 0;
    bool observedNoPrematureLD = false;
    std::function<void(Peer&)> onPoll;
};

static void Handshake(TDCLMNPPipe& pipe, Peer& peer)
{
    peer.Feed(PeerLR());
    peer.Feed({3, 5, 0, 1});
    pipe.Connected(nullptr);
    auto lr = peer.TakeFrame();
    Check(lr == HostLR(), "outgoing LR differs from hardware trace");
}

static void TestPeerLDParsing()
{
    // D1: the Newton's captured inactivity LD has two bytes beyond its
    // declared header, and its normal Dock close LD uses reason 0. A CRC-valid
    // frame boundary, not the header indicator, determines the actual body size.
    const Bytes capturedInactivityLD{7, 2, 1, 1, 5, 2, 1, 0, 0, 0};
    const Bytes capturedNormalLD{7, 2, 1, 1, 0, 2, 1, 0, 0, 0};
    const Bytes nonzeroTrailingLD{7, 2, 1, 1, 5, 2, 1, 0, 0x10, 0xa5};
    const Bytes reasonOnlyWithTrailing{4, 2, 1, 1, 5, 0x10, 0xa5};
    for (const auto& ld : {capturedInactivityLD, capturedNormalLD,
                           nonzeroTrailingLD, reasonOnlyWithTrailing}) {
        auto* peer = new Peer;
        TDCLMNPPipe pipe(peer);
        Handshake(pipe, *peer);
        peer->Feed(ld);
        bool rejected = false;
        try { pipe.PeerDisconnected(); }
        catch (const TDCLMNPProtocolError&) { rejected = true; }
        Check(!rejected && pipe.PeerDisconnected() &&
              pipe.PeerDisconnectReason() == ld[4] && peer->closed &&
              peer->outgoing.empty(),
              "D1 rejected a CRC-valid peer LD with trailing bytes");
        KUInt8 data = 0;
        KUInt32 count = 1;
        bool eof = false;
        try { pipe.Read(&data, &count); }
        catch (const TDCLEOFException&) { eof = true; }
        Check(eof && count == 0 && !pipe.BytesAvailable(),
              "D1 did not report EOF after the peer's LD");
        pipe.Disconnect();
        Check(peer->outgoing.empty(), "D1 sent LD in response to peer LD");
    }

    // A recognized header indicator still requires complete and correctly
    // typed reason/user-code TLVs; extra bytes cannot repair a broken TLV.
    const Bytes malformed[] = {
        {7, 2, 1, 1, 5, 2, 1},              // user-code value missing
        {7, 2, 1, 1, 5, 2, 2, 0, 0},        // wrong user-code length
        {7, 2, 1, 1, 5, 3, 1, 0, 0, 0},     // wrong user-code type
        {4, 2, 1, 1},                       // reason missing
        {4, 2, 2, 1, 5, 0, 0},              // wrong reason type
        {4, 2, 1, 2, 5, 0, 0},              // wrong reason length
        {6, 2, 1, 1, 5, 2, 1, 0},           // unsupported indicator
    };
    for (const auto& ld : malformed) {
        auto* peer = new Peer;
        TDCLMNPPipe pipe(peer);
        Handshake(pipe, *peer);
        peer->Feed(ld);
        bool failed = false;
        try { pipe.PeerDisconnected(); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && peer->closed && !pipe.BytesAvailable() &&
              ValidLD(peer->TakeFrame(), 0xfe) && peer->outgoing.empty(),
              "D1 accepted an LD with malformed required parameters");
    }

    // Even a well-formed LD cannot bypass wire integrity or the X2 limit.
    for (bool badCRC : {true, false}) {
        auto* peer = new Peer;
        TDCLMNPPipe pipe(peer);
        Handshake(pipe, *peer);
        Bytes oversized = capturedInactivityLD;
        oversized.resize(260, 0x10);
        Bytes wire = Encode(badCRC ? capturedInactivityLD : oversized);
        if (badCRC) wire.back() ^= 1;
        peer->incoming.insert(peer->incoming.end(), wire.begin(), wire.end());
        bool failed = false;
        try { pipe.PeerDisconnected(); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && peer->closed && ValidLD(peer->TakeFrame(), 0xfe) &&
              peer->outgoing.empty(), "D1 bypassed CRC or X2 size validation");
    }
}

static void TestAdditionalScenarios()
{
    // T2/X2: the peer may transmit 256 bytes (259-byte decoded LT), but
    // anything larger is fatal before any of its data is delivered.
    auto* inboundPeer = new Peer;
    TDCLMNPPipe inbound(inboundPeer);
    Handshake(inbound, *inboundPeer);
    Bytes payload(256);
    for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<KUInt8>(i);
    Bytes lt{2, 4, 1};
    lt.insert(lt.end(), payload.begin(), payload.end());
    inboundPeer->Feed(lt);
    Check(inbound.BytesAvailable(), "256-byte LT rejected");
    Check(inboundPeer->TakeFrame() == Bytes({3, 5, 1, 1}), "256-byte LT not ACKed");
    Bytes received(256);
    for (size_t i = 0; i < received.size(); ++i) {
        KUInt32 count = 1;
        inbound.Read(&received[i], &count);
        Check(count == 1, "small read of 256-byte LT failed");
    }
    Check(received == payload && !inbound.BytesAvailable(), "256-byte LT changed");

    auto* oversizedPeer = new Peer;
    TDCLMNPPipe oversized(oversizedPeer);
    Handshake(oversized, *oversizedPeer);
    lt.push_back(0x42);
    oversizedPeer->Feed(lt);
    bool failed = false;
    try { oversized.BytesAvailable(); }
    catch (const TDCLMNPProtocolError&) { failed = true; }
    Check(failed && !oversized.BytesAvailable() &&
          ValidLD(oversizedPeer->TakeFrame(), 0xfe) && oversizedPeer->outgoing.empty(),
          "260-byte LT accepted");

    auto* unfinishedPeer = new Peer;
    TDCLMNPPipe unfinished(unfinishedPeer);
    unfinished.SetStallTimeout(std::chrono::milliseconds(15));
    Handshake(unfinished, *unfinishedPeer);
    unfinishedPeer->incoming.insert(unfinishedPeer->incoming.end(),
                                    {0x16, 0x10, 0x02, 2, 4, 1, 0x41});
    failed = false;
    try { unfinished.BytesAvailable(); }
    catch (const TDCLMNPProtocolError&) { failed = true; }
    Check(failed && ValidLD(unfinishedPeer->TakeFrame(), 0xfe),
          "unterminated frame did not fail with protocol LD");

    // T3/A5: an inbound LT is usable while an outbound one is still pending;
    // stale NAKs must not release that LT or change credit.
    auto* duplexPeer = new Peer;
    TDCLMNPPipe duplex(duplexPeer, 1);
    Handshake(duplex, *duplexPeer);
    KUInt8 byte = 0x42;
    KUInt32 count = 1;
    duplex.Write(&byte, &count);
    Check(duplexPeer->TakeFrame() == Bytes({2, 4, 1, 0x42}), "T3 first LT missing");
    duplexPeer->Feed({3, 5, 0, 0}); // stale LA with zero credit
    duplexPeer->Feed({2, 4, 1, 0x57});
    Check(duplex.BytesAvailable() && !duplexPeer->outgoing.empty(),
          "T3 inbound LT not processed during send wait");
    Check(duplexPeer->TakeFrame() == Bytes({3, 5, 1, 1}), "T3 LA missing");
    count = 1;
    byte = 0;
    duplex.Read(&byte, &count);
    Check(byte == 0x57 && count == 1, "T3 input not readable");
    Check(duplexPeer->outgoing.empty(), "stale LA released pending LT");
    duplexPeer->Feed({3, 5, 1, 1});
    count = 1;
    duplex.Write(&byte, &count);
    Check(duplexPeer->TakeFrame() == Bytes({2, 4, 2, 0x57}),
          "matching LA failed to release pending LT");
    duplexPeer->Feed({3, 5, 1, 0}); // stale LA must not change credit
    Check(!duplex.BytesAvailable() && duplexPeer->outgoing.empty(),
          "pending stale LA produced a frame");
    duplexPeer->Feed({3, 5, 2, 0});
    Check(!duplex.BytesAvailable(), "zero-credit ACK rejected");
    duplexPeer->Feed({3, 5, 2, 1});
    Check(!duplex.BytesAvailable(), "credit-restoring repeated LA rejected");
    duplexPeer->Feed({3, 5, 2, 1});
    Check(!duplex.BytesAvailable() && duplexPeer->outgoing.empty(),
          "idle repeated LA produced a frame");
    count = 1;
    duplex.Write(&byte, &count);
    Check(duplexPeer->TakeFrame() == Bytes({2, 4, 3, 0x57}),
          "repeated idle LA did not preserve credit");

    // K1: InspectorLink's opt-in replies to every valid LA, including stale
    // ones; default Dock sessions above did not reply to any LA.
    auto* keepPeer = new Peer;
    TDCLMNPPipe keep(keepPeer, 1);
    Handshake(keep, *keepPeer);
    keep.SetKeepAlive(true);
    keepPeer->Feed({3, 5, 0, 1});
    Check(!keep.BytesAvailable() && keepPeer->TakeFrame() == Bytes({3, 5, 0, 1}),
          "keepalive did not reply to idle LA");
    count = 1;
    keep.Write(&byte, &count);
    Check(keepPeer->TakeFrame() == Bytes({2, 4, 1, 0x57}), "keepalive LT missing");
    keepPeer->Feed({3, 5, 0, 0});
    Check(!keep.BytesAvailable() && keepPeer->TakeFrame() == Bytes({3, 5, 0, 1}),
          "keepalive did not reply to stale LA");
    keepPeer->Feed({3, 5, 1, 1});
    Check(!keep.BytesAvailable() && keepPeer->TakeFrame() == Bytes({3, 5, 0, 1}),
          "keepalive did not reply to matching LA");

    // A3'/S1: a new inbound LT extends the pending ACK stall deadline;
    // repeated stale LAs do not. Poll hooks enforce an upper bound.
    auto* progressPeer = new Peer;
    TDCLMNPPipe progress(progressPeer, 1);
    progress.SetStallTimeout(std::chrono::milliseconds(25));
    Handshake(progress, *progressPeer);
    count = 1;
    progress.Write(&byte, &count);
    progressPeer->TakeFrame();
    auto start = std::chrono::steady_clock::now();
    bool injectedLT = false, injectedLA = false;
    progressPeer->onPoll = [&](Peer& p) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        Check(elapsed < std::chrono::milliseconds(150), "A3 progress wait hung");
        if (!injectedLT && elapsed >= std::chrono::milliseconds(15)) {
            p.Feed({2, 4, 1, 0x68});
            injectedLT = true;
        } else if (injectedLT && !injectedLA &&
                   elapsed >= std::chrono::milliseconds(32)) {
            p.Feed({3, 5, 1, 1});
            injectedLA = true;
        }
    };
    count = 1;
    progress.Write(&byte, &count);
    Check(injectedLT && injectedLA &&
          progressPeer->TakeFrame() == Bytes({3, 5, 1, 1}) &&
          progressPeer->TakeFrame() == Bytes({2, 4, 2, 0x57}),
          "inbound LT did not extend ACK deadline");
    progressPeer->onPoll = nullptr;

    auto* stalePeer = new Peer;
    TDCLMNPPipe stale(stalePeer, 1);
    stale.SetStallTimeout(std::chrono::milliseconds(20));
    Handshake(stale, *stalePeer);
    count = 1;
    stale.Write(&byte, &count);
    stalePeer->TakeFrame();
    start = std::chrono::steady_clock::now();
    int stalePolls = 0;
    int staleFrames = 0;
    stalePeer->onPoll = [&](Peer& p) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        Check(elapsed < std::chrono::milliseconds(150), "stale LA kept ACK wait alive");
        ++stalePolls;
        if (p.incoming.empty() && staleFrames < 4 &&
            elapsed >= std::chrono::milliseconds(3 + 4 * staleFrames)) {
            p.Feed({3, 5, 0, 1});
            ++staleFrames;
        }
    };
    failed = false;
    count = 1;
    try { stale.Write(&byte, &count); }
    catch (const TDCLMNPProtocolError&) { failed = true; }
    Check(failed && stalePolls > 3 &&
          ValidLD(stalePeer->TakeFrame(), 4) && stalePeer->outgoing.empty(),
          "stale LA extended ACK deadline");

    // D1: accepted bytes survive peer LD; all subsequent I/O reports EOF.
    auto* eofPeer = new Peer;
    TDCLMNPPipe eof(eofPeer);
    Handshake(eof, *eofPeer);
    eofPeer->Feed({2, 4, 1, 0x60, 0x61});
    eofPeer->Feed({4, 2, 1, 1, 5});
    Check(eof.PeerDisconnected() && eof.PeerDisconnectReason() == 5 &&
          eofPeer->TakeFrame() == Bytes({3, 5, 1, 1}) &&
          eofPeer->outgoing.empty(), "peer reason-only LD mishandled");
    KUInt8 input[2]{};
    count = 2;
    eof.Read(input, &count);
    Check(count == 2 && input[0] == 0x60 && input[1] == 0x61 &&
          !eof.BytesAvailable(), "buffered bytes lost after LD");
    bool gotEOF = false;
    count = 1;
    try { eof.Read(input, &count); }
    catch (const TDCLEOFException&) { gotEOF = true; }
    Check(gotEOF && count == 0, "Read did not report EOF after draining");
    gotEOF = false;
    count = 1;
    try { eof.Write(input, &count); }
    catch (const TDCLEOFException&) { gotEOF = true; }
    Check(gotEOF, "Write did not report peer EOF");
    gotEOF = false;
    try { eof.FlushOutput(); }
    catch (const TDCLEOFException&) { gotEOF = true; }
    Check(gotEOF && eofPeer->outgoing.empty(), "FlushOutput did not report EOF");
    eof.Disconnect();
    Check(!eof.DisconnectedCleanly() && eofPeer->outgoing.empty(),
          "Disconnect sent LD after peer LD");

    auto* waitingPeer = new Peer;
    TDCLMNPPipe waiting(waitingPeer, 1);
    Handshake(waiting, *waitingPeer);
    count = 1;
    waiting.Write(input, &count);
    waitingPeer->TakeFrame();
    waitingPeer->Feed(UserLD());
    gotEOF = false;
    count = 1;
    try { waiting.Write(input, &count); }
    catch (const TDCLEOFException&) { gotEOF = true; }
    Check(gotEOF && waiting.PeerDisconnectReason() == 0xff &&
          waitingPeer->outgoing.empty() && waitingPeer->closed,
          "peer LD during send wait did not report EOF");
}

int main()
{
    try {
        Check(Decode(Encode({3, 4, 0x10, 0x16, 0x10, 0x03})) ==
              Bytes({3, 4, 0x10, 0x16, 0x10, 0x03}), "codec roundtrip");
        auto* peer = new Peer;
        TDCLMNPPipe pipe(peer);
        Check(pipe.GetStallTimeout() == std::chrono::milliseconds(6000),
              "default stall timeout is not 6 s");
        pipe.SetStallTimeout(std::chrono::milliseconds(20));
        Handshake(pipe, *peer);
        KUInt8 data[] = {0x10, 0x16, 0x10, 0x03};
        KUInt32 size = sizeof(data);
        pipe.Write(data, &size);
        pipe.FlushOutput();
        Check(peer->TakeFrame() == Bytes({2, 4, 1, 0x10, 0x16, 0x10, 0x03}),
              "LT data or sequence incorrect");
        peer->Feed({3, 5, 1, 1});
        peer->Feed({2, 4, 1, 0x10, 0x16});
        KUInt8 result[2]{};
        size = 2;
        pipe.Read(result, &size);
        Check(size == 2 && result[0] == 0x10 && result[1] == 0x16,
              "inbound data incorrect");
        Check(peer->TakeFrame() == Bytes({3, 5, 1, 1}), "missing inbound ACK");
        peer->Feed({2, 4, 1, 0x10, 0x16});
        Check(!pipe.BytesAvailable(), "duplicate data delivered twice");
        Check(peer->TakeFrame() == Bytes({3, 5, 1, 1}), "duplicate not re-ACKed");
        pipe.Disconnect();
        Check(peer->TakeFrame() == UserLD() && pipe.DisconnectedCleanly(),
              "invalid user-initiated LD");
        Check(peer->closed, "underlying pipe left open");

        // D2': Dock 'disc' is a final LT; defer MNP LD until its LA arrives,
        // wait for the peer's LD, and fall back to LD 0xff if none arrives.
        auto* closingPeer = new Peer;
        TDCLMNPPipe closing(closingPeer);
        closing.SetStallTimeout(std::chrono::milliseconds(25));
        Handshake(closing, *closingPeer);
        const Bytes dockDisc{'n','e','w','t','d','o','c','k',
                             'd','i','s','c', 0, 0, 0, 0};
        size = dockDisc.size();
        closing.Write(dockDisc.data(), &size);
        closing.FlushOutput();
        auto finalLT = closingPeer->TakeFrame();
        Check(finalLT.size() == dockDisc.size() + 3 && finalLT[0] == 2 &&
              finalLT[1] == 4 && finalLT[2] == 1 &&
              std::equal(dockDisc.begin(), dockDisc.end(), finalLT.begin() + 3),
              "Dock disc LT missing");
        closingPeer->pendingAckSequence = 1;
        closingPeer->pollsUntilAck = 3; // injected after observing no early LD
        closing.Disconnect();
        Check(closing.DisconnectedCleanly() && closing.DisconnectReason() == 0xff &&
              closingPeer->observedNoPrematureLD &&
              closingPeer->TakeFrame() == UserLD() &&
              closingPeer->outgoing.empty() && closingPeer->closed,
              "disconnect did not wait for final ACK and send one valid LD");

        // D2' step 2: if the Newton sends its own LD (reason 0) after LA,
        // cDCL sends no LD and records a clean disconnect.
        auto* peerClosesPeer = new Peer;
        TDCLMNPPipe peerCloses(peerClosesPeer);
        peerCloses.SetStallTimeout(std::chrono::milliseconds(30));
        Handshake(peerCloses, *peerClosesPeer);
        size = dockDisc.size();
        peerCloses.Write(dockDisc.data(), &size); // left in mOutput for Disconnect to flush
        int closeStep = 0;
        peerClosesPeer->onPoll = [&](Peer& p) {
            if (closeStep == 0 && !p.outgoing.empty()) {
                Check(p.TakeFrame()[2] == 1, "Disconnect did not flush buffered LT");
                p.Feed({3, 5, 1, 1});
                closeStep = 1;
            } else if (closeStep == 1) {
                p.Feed({7, 2, 1, 1, 0, 2, 1, 0, 0, 0});
                closeStep = 2;
            }
        };
        peerCloses.Disconnect();
        Check(closeStep == 2 && peerCloses.DisconnectedCleanly() &&
              peerCloses.PeerDisconnectReason() == 0 &&
              peerCloses.DisconnectReason() == 0 &&
              peerClosesPeer->outgoing.empty() && peerClosesPeer->closed,
              "D2' sent LD after the Newton's own LD");

        // D2/A3: absent final LA must time out, not report clean disconnect.
        auto* unackedPeer = new Peer;
        TDCLMNPPipe unacked(unackedPeer);
        unacked.SetStallTimeout(std::chrono::milliseconds(20));
        Handshake(unacked, *unackedPeer);
        size = 1;
        unacked.Write(data, &size);
        unacked.FlushOutput();
        unackedPeer->TakeFrame();
        unacked.Disconnect();
        Check(!unacked.DisconnectedCleanly() && unacked.DisconnectReason() == 4 &&
              unackedPeer->closed &&
              ValidLD(unackedPeer->TakeFrame(), 4) && unackedPeer->outgoing.empty(),
              "unacknowledged final LT was treated as clean disconnect");

        // T1/T5: segmentation, no write truncation, and a smaller output buffer.
        auto* segmentedPeer = new Peer;
        TDCLMNPPipe segmented(segmentedPeer, 100);
        Handshake(segmented, *segmentedPeer);
        segmentedPeer->autoAck = true;
        Bytes payload(357);
        for (size_t i = 0; i < payload.size(); ++i) payload[i] = i;
        size = payload.size();
        segmented.Write(payload.data(), &size);
        Check(size == payload.size(), "oversized write truncated");
        segmented.FlushOutput();
        Bytes reconstructed;
        for (KUInt8 seq = 1; seq <= 4; ++seq) {
            auto frame = segmentedPeer->TakeFrame();
            Check(frame.size() >= 4 && frame.size() <= 103 &&
                  frame[0] == 2 && frame[1] == 4 && frame[2] == seq,
                  "bad segmentation or sequence");
            reconstructed.insert(reconstructed.end(), frame.begin() + 3, frame.end());
        }
        Check(reconstructed == payload, "segmented payload changed");

        // T1: the default payload bound is 253, not the LR's N401=64.
        auto* maximumPeer = new Peer;
        TDCLMNPPipe maximum(maximumPeer);
        Handshake(maximum, *maximumPeer);
        maximumPeer->autoAck = true;
        Bytes large(254, 0x33);
        size = large.size();
        maximum.Write(large.data(), &size);
        maximum.FlushOutput();
        Check(maximumPeer->TakeFrame().size() == 256 &&
              maximumPeer->TakeFrame() == Bytes({2, 4, 2, 0x33}),
              "default payload exceeds or falls short of 253");

        // A4: modulo-256 send and receive sequence.
        auto* wrapPeer = new Peer;
        TDCLMNPPipe wrap(wrapPeer, 1);
        Handshake(wrap, *wrapPeer);
        wrapPeer->autoAck = true;
        Bytes wrapping(256, 0x10);
        size = wrapping.size();
        wrap.Write(wrapping.data(), &size);
        Check(size == wrapping.size(), "wrap write truncated");
        for (int i = 1; i <= 256; ++i)
            Check(wrapPeer->TakeFrame() == Bytes({2, 4, static_cast<KUInt8>(i), 0x10}),
                  "outbound wrap or escaping failed");
        for (int i = 1; i <= 256; ++i) {
            wrapPeer->Feed({2, 4, static_cast<KUInt8>(i), 0x41});
            Check(wrap.BytesAvailable(), "inbound sequence rejected");
            wrapPeer->TakeFrame(); // LA, credit 1
        }
        wrapPeer->Feed({2, 4, 0, 0x41});
        Check(wrap.BytesAvailable(), "duplicate at wrap rejected");
        Check(wrapPeer->TakeFrame() == Bytes({3, 5, 0, 1}),
              "duplicate sequence at wrap mishandled");

        // A1: an ACK with zero credit releases the frame, but does not
        // authorize another until a subsequent LA restores credit.
        auto* creditPeer = new Peer;
        TDCLMNPPipe credit(creditPeer);
        Handshake(credit, *creditPeer);
        size = sizeof(data);
        credit.Write(data, &size);
        credit.FlushOutput();
        Check(creditPeer->TakeFrame()[2] == 1, "first LT missing");
        creditPeer->Feed({3, 5, 1, 0});
        Check(!credit.BytesAvailable(), "unexpected data while waiting for credit");
        Check(creditPeer->outgoing.empty(), "LT sent at zero credit");
        creditPeer->Feed({3, 5, 1, 1});
        size = sizeof(data);
        credit.Write(data, &size);
        credit.FlushOutput();
        Check(creditPeer->TakeFrame()[2] == 2, "credit restoration failed");

        // A3: no retransmission, single error type and LD on ACK timeout.
        auto* timeoutPeer = new Peer;
        TDCLMNPPipe timeout(timeoutPeer, 1);
        timeout.SetStallTimeout(std::chrono::milliseconds(20));
        Handshake(timeout, *timeoutPeer);
        size = 1;
        timeout.Write(data, &size);
        Check(timeoutPeer->TakeFrame() == Bytes({2, 4, 1, 0x10}),
              "timed LT missing");
        size = 1;
        bool failed = false;
        try { timeout.Write(data, &size); timeout.FlushOutput(); }
        catch (const TDCLMNPProtocolError& e) {
            failed = std::strcmp(e.GetExceptionName(), "protocol error, link dropped") == 0;
        }
        Check(failed && timeoutPeer->closed && !timeout.PeerDisconnected() &&
              timeout.DisconnectReason() == 4,
              "ACK timeout did not drop link");
        Check(ValidLD(timeoutPeer->TakeFrame(), 4) &&
              timeoutPeer->outgoing.empty(), "LT retransmitted on timeout");

        // H3/X1: malformed negotiation and corrupt wire both drop the link.
        auto* badPeer = new Peer;
        TDCLMNPPipe bad(badPeer);
        auto badLR = PeerLR();
        badLR[13] = 3; // unsupported framing mode
        badPeer->Feed(badLR);
        failed = false;
        try { bad.Connected(nullptr); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && ValidLD(badPeer->TakeFrame(), 3),
              "unsupported negotiation was accepted");

        // H3: reject missing/duplicate mandatory entries, malformed TLVs and
        // unsupported offers without accepting the peer's additional fields.
        Bytes missing = PeerLR();
        missing.erase(missing.begin() + 21, missing.begin() + 24); // no optimization
        Bytes duplicate = PeerLR();
        duplicate.insert(duplicate.begin() + 24, {2, 1, 2});
        Bytes truncated = PeerLR();
        truncated.back() = 0x10; // last TLV promises six bytes, now short
        truncated.pop_back();
        Bytes dangling = PeerLR();
        dangling.push_back(0x7f); // incomplete unknown TLV, not ignorable data
        Bytes badConstant = PeerLR();
        badConstant[10] = 0;
        Bytes badWindow = PeerLR();
        badWindow[16] = 0;
        Bytes badN401 = PeerLR();
        badN401[19] = 63;
        Bytes badOptimization = PeerLR();
        badOptimization[23] = 1;
        for (const auto& invalid : {missing, duplicate, truncated, dangling,
                                    badConstant, badWindow, badN401, badOptimization}) {
            auto* p = new Peer;
            TDCLMNPPipe link(p);
            p->Feed(invalid);
            bool rejected = false;
            try { link.Connected(nullptr); }
            catch (const TDCLMNPProtocolError&) { rejected = true; }
            Check(rejected && ValidLD(p->TakeFrame(), 3) &&
                  p->outgoing.empty(), "invalid LR accepted");
        }

        auto* corruptPeer = new Peer;
        TDCLMNPPipe corrupt(corruptPeer);
        Handshake(corrupt, *corruptPeer);
        auto damaged = Encode({2, 4, 1, 0x41});
        damaged.back() ^= 1;
        corruptPeer->incoming.insert(corruptPeer->incoming.end(),
                                     damaged.begin(), damaged.end());
        failed = false;
        try { corrupt.BytesAvailable(); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && ValidLD(corruptPeer->TakeFrame(), 0xfe) &&
              corruptPeer->closed, "bad CRC was not fatal");

        // H2: additional entries may be omitted or replaced by unknown TLVs.
        auto minimal = PeerLR();
        minimal.erase(minimal.begin() + 24, minimal.end());
        auto* minimalPeer = new Peer;
        TDCLMNPPipe minimalLink(minimalPeer);
        minimalPeer->Feed(minimal);
        minimalPeer->Feed({3, 5, 0, 1});
        minimalLink.Connected(nullptr);
        Check(minimalPeer->TakeFrame() == HostLR(), "minimal LR rejected");

        auto extended = minimal;
        extended.insert(extended.end(), {0x7f, 3, 0x10, 0x55, 0xff});
        auto* optionalPeer = new Peer;
        TDCLMNPPipe optional(optionalPeer);
        optionalPeer->Feed(extended);
        optionalPeer->Feed({3, 5, 0, 1});
        optional.Connected(nullptr);
        Check(optionalPeer->TakeFrame() == HostLR(), "unknown LR entry rejected");

        // H2: mandatory entries need not arrive in the trace's order.
        auto reordered = minimal;
        std::swap_ranges(reordered.begin() + 11, reordered.begin() + 14,
                         reordered.begin() + 21); // framing and optimization
        auto* reorderedPeer = new Peer;
        TDCLMNPPipe reorderedLink(reorderedPeer);
        reorderedPeer->Feed(reordered);
        reorderedPeer->Feed({3, 5, 0, 1});
        reorderedLink.Connected(nullptr);
        Check(reorderedPeer->TakeFrame() == HostLR(), "reordered LR rejected");

        // H4: no LR or missing confirming LA must expire rather than hang.
        auto* noLRPeer = new Peer;
        TDCLMNPPipe noLR(noLRPeer);
        noLR.SetStallTimeout(std::chrono::milliseconds(15));
        failed = false;
        try { noLR.Connected(nullptr); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && ValidLD(noLRPeer->TakeFrame(), 1),
              "handshake timeout failed");
        auto* noLAPeer = new Peer;
        TDCLMNPPipe noLA(noLAPeer);
        noLA.SetStallTimeout(std::chrono::milliseconds(15));
        noLAPeer->Feed(PeerLR());
        failed = false;
        try { noLA.Connected(nullptr); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && noLAPeer->TakeFrame().size() == 24 &&
              ValidLD(noLAPeer->TakeFrame(), 1),
              "missing handshake LA did not expire");

        // A2: an ACK with zero credit eventually fails a queued send.
        auto* stallPeer = new Peer;
        TDCLMNPPipe stall(stallPeer);
        stall.SetStallTimeout(std::chrono::milliseconds(15));
        Handshake(stall, *stallPeer);
        size = 1;
        stall.Write(data, &size);
        stall.FlushOutput();
        stallPeer->TakeFrame();
        stallPeer->Feed({3, 5, 1, 0});
        size = 1;
        stall.Write(data, &size);
        failed = false;
        try { stall.FlushOutput(); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && ValidLD(stallPeer->TakeFrame(), 4),
              "credit stall did not drop link");

        // X3-X6: all other unexpected frames are fatal, never delivered.
        const Bytes anomalies[] = {
            {3, 5, 3, 1}, {2, 4, 3, 0x44}, {3, 5, 0, 2},
            PeerLR(), {2, 4, 1}
        };
        for (const auto& anomaly : anomalies) {
            auto* p = new Peer;
            TDCLMNPPipe link(p);
            Handshake(link, *p);
            p->Feed(anomaly);
            failed = false;
            try { link.BytesAvailable(); }
            catch (const TDCLMNPProtocolError&) { failed = true; }
            Check(failed && !link.BytesAvailable() &&
                  ValidLD(p->TakeFrame(), 0xfe) && p->outgoing.empty(),
                  "anomalous frame accepted");
        }

        // D1: a valid peer LD ends the link; malformed LD is fatal.
        auto* disconnectPeer = new Peer;
        TDCLMNPPipe disconnect(disconnectPeer);
        Handshake(disconnect, *disconnectPeer);
        disconnectPeer->Feed(UserLD());
        Check(disconnect.PeerDisconnected() && disconnect.PeerDisconnectReason() == 0xff &&
              disconnectPeer->closed && disconnectPeer->outgoing.empty(),
              "valid peer LD not processed");

        auto* malformedPeer = new Peer;
        TDCLMNPPipe malformed(malformedPeer);
        Handshake(malformed, *malformedPeer);
        malformedPeer->Feed({2, 2, 0});
        failed = false;
        try { malformed.PeerDisconnected(); }
        catch (const TDCLMNPProtocolError&) { failed = true; }
        Check(failed && malformedPeer->closed &&
              ValidLD(malformedPeer->TakeFrame(), 0xfe) && malformedPeer->outgoing.empty(),
              "malformed peer LD accepted");

        TestAdditionalScenarios();
        TestPeerLDParsing();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "MNP acceptance: %s\n", e.what());
        return 1;
    } catch (const TDCLException& e) {
        std::fprintf(stderr, "MNP acceptance: %s\n", e.GetExceptionName());
        return 1;
    }
}
