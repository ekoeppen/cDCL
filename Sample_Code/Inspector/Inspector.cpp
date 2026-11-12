// Minimal, receive-only Newton Inspector console over a serial device or PTY.
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/TDCLInspectorLink.h>
#include <DCL/Server/TDCLSimpleServer.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
volatile std::sig_atomic_t interrupted = 0;

void OnSignal(int) { interrupted = 1; }

class InspectorApp final : public TDCLApplication {
public:
    bool Done() const { return done.load(); }
    bool Failed() const { return failed.load(); }

    void WaitConnection(TDCLServer*, TDCLCommLayer*) override {
        std::cerr << "Waiting for Inspector connection...\n";
    }
    void Connected(TDCLLink*, TDCLCommLayer*) override {
        std::cerr << "MNP connection established\n";
    }
    void ConnectedToNewtonDevice(TDCLLink*, const KUInt16*) override {
        std::cerr << "Inspector connected (Newton text follows)\n";
    }
    void Disconnecting(TDCLLink*) override {}
    void Disconnected(TDCLLink*) override {
        std::cerr << "Inspector disconnected\n";
        done = true;
    }
    void ServerIsDown(TDCLServer*) override { done = true; }
    void Cancel(TDCLLink*) override { done = true; }
    void Timeout(TDCLLink*) override { Error("Inspector timed out"); }
    void AppCommandBeingProcessed(TDCLLink*, TDCLAppCommand*) override {}
    void AppCommandProcessed(TDCLLink*, TDCLAppCommand*, TDCLException* error) override {
        if (error) Error(error->GetExceptionName());
    }
    void DockCommandBeingProcessed(TDCLLink*, TDCLDockCommand*, TDCLLinkEngine*) override {}
    void DockCommandProcessed(TDCLLink*, TDCLLinkEngine*, TDCLException* error) override {
        if (error) Error(error->GetExceptionName());
    }
    void HandleServerException(TDCLServer*, TDCLException* error) override { Error(error); }
    void HandleCommLayerException(TDCLCommLayer*, TDCLException* error) override { Error(error); }
    void HandleLinkException(TDCLLink*, TDCLException* error) override { Error(error); }
    void HandleException(TDCLException* error) override { Error(error); }

protected:
    IDCLFiles* CreateFilesIntf() override { return new TDCLPOSIXFiles; }
    IDCLThreads* CreateThreadsIntf() override { return new TDCLStdThreads; }

private:
    void Error(TDCLException* error) {
        Error(error ? error->GetExceptionName() : "unknown error");
    }
    void Error(const char* message) {
        std::cerr << "Inspector error: " << message << '\n';
        failed = true;
        done = true;
    }
    std::atomic<bool> done{false};
    std::atomic<bool> failed{false};
};

// TDCLInspectorLink::Connected() reads forever, bypassing TDCLLink's event
// loop. Use that loop so a posted disconnect can actually be processed.
class ConsoleLink final : public TDCLInspectorLink {
public:
    explicit ConsoleLink(TDCLApplication* app) : TDCLInspectorLink(app) {}
    bool Stopped() { return GetThreadState() == IDCLThreads::kStopped; }

    void Connected() override {
        static_cast<TDCLMNPPipe*>(GetPipe())->SetKeepAlive(true);
        TDCLLink::Connected();
    }

    void DoDisconnect() override {
        // Inspector is not the Dock protocol: do not send a Dock 'disc'.
        GetApplication()->Disconnecting(this);
        if (GetPipe() && !GetPipe()->PeerDisconnected()) GetPipe()->Disconnect();
    }
};

bool WaitUntilStopped(TDCLSimpleServer& server, ConsoleLink& link,
                      std::chrono::milliseconds limit) {
    auto deadline = std::chrono::steady_clock::now() + limit;
    while (std::chrono::steady_clock::now() < deadline) {
        if (server.GetState() == TDCLServer::kStopped &&
            link.Stopped()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return server.GetState() == TDCLServer::kStopped &&
           link.Stopped();
}

void Usage(const char* name) {
    std::cout << "Usage: " << name << " --serial <device-or-pty> [--baud <rate>]\n"
              << "Receive and display Newton Inspector text over MNP (default: 38400 baud).\n";
}
} // namespace

int main(int argc, char** argv) {
    std::string device;
    long baud = 38400;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { Usage(argv[0]); return 0; }
        if (arg == "--serial" && i + 1 < argc) device = argv[++i];
        else if (arg == "--baud" && i + 1 < argc) {
            try {
                std::string value = argv[++i];
                size_t used = 0;
                baud = std::stol(value, &used);
                if (used != value.size()) throw std::invalid_argument("baud");
            } catch (const std::exception&) {
                std::cerr << "Invalid baud rate\n";
                return 1;
            }
        } else { Usage(argv[0]); return 1; }
    }
    if (device.empty()) { Usage(argv[0]); return 1; }
    if (baud != 2400 && baud != 4800 && baud != 9600 && baud != 19200 &&
        baud != 38400 && baud != 57600 && baud != 115200 && baud != 230400) {
        std::cerr << "Unsupported baud rate\n";
        return 1;
    }

    InspectorApp app;
    auto port = std::make_unique<TDCLFDSerialPort>(
        app.GetThreadsIntf(), device.c_str(), baud,
        static_cast<KUInt32>(TDCLFDSerialPort::kModeMNP));
    // A link's destructor may still notify its server after a worker error.
    // Keep the server alive until after the link has been destroyed.
    TDCLSimpleServer server(&app, port.get());
    ConsoleLink link(&app);
    server.SetLink(&link);
    std::signal(SIGINT, OnSignal);
    std::signal(SIGTERM, OnSignal);
    try {
        server.Start();
        while (!interrupted && !app.Done())
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        server.Stop();
        // A pending MNP LT can take the full 6 s ACK deadline to release.
        if (!WaitUntilStopped(server, link, std::chrono::seconds(8))) {
            // Never destruct a live worker and its pipe.
            std::cerr << "Inspector shutdown timed out (server state "
                      << server.GetState() << ", link stopped " << link.Stopped() << ")\n";
            std::_Exit(1);
        }
    } catch (const TDCLException& error) {
        std::cerr << "Inspector error: " << error.GetExceptionName() << '\n';
        std::_Exit(1);
    } catch (const std::exception& error) {
        std::cerr << "Inspector error: " << error.what() << '\n';
        std::_Exit(1);
    }
    return interrupted ? 130 : (app.Failed() ? 1 : 0);
}
