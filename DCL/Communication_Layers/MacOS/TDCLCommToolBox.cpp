// ==============================
// Fichier:			TDCLCommToolBox.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			20/01/2002
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
// The Original Code is TDCLCommToolBox.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLCommToolBox.cp,v 1.5 2004/11/24 18:11:01 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLCommToolBox.h>

// ANSI C headers
#include <stdlib.h>
#include <string.h>

// MacOS headers
#include <CommResources.h>
#include <CTBUtilities.h>
#include <Traps.h>

// K
#include <K/Misc/UPStrings.h>

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Interfaces/IDCLThreads.h>

// Static variable for the state of the CTB.

TDCLCommToolBox::ECTBState TDCLCommToolBox::gCTBState = kUninitialized;

// ------------------------------------------------------------------------- //
//  * TDCLCommToolBox( const char*, const char*, TDCLInterface*, KUInt32, ... )
// ------------------------------------------------------------------------- //
TDCLCommToolBox::TDCLCommToolBox(
	const char* inToolName,
	const char* inConfigString,
	IDCLThreads* inThreadsIntf,
	KUInt32	inMaxPipeCount, /* = kDefaultPipeCount */
	long	inTimeout /* = kDefaultTimeout */
)
	:
		TDCLCommLayer(
			false,	/* inFlushAvailable */
			inMaxPipeCount ),
		mThreadsIntf( inThreadsIntf ),
		mToolProcID( -1 ),
		mToolConfig( nil ),
		mCurrentConnHandle( nil ),
		mCurrentPipeCount( 0 ),
		mListening( false )
{
	// Convert the timeout
	mTimeout = ConvertTimeoutFromSeconds( inTimeout );

	// Initialize the layer.
	Init( inToolName, inConfigString );
}

// ------------------------------------------------------------------------- //
//  * TDCLCommToolBox( const char*, const char*, TDCLInterface*, KUInt32, ... )
// ------------------------------------------------------------------------- //
TDCLCommToolBox::TDCLCommToolBox(
	IDCLThreads* inThreadsIntf,
	KUInt32	inMaxPipeCount, /* = kDefaultPipeCount */
	long	inTimeout /* = kDefaultTimeout */
)
	:
		TDCLCommLayer(
			false,	/* inFlushAvailable */
			inMaxPipeCount ),
		mThreadsIntf( inThreadsIntf ),
		mToolProcID( -1 ),
		mToolConfig( nil ),
		mCurrentConnHandle( nil ),
		mCurrentPipeCount( 0 ),
		mListening( false )
{
	// Convert the timeout
	mTimeout = ConvertTimeoutFromSeconds( inTimeout );
}

// ------------------------------------------------------------------------- //
//  * ~TDCLCommToolBox( void )
// ------------------------------------------------------------------------- //
TDCLCommToolBox::~TDCLCommToolBox( void )
{
	// Free the copy of the configuration string, if any.
	if (mToolConfig)
	{
		::free( mToolConfig );
	}
	
	StopListening();
	
	// All the connection handles that were connected must have been freed
	// earlier.
	// It's your problem if they haven't.
}

// ------------------------------------------------------------------------- //
//  * DoStartListening( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::DoStartListening( void )
{
	if ( mCurrentConnHandle == nil )
	{
		// Note that we're listening.
		mListening = true;

		// Do listen.
		DoListen();
	}	// Otherwise, do nothing.
}

// ------------------------------------------------------------------------- //
//  * DoStopListening( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::DoStopListening( void )
{
	if ( mCurrentConnHandle )
	{
		::HLock( (Handle) mCurrentConnHandle );
		
		// Note the error and throw it if it's not noErr/cmNotOpen
		OSErr theErr = ::CMClose(
							mCurrentConnHandle,
							false /* async */,
							nil /* completor */,
							0 /* timeout */,
							true /* now */ );
		
		if ((theErr != noErr) && (theErr != cmNotOpen))
		{
			// I unlock the handle anyway.
			::HUnlock( (Handle) mCurrentConnHandle );

			throw DCLPlatformUnknownError( theErr );;
		}

		::CMDispose( mCurrentConnHandle );
		
		mCurrentConnHandle = nil;
	}

	// We're no longer listening.
	mListening = false;
}

#if 0
// ------------------------------------------------------------------------- //
//  * ConnectionRequestPresent( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCommToolBox::ConnectionRequestPresent( void )
{
	Boolean theResult = false;	// Default result (when no listening pipe is
								// present)
	
	if ( mCurrentConnHandle )
	{
		::HLock( (Handle) mCurrentConnHandle );
		
		CMStatFlags theStatus;			// The status of the CTB connection.
		CMBufferSizes theBufSizes;		// I don't care about it actually.
		
		// Get the error result & throw an exception if it's not null.
		OSErr theErr =
			::CMStatus( mCurrentConnHandle, theBufSizes, &theStatus );
		if (theErr != noErr)
		{
			// I unlock the handle anyway.
			::HUnlock( (Handle) mCurrentConnHandle );

			throw DCLPlatformUnknownError( theErr );;
		}
		
		if (theStatus & cmStatusIncomingCallPresent)
		{
			theResult = true;
		}

		::HUnlock( (Handle) mCurrentConnHandle );
	}
	
	return theResult;
}
#endif

// ------------------------------------------------------------------------- //
//  * Accept( void )
// ------------------------------------------------------------------------- //
TDCLPipe*
TDCLCommToolBox::Accept( void )
{
	TDCLPipe* theResult = nil;	// Default result (when no connection request is
								// present)
	
	// Check that the current pipe is ready.
#warning incompatible with the new interfaces
	throw DCLNotAvailable;

#if 0	
	if (ConnectionRequestPresent())
	{
		// [THIS IS NOT THREAD SAFE YET]

		if (mCurrentPipeCount < GetMaxPipeCount())
		{
			// Accept the connection
			OSErr theErr = ::CMAccept( mCurrentConnHandle, true );
			if (theErr != noErr)
			{
				// I unlock the handle anyway.
				::HUnlock( (Handle) mCurrentConnHandle );

				throw DCLPlatformUnknownError( theErr );;
			}
		
			// Create a new TCTBPipe object with the connection handle.
			TCTBPipe* theCTBResult =
				new TCTBPipe( this, mCurrentConnHandle, mTimeout );
			if (theCTBResult == nil)
			{
				throw DCLMemError;
			}
			theResult = new TDCLBufferedPipe( theCTBResult );
			if (theResult == nil)
			{
				delete theCTBResult;
				throw DCLMemError;
			}

			// Increase the number of pipes around.
			mCurrentPipeCount++;
		
			// Create a new ConnHandle if required/possible (i.e. re-listen)
			mCurrentConnHandle = nil;
			DoListen();
		}
	}
#endif

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * Idle( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::Idle( void )
{
	// CMIdle is required for some tool (such as Apple Modem Tool for MNP
	// Serial)
	if (mCurrentConnHandle)
		::CMIdle( mCurrentConnHandle );

	mThreadsIntf->Yield();
}

// ------------------------------------------------------------------------- //
//  * SetTimeout( long )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::SetTimeout( long inNewTimeout )
{
	// Convert this value to our own scale.
	mTimeout = ConvertTimeoutFromSeconds( inNewTimeout );
}

// ------------------------------------------------------------------------- //
//  * GetTimeout( void )
// ------------------------------------------------------------------------- //
long
TDCLCommToolBox::GetTimeout( void )
{
	// Convert this value from our own scale.
	return ConvertTimeoutToSeconds( mTimeout );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::Disconnected( TDCLPipe* /* inPipe */ )
{
	// Decrease the counter.
	mCurrentPipeCount--;
	
	// Let's see if we can re-listen in case we stopped.
	if (mListening)
	{
		DoListen();	// Won't do anything if mCurrentConnHandle is not nil.
	}
}

// ------------------------------------------------------------------------- //
//  * ConvertTimeoutFromSeconds( long )
// ------------------------------------------------------------------------- //
long
TDCLCommToolBox::ConvertTimeoutFromSeconds( long inSecsTimeout )
{
	long theResult;
	
	if (inSecsTimeout == kDefaultTimeout)
	{
		theResult = kCTBDefaultTimeout;
	} else if (inSecsTimeout == kNoTimeout) {
		theResult = kCTBNoTimeout;
	} else {
		theResult = inSecsTimeout * 60;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * ConvertTimeoutToSeconds( long )
// ------------------------------------------------------------------------- //
long
TDCLCommToolBox::ConvertTimeoutToSeconds( long inCTBTimeout )
{
	long theResult;
	
	if (inCTBTimeout == kCTBDefaultTimeout)
	{
		theResult = kDefaultTimeout;
	} else if (inCTBTimeout == kCTBNoTimeout) {
		theResult = kNoTimeout;
	} else {
		theResult = inCTBTimeout / 60;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * Init( const char*, const char* )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::Init( const char* inToolName, const char* inConfigString )
{
	// Initialize the CTB.
	if (!IsAvailable())
	{
		throw DCLNotAvailable;
	}

	// I try to get the tool.
	Str255 theToolName;		// Name of the tool as a PString
	UPStrings::C2PStrCopy( theToolName, inToolName );
	
	mToolProcID = ::CMGetProcID( theToolName );
	if (mToolProcID == -1)
	{
		throw DCLNotAvailable;
	}
	
	// Duplicate the tool configuration string.
	{
		unsigned long theLength = ::strlen( inConfigString ) + 1;
			/* incl. Null term. */
		mToolConfig = (char*) ::malloc( theLength );
		if (mToolConfig == nil)
		{
			throw DCLMemError;
		}
		(void) ::memcpy( mToolConfig, inConfigString, theLength );
	}
	
	// Adjust mMaxPipeCount if required.
	if (GetMaxPipeCount() == kDefaultPipeCount)
	{
		SetMaxPipeCount( kCTBDefaultPipeCount );
	}
}	

// ------------------------------------------------------------------------- //
//  * IsAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCommToolBox::IsAvailable( void )
{
	InitCTB();
	
	return (gCTBState == kInitialized);
}

// ------------------------------------------------------------------------- //
//  * IsAvailable( const char* )
// ------------------------------------------------------------------------- //
Boolean
TDCLCommToolBox::IsAvailable( const char* inToolName )
{
	Boolean theResult = false;
	
	if ( IsAvailable() )
	{
		// Otherwise, CTB is just not available.
		
		// I try to get the tool.
		Str255 theToolName;			// Name of the tool as a PString
		UPStrings::C2PStrCopy( theToolName, inToolName );
		if (::CMGetProcID( theToolName ) != -1)
		{
			theResult = true;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetNewConnHandle( void )
// ------------------------------------------------------------------------- //
ConnHandle
TDCLCommToolBox::GetNewConnHandle( void )
{
	ConnHandle theResult = nil;
	
	CMBufferSizes theBufSizes;		// Initialize the buffer sizes.

	// Set the CMBufferSizes to zero to get the default.
	theBufSizes[cmDataIn] = 0;
	theBufSizes[cmDataOut] = 0;
	theBufSizes[cmCntlIn] = 0;
	theBufSizes[cmCntlOut] = 0;
	theBufSizes[cmAttnIn] = 0;
	theBufSizes[cmAttnOut] = 0;

	// Create a new connection handle
	theResult =
		::CMNew( mToolProcID, cmData|cmNoMenus|cmQuiet, theBufSizes, 0, 0 );
	if (theResult == nil)
	{
		throw DCLMemError;
	}
	
	// Configure it.
	
	::HLock( (Handle) theResult );
	
	OSErr theErr = ::CMSetConfig( theResult, mToolConfig );
	if (theErr != noErr)
	{
		// I dispose the handle.
		::CMDispose( theResult );

		throw DCLPlatformUnknownError( theErr );;
	}
	
	// Unlock the Handle
	::HUnlock( (Handle) theResult );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * InitCTB( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::InitCTB( void )
{
	if (gCTBState == kUninitialized)
	{
		do {	// Fake loop.
			gCTBState = kNotAvailable;

			if (::NGetTrapAddress(_CommToolboxDispatch, OSTrap) ==
				::NGetTrapAddress(_Unimplemented, OSTrap))
			{
				break;
			}
	
			if (::InitCRM() != noErr)
			{
				break;
			}
	
			if (::InitCTBUtilities() != noErr)
			{
				break;
			}

			if (cmNoTools == ::InitCM())
			{
				break;
			}

			gCTBState = kInitialized;	// The CTB is initialized.
		} while (0);
	}
}

// ------------------------------------------------------------------------- //
//  * DoListen( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::DoListen( void )
{
	if ( mCurrentConnHandle == nil)
	{
		// First part: look for a new next index.
		// Look for a new next index.
	
		KUInt32 theMaxPipeCount = GetMaxPipeCount();
		if (mCurrentPipeCount < theMaxPipeCount)
		{
			mCurrentConnHandle = GetNewConnHandle();
				
			::HLock( (Handle) mCurrentConnHandle );

			// Let's listen on it.
			// If it fails, we dispose the conn handle and consider that we
			// won't take any additional new connection.
		
			if ( ::CMListen(
					mCurrentConnHandle,
					true /* async */,
					nil,
					-1 /* no timeout */ ) != noErr)
			{
				// It failed.

				::CMDispose( mCurrentConnHandle );
					
				mCurrentConnHandle = nil;
			} else {
				::HUnlock( (Handle) mCurrentConnHandle );
			}
		}
	}
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::TCTBPipe( TDCLCommToolBox*, ConnHandle, long, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLCommToolBox::TCTBPipe::TCTBPipe(
		TDCLCommToolBox*	inCommLayer,
		ConnHandle			inConnHandle,
		long				inTimeout		// in CTB format.
	)
		:
			TDCLPipe( inCommLayer ),
			mConnection( inConnHandle ),
			mTimeout( inTimeout )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::~TCTBPipe( void )
// ------------------------------------------------------------------------- //
TDCLCommToolBox::TCTBPipe::~TCTBPipe( void )
{
	if (mConnection)
	{
		Disconnect();
	}
}

// ------------------------------------------------------------------------- //
//	* TCTBPipe::DoRead( void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::TCTBPipe::DoRead( void* outBuffer, KUInt32* ioCount )
{
	if (mConnection)
	{
		// I wait until all bytes are received.
		// (of course, I use mTimeout)

		::HLock( (Handle) mConnection );

		long toRead = (long) *ioCount;	// Nombre d'octets ˆ lire.
		char* theBuffer = (char*) outBuffer;	
		
		CMFlags theFlags;	// Some flags I don't care about
		
		// Because inCount can be long, I loop until I got all the bytes.
		// I say timeout if I haven't read any byte.

		while( toRead > 0 )
		{
			long count = toRead;
			
			// ¤!ˆ"&! Bug in the CTB
			
			if (count > 65535)
				count = 65535;
			
			// Get the error result & throw an exception if it's not null.
			OSErr theErr = ::CMRead(
										mConnection,
										theBuffer,
										&count,
										cmData,
										false /* async */,
										nil /* completor */,
										mTimeout,
										&theFlags );
			if (theErr != noErr)
			{
				// I unlock the handle anyway.
				::HUnlock( (Handle) mConnection );
			
				// Mise ˆ jour du nombre d'octets effectivement lus.
				*ioCount -= (toRead - count);
			
				throw DCLPlatformUnknownError( theErr );;
			}

			if (count == 0)
			{
				// DŽpassement de temporisation.
			
				// I unlock the handle anyway.
				::HUnlock( (Handle) mConnection );
			
				throw DCLTimeout;
			}
		
			// Octets suivants.
			toRead -= count;
			theBuffer += count;
		}
		
		::HUnlock( (Handle) mConnection );
	}
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::DoWrite( const void*, KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::TCTBPipe::DoWrite( const void* inBuffer, KUInt32* ioCount )
{
	if (mConnection)
	{
		::HLock( (Handle) mConnection );
		
		long toWrite = (long) *ioCount;	// Nombre d'octets ˆ lire.
		const char* theBuffer = (const char*) inBuffer;	
		
		// Because inCount can be long, I loop until I wrote all bytes.
		// I say timeout if I haven't written any byte.

		while( toWrite > 0 )
		{
			long count = toWrite;
			
			// ¤!ˆ"&! Bug in the CTB
			
			if (count > 65535)
				count = 65535;
			
			// Get the error result & throw an exception if it's not null.
			OSErr theErr =
				::CMWrite(
					mConnection,
					theBuffer,
					&count,
					cmData,
					false /* async */,
					nil /* completor */,
					mTimeout,
					0 /* flags */ );
			if (theErr != noErr)
			{
				// I unlock the handle anyway.
				::HUnlock( (Handle) mConnection );
			
				// Mise ˆ jour du nombre d'octets effectivement Žcrits.
				*ioCount -= (toWrite - count);

				throw DCLPlatformUnknownError( theErr );;
			}
			
			if (count == 0)
			{
				// DŽpassement de temporisation.
			
				// I unlock the handle anyway.
				::HUnlock( (Handle) mConnection );
			
				throw DCLTimeout;
			}
		
			// Octets suivant.
			toWrite -= count;
			theBuffer += count;
		}
		
		::HUnlock( (Handle) mConnection );
	}
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::BytesAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCommToolBox::TCTBPipe::BytesAvailable( void )
{
	KUInt32 theResult = 0;		// Local var for the result.
	
	if (mConnection)
	{
		::HLock( (Handle) mConnection );
		
		CMStatFlags 	theStatus;		// I don't actually care about the
										// status, all I want is buffer sizes.
		CMBufferSizes	theBufSizes;	// This is all I want.
		
		// Get the error result & throw an exception if it's not null.
		OSErr theErr = ::CMStatus( mConnection, theBufSizes, &theStatus );
		if (theErr != noErr)
		{
			// I unlock the handle anyway.
			::HUnlock( (Handle) mConnection );

			throw DCLPlatformUnknownError( theErr );;
		}
		
		theResult = (KUInt32) theBufSizes[cmDataIn];	// It's the number of
		// bytes in this buffer that the caller wants.
		
		::HUnlock( (Handle) mConnection );
	}
	
	return (theResult > 0);
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::Disconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::TCTBPipe::Disconnect( void )
{
	if (mConnection)
	{
		// Close the connection handle.

		// Catch the err to throw an exception if it's not noErr nor cmNotOpen
		OSErr theErr = ::CMClose(
								mConnection,
								false /* async */,
								nil /* completor */,
								0 /* timeout */,
								true /* now */
							);
		if ((theErr != noErr) && (theErr != cmNotOpen))
		{
			// I unlock the handle anyway.
			::HUnlock( (Handle) mConnection );

			throw DCLPlatformUnknownError( theErr );;
		}

		::HUnlock( (Handle) mConnection );
	
		// Original method calls comm layer disconnected method which will
		// dispose the handle.
		TDCLPipe::Disconnect();
	
		mConnection = nil;	// So I won't answer to
							// read/write/disconnect/bytesavailable.
	}
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::Idle( void )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::TCTBPipe::Idle( void )
{
	if (mConnection)
	{
		// Idle on the connection.
		::CMIdle( mConnection );
	}

	// Original method calls comm layer Idle method.
	TDCLPipe::Idle();
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::GetTimeout( void )
// ------------------------------------------------------------------------- //
long
TDCLCommToolBox::TCTBPipe::GetTimeout( void )
{
	return ConvertTimeoutToSeconds( mTimeout );
}

// ------------------------------------------------------------------------- //
//  * TCTBPipe::SetTimeout( long inTimeout )
// ------------------------------------------------------------------------- //
void
TDCLCommToolBox::TCTBPipe::SetTimeout( long inTimeout )
{
	mTimeout = ConvertTimeoutFromSeconds( inTimeout );
}

// ========================================================================= //
// As in Protestant Europe, by contrast, where sects divided endlessly into  //
// smaller competing sects and no church dominated any other, all is         //
// different in the fragmented world of IBM.  That realm is now a chaos of   //
// conflicting norms and standards that not even IBM can hope to control.    //
// You can buy a computer that works like an IBM machine but contains        //
// nothing made or sold by IBM itself.  Renegades from IBM constantly set up //
// rival firms and establish standards of their own.  When IBM recently      //
// abandoned some of its original standards and decreed new ones, many of    //
// its rivals declared a puritan allegiance to IBM's original faith, and     //
// denounced the company as a divisive innovator.  Still, the IBM world is   //
// united by its distrust of icons and imagery.  IBM's screens are designed  //
// for language, not pictures.  Graven images may be tolerated by the        //
// luxurious cults, but the true IBM faith relies on the austerity of the    //
// word.                                                                     //
//               -- Edward Mendelson, "The New Republic", February 22, 1988  //
// ========================================================================= //
