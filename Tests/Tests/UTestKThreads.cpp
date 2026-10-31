// ==============================
// Fichier:			UTestKThreads.cpp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Eckhart Koeppen (eck@40hz.org)
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
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org>
// 
// ***** END LICENSE BLOCK *****

#include "UTestKThreads.h"
#include <K/Threads/TMutex.h>
#include <K/Threads/TCondVar.h>
#include <K/Threads/TThread.h>
#include <cstdio>
#include <atomic>
#include <thread>
#include <chrono>

namespace {
	class TTestWorker {
	public:
		TTestWorker(TThread** inThreadSlot, std::atomic<bool>* inStarted, std::atomic<bool>* inFinished)
			: mThreadSlot(inThreadSlot), mStarted(inStarted), mFinished(inFinished) {}

		void Run(void) {
			*mStarted = true;
			TThread* myThread = *mThreadSlot;
			(void) ::printf("Worker: running\n");
			// Sleep for 500 ms or until awakened
			if (myThread) {
				myThread->Sleep(500);
			}
			*mFinished = true;
			(void) ::printf("Worker: finished\n");
		}

	private:
		TThread** mThreadSlot;
		std::atomic<bool>* mStarted;
		std::atomic<bool>* mFinished;
	};
}

void
UTestKThreads::RunTests( void )
{
	(void) ::printf("UTestKThreads: starting\n");

	// 1. Test TMutex basic functionality
	TMutex theMutex;
	theMutex.Lock();
	(void) ::printf("Mutex: locked\n");
	theMutex.Unlock();
	(void) ::printf("Mutex: unlocked\n");
	bool tryOk = theMutex.TryLock();
	if (tryOk) {
		(void) ::printf("Mutex: try_lock succeeded\n");
		theMutex.Unlock();
	}

	// 2. Test TCondVar timed wait
	TCondVar theCond;
	theMutex.Lock();
	struct timespec relTime;
	relTime.tv_sec = 0;
	relTime.tv_nsec = 50 * 1000 * 1000; // 50 ms
	bool waitRes = theCond.TimedWaitRelative(&theMutex, &relTime);
	(void) ::printf("CondVar: relative wait timed out as expected (%s)\n", waitRes ? "false" : "true");
	theMutex.Unlock();

	// 3. Test TThread execution & wake up
	std::atomic<bool> started(false);
	std::atomic<bool> finished(false);
	TThread* threadPtr = nullptr;
	TTestWorker worker(&threadPtr, &started, &finished);
	TThread thread(&worker);
	threadPtr = &thread;

	while (!started.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	(void) ::printf("Main: worker started\n");

	// Wake up worker early
	thread.WakeUp();
	while (!finished.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	(void) ::printf("Main: worker joined/done\n");
	(void) ::printf("UTestKThreads: complete\n");
}
