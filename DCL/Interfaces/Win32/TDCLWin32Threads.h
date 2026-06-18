// ==============================
// Fichier:			TDCLWin32Threads.h
// Projet:			Desktop Connection Library
//
// Créé le:			28/05/2026
// ===========

#ifndef _TDCLWIN32THREADS_H
#define _TDCLWIN32THREADS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLThreads.h>

#include <windows.h>

///
/// Implementation of IDCLThreads for Win32.
///
class TDCLWin32Threads
	:
		public IDCLThreads
{
public:
	virtual	void	Yield( void );
	virtual IThread*	CreateThread( TDCLThread* inThreadedObject );
	virtual ISemaphore*	CreateSemaphore( void );

	class TThread
		:
			public IThread
	{
	public:
		TThread( TDCLThread* inThreadedObject );
		virtual ~TThread( void );

		virtual void	Start( void );
		virtual void	Stop( void );
		virtual	void	Suspend( void );
		virtual	void	Resume( void );
		virtual	Boolean	Sleep( KUInt32 inMillisecs );
		virtual	void	WakeUp( void );
		virtual	EState	GetState( void );

	private:
		static DWORD WINAPI Run( LPVOID inThis );

		HANDLE			mThread;
		DWORD			mThreadId;
		EState			mState;
		TDCLThread*     mThreadedObject;
		HANDLE          mSleepEvent;
	};

private:
	class TSemaphore
		:
			public ISemaphore
	{
	public:
		TSemaphore( void );
		virtual				~TSemaphore( void );
		virtual	void		Acquire ( void );
		virtual	void		Release ( void );

	private:
		HANDLE				mSemaphore;
	};
};

#endif
