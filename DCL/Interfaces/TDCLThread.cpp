// ==============================
// Fichier:			TDCLThread.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLThread.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLThread.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLThread.h>

// DCL
#include <DCL/Interfaces/TDCLEvent.h>

// ------------------------------------------------------------------------- //
//  * TDCLThread( IDCLThreads* )
// ------------------------------------------------------------------------- //
TDCLThread::TDCLThread( IDCLThreads* inThreadFactory )
	:
		mThread( nil ),
		mQueueSemaphore( nil ),
		mNextEvent( nil ),
		mLastEvent( nil ),
		mThreadsIntf( inThreadFactory )
{
	mThread = inThreadFactory->CreateThread( this );
	mQueueSemaphore = inThreadFactory->CreateSemaphore();
}

// ------------------------------------------------------------------------- //
//  * ~TDCLThread( void )
// ------------------------------------------------------------------------- //
TDCLThread::~TDCLThread( void )
{
	if (mThread)
	{
		delete mThread;
	}
	
	if (mQueueSemaphore)
	{
		delete mQueueSemaphore;
	}
	
	// Libération des événements.
	TDCLEvent* theEvent = mNextEvent;
	while (theEvent)
	{
		TDCLEvent* theNextEvent = theEvent->GetNextEvent();
		delete theEvent;
		
		theEvent = theNextEvent;
	}
}

// ------------------------------------------------------------------------- //
//  * WaitNextEvent( KUInt32, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLEvent* 
TDCLThread::WaitNextEvent(
				KUInt32 inEventMask /* = TDCLEvent::kAnyEventMask */,
				KUInt32 inMilliseconds /* = IDCLThreads::IThread::kForever */ )
{
	TDCLEvent* theResult = nil;
	
	do {
		// On regarde si un événement n'est pas déjà présent
		// et cadre avec le masque.

		// Acquisistion du mutex.
		mQueueSemaphore->Acquire();
		theResult = mNextEvent;
		TDCLEvent* thePreviousEvent = nil;
		while (theResult)
		{
			if (theResult->GetEventKind() & inEventMask)
			{
				break;
			} else {
				thePreviousEvent = theResult;
				theResult = theResult->GetNextEvent();
			}
		}
	
		if (theResult)
		{
			// Trouvé.
			TDCLEvent* theNextEvent = theResult->GetNextEvent();
			
			// On le retire de la liste.
			if (thePreviousEvent)
			{
				// Notre événement n'est pas le premier.
				thePreviousEvent->SetNextEvent( theNextEvent );
			} else {
				// C'était le premier.
				mNextEvent = theNextEvent;
			}
			
			if (theNextEvent == nil)
			{
				// Notre événement était le dernier.
				mLastEvent = thePreviousEvent;
			}
		}

		// Libération du mutex.
		mQueueSemaphore->Release();

		if (theResult)
		{
			break;
		}
		
		// Sinon, on dort tout le temps indiqué.
		if (mThread->Sleep( inMilliseconds ))
		{
			// On a dormi tout le temps demandé.
			// On sort.
			break;
		}
		
		// On n'a pas dormi tout le temps demandé: un événement
		// est sans doute présent. On le traite en haut de la boucle.
	} while ( true );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PostEvent( TDCLEvent* inEvent )
// ------------------------------------------------------------------------- //
void
TDCLThread::PostEvent( TDCLEvent* inEvent )
{
	// Acquisistion du mutex.
	mQueueSemaphore->Acquire();
	
	if (mLastEvent == nil)
	{
		mNextEvent = inEvent;
		mLastEvent = inEvent;
	} else {
		mLastEvent->SetNextEvent( inEvent );
		mLastEvent = inEvent;
	}
	inEvent->SetNextEvent( nil );
		
	// Libération du mutex.
	mQueueSemaphore->Release();	
	
	// Réveil du processus.
	mThread->WakeUp();
}

// ------------------------------------------------------------------------- //
//  * DoRun( void )
// ------------------------------------------------------------------------- //
void
TDCLThread::DoRun( void )
{
	try {
		Run();
	} catch ( TDCLException& inException ) {
		try {
			HandleException( &inException );
		} catch ( ... ) {
			// On ne fait rien.
		}
	} catch ( ... ) {
		try {
			HandleException( nil );
		} catch ( ... ) {
			// Pareil. C'est bon d'être oisif.
		}
	}
}

// ======================================================================= //
// Those parts of the system that you can hit with a hammer (not advised)  //
// are called hardware; those program instructions that you can only curse //
// at are called software.                                                 //
//                 -- Levitating Trains and Kamikaze Genes: Technological  //
//                    Literacy for the 1990's.                             //
// ======================================================================= //
