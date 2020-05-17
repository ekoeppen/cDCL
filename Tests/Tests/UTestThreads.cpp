//==============================
// Fichier:			UTestThreads.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			14/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003-2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestThreads.cp,v 1.9 2004/04/30 16:20:45 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "UTestThreads.h"

// ANSI C
#include <stdio.h>
#include <time.h>

// POSIX
#include <unistd.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Interfaces/POSIX/TDCLPThreads.h>

// -------------------------------------------------------------------------- //
//  * Posix( void )
// -------------------------------------------------------------------------- //
void
UTestThreads::Posix( void )
{
	TDCLPThreads thePThreads;

	TestThreads( &thePThreads );
}

// -------------------------------------------------------------------------- //
//  * TestThreads( IDCLThreads* )
// -------------------------------------------------------------------------- //
void
UTestThreads::TestThreads( IDCLThreads* inThreadsIntf )
{
	// Création du mutex.
	IDCLThreads::ISemaphore* theSemaphore = inThreadsIntf->CreateSemaphore();
	IDCLThreads::ISemaphore* theMutex = inThreadsIntf->CreateSemaphore();
	
	{
		// Création du bloqueur.
		TBlocker theBlocker( inThreadsIntf, theSemaphore, theMutex );
		
		// Lancement du processus léger.
		(void) ::printf( "TestThreads - Start\n" );
		theBlocker.Start();
		
		// On dort une seconde.
//		(void) ::printf( "TestThreads - Sleep - 1 ...\n" );
		(void) ::sleep( 1 );
		(void) ::printf( "TestThreads - ... Sleep - 1\n" );
		
		// Acquisition d'une ressource partagée.
		(void) ::printf( "TestThreads - mutex->Acquire\n" );
		theMutex->Acquire();
		
		// Libération.
		(void) ::printf( "TestThreads - sem->Release\n" );
		theSemaphore->Release();

		// On dort une seconde.
//		(void) ::printf( "TestThreads - Sleep - 2 ...\n" );
		(void) ::sleep( 1 );
		(void) ::printf( "TestThreads - ... Sleep - 2\n" );
		
		// Libération.
		(void) ::printf( "TestThreads - mutex->Release\n" );
		theMutex->Release();
		
		// On dort deux secondes.
//		(void) ::printf( "TestThreads - Sleep - 3 ...\n" );
		(void) ::sleep( 2 );
		(void) ::printf( "TestThreads - ... Sleep - 3\n" );
		
		// On réveille le processus.
		(void) ::printf( "TestThreads - thread->WakeUp\n" );
		theBlocker.WakeUp();
		
		// On dort une seconde.
//		(void) ::printf( "TestThreads - Sleep - 4 ...\n" );
		(void) ::sleep( 1 );
		(void) ::printf( "TestThreads - ... Sleep - 4\n" );

		// Suspension.
		(void) ::printf( "TestThreads - thread->Suspend ...\n" );
		theBlocker.Suspend();

		// On dort une seconde.
//		(void) ::printf( "TestThreads - Sleep - 5 ...\n" );
		(void) ::sleep( 1 );
		(void) ::printf( "TestThreads - ... Sleep - 5\n" );

		// On le réveille.
		(void) ::printf( "TestThreads - thread->Resume ...\n" );
		theBlocker.Resume();

		// On dort trois secondes.
//		(void) ::printf( "TestThreads - Sleep - 6 ...\n" );
		(void) ::sleep( 3 );
		(void) ::printf( "TestThreads - ... Sleep - 6\n" );

		(void) ::printf( "TestThreads - thread->GetState: %i\n",
						theBlocker.GetThreadState() );
	} // <-- Suppression du bloqueur.
	
	// Suppression du mutex.
	delete theSemaphore;
}

// -------------------------------------------------------------------------- //
//  * TBlocker( IDCLThreads*, IDCLThreads::ISemaphore*, ... )
// -------------------------------------------------------------------------- //
UTestThreads::TBlocker::TBlocker(
						IDCLThreads* inThreadsIntf,
						IDCLThreads::ISemaphore* inSemaphore,
						IDCLThreads::ISemaphore* inMutex )
	:
		TDCLThread( inThreadsIntf ),
		mSemaphore( inSemaphore ),
		mMutex( inMutex )
{
	(void) ::printf( "TBlocker::TBlocker\n" );
}

// -------------------------------------------------------------------------- //
//  * Run( void )
// -------------------------------------------------------------------------- //
void
UTestThreads::TBlocker::Run( void )
{
	(void) ::printf( "TBlocker::Run...\n" );

	(void) ::printf( "TBlocker::Run - sem->Acquire - 1\n" );
	mSemaphore->Acquire();
	
	(void) ::printf( "TBlocker::Run - sem->Acquire - 2\n" );
	mSemaphore->Acquire();
	
	(void) ::printf( "TBlocker::Run - sem->Release\n" );
	mSemaphore->Release();

	(void) ::printf( "TBlocker::Run - mutex->Acquire\n" );
	mMutex->Acquire();

	(void) ::printf( "TBlocker::Run - mutex->Release\n" );
	mMutex->Release();
	
	// On dort pendant une seconde.
//	(void) ::printf( "TBlocker::Run - Sleep - 1...\n" );
	if (Sleep( 1000 ))
	{
		(void) ::printf( "TBlocker::Run - ... Sleep - 1 (true)\n" );
	} else {
		(void) ::printf( "TBlocker::Run - ... Sleep - 1 (false)\n" );
	}
	
//	(void) ::printf( "TBlocker::Run - Sleep - 2...\n" );
	if (Sleep())
	{
		(void) ::printf( "TBlocker::Run - ... Sleep - 2 (true)\n" );
	} else {
		(void) ::printf( "TBlocker::Run - ... Sleep - 2 (false)\n" );
	}
	
	// On tourne pendant plus de deux secondes.
	(void) ::printf( "TBlocker::Run - Loop ...\n" );
	time_t theEnd = (::time(NULL)) + 3;
	while (::time(NULL) <= theEnd) {} ;
	(void) ::printf( "TBlocker::Run - ... Loop\n" );

	// On tente l'envoi d'une exception.
	(void) ::printf( "TBlocker::Run - throw DCLUnknownError\n" );
	throw DCLUnknownError;
}

// -------------------------------------------------------------------------- //
//  * HandleException( TDCLException* inException )
// -------------------------------------------------------------------------- //
void
UTestThreads::TBlocker::HandleException( TDCLException* inException )
{
	(void) ::printf( "TBlocker::HandleException\n" );

	if (inException)
	{
#if kDCLDebugFILEandLINE
		(void) ::printf( ">> %s (code: %i, error: %i) [%s:%u]\n",
						inException->GetExceptionName(),
						inException->GetExCode(),
						(int) inException->GetError(),
						inException->GetFileName(),
						(unsigned int) inException->GetLine() );
#else
		(void) ::printf( ">> %s (code: %i, error: %i)\n",
						inException->GetExceptionName(),
						inException->GetExCode(),
						inException->GetError() );
#endif
	} else {
		(void) ::printf( ">> Unknown Exception\n" );
	}
}

// ========================================================================== //
// "Now this is a totally brain damaged algorithm.  Gag me with a smurfette." //
//                 -- P. Buhr, Computer Science 354                           //
// ========================================================================== //
