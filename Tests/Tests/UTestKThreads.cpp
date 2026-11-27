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

#include <K/Threads/TMutex.h>
#include <K/Threads/TCondVar.h>
#include <K/Threads/TThread.h>

#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace {
	class TTestWorker {
	public:
		TTestWorker(std::atomic<TThread*>* inThreadSlot, std::atomic<bool>* inStarted, std::atomic<bool>* inFinished)
			: mThreadSlot(inThreadSlot), mStarted(inStarted), mFinished(inFinished) {}

		void Run(void) {
			TThread* myThread = nullptr;
			while ((myThread = mThreadSlot->load()) == nullptr) {
				std::this_thread::yield();
			}
			*mStarted = true;
			myThread->Sleep(500);
			*mFinished = true;
		}

	private:
		std::atomic<TThread*>* mThreadSlot;
		std::atomic<bool>* mStarted;
		std::atomic<bool>* mFinished;
	};

	class TSignalExitWorker {
	public:
		TSignalExitWorker(TMutex* inMutex, TCondVar* inCond, std::atomic<bool>* inExiting)
			: mMutex(inMutex), mCond(inCond), mExiting(inExiting) {}

		void Run(void) {
			mMutex->Lock();
			while (!mExiting->load()) {
				mCond->Wait(mMutex);
			}
			mMutex->Unlock();
			mExiting->store(false);
		}

	private:
		TMutex* mMutex;
		TCondVar* mCond;
		std::atomic<bool>* mExiting;
	};
}

TEST_SUITE("KThreads")
{
	TEST_CASE("TMutex lock, unlock, and TryLock")
	{
		TMutex theMutex;
		theMutex.Lock();
		std::atomic<bool> contendedTry{true};
		std::thread other([&] {
			contendedTry = theMutex.TryLock();
			if (contendedTry) {
				theMutex.Unlock();
			}
		});
		other.join();
		CHECK_FALSE(contendedTry.load());
		theMutex.Unlock();

		bool tryOk = theMutex.TryLock();
		CHECK(tryOk);
		if (tryOk) {
			theMutex.Unlock();
		}
	}

	TEST_CASE("TCondVar TimedWaitRelative times out and wakes on Signal")
	{
		TMutex theMutex;
		TCondVar theCond;

		theMutex.Lock();
		struct timespec relTime;
		relTime.tv_sec = 0;
		relTime.tv_nsec = 50 * 1000 * 1000; // 50 ms
		bool waitRes = theCond.TimedWaitRelative(&theMutex, &relTime);
		CHECK_FALSE(waitRes);
		theMutex.Unlock();

		std::atomic<bool> waiting{false};
		std::atomic<bool> signaled{false};
		std::thread waiter([&] {
			theMutex.Lock();
			waiting = true;
			struct timespec longWait = { 2, 0 };
			signaled = theCond.TimedWaitRelative(&theMutex, &longWait);
			theMutex.Unlock();
		});
		while (!waiting.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		theMutex.Lock();
		theCond.Signal();
		theMutex.Unlock();
		waiter.join();
		CHECK(signaled.load());
	}

	TEST_CASE("TThread WakeUp wakes sleeping worker before timeout")
	{
		std::atomic<bool> started(false);
		std::atomic<bool> finished(false);
		std::atomic<TThread*> threadPtr{nullptr};
		TTestWorker worker(&threadPtr, &started, &finished);
		TThread thread(&worker);
		threadPtr.store(&thread);

		auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (!started.load() && std::chrono::steady_clock::now() < deadline) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		REQUIRE(started.load());

		auto t0 = std::chrono::steady_clock::now();
		thread.WakeUp();
		thread.Join();
		REQUIRE(finished.load());
		auto elapsed = std::chrono::steady_clock::now() - t0;
		CHECK(elapsed < std::chrono::milliseconds(400));
	}

	TEST_CASE("TThread immediate teardown after Run completes is safe")
	{
		for (int i = 0; i < 100; ++i) {
			TMutex mutex;
			TCondVar cond;
			std::atomic<bool> exiting(false);
			TSignalExitWorker worker(&mutex, &cond, &exiting);
			TThread* thread = new TThread(&worker);

			mutex.Lock();
			exiting.store(true);
			cond.Signal();
			mutex.Unlock();

			while (exiting.load()) {
				std::this_thread::yield();
			}
			delete thread;
		}
	}

	TEST_CASE("TThread preserves multiple WakeUp signals")
	{
		struct TDoubleWakeWorker {
			std::atomic<TThread*> threadPtr{nullptr};
			std::atomic<bool> firstAwake{false};
			std::atomic<bool> secondAwake{false};
			void Run(void) {
				TThread* t = nullptr;
				while ((t = threadPtr.load()) == nullptr) {
					std::this_thread::yield();
				}
				CHECK(t->IsCurrentThread());
				CHECK_FALSE(t->Sleep(500));
				firstAwake.store(true);
				CHECK_FALSE(t->Sleep(500));
				secondAwake.store(true);
			}
		} worker;

		TThread thread(&worker);
		CHECK_FALSE(thread.IsCurrentThread());
		thread.WakeUp();
		thread.WakeUp();
		worker.threadPtr.store(&thread);
		thread.Join();
		CHECK(worker.firstAwake.load());
		CHECK(worker.secondAwake.load());
	}
}
