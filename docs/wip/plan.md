# Implementation Plan

## Scope

## Evidence


## Goal

### Objective

### Success criteria

## Design

### Decisions

### Packages

### Functions

### Structures

## Steps

### [x] 1. Add TStoppable and RequestStop/Join to IDCLThreads and TDCLThread

#### Precondition

Clean baseline with Yield(), Suspend(), and Resume() removed; IDCLThreads/TDCLThread lack Join() and std::stop_token support.

#### Action

Create TStoppable and TStopForwarder (e.g. DCL/Interfaces/TStoppable.h), add Join() to IDCLThreads::IThread (implemented via pthread_join in TDCLPThreads and WaitForSingleObject in TDCLWin32Threads), inherit TStoppable in TDCLThread with RequestStop()/Join()/GetStopToken()/StopRequested(), and make TDCLThread::WaitNextEvent() exit cleanly when StopRequested() is true. Keep IThread::Stop() temporarily for unmigrated callers. Register TStoppable.h in CMakeLists.txt DCL_HEADERS.

#### Result

TDCLThread supports cooperative cancellation (RequestStop, GetStopToken, StopRequested) and deterministic joining (Join), and WaitNextEvent unblocks on cancellation.

#### Verification

Build macOS (cmake --build build) and MinGW (cmake --build build-mingw), add/update UTestThreads tests for RequestStop and Join, and run ctest --test-dir build -C Debug -LE optional --output-on-failure.

### [x] 2. Add TStoppable to TDCLCommLayer/TDCLPipe and replace sync yield loops with Join

#### Precondition

Step 1 complete: TStoppable, RequestStop(), and Join() are available on TDCLThread.

#### Action

Make TDCLCommLayer and TDCLPipe inherit TStoppable. Update TDCLPipeProxy (TDCLMNPPipe, TDCLBufferedPipe, TDCLHammerPipe) to forward RequestStop() to mSubPipe. Trigger RequestStop() in TDCLCommLayer::StopListening() and TDCLPipe::Disconnect(). Replace the std::this_thread::yield() spin-wait loops in TDCLSyncCommLayer::StopListening() and TSyncPipe::Disconnect() with Join(), guarding against self-join or double-join.

#### Result

Comm layers and pipes propagate stop signals down the proxy chain, and TDCLSyncCommLayer / TSyncPipe shut down cleanly via Join() without spin-waiting.

#### Verification

Build macOS and MinGW targets and run ctest --test-dir build -C Debug -LE optional --output-on-failure.

### [x] 3. Encapsulate wakeable blocking I/O with std::stop_token in sockets and serial ports

#### Precondition

Step 2 complete: TDCLCommLayer and TDCLPipe expose GetStopToken() and RequestStop().

#### Action

Introduce a platform-isolated wakeable I/O wait helper (WaitReady with direction, timeout, and std::stop_token using std::stop_callback + self-pipe/poll on POSIX and WSAPoll/event on Win32). Migrate TDCLSocket (TDCLBSDSocket, TDCLWin32Socket) WaitForIncomingRequest, DoAccept, WaitForIncomingData, Read, and Write to use it (replacing duplicate socketpairs and SO_RCVTIMEO/SO_SNDTIMEO uninterruptible blocks). Migrate TDCLFDSerialPort WaitForIncomingRequest, WaitForIncomingData, Read, Write, and DoDisconnect (avoiding cross-thread close(mSerialFD) and indefinite tcdrain on cancel) to use it. Remove TDCLThread::Stop() calls from DoStopListening().

#### Result

All socket and POSIX serial blocking I/O (both input waits and backpressured writes) unblock immediately on stop_token cancellation without polling loops or cross-thread fd closing.

#### Verification

Build macOS and MinGW targets, run ctest --test-dir build -C Debug -LE optional --output-on-failure, and verify socket/serial cancellation tests.

### [x] 4. Migrate TDCLLink/TDCLServer shutdown to RequestStop+Join and remove IThread::Stop

#### Precondition

Step 3 complete: socket and serial blocking I/O exit cleanly when their stop_token is cancelled.

#### Action

Attach TStopForwarder in TDCLLink::Run() to forward TDCLLink's stop token to mPipe. Update TDCLSimpleServer::DoKill(), TDCLOneLinkServer::DoKill(), TDCLServer::Kill(), and destructors to cancel first (mPipe->RequestStop(), mLink->RequestStop(), mCommLayer->RequestStop()), Join() TDCLLink before deleting mPipe, StopListening() on comm layers, and Join() TDCLServer's thread (replacing TDCLThread::Stop()). Update UTestThreads and remove IThread::Stop() and TDCLThread::Stop() (pthread_cancel / TerminateThread) from IDCLThreads, TDCLThread, TDCLPThreads, and TDCLWin32Threads.

#### Result

Server, link, comm layer, and pipe shutdown are fully cooperative and deterministic with no use-after-free on mPipe and no forced thread termination (pthread_cancel / TerminateThread).

#### Verification

Build macOS and MinGW targets, update UTestThreads master outputs if needed, and run ctest --test-dir build -C Debug -LE optional --output-on-failure.

### [x] 5. Consolidate DCL threading backends onto C++20 std primitives

#### Precondition

Step 4 complete: Suspend, Resume, Yield, and forced Stop are all removed from IDCLThreads.

#### Action

Replace the separate POSIX (TDCLPThreads) and Win32 (TDCLWin32Threads) backends with a single portable C++20 implementation backed by std::jthread / std::thread, std::mutex, and std::condition_variable_any (using TStoppable's stop_token), removing platform-specific pthread/Win32 thread and semaphore code.

#### Result

DCL uses a single standard C++20 threading implementation across macOS, Linux, and Windows.

#### Verification

Build macOS and MinGW targets and run ctest --test-dir build -C Debug -LE optional --output-on-failure.

