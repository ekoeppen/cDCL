// ==============================
// File:			CommLayerShutdownTests.cpp
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
// The Original Code is CommLayerShutdownTests.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>
#include "TTestApplication.h"
#include "TTestServer.h"

#ifdef _WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
using PlatformSocket = TDCLWin32Socket;
#else
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
using PlatformSocket = TDCLBSDSocket;
#endif

#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>

namespace {

// Keep the listener alive after its blocking wait has been interrupted. This
// makes returning from StopListening before actual worker exit observable.
class DelayedLayer : public TDCLSyncCommLayer
{
public:
    DelayedLayer() = default;

    void Prepare()
    {
        started = false;
        stopping = false;
        allowExit = false;
        exited = false;
        serverKeptAlive = false;
        exception = false;
    }

    void WaitStarted()
    {
        std::unique_lock<std::mutex> lock(mutex);
        changed.wait(lock, [this] { return started; });
    }

    void WaitStopping()
    {
        std::unique_lock<std::mutex> lock(mutex);
        changed.wait(lock, [this] { return stopping; });
    }

    void AllowExit()
    {
        std::lock_guard<std::mutex> lock(mutex);
        allowExit = true;
        changed.notify_all();
    }

    bool HasServer() const { return GetServer() != nullptr; }
    void SetTimeout(long) override {}
    long GetTimeout() override { return 0; }
    void Disconnected(TDCLPipe*) override {}

    std::atomic<bool> exited{false};
    std::atomic<bool> serverKeptAlive{false};
    std::atomic<bool> exception{false};

protected:
    void DoStartListening() override {}

    void DoStopListening() override
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
        changed.notify_all();
    }

    bool WaitForIncomingRequest() override
    {
        std::unique_lock<std::mutex> lock(mutex);
        started = true;
        changed.notify_all();
        changed.wait(lock, [this] { return stopping && allowExit; });
        serverKeptAlive = HasServer();
        return false;
    }

    std::unique_ptr<TDCLPipe> DoAccept() override { return nullptr; }
    void DoRefuse() override {}

    void Run() override
    {
        TDCLSyncCommLayer::Run();
        exited = true;
    }

    void HandleException(TDCLException*) override { exception = true; }

private:
    std::mutex mutex;
    std::condition_variable changed;
    bool started = false;
    bool stopping = false;
    bool allowExit = false;
};

// Signal readiness only after the actual sockets have been initialized. Port 0
// avoids collisions with other test processes or a running Newton dock server.
class TestSocket : public PlatformSocket
{
public:
    TestSocket() : PlatformSocket(0) {}

    void StartAndWait(TDCLServer* server)
    {
        ready = std::promise<void>();
        auto started = ready.get_future();
        StartListening(server);
        started.get();
    }

    std::atomic<unsigned> exits{0};

protected:
    void DoStartListening() override
    {
        try {
            PlatformSocket::DoStartListening();
            ready.set_value();
        } catch (...) {
            ready.set_exception(std::current_exception());
            throw;
        }
    }

    void Run() override
    {
        TDCLSyncCommLayer::Run();
        ++exits;
    }

private:
    std::promise<void> ready;
};

} // namespace

TEST_SUITE("CommLayerShutdown")
{
    TEST_CASE("Delayed shutdown waits for worker exit")
    {
        TTestApplication application;
        TTestServer server(&application);
        DelayedLayer layer;
        for (unsigned run = 0; run < 10; ++run) {
            layer.Prepare();
            layer.StartListening(&server);
            layer.WaitStarted();
            auto stopped = std::async(std::launch::async, [&layer] { layer.StopListening(); });
            layer.WaitStopping();
            bool waited = stopped.wait_for(std::chrono::milliseconds(20)) == std::future_status::timeout;
            // Always release the worker before checking assertions or joining.
            layer.AllowExit();
            stopped.get();
            REQUIRE_MESSAGE(waited, "StopListening returned before the worker was allowed to exit");
            REQUIRE_MESSAGE((layer.exited && !layer.exception),
                            "StopListening did not wait for normal worker exit");
            REQUIRE_MESSAGE(layer.serverKeptAlive,
                            "Server association was cleared while the worker was running");
            REQUIRE_MESSAGE(!layer.HasServer(),
                            "Server association was not cleared after worker exit");
            layer.StopListening(); // Stopping twice must remain harmless.
        }
    }

    TEST_CASE("Socket restart and destruction")
    {
        TTestApplication application;
        TTestServer server(&application);
        TestSocket layer;
        for (unsigned run = 1; run <= 100; ++run) {
            layer.StartAndWait(&server);
            layer.StopListening();
            REQUIRE_MESSAGE(layer.exits == run,
                            "Socket listener was still running after StopListening");
        }
        for (unsigned run = 0; run < 100; ++run) {
            TestSocket temporary;
            temporary.StartAndWait(&server);
            temporary.StopListening();
            REQUIRE_MESSAGE(temporary.exits == 1,
                            "Socket worker outlived shutdown before destruction");
        }
    }
}
