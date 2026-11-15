# DCL Threading Architecture

DCL uses **C++20 standard concurrency primitives** (`std::jthread`, `std::mutex`,
`std::condition_variable_any`, `std::stop_token`, `<atomic>`, `<chrono>`) for its
threading and cancellation model. Legacy threading runtime abstractions (`IDCLThreads`,
`TDCLThread`) are removed from the public API: callers construct communication layers,
servers, and links directly without supplying a thread factory.

## Core Building Blocks

### `TStoppable` and `TStopForwarder` (`DCL/Interfaces/TStoppable.h`)
- **`TStoppable`** wraps a `std::stop_source` for transport cancellation on `TDCLCommLayer` and
  `TDCLPipe`, exposing `RequestStop()`, `StopRequested()`, `GetStopToken()`, and
  `ResetStopSource()`.
- **`TStopForwarder`** is an RAII guard using `std::stop_callback` to propagate stop requests
  from `TDCLLink`'s `std::jthread` `stop_token` down into the active `TDCLPipe`.

### `TDCLWakeableWait` (`DCL/Communication_Layers/Common/TDCLWakeableWait.h`)
- Shared `WaitReady()`, `PollNow()`, deadline loop, and `std::stop_callback` wakeup in
  `Common/TDCLWakeableWait.cpp`, backed by minimal OS primitives:
  - **POSIX (`POSIX/TDCLWakeableWaitPOSIX.cpp`)**: `poll()` with a non-blocking self-pipe.
  - **Win32 (`Win32/TDCLWakeableWaitWin32.cpp`)**: `WSAPoll()` with a loopback socket pair.

## Server Setup & Ownership

- **Unique-ownership configuration**: `TDCLSingleLinkServer`, `TDCLSimpleServer` and `TDCLOneLinkServer`
  take ownership of communication layers and links.
- **Pre-`Start()` configuration lock**: `SetLink()` and `AddCommLayer()` acquire
  `mStateMutex` and reject modifications with `TDCLBadStateError` once `Start()` has been
  called. Servers can still be stopped/killed and restarted with their configured link and communication layers.

## Cooperative Shutdown Lifecycle

- **`TDCLSyncCommLayer` and `TSyncPipe`**: each owns a private worker plus a wake gate
  (`WakeWorker()` / `WaitForWorkerWake()`) observing `TStoppable::GetStopToken()`.
  `TDCLFDSerialPort::Select()` additionally uses a `std::stop_callback` (`TForwardStop`)
  to wake `TDCLWakeableWait` on port `Reset()` as well as layer stop.
- **`TDCLServer`**: `Stop()` posts `kStopServer` for graceful shutdown. `Kill()` synchronizes
  with concurrent `Start()` / `Kill()` calls, requests stop on `mThread` and active
  comm layers/links (via `DoRequestStop()`), waits via `Wait()`, and runs `DoKill()`.
- **`TDCLSingleLinkServer`** (`TDCLSimpleServer` and `TDCLOneLinkServer`): coordinates
  cooperative stop and joining of the owned `TDCLLink` and communication layers via
  `IsShutdownWorkerThread()`, `DoRequestStop()`, and `DoKill()` when `Kill()` is triggered from
  inside or outside the link worker (and in subclass destructors before member teardown).
- **`TDCLLink`**: `Run()` attaches a `TStopForwarder` to `*mPipe`; `RequestStop()` wakes
  `mEventQueue` and interrupts blocking socket/serial I/O cleanly via
  `TDCLUserCancelledException` so the link exits without forced thread termination.
