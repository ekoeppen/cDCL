// ==============================
// Fichier:			TDCLWin32Threads.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			28/05/2026
// ===========

#include <DCL/Interfaces/Win32/TDCLWin32Threads.h>
#include <DCL/Interfaces/TDCLThread.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadStoppedException.h>

void
TDCLWin32Threads::Yield( void )
{
	::Sleep(0);
}

IDCLThreads::IThread*
TDCLWin32Threads::CreateThread( TDCLThread* inThreadedObject )
{
	return new TThread( inThreadedObject );
}

IDCLThreads::ISemaphore*
TDCLWin32Threads::CreateSemaphore( void )
{
	return new TSemaphore();
}

// -------------------------------------------------------------------------- //

TDCLWin32Threads::TSemaphore::TSemaphore( void )
{
	mSemaphore = ::CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
}

TDCLWin32Threads::TSemaphore::~TSemaphore( void )
{
	::CloseHandle(mSemaphore);
}

void
TDCLWin32Threads::TSemaphore::Acquire( void )
{
	::WaitForSingleObject(mSemaphore, INFINITE);
}

void
TDCLWin32Threads::TSemaphore::Release( void )
{
	::ReleaseSemaphore(mSemaphore, 1, NULL);
}

// -------------------------------------------------------------------------- //

TDCLWin32Threads::TThread::TThread( TDCLThread* inThreadedObject )
	: IThread(inThreadedObject), mThread(NULL), mThreadId(0), mState(kStopped), mThreadedObject(inThreadedObject)
{
    mSleepEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

TDCLWin32Threads::TThread::~TThread( void )
{
	if (mState != kStopped) {
        Stop();
    }
    ::CloseHandle(mSleepEvent);
}

void
TDCLWin32Threads::TThread::Start( void )
{
	if (mState != kStopped) {
#if kDCLDebugFILEandLINE
		throw TDCLThreadActiveException(__FILE__, __LINE__);
#else
		throw TDCLThreadActiveException();
#endif
	}

	mThread = ::CreateThread(NULL, 0, Run, this, 0, &mThreadId);
	if (mThread == NULL) {
#if kDCLDebugFILEandLINE
		throw TDCLThreadException(TDCLException::exThreadActive, 0, __FILE__, __LINE__);
#else
		throw TDCLThreadException(TDCLException::exThreadActive, 0);
#endif
	}
	mState = kRunning;
}

void
TDCLWin32Threads::TThread::Stop( void )
{
	if (mState == kStopped) {
#if kDCLDebugFILEandLINE
		throw TDCLThreadStoppedException(__FILE__, __LINE__);
#else
		throw TDCLThreadStoppedException();
#endif
	}

	::TerminateThread(mThread, 0);
	::CloseHandle(mThread);
	mThread = NULL;
	mState = kStopped;
}

void
TDCLWin32Threads::TThread::Suspend( void )
{
	if (mState == kStopped) {
#if kDCLDebugFILEandLINE
		throw TDCLThreadStoppedException(__FILE__, __LINE__);
#else
		throw TDCLThreadStoppedException();
#endif
	}
	if (mState == kRunning) {
		::SuspendThread(mThread);
		mState = kSuspended;
	}
}

void
TDCLWin32Threads::TThread::Resume( void )
{
	if (mState == kStopped) {
#if kDCLDebugFILEandLINE
		throw TDCLThreadStoppedException(__FILE__, __LINE__);
#else
		throw TDCLThreadStoppedException();
#endif
	}
	if (mState == kSuspended) {
		::ResumeThread(mThread);
		mState = kRunning;
	}
}

Boolean
TDCLWin32Threads::TThread::Sleep( KUInt32 inMillisecs )
{
	DWORD result = ::WaitForSingleObject(mSleepEvent, inMillisecs);
    return (result == WAIT_TIMEOUT);
}

void
TDCLWin32Threads::TThread::WakeUp( void )
{
	::SetEvent(mSleepEvent);
}

IDCLThreads::EState
TDCLWin32Threads::TThread::GetState( void )
{
	return mState;
}

DWORD WINAPI
TDCLWin32Threads::TThread::Run( LPVOID inThis )
{
	TThread* theThread = (TThread*) inThis;
	try {
		theThread->mThreadedObject->Run();
	} catch (TDCLException& e) {
		theThread->mThreadedObject->HandleException(&e);
	} catch (...) {
		theThread->mThreadedObject->HandleException(NULL);
	}
	theThread->mState = kStopped;
	return 0;
}
