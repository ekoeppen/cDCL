// ==============================
// Fichier:			TDCLLink.cp
// Projet:			Desktop Connection Library
//
// Créé le:			28/06/2000
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
// The Original Code is TDCLLink.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLLink.cp,v 1.7 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLink.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>

// K
#include <K/Misc/TSmartPtr.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadAppCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Server/TDCLServer.h>

// ------------------------------------------------------------------------- //
//  * Déverminage
// ------------------------------------------------------------------------- //

#undef KERROR_ENABLED
#if defined(error_TDCLLink) && error_TDCLLink
#define KERROR_ENABLED 1
#else
#define KERROR_ENABLED 0
#endif

#undef KTRACE_ENABLED
#if defined(trace_TDCLLink) && trace_TDCLLink
#define KTRACE_ENABLED 1
#else
#define KTRACE_ENABLED 0
#endif

#undef KDEBUG_ENABLED
#if defined(debug_TDCLLink) && debug_TDCLLink
#define KDEBUG_ENABLED 1
#else
#define KDEBUG_ENABLED 0
#endif

// --------------------------------------------------------------------------------	//
//  * TDCLLink( TDCLApplication*, TDCLPipe*, KUInt32 )
// --------------------------------------------------------------------------------	//
TDCLLink::TDCLLink(
					TDCLApplication* inApplication,
					KUInt32 inType /* = TDCLDockCommand::kNone_SType */,
					KUInt32 inIdlePeriod /* = kDefaultIdlePeriod */)
	:
		TDCLThread( inApplication->GetThreadsIntf() ),
		mPipe( nil ),
		mServer( nil ),
		mApplication( inApplication ),
		mStateMutex( nil ),
		mState( kDisconnected ),
		mType( inType ),
		mIdlePeriod( inIdlePeriod ),
		mIsBusy( false ),
		mNewtonName( nil ),
		mNewtonID( 0 ),
		mManufacturer( 0 ),
		mROMVersion( 0 ),
		mROMStage( 0 ),
		mRAMSize( 0 ),
		mScreenHeight( 0 ),
		mScreenWidth( 0 ),
		mSystemUpdate( 0 ),
		mNOSVersion( 0 ),
		mInternalStoreSig( 0 ),
		mVertScreenRes( 0 ),
		mHoriScreenRes( 0 ),
		mScreenDepth( 0 )
{
	mStateMutex = GetThreadsIntf()->CreateSemaphore();
}

// --------------------------------------------------------------------------------	//
//  * ~TDCLLink( void )
// --------------------------------------------------------------------------------	//
TDCLLink::~TDCLLink( void )
{
	if (mPipe)
	{
		Disconnected();
	}
	
	if (mStateMutex)
	{
		delete mStateMutex;
	}
}

// --------------------------------------------------------------------------------	//
//  * Connect( TDCLPipe*, TDCLServer* )
// --------------------------------------------------------------------------------	//
Boolean
TDCLLink::Connect( TDCLPipe* inPipe, TDCLServer* inServer )
{
	Boolean theResult = false;
	
	// Acquisition du sémaphore sur l'état.
	mStateMutex->Acquire();
	
	if (mState == kDisconnected)
	{
		// Vérification que le processus léger ne tourne pas.
		IDCLThreads::EState theState = GetThreadState();
		if( theState != IDCLThreads::kStopped )
		{
			throw DCLThreadActive;
		}
	
		theResult = true;
		mState = kConnecting;
		mPipe = inPipe;
		mServer = inServer;
		
		// Lancement du processus léger.
		Start();
	}
	
	// Libération du sémaphore
	mStateMutex->Release();
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * Disconnect( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLLink::Disconnect( void )
{
	Boolean theResult = false;
	
	// Acquisition du sémaphore sur l'état.
	mStateMutex->Acquire();
	
	if ((mState != kDisconnected) && (mState != kDisconnecting))
	{
		theResult = true;
	}
	
	// Libération du sémaphore
	mStateMutex->Release();
	
	// On envoie un événement si le processus tourne toujours.
	if (theResult)
	{
		PostEvent( new TDCLEvent(
							 TDCLEvent::kLinkEvent,
							 TDCLEvent::kDisconnectLink) );
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * PostAppCommand( TDCLAppCommand* )
// --------------------------------------------------------------------------------	//
Boolean
TDCLLink::PostAppCommand( TDCLAppCommand* inCommand )
{
	Boolean theResult = false;
	
	// Acquisition du sémaphore sur l'état.
	mStateMutex->Acquire();
	
	if ((mState != kDisconnected) && (mState != kDisconnecting))
	{
		theResult = true;
	}
	
	// Libération du sémaphore
	mStateMutex->Release();
	
	// On envoie un événement si le processus tourne toujours.
	if (theResult)
	{
		PostEvent( inCommand );
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * Close( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLLink::Close( void )
{
	Boolean theResult = IsConnected();
	
	// On envoie un événement si on est connecté.
	if (theResult)
	{
		PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kCloseLink ) );
	}
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * IsConnected( void )
// --------------------------------------------------------------------------------	//
Boolean
TDCLLink::IsConnected( void )
{
	Boolean theResult = false;
	
	// Acquisition du sémaphore sur l'état.
	mStateMutex->Acquire();
	
	if (mState != kDisconnected)
	{
		theResult = true;
	}
	
	// Libération du sémaphore
	mStateMutex->Release();
	
	return theResult;
}

// --------------------------------------------------------------------------------	//
//  * Disconnected( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::Disconnected( void )
{
	mPipe = nil;	// Je ne suis plus connecté.

	// On indique au serveur qu'on n'est déconnecté.
	mServer->Disconnected( this );
		
	if (mNewtonName)
	{
		delete [] (char*) mNewtonName;
		mNewtonName = nil;
	}
}

// --------------------------------------------------------------------------------	//
//  * Connected( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::Connected( void )
{
	// Je transmets le nom à l'application.
	mApplication->ConnectedToNewtonDevice( this, mNewtonName );
}

// --------------------------------------------------------------------------------	//
//  * DoConnect( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::DoConnect( void )
{
	// On dit à l'application que le lien est connecté. Ainsi elle pourra lui envoyer
	// des messages.
	// On dit aussi à la connexion que le lien va l'utiliser. (idem)
	mApplication->Connected( this, mPipe->Connected( this ) );	
	
	// Discussion avec le Newton.
	TSmartPtr<TDCLDockCommand> theNewtMessage;

	//	I read the kDRequestToDock message from the Newton.
	{
		theNewtMessage = TDCLDockCommand::ReceiveCommand( mPipe );

	// I check it's a kDRequestToDock command.
		if (theNewtMessage->GetCommand() != TDCLDockCommand::kDRequestToDock)
		{
			// Duh!
			throw DCLUnexpDockCmd;
		}
		theNewtMessage.Delete();
	}
	
	//	I say kDInitiateDocking
	{
		TDCLDockCmdSingleLong theInitiateDockingMessage(
			TDCLDockCommand::kDInitiateDocking, mType );

		theInitiateDockingMessage.SendCommand( mPipe );
	}
	
	// I read the kDNewtonName message from the Newton.
	{
		theNewtMessage = TDCLDockCommand::ReceiveCommand( mPipe );

		// I check it's a kDNewtonName command.
		if (theNewtMessage.Get()->GetCommand() != TDCLDockCommand::kDNewtonName)
		{
			// Duh!
			
			// Je devrait regarder si ce n'est pas un message d'erreur venant du Newton
			// (on ne sait jamais)
			
			throw DCLUnexpDockCmd;
		}

		// I cast the object to TDCLDockCmdNewtonName since it's a TDCLDockCmdNewtonName.
		TDCLDockCmdNewtonName* theNewtonName =
			(TDCLDockCmdNewtonName*) theNewtMessage.Get();
		
		// I then copy the fields of the command object to my own variable members.
		mNewtonID			= theNewtonName->GetNewtonUniqueID();
		mManufacturer		= theNewtonName->GetManufacturer();
		mMachineType		= theNewtonName->GetMachineType();
		mROMVersion			= theNewtonName->GetROMVersion();
		mROMStage			= theNewtonName->GetROMStage();
		mRAMSize			= theNewtonName->GetRAMSize();
		mScreenHeight		= theNewtonName->GetScreenHeight();
		mScreenWidth		= theNewtonName->GetScreenWidth();
		mSystemUpdate		= theNewtonName->GetSystemUpdate();
		mNOSVersion			= theNewtonName->GetNOSVersion();
		mInternalStoreSig	= theNewtonName->GetInternalStoreSig();
		mVertScreenRes		= theNewtonName->GetVertScreenRes();
		mHoriScreenRes 		= theNewtonName->GetHoriScreenRes();
		mScreenDepth 		= theNewtonName->GetScreenDepth();

		// I copy the Newton Name.
		// I get the size.
		KUInt32 theSize = theNewtonName->GetNameLength() + 1;	// I also copy the zero.
		theSize *= sizeof( KUInt16 );

		// then I allocate the name.
		mNewtonName = (KUInt16*) ::malloc( theSize );

		if (mNewtonName == nil)
		{
			throw DCLMemError;
		}

		// and finally, I copy it.
	
		(void) ::memcpy( (void*) mNewtonName, theNewtonName->GetNewtonName(), theSize );

		// I get rid of the message.
		theNewtMessage.Delete();
	}
}

// --------------------------------------------------------------------------------	//
//  * DoDisconnect( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::DoDisconnect( void )
{
	if (mPipe)
	{
		// Je dis à l'application que je déconnecte.
		mApplication->Disconnecting( this );

		// Then, I send the disconnect command.
		{
			TDCLDockCmdNoData theDisconnectMessage( TDCLDockCommand::kDDisconnect );

			theDisconnectMessage.SendCommand( mPipe );
		}

		// Récupération du résultat.
		Boolean gotResult = false;
		
		try {
			do
			{
				TSmartPtr<TDCLDockCommand> theNewtMessage;
				theNewtMessage =
					TDCLDockCommand::ReceiveCommand( mPipe );
				switch (theNewtMessage->GetCommand())
				{
					case TDCLDockCommand::kDResult:
					{
						// It's a kDResult, process it.
						TDCLDockCmdSingleLong* theResultCmd =
							(TDCLDockCmdSingleLong*) theNewtMessage.Get();
						KSInt32 theResult = (KSInt32) theResultCmd->GetLong();
						theNewtMessage.Delete();	// I no longer need it.
						if (theResult)
						{
						// Throw the error that the Newton sent.
							throw DCLNewton( theResult );
						}
						
						// On sort.
						gotResult = true;
						break;
					}

					case TDCLDockCommand::kDHello:
						// On ignore les bonjours qui trainent.
						break;

					default:
						// Commande non prévue.
						throw DCLUnexpDockCmd;
				}
			} while ((gotResult == false) && mPipe->BytesAvailable());
			
			// Si la connexion ne gère pas flush, on attend 5 secondes.

			if (!mPipe->FlushAvailable())
			{
				IDCLThreads* theThreadsIntf = mApplication->GetThreadsIntf();
				time_t now = ::time(NULL);
				while( (::time(NULL) - now) < kClosingTimeout )
				{
					mPipe->Idle();				// Give time to the pipe,
						  // in case its implementation requires it.

					theThreadsIntf->Yield();	// Il faut savoir partager.
				}
			}
		} catch (TDCLEOFException& anException) {
			// On ignore une exception EOF (déconnexion de l'autre côté).
		}
		
		// Le serveur va gérer la déconnexion dans HandleLinkDisconnection
		// mPipe sera mis à nil au prochain passage dans la boucle.
	} else {
		throw DCLBadStateError;
	}
}

// --------------------------------------------------------------------------------	//
//  * Run( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::Run( void )
{
	// Boucle principale du lien.
	Boolean keepLooping = true;
	while( keepLooping )
	{
		// On donne un peu de temps.
		Yield();

		EState theState;
		EState theNextState;

		// Acquisition
		mStateMutex->Acquire();

		// Récupération.
		theState = mState;
		theNextState = theState;
	
		// Libération
		mStateMutex->Release();

		try {
			switch ( theState )
			{
				case kConnecting:
					DoConnect();
					theNextState = kConnected;
					break;

				case kConnected:
					Connected();
					theNextState = kRunning;
					// On attend des données (ou un événement de l'application).
					mPipe->ClearDataPresent();
					break;
				
				case kRunning:
					theNextState = TransitionFromRunning();
					break;

				case kDisconnecting:
					DoDisconnect();
					theNextState = kDisconnected;
					break;
				
				default:	// kDisconnected, kUnknown
					Disconnected();
					keepLooping = false;
					theNextState = kDisconnected;
					break;
			}
		} catch (TDCLEOFException& inException) {
			// On a été déconnecté (le Newton a probablement coupé).
			Disconnected();
			keepLooping = false;
			theNextState = kDisconnected;
		}
		
		if (theState != theNextState)
		{
			// Acquisition
			mStateMutex->Acquire();

			// Vérification que l'état n'a pas été changé pendant qu'on avait le
			// dos tourné.
			if (mState == theState)
			{
				// Changement
				mState = theNextState;
			}
	
			// Libération
			mStateMutex->Release();
		}
	}
}

// --------------------------------------------------------------------------------	//
//  * TransitionFromRunning( void )
// --------------------------------------------------------------------------------	//
TDCLLink::EState
TDCLLink::TransitionFromRunning( void )
{
	EState theNextState = kRunning;
	
	// On attend le prochain événement.
	TDCLEvent* theNextEvent;
	KUInt32 theEventMask =
			TDCLEvent::kLinkEvent
			| (GetAppCommandMask() & TDCLEvent::kAnyAppCommandMask);
	theNextEvent = WaitNextEvent( theEventMask, mIdlePeriod );
	
	if (theNextEvent == nil)
	{
		// On a dormi tout le temps.
		theNextState = Idle();
	} else {
		// On est occupé.
		mIsBusy = true;
		try {
			// L'événement est-il une commande de l'application?
			// Si on a un événement de l'application, on le gère.
			Boolean outProcessed = false;
			if (theNextEvent->GetEventKind() & TDCLEvent::kAnyAppCommandMask)
			{
				try {
					mApplication->AppCommandBeingProcessed(
							this,
							(TDCLAppCommand*) theNextEvent );

					theNextState =
						ProcessAppCommand(
							(TDCLAppCommand*) theNextEvent, &outProcessed );

					if (!outProcessed)
					{
						throw DCLBadAppCmd;
					}

					mApplication->AppCommandProcessed(
							this,
							(TDCLAppCommand*) theNextEvent, nil );
				} catch( TDCLException& inException ) {
					mApplication->AppCommandProcessed(
							this,
							(TDCLAppCommand*) theNextEvent,
							&inException );
				}
				
				delete theNextEvent;
				theNextEvent = nil;
			} else {
				TDCLEvent::EEventID theEventID = theNextEvent->GetEventID();

				// Sinon, on regarde si le Newton a envoyé des données.
				if (theEventID == TDCLEvent::kBytesAvailable)
				{
					// Nous avons été réveillés par des données. Cependant,
					// il est possible que nous ayions déjà lu ces données.
					// On vérifie s'il en reste.
					if (mPipe->BytesAvailable())
					{
						TSmartPtr< TDCLDockCommand > theNewtMessage;
						theNewtMessage = ReceiveCommand();

						theNextState =
							ProcessDockCommand(
								theNewtMessage.Get(),
								&outProcessed );

						if (!outProcessed)
						{
							KDEBUG1( "Unknown command: %.8X\n",
								(unsigned int)
									theNewtMessage.Get()->GetCommand() );

							// On envoie une commande unknown command.
							TDCLDockCmdSingleLong theUnknownCmdMsg(
									TDCLDockCommand::kDUnknownCommand,
									theNewtMessage.Get()->GetCommand() );
							theUnknownCmdMsg.SendCommand( mPipe );
						}
					}
					
					// On attend à nouveau des données.
					mPipe->ClearDataPresent();
				} else if (theEventID == TDCLEvent::kDisconnectLink) {
					// Il faut se déconnecter.
					theNextState = kDisconnecting;
				} else if (theEventID == TDCLEvent::kCloseLink) {
					// On est déconnecté.
					theNextState = kDisconnected;
				} // if (theEventID == kBytesAvailable) ... else if ...
			}
		} catch( ... ) {
			// Nettoyage
			if (theNextEvent)
			{
				delete theNextEvent;
				theNextEvent = nil;
			}
			
			// On n'est plus occupé.
			mIsBusy = false;
			throw;	// On relance.
		}
		// On n'est plus occupé.
		mIsBusy = false;
		
		if (theNextEvent)
		{
			delete theNextEvent;
		}
	}
	
	return theNextState;
}

// --------------------------------------------------------------------------------	//
//  * HandleException( TDCLException* )
// --------------------------------------------------------------------------------	//
void
TDCLLink::HandleException( TDCLException* inException )
{
	// Acquisition
	mStateMutex->Acquire();

	switch ( mState )
	{
		case kDisconnected:
		case kConnecting:
		case kDisconnecting:
			mState = kDisconnected;
			break;
	
		case kConnected:
		case kRunning:
		default:
			mState = kUnknown;
			break;
	}
	
	// Libération
	mStateMutex->Release();

	mApplication->HandleLinkException( this, inException );
}

// --------------------------------------------------------------------------------	//
//  * Idle( void )
// --------------------------------------------------------------------------------	//
TDCLLink::EState
TDCLLink::Idle( void )
{
	Yield();
	
	return kRunning;
}

// --------------------------------------------------------------------------------	//
//  * ProcessAppCommand( TDCLAppCommand*, Boolean* )
// --------------------------------------------------------------------------------	//
TDCLLink::EState
TDCLLink::ProcessAppCommand(
					TDCLAppCommand* /* inCommand */,
					Boolean* outProcessed )
{
	// On ne fait rien par défaut.
	*outProcessed = false;
	return kRunning;
}

// --------------------------------------------------------------------------------	//
//  * DataPresent( void )
// --------------------------------------------------------------------------------	//
void
TDCLLink::DataPresent( void )
{
	PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kBytesAvailable ) );
}

// --------------------------------------------------------------------------------	//
//  * GetAppCommandMask( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLLink::GetAppCommandMask( void )
{
	return (KUInt32) TDCLEvent::kAnyAppCommandMask;
}

// --------------------------------------------------------------------------------	//
//  * ReceiveCommand( void )
// --------------------------------------------------------------------------------	//
TDCLDockCommand*
TDCLLink::ReceiveCommand( void )
{
	return TDCLDockCommand::ReceiveCommand( mPipe );
}

// ============================================================================= //
// **** IMPORTANT ****  ALL USERS PLEASE NOTE ****                               //
//                                                                               //
// Due to a recent systems overload error your recent disk files have been       //
// erased.  Therefore, in accordance with the UNIX Basic Manual, University of   //
// Washington Geophysics Manual, and Bylaw 9(c), Section XII of the Revised      //
// Federal Communications Act, you are being granted Temporary Disk Space,       //
// valid for three months from this date, subject to the restrictions set forth  //
// in Appendix II of the Federal Communications Handbook (18th edition) as well  //
// as the references mentioned herein.  You may apply for more disk space at any //
// time.  Disk usage in or above the eighth percentile will secure the removal   //
// of all restrictions and you will immediately receive your permanent disk      //
// space.  Disk usage in the sixth or seventh percentile will not effect the     //
// validity of your temporary disk space, though its expiration date may be      //
// extended for a period of up to three months.  A score in the fifth percentile //
// or below will result in the withdrawal of your Temporary Disk space.          //
// ============================================================================= //
