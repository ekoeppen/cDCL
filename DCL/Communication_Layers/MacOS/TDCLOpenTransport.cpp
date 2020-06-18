// ==============================
// Fichier:			TDCLOpenTransport.cp
// Projet:			Desktop Connection Library
//
// Créé le:			23/01/2002
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
// The Original Code is TDCLOpenTransport.cp.
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
// $Id: TDCLOpenTransport.cp,v 1.7 2004/11/24 18:11:02 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLOpenTransport.h>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <time.h>

// MacOS
#if TARGET_OS_MACOS
	#include <Gestalt.h>
	// J'ai besoin de FLUSHRW
	#include <OpenTransportProtocol.h>
#else
	#include <CoreServices/CoreServices.h>
#endif

// K
#include <K/Misc/UPStrings.h>

// DCL
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Link/TDCLLink.h>
#include <DCL/Server/TDCLServer.h>

// -------------------------------------------------------------------------- //
//  Variables statiques
// -------------------------------------------------------------------------- //

// Static variable to count instances of TDCLOpenTransport
// for use with Open/Close OT management.

KUInt32 TDCLOpenTransport::gInstanceCount = 0;

// Static variable to know what is the status of OT initializations.

TDCLOpenTransport::EOTState TDCLOpenTransport::gOTState = kUninitialized;

// -------------------------------------------------------------------------- //
//  * TDCLOpenTransport( void )
// -------------------------------------------------------------------------- //
TDCLOpenTransport::TDCLOpenTransport(
		OTConfigurationRef	inConfiguration,
		TBind*				inLocalBindInfo,
		IDCLThreads*		inThreadsIntf,
		KUInt32				inMaxPipeCount /* = kDefaultPipeCount */,
		long				inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLCommLayer( true /* inFlushAvailable */, inMaxPipeCount ),
			mConfiguration( nil ),
			mCurrentPipeCount( 0 ),
			mServerEndpoint( nil ),
			mTimeout( inTimeout ),
			mThreadsIntf( inThreadsIntf ),
			mListening( false ),
			mNotifierUPP( nil )	
{
	// Initialize the buffer informations.
	mLocalBindInfo.addr.buf = nil;
	
	Init( inConfiguration, inLocalBindInfo );
}

// -------------------------------------------------------------------------- //
//  * TDCLOpenTransport( void )
// -------------------------------------------------------------------------- //
TDCLOpenTransport::TDCLOpenTransport(
		IDCLThreads* inThreadsIntf,
		KUInt32	inMaxPipeCount /* = kDefaultPipeCount */,
		long	inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLCommLayer( true /* inFlushAvailable */, inMaxPipeCount ),
			mConfiguration( nil ),
			mCurrentPipeCount( 0 ),
			mServerEndpoint( nil ),
			mTimeout( inTimeout ),
			mThreadsIntf( inThreadsIntf ),
			mListening( false ),
			mNotifierUPP( nil )
{
	// Initialize the buffer informations.
	mLocalBindInfo.addr.buf = nil;
}

// -------------------------------------------------------------------------- //
//  * ~TDCLOpenTransport( void )
// -------------------------------------------------------------------------- //
TDCLOpenTransport::~TDCLOpenTransport( void )
{
	// Free the copy of the configuration, if any.
	if (mConfiguration)
	{
		::OTDestroyConfiguration( mConfiguration );
	}
	
	// Free the local bind information buffer, if any.
	if (mLocalBindInfo.addr.buf)
	{
		::free( mLocalBindInfo.addr.buf );
	}

	StopListening();
	
	// Dispose the NotifierUPP if required.
	if (mNotifierUPP)
	{
		DisposeOTNotifyUPP( mNotifierUPP );
	}

	// All the connection handles that were connected must have been freed
	// earlier.
	// It's your problem if they haven't.

	gInstanceCount--;
	
	CloseOT();
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::DoStartListening( void )
{
	if ( mServerEndpoint == nil )
	{
		// Note that we're listening.
		mListening = true;

		// Create the endpoint.
		mServerEndpoint = GetNewEndpoint( true );

		// Attach a notifier to the endpoint.
		OSStatus theErr =
						mServerEndpoint->InstallNotifier( mNotifierUPP, this );
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	}	// Otherwise, do nothing.
}

// -------------------------------------------------------------------------- //
//  * DoStopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::DoStopListening( void )
{
	if ( mServerEndpoint )
	{
		// Switch to synchronous to kill the endpoint.
	 	OSStatus theErr = mServerEndpoint->SetSynchronous();
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		// Remove the notifier
	 	mServerEndpoint->RemoveNotifier();

		// Close the endpoint
		theErr = mServerEndpoint->Close();
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		mServerEndpoint = nil;
	}

	// We're no longer listening.
	mListening = false;
}

// -------------------------------------------------------------------------- //
//  * Accept( void )
// -------------------------------------------------------------------------- //
TDCLPipe*
TDCLOpenTransport::Accept( void )
{
	TDCLPipe* theResult = nil;	// Default result (when no connection request is
								// present)
	
	// Récupération de l'appel.
	TCall theCall;
	// On ne veut aucune information.
	theCall.addr.len = 0;
	theCall.addr.maxlen = 0;
	theCall.addr.buf = nil;
	theCall.opt.len = 0;
	theCall.opt.maxlen = 0;
	theCall.opt.buf = nil;
	theCall.udata.len = 0;
	theCall.udata.maxlen = 0;
	theCall.udata.buf = nil;
	OSStatus theErr = mServerEndpoint->Listen( &theCall );
	if (theErr == noErr)
	{
		// Une requête est présente.
		KUInt32 theMaxPipeCount = GetMaxPipeCount();
		if ((theMaxPipeCount == 0) || (mCurrentPipeCount < theMaxPipeCount))
		{
			// Create a new endpoint
			TEndpoint* theEndpoint = GetNewEndpoint( false );
			if (theEndpoint == nil)
			{
				throw DCLMemError;
			}

			// Accept the connection
			theErr = mServerEndpoint->Accept( theEndpoint, &theCall );
			if (theErr != noErr)
			{
				throw DCLPlatformUnknownError( theErr );
			}

			// Create the pipe object.
			theResult = new TDCLBufferedPipe(
								new TOTPipe( this, theEndpoint, mTimeout ) );
			if (theResult == nil)
			{
				throw DCLMemError;
			}

			// Incrément du compteur
			mCurrentPipeCount++;			
		} else {
			theErr = mServerEndpoint->SndDisconnect( &theCall );
			
			if (theErr != noErr)
			{
				throw DCLPlatformUnknownError( theErr );
			}
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * Refuse( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::Refuse( void )
{
	// Récupération de l'appel.
	TCall theCall;
	// On ne veut aucune information.
	theCall.addr.len = 0;
	theCall.addr.maxlen = 0;
	theCall.addr.buf = nil;
	theCall.opt.len = 0;
	theCall.opt.maxlen = 0;
	theCall.opt.buf = nil;
	theCall.udata.len = 0;
	theCall.udata.maxlen = 0;
	theCall.udata.buf = nil;
	OSStatus theErr = mServerEndpoint->Listen( &theCall );
	if (theErr == noErr)
	{
		// Une requête est présente.
		theErr = mServerEndpoint->SndDisconnect( &theCall );
		
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Idle( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::Idle( void )
{
	// OTIdle isn't required. I guess it cannot hurt much.
	::OTIdle();

	mThreadsIntf->Yield();
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::SetTimeout( long inNewTimeout )
{
	mTimeout = inNewTimeout;
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long
TDCLOpenTransport::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * Disconnected( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::Disconnected( TDCLPipe* /* inPipe */ )
{
	// Decrease the counter.
	mCurrentPipeCount--;
}

// -------------------------------------------------------------------------- //
//  * Init( OTConfigurationRef inConfiguration )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::Init(
					OTConfigurationRef inConfiguration,
					TBind* inLocalBindInfo )
{
	// Initialize OT.
	InitOT();
	
	// Increase gInstanceCount.
	gInstanceCount++;
	
	// Clone the configuration.
	mConfiguration = ::OTCloneConfiguration( inConfiguration );
	
	// qlen is the number of connection request we accept. Here, it's
	// theMaxPipeCount or zero depending on the endpoint. This will be set in
	// GetNewEndpoint method.

	// Copy all other info from the parameter.
	unsigned long theSize = mLocalBindInfo.addr.len = inLocalBindInfo->addr.len;
	// Create a new buffer.
	KUInt8* theBuffer = mLocalBindInfo.addr.buf = (KUInt8*) ::malloc( theSize );
	
	if (theSize > 0)
	{
		if (theBuffer == nil)
		{
			throw DCLMemError;
		}
	
		// Copy the data.
		(void) ::memcpy( theBuffer, inLocalBindInfo->addr.buf, theSize );
	}
		
	// Create the NotifierUPP.
	mNotifierUPP = NewOTNotifyUPP( &Notify );
}	

// -------------------------------------------------------------------------- //
//  * IsAvailable( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLOpenTransport::IsAvailable( void )
{
	Boolean theResult = false;	// Default is not available.

	// Check that this wasn't computed already.
	if (gOTState == kUninitialized)
	{
		gOTState = kNotAvailable;	// Let's suppose it's not.
		
		// I need OT 1.1.1 or higher, which has gestaltOpenTptVersions
		NumVersion theVersion;		// Version of OT
		OSErr theErr = ::Gestalt( gestaltOpenTptVersions, (long*) &theVersion );

		if (theErr == noErr)
		{
			// Check that I have OT >= 1.1.1
			if ((theVersion.majorRev > 1)
				|| ((theVersion.majorRev == 1)
					&& (theVersion.minorAndBugRev > 0x11)))
			{
				theResult = true;
				gOTState = kAvailable;
			}
		}
	} else {
		theResult = (gOTState != kNotAvailable);
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetNewEndpoint( void )
// -------------------------------------------------------------------------- //
TEndpoint*
TDCLOpenTransport::GetNewEndpoint( Boolean forListening )
{
	TEndpoint* theResult = nil;
	
	OTResult theErr;

	// OTEndpoint requires a copy of the configuration since it disposes it.
	OTConfigurationRef theConfigCopy = OTCloneConfiguration( mConfiguration );
	theResult = ::OTOpenEndpoint(
						theConfigCopy,
						0 /* reserved */,
						nil /* info */,
						&theErr );

	// Check for errors.
	if (theErr != noErr)
	{
		// For the moment, I throw a DCLPlatformErr
		throw DCLPlatformUnknownError( theErr );
	}
	
	// Various options with the endpoint.
	// Bind the endpoint with the local address.
	if (forListening)
	{
		// For listening is Blocking & Async
		theErr = theResult->SetBlocking();
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		theErr = theResult->SetAsynchronous();
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	
		// This endpoint is for listening.
		// We'll accept up to theMaxPipeCount connection requests.
		KUInt32 theBackLogSize = GetMaxPipeCount();
		if ((theBackLogSize == 0) || (theBackLogSize > kOTMaxBackLogSize))
		{
			theBackLogSize = kOTMaxBackLogSize;
		}
		mLocalBindInfo.qlen = theBackLogSize;
		
		theErr = theResult->Bind( &mLocalBindInfo, nil );
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	} else {
		// For streaming is Nonblocking & Sync
		theErr = theResult->SetNonBlocking();
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		theErr = theResult->SetSynchronous();
		if (theErr != noErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	
		// This endpoint is not for listening.
		// So we don't accept connection requests.
		// Je ne lie pas cet EP. En effet, la doc dit que je ne suis
		// pas obligé, et si je le fais, même avec qlen mis à 0, j'ai une
		// erreur.
		// mLocalBindInfo.qlen = 0;
	}

	return theResult;
}

// -------------------------------------------------------------------------- //
//  * InitOT( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::InitOT( void )
{
	if (!IsAvailable())
	{
		throw DCLNotAvailable;
	}
	
	if (gOTState == kAvailable)
	{
		// I need to init open transport.
		if (::InitOpenTransport() != noErr)
		{
			gOTState = kNotAvailable;
			throw DCLNotAvailable;
		} else {
			gOTState = kInitialized;
		}
	}
}

// -------------------------------------------------------------------------- //
//  * CloseOT( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::CloseOT( void )
{
	if (gOTState == kInitialized)
	{
		// OT is open.
		// Let's see if there is any instance left.
		if (gInstanceCount == 0)
		{
			// There is no instance left. I can close OT.
			::CloseOpenTransport();
		}
	}
}

// -------------------------------------------------------------------------- //
//  * Notify( void*, OTEventCode, OTResult, void* )
// -------------------------------------------------------------------------- //
pascal void
TDCLOpenTransport::Notify(
		void* inContext,
		OTEventCode inEvent,
		OTResult /* inResult */,
		void* /* inCookie */
	)
{
	if (inEvent == T_LISTEN)
	{
		TDCLOpenTransport* theLayer = (TDCLOpenTransport*) inContext;
	
		// On dit au serveur qu'on a reçu une requête.
		theLayer->GetServer()->RequestPresent( theLayer );
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::TOTPipe( TDCLCommToolBox*, ConnHandle, long, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLOpenTransport::TOTPipe::TOTPipe(
		TDCLOpenTransport*	inCommLayer,
		TEndpoint*			inEndpoint,
		long				inTimeout		// in CommLayer format.
	)
		:
			TDCLPipe( inCommLayer ),
			mTimeout( inTimeout ),
			mEndpoint( inEndpoint ),
			mDisconnected( false ),
			mNotifierUPP( nil ),
			mDataPresent( false )
{
	// Create the NotifierUPP.
	mNotifierUPP = NewOTNotifyUPP( &Notify );
	
	// Install it.
	// This can fail if the endpoint isn't ready yet.
	
	OSStatus theErr;	// Result code.
	do {
		theErr = inEndpoint->InstallNotifier( mNotifierUPP, this );
		
		if (theErr == noErr)
		{
			break;	// Done.
		}
		
		if (theErr != kOTStateChangeErr)	// & != noErr
		{
			throw DCLPlatformUnknownError( theErr );
		}
		
		inCommLayer->Idle();	// Spend some time.
		
	} while (theErr == kOTStateChangeErr);
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::~TOTPipe( void )
// -------------------------------------------------------------------------- //
TDCLOpenTransport::TOTPipe::~TOTPipe( void )
{
	if (mEndpoint)
	{
		Disconnect();
	}

	// Dipose the NotifierUPP if required.
	if (mNotifierUPP)
	{
		DisposeOTNotifyUPP( mNotifierUPP );
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::Read( void*, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	if (mEndpoint)
	{
		// Nombre d'octets à lire.
		OTByteCount toRead = (OTByteCount) *ioCount;
		
		// Curseur sur les données.
		char* theBuffer = (char*) outBuffer;	
		
		OTFlags someFlags;	// I don't care about.
		
		// Temporisation.
		long timeoutInSecs = mTimeout;
		if (timeoutInSecs == kDefaultTimeout)
		{
			timeoutInSecs = kOTDefaultTimeout;
		}
		
		IDCLThreads* theThreadsIntf =
						((TDCLOpenTransport*) GetCommLayer())->mThreadsIntf;
		time_t lastOperation = ::time(NULL);
		
		// Because inCount can be long, I loop until I got all the bytes.
		// I say timeout if I haven't read any byte for too long.

		while( toRead > 0 )
		{
			// Result of Rcv
			// Indicates how many bytes were recevied or an error.
			OTResult theResult =
						mEndpoint->Rcv( theBuffer, toRead, &someFlags );
			
			if ((theResult == kOTNoDataErr) || (theResult == kOTStateChangeErr))
			{
				// Check that I haven't timed out.

				if ((timeoutInSecs != kNoTimeout) && 
					(((long)(::time(NULL) - lastOperation))
															> timeoutInSecs))
				{
					// Mise à jour du nombre d'octets lus.
					*ioCount -= toRead;

					throw DCLTimeout;
				}
			} else {
				if (theResult < 0)
				{
					// Erreur
					// Mise à jour du nombre d'octets lus.
					*ioCount -= toRead;
					
					throw DCLPlatformUnknownError( theResult );
				}
		
				if (theResult > 0)
				{
					// Write that I got some bytes.
					lastOperation = ::time(NULL);
				
					// Octets suivants.
					toRead -= theResult;
					theBuffer += theResult;
				}
			}

			// On partage le CPU.
			theThreadsIntf->Yield();
		}
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::Write( const void*, KUInt32 )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	if (mEndpoint)
	{
		// Nombre d'octets à écrire.
		OTByteCount toWrite = (OTByteCount) *ioCount;
		
		// Curseur sur les données.
		const char* theBuffer = (const char*) inBuffer;	
		
		// Temporisation		
		long timeoutInSecs = mTimeout;
		if (timeoutInSecs == kDefaultTimeout)
		{
			timeoutInSecs = kOTDefaultTimeout;
		}
		
		IDCLThreads* theThreadsIntf =
					((TDCLOpenTransport*) GetCommLayer())->mThreadsIntf;
		time_t lastOperation = ::time(NULL);
		
		// Because inCount can be long, I loop until I got all the bytes.
		// I say timeout if I haven't written any byte for too long.

		while( toWrite > 0 )
		{
			// Result of Rcv
			// Indicates how many bytes were recevied or an error.
			OTResult theResult =
					mEndpoint->Snd( (void *) theBuffer, toWrite, 0 );
			if (theResult == kOTFlowErr)
			{
#warning to improve: we should wait for T_GODATA
				// On vérifie que la temporisation n'est pas terminée.

				if ((timeoutInSecs != kNoTimeout) && 
					(((long) (::time(NULL) - lastOperation)) > timeoutInSecs))
				{
					// Mise à jour du nombre d'octets écrits.
					*ioCount -= toWrite;
					
					throw DCLTimeout;
				}
			} else {
				if (theResult < 0)
				{
					// Mise à jour du nombre d'octets écrits.
					*ioCount -= toWrite;
					
					throw DCLPlatformUnknownError( theResult );
				}
		
				if (theResult > 0)
				{
					// Write that I got some bytes.
					lastOperation = ::time(NULL);
				
					// Next bytes.
					toWrite -= theResult;
					theBuffer += theResult;
				}
			}

			// On partage le CPU.
			theThreadsIntf->Yield();
		}
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::BytesAvailable( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLOpenTransport::TOTPipe::BytesAvailable( void )
{
	KUInt32 theResult = 0;		// Local var for the result.
	
	if (mEndpoint)
	{
		OTResult theErr =
			mEndpoint->CountDataBytes((OTByteCount*) &theResult );
		
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}
	}
	
	return (theResult > 0);
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::FlushOutput( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::FlushOutput( void )
{
	if (mEndpoint)
	{
		KSInt32 theErr =
					mEndpoint->Ioctl((unsigned long) FLUSHRW, (void*) nil );
		if ((theErr < 0)
			&& (theErr != kOTNotFoundErr)
			&& (theErr != kEINVALErr))
		{
			// This is an error.
			throw DCLPlatformUnknownError( theErr );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::Disconnect( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::Disconnect( void )
{
	if (mEndpoint)
	{
		// Brutally close
		OSStatus theErr = mEndpoint->Close();
		if (theErr)
		{
			throw DCLPlatformUnknownError( theErr );
		}

		// Original method calls comm layer disconnected method which will
		// dispose the handle.
		TDCLPipe::Disconnect();
	
		mEndpoint = nil;	// So I won't answer to
							// read/write/disconnect/bytesavailable.
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::GetTimeout( void )
// -------------------------------------------------------------------------- //
long
TDCLOpenTransport::TOTPipe::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::SetTimeout( long inTimeout )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// -------------------------------------------------------------------------- //
//  * ClearDataPresent( void )
// -------------------------------------------------------------------------- //
void
TDCLOpenTransport::TOTPipe::ClearDataPresent( void )
{
	mDataPresent = false;

	// Des données peuvent être présentes à l'instant.
	if (BytesAvailable())
	{
		// (ici le risque est d'envoyer deux messages, ce qui n'est
		// pas tellement grave, donc pas besoin de sémaphore, ce qui
		// poserait d'ailleurs des problèmes vu que notre classe sémaphore
		// utilise YieldToAnyThread pour attendre ce qui est sans effet
		// lorsqu'on est dans une interruption).
		GetLink()->DataPresent();
		
		mDataPresent = true;
	}
}

// -------------------------------------------------------------------------- //
//  * TOTPipe::Notify( void*, OTEventCode, OTResult, void* )
// -------------------------------------------------------------------------- //
pascal void
TDCLOpenTransport::TOTPipe::Notify(
		void* inContext,
		OTEventCode inEvent,
		OTResult /* inResult */,
		void* /* inCookie */
	)
{
	TOTPipe* thePipe = (TOTPipe*) inContext;
	
	switch ( inEvent )
	{
		case T_DATA:
			if (!thePipe->mDataPresent)
			{
				thePipe->GetLink()->DataPresent();
				thePipe->mDataPresent = true;
			}
	}
}

// ====================== //
// Send some filthy mail. //
// ====================== //
