#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>
#include <DCL/Interfaces/TDCLThread.h>
#include "TTestApplication.h"
#include "TTestServer.h"

#if TARGET_OS_WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
#include <DCL/Interfaces/Win32/TDCLWin32Threads.h>
using PlatformSocket = TDCLWin32Socket;
using PlatformThreads = TDCLWin32Threads;
#else
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
#include <DCL/Interfaces/POSIX/TDCLPThreads.h>
using PlatformSocket = TDCLBSDSocket;
using PlatformThreads = TDCLPThreads;
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <future>
#include <mutex>
#include <stdexcept>

namespace {

void Require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Keep the listener alive after its blocking wait has been interrupted. This
// makes returning from StopListening before actual worker exit observable.
class DelayedLayer : public TDCLSyncCommLayer
{
public:
    explicit DelayedLayer(IDCLThreads* threads) : TDCLSyncCommLayer(threads) {}

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
    void DoStartListening() override
    {
        std::lock_guard<std::mutex> lock(mutex);
        started = true;
        changed.notify_all();
    }

    void DoStopListening() override
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
        changed.notify_all();
    }

    Boolean WaitForIncomingRequest() override
    {
        std::unique_lock<std::mutex> lock(mutex);
        changed.wait(lock, [this] { return stopping && allowExit; });
        serverKeptAlive = HasServer();
        return false;
    }

    TDCLPipe* DoAccept() override { return nullptr; }
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
    explicit TestSocket(IDCLThreads* threads) : PlatformSocket(threads, 0) {}

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

void TestDelayedShutdown(IDCLThreads* threads, TDCLServer* server)
{
    DelayedLayer layer(threads);
    for (unsigned run = 0; run < 10; ++run) {
        layer.Prepare();
        layer.StartListening(server);
        layer.WaitStarted();
        auto stopped = std::async(std::launch::async, [&layer] { layer.StopListening(); });
        layer.WaitStopping();
        bool waited = stopped.wait_for(std::chrono::milliseconds(20)) == std::future_status::timeout;
        // Always release the worker before checking assertions or joining.
        layer.AllowExit();
        stopped.get();
        Require(waited, "StopListening returned before the worker was allowed to exit");
        Require(layer.exited && !layer.exception, "StopListening did not wait for normal worker exit");
        Require(layer.serverKeptAlive, "Server association was cleared while the worker was running");
        Require(!layer.HasServer(), "Server association was not cleared after worker exit");
        layer.StopListening(); // Stopping twice must remain harmless.
    }
}

void TestSocketRestartAndDestruction(IDCLThreads* threads, TDCLServer* server)
{
    TestSocket layer(threads);
    for (unsigned run = 1; run <= 100; ++run) {
        layer.StartAndWait(server);
        layer.StopListening();
        Require(layer.exits == run, "Socket listener was still running after StopListening");
    }
    for (unsigned run = 0; run < 100; ++run) {
        TestSocket temporary(threads);
        temporary.StartAndWait(server);
        temporary.StopListening();
        Require(temporary.exits == 1, "Socket worker outlived shutdown before destruction");
    }
}

} // namespace

int main()
{
    try {
        auto* threads = new PlatformThreads();
        TTestApplication application(threads); // Owns the thread factory via GetThreadsIntf().
        TTestServer server(&application);
        TestDelayedShutdown(threads, &server);
        TestSocketRestartAndDestruction(threads, &server);
        std::puts("Communication layer shutdown tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
    } catch (...) {
        std::fputs("Unexpected shutdown test exception\n", stderr);
    }
    return 1;
}
