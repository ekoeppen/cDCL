// ==============================
// Fichier:			TDCLPThreads.cp
// Projet:			Desktop Connection Library
//
// Créé le:			26/1/2003
// Tabulation:		4 espaces
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
// The Original Code is TDCLPThreads.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPThreads.cp,v 1.6 2004/11/24 14:09:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/POSIX/TDCLPThreads.h>

// POSIX
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLSemaphoreBusyException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadStoppedException.h>
#include <DCL/Interfaces/TDCLThread.h>

// Je ne veux pas de GNU Pth
#ifdef _POSIX_THREAD_IS_GNU_PTH
#error	Some versions of GNU Pth (like 0x104201) are not fully POSIX \
		compliant and will not work with the DCL -- more details in the source \
		file in case you think your version will work.
// Two reasons:
// - we need SIGUSR1 and SIGUSR2 for Suspend/Resume
// - exceptions will not propagate through their scheduler (and lead to an
// abort, as if there was no try/catch block).
//
// Use the threads test to see if the implementation is compatible with the
// DCL.
#endif

// ------------------------------------------------------------------------- //
//  * Yield( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::Yield( void )
{
	// Les processus légers POSIX sont préemptifs.
}

// ------------------------------------------------------------------------- //
//  * CreateThread( TDCLThread* )
// ------------------------------------------------------------------------- //
TDCLPThreads::IThread*
TDCLPThreads::CreateThread( TDCLThread* inThreadedObject )
{
	return new TThread( inThreadedObject );
}

// ------------------------------------------------------------------------- //
//  * CreateSemaphore( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::ISemaphore*
TDCLPThreads::CreateSemaphore( void )
{
	return new TDCLPThreads::TSemaphore();
}

// ------------------------------------------------------------------------- //
//  * TSemaphore( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::TSemaphore::TSemaphore( void )
	:
		mNbThreads( 0 )
{
	int theErr = ::pthread_mutex_init( &mMutex, NULL );
	
	switch ( theErr )
	{
		case 0:
			break;
		
		case ENOMEM:
			throw DCLPlatformMemError( theErr );
			
		case EAGAIN:
			throw DCLPlatformLimitReachedError( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
	
	theErr = ::pthread_cond_init( &mWaitCond, NULL );
}

// ------------------------------------------------------------------------- //
//  * ~TSemaphore( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::TSemaphore::~TSemaphore( void )
{
	// Lance une exception si le sémaphore est occupé.
	if (mNbThreads != 0)
	{
		throw DCLSemaphoreBusy;
	}
	
	// On nettoie.
	int theErr = ::pthread_cond_destroy( &mWaitCond );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EBUSY:
			throw DCLPlatformSemaphoreBusy( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}

	theErr = ::pthread_mutex_destroy( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EBUSY:
			throw DCLPlatformSemaphoreBusy( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * Acquire( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TSemaphore::Acquire( void )
{
	// On acquiert le mutex.
	int theErr = ::pthread_mutex_lock( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EDEADLK:	// DeadLock (bogue dans le code.)
			throw DCLPlatformThreadDeadLock( theErr );
			
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}

	// Go!
	mNbThreads++;

	if (mNbThreads > 1)
	{
		// Attente.
		theErr = ::pthread_cond_wait( &mWaitCond, &mMutex );
		switch ( theErr )
		{
			case 0:
				break;
			
			case ETIMEDOUT:	// Ne doit pas arriver, on ne fait pas un try_wait.

			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}

	// Libération du mutex.
	theErr = ::pthread_mutex_unlock( &mMutex );		
	switch ( theErr )
	{
		case 0:
			break;
		
		case EPERM:		// Impossible de libérer un sémaphore dans un autre
						// processus léger.
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * Release( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TSemaphore::Release( void )
{
	// On acquiert le mutex.
	int theErr = ::pthread_mutex_lock( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EDEADLK:	// DeadLock (bogue dans le code.)
			throw DCLPlatformThreadDeadLock( theErr );
			
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}

	mNbThreads--;
	
	if (mNbThreads > 0)
	{
		theErr = ::pthread_cond_signal( &mWaitCond );
		switch ( theErr )
		{
			case 0:
				break;

			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				(void) ::pthread_mutex_unlock( &mMutex );
				throw DCLPlatformUnknownError( theErr );
		}
	}

	// Libération du mutex.
	theErr = ::pthread_mutex_unlock( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EPERM:		// Impossible de libérer un sémaphore dans un autre
						// processus léger.
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * TThread( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::TThread::TThread( TDCLThread* inThreadedObject )
	:
		IThread( inThreadedObject ),
		mState( kStopped ),
		mSyncSemaphore(),
		mWakeCount( 0 )
{
	// Initialisation du mutex.
	int theErr = ::pthread_mutex_init( &mMutex, NULL );
	switch ( theErr )
	{
		case 0:
			break;
		
		case ENOMEM:
			throw DCLPlatformMemError( theErr );
			
		case EAGAIN:
			throw DCLPlatformLimitReachedError( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
	
	// Initialisation de la variable de condition.
	theErr = ::pthread_cond_init( &mSleepCond, NULL );
	
	// Le processus sera initialisé dans la méthode start.
	
	switch ( theErr )
	{
		case 0:
			break;
		
		case ENOMEM:
			throw DCLPlatformMemError( theErr );
			
		case EAGAIN:
			throw DCLPlatformLimitReachedError( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * ~TThread( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::TThread::~TThread( void )
{
	if (mState != kStopped)
	{
		(void) ::pthread_join( mThread, NULL );
		if (mState != kStopped)
		{
			throw DCLThreadActive;
		}
	}

	// On nettoie.
	int theErr = ::pthread_cond_destroy( &mSleepCond );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EBUSY:
			throw DCLPlatformSemaphoreBusy( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}

	theErr = ::pthread_mutex_destroy( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EBUSY:
			throw DCLPlatformSemaphoreBusy( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * Start( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::Start( void )
{
	AcquireMutex();

	if (mState != kStopped)
	{
		ReleaseMutex();
		throw DCLThreadActive;
	}
	
	// On synchronise.
	mSyncSemaphore.Acquire();
	
	int theErr = ::pthread_create(
						&mThread,
						NULL,
						(void *(*)(void *)) Run,
						this );
	
	switch ( theErr )
	{
		case 0:
			break;
		
		case EAGAIN:	// Nombre maximum de processus légers atteint?
			mSyncSemaphore.Release();
			ReleaseMutex();
			throw DCLPlatformLimitReachedError( theErr );
		
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			mSyncSemaphore.Release();
			ReleaseMutex();
			throw DCLPlatformUnknownError( theErr );
	}

	// Attente que Run soit appelé et mState mis à jour.
	mSyncSemaphore.Acquire();

	// Libération du sémaphore.
	mSyncSemaphore.Release();

	ReleaseMutex();
}

// ------------------------------------------------------------------------- //
//  * Stop( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::Stop( void )
{
	AcquireMutex();
	
	if (mState == kStopped)
	{
		ReleaseMutex();
		throw DCLThreadStopped;
	}
	
	mState = kStopped;
	
	int theErr = ::pthread_cancel( mThread );	// PAF.
	
	ReleaseMutex();

	switch ( theErr )
	{
		case 0:
			break;
		
		case ESRCH:		// mThread n'est pas valide: ne doit pas arriver.
						// sauf peut-être dans un cas très particulier où
						// le processus léger est fini (?)
			throw DCLPlatformThreadStopped( theErr );
			
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * Suspend( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::Suspend( void )
{
	AcquireMutex();

	if (mState == kStopped)
	{
		ReleaseMutex();
		throw DCLThreadStopped;
	}
	
	if (mState == kRunning)
	{
		mState = kSuspended;

		// Lorsque ce signal est appelé, on appelle sigsuspend.
		// Cf SignalUSR2.
		int theErr = ::pthread_kill( mThread, SIGUSR2 );
	
		ReleaseMutex();

		switch ( theErr )
		{
			case 0:
				break;
		
			case ESRCH:		// mThread n'est pas valide: ne doit pas arriver.
							// sauf peut-être dans un cas très particulier où
							// le processus léger est fini (?)
				throw DCLPlatformThreadStopped( theErr );
			
			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	} else {
		ReleaseMutex();
	}
}

// ------------------------------------------------------------------------- //
//  * Resume( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::Resume( void )
{
	AcquireMutex();

	if (mState == kStopped)
	{
		ReleaseMutex();
		throw DCLThreadStopped;
	}
	
	if (mState == kSuspended)
	{
		mState = kRunning;

		int theErr = ::pthread_kill( mThread, SIGUSR1 );
	
		ReleaseMutex();

		switch ( theErr )
		{
			case 0:
				break;
		
			case ESRCH:		// mThread n'est pas valide: ne doit pas arriver.
							// sauf peut-être dans un cas très particulier où
							// le processus léger est fini (?)
				throw DCLPlatformThreadStopped( theErr );
			
			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	} else {
		ReleaseMutex();
	}
}

// ------------------------------------------------------------------------- //
//  * Sleep( KUInt32 )
// ------------------------------------------------------------------------- //
Boolean
TDCLPThreads::TThread::Sleep( KUInt32 inMilliseconds /* = kForever */ )
{
	Boolean theResult;
	
	Boolean forever = (inMilliseconds == kForever);
	
	// Détermination de l'heure du réveil.
	timespec theTimeToWake;
	if (!forever)
	{
		timeval theCurrentTime;
		(void) ::gettimeofday( &theCurrentTime, nil );
		// Traduction en timespec et ajout du délai.		
		theTimeToWake.tv_sec =
			((long) (inMilliseconds / 1000)) + theCurrentTime.tv_sec;
		theTimeToWake.tv_nsec =
			(((long) ((inMilliseconds % 1000) * 1000))
				+ theCurrentTime.tv_usec) * 1000;
	}
	
	// Acquisition du mutex
	AcquireMutex();

	// Le nombre de réveil est-il nul?
	if (mWakeCount > 0)
	{
		mWakeCount--;
		theResult = false;	// On n'a pas dormi tout le temps demandé.

		// Libération du mutex.
		ReleaseMutex();
	} else {
		// On dort.
		mState = kSleeping;
		
		int theErr;
		if (!forever)
		{
			theErr = ::pthread_cond_timedwait(
							&mSleepCond,
							&mMutex,
							&theTimeToWake );
		} else {
			theErr = ::pthread_cond_wait( &mSleepCond, &mMutex );
		}

		// On ne dort plus.
		mState = kRunning;
		
		// Libération du mutex.
		ReleaseMutex();
		
		switch ( theErr )
		{
			case 0:
				theResult = false;	// On n'a pas dormi tout le temps demandé.
				break;
			
			case ETIMEDOUT:
				theResult = true;	// On a dormi tout le temps demandé.
				break;

			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				throw DCLPlatformUnknownError( theErr );
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * WakeUp( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::WakeUp( void )
{
	// Acquisition du mutex.
	AcquireMutex();

	// Le processus léger dort-il?
	if (mState == kSleeping)
	{
		int theErr = ::pthread_cond_signal( &mSleepCond );

		switch ( theErr )
		{
			case 0:
				break;

			case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
			default:
				ReleaseMutex();
				throw DCLPlatformUnknownError( theErr );
		}
	} else {
		// On incrémente juste le compteur.
		mWakeCount++;
	}
	
	// Libération du mutex.
	ReleaseMutex();
}

// ------------------------------------------------------------------------- //
//  * Run( TThread* )
// ------------------------------------------------------------------------- //
void*
TDCLPThreads::TThread::Run( TThread* inThis )
{
	inThis->mState = kRunning;

	// On enregistre les signaux pour Suspend/Resume.
	// Suspend (SIGUSR2) d'abord.
	struct sigaction theSigAction;
	theSigAction.sa_flags = 0;
	theSigAction.sa_handler = SignalUSR2;
	sigemptyset(&theSigAction.sa_mask);

	if (sigaction( SIGUSR2, &theSigAction, NULL ))
	{
		throw DCLPlatformUnknownError( errno );
	}

	// Ensuite, SIGUSR1 (resume).
	// On bloque SIGUSR2 à ce moment pour éviter une
	// suspension pendant la reprise.
	theSigAction.sa_handler = SignalUSR1;
	sigaddset(&theSigAction.sa_mask, SIGUSR2);
	
	if (sigaction( SIGUSR1, &theSigAction, NULL ))
	{
		throw DCLPlatformUnknownError( errno );
	}

	// Synchronisation.
	inThis->mSyncSemaphore.Release();
	
	// On tourne.
	inThis->GetThreadedObject()->DoRun();
	
	inThis->mState = kStopped;
	
	return nil;
}

// ------------------------------------------------------------------------- //
//  * SignalUSR2( int )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::SignalUSR2( int /* inSignal */ )
{
	// On attend SIGUSR1.
	sigset_t theSigset;
	
	(void) sigfillset( &theSigset );			// retourne toujours 0.
	(void) sigdelset( &theSigset, SIGUSR1 );	// idem.
	
	(void) sigsuspend( &theSigset );			// retourne toujours -1.
}

// ------------------------------------------------------------------------- //
//  * SignalUSR1( int )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::SignalUSR1( int /* inSignal */ )
{
}

// ------------------------------------------------------------------------- //
//  * GetState( void )
// ------------------------------------------------------------------------- //
TDCLPThreads::EState
TDCLPThreads::TThread::GetState( void )
{
	return mState;
}

// ------------------------------------------------------------------------- //
//  * AcquireMutex( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::AcquireMutex( void )
{
	// Acquisition du mutex.
	int theErr = ::pthread_mutex_lock( &mMutex );
	switch ( theErr )
	{
		case 0:
			break;
		
		case EDEADLK:	// DeadLock (bogue dans le code.)
			throw DCLPlatformThreadDeadLock( theErr );
			
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ------------------------------------------------------------------------- //
//  * ReleaseMutex( void )
// ------------------------------------------------------------------------- //
void
TDCLPThreads::TThread::ReleaseMutex( void )
{
	// Libération du mutex.
	int theErr = ::pthread_mutex_unlock( &mMutex );		
	switch ( theErr )
	{
		case 0:
			break;
		
		case EPERM:		// Impossible de libérer un sémaphore dans un autre
						// processus léger.
		case EINVAL:	// Problèmes d'arguments (ne doit pas arriver)
		default:
			throw DCLPlatformUnknownError( theErr );
	}
}

// ======================================================================= //
// Trying to be happy is like trying to build a machine for which the only //
// specification is that it should run noiselessly.                        //
// ======================================================================= //
