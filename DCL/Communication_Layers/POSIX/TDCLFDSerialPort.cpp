// ==============================
// Fichier:			TDCLFDSerialPort.cp
// Projet:			Desktop Connection Library
//
// Créé le:			4/4/2003
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
// The Original Code is TDCLFDSerialPort.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Simon Stapleton <simon@tufty.co.uk>
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLFDSerialPort.cp,v 1.9 2006/02/13 01:55:30 pguyot Exp $
// ===========

#define LOGGING

// POSIX & ANSI
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>

// DCL
#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Communication_Layers/TDCLHammerPipe.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

#ifdef LOGGING
#define LOGFILE "/tmp/dclserlog.txt"
#define INITLOG() {mLog = fopen (LOGFILE, "w+");}
#define ENDLOG() {fclose (mLog);}
#define LOGDATA(dir, data, len)\
	{int i; \
	fprintf( mLog, "[%08d] %c ", (int) time( NULL ), dir); \
	for ( i = 0; i < len; i++ ) fprintf (mLog, "%02x ", ((unsigned char *) (data))[i] ); \
	fputc( '\n', mLog);}
#define LOGREAD(data, len) LOGDATA('>', data, len)
#define LOGWRITE(data, len) LOGDATA('<', data, len)
#else
#define INITLOG()
#define ENDLOG()
#define LOGDATA(dir, data, len)
#define LOGREAD(data, len)
#define LOGWRITE(data, len)
#endif

// -------------------------------------------------------------------------- //
//  * TDCLFDSerialPort( IDCLThreads*, const char*, long )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud /* = 38400 */,
				Boolean inUseMNP /* = true */ )
	:
		TDCLSyncCommLayer( inThreadsIntf, true ),
		mIsConnected( false ),
		mSerialFD( -1 ),
		mDevPath( nil ),
		mBaud( inBaud ),
		mPipeMode( inUseMNP ? kModeMNP : kModePlain ),
		mReset( false ),
		mDisconnecting( false ),
		mRequestPresent( false ),
		mResetMutex( nil ),
		mWaitRequestMutex( nil )
{
	if (inDevPath)
	{
		Init( inDevPath );
	}
	
	mResetMutex = inThreadsIntf->CreateSemaphore();
	mWaitRequestMutex = inThreadsIntf->CreateSemaphore();
}


// -------------------------------------------------------------------------- //
//  * TDCLFDSerialPort( IDCLThreads*, const char*, long )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud,
				KUInt32 inMode )
	:
		TDCLSyncCommLayer( inThreadsIntf, true ),
		mIsConnected( false ),
		mSerialFD( -1 ),
		mDevPath( nil ),
		mBaud( inBaud ),
		mPipeMode( inMode ),
		mReset( false ),
		mDisconnecting( false ),
		mRequestPresent( false ),
		mResetMutex( nil ),
		mWaitRequestMutex( nil )
{
	if (inDevPath)
	{
		Init( inDevPath );
	}
	
	mResetMutex = inThreadsIntf->CreateSemaphore();
	mWaitRequestMutex = inThreadsIntf->CreateSemaphore();
}

// -------------------------------------------------------------------------- //
//  * ~TDCLFDSerialPort( void )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::~TDCLFDSerialPort( void )
{
	if (mWaitRequestMutex)
	{
		delete mWaitRequestMutex;
	}
	
	if (mResetMutex)
	{
		delete mResetMutex;
	}
	
	if (mDevPath)
	{
		::free( mDevPath );
	}
}

// -------------------------------------------------------------------------- //
//  * Init( const char* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Init( const char* inDevPath )
{
	// Copie du nom.
	size_t theLength = ::strlen( inDevPath ) + 1;	// Taille avec terminateur.
	mDevPath = (char*) ::malloc( theLength );
	(void) ::memcpy( mDevPath, inDevPath, theLength );
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoStartListening( void )
{
	if (mSerialFD == -1)
	{
		// Ouverture et configuration du port.
		OpenAndConfigure();
	}
}

// -------------------------------------------------------------------------- //
//  * DoStopListening( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoStopListening( void )
{
	if (mRequestPresent)
	{
		mRequestPresent = false;
		mWaitRequestMutex->Release();
	}
	
	if ((!mIsConnected) && (mSerialFD != -1))
	{
		// Rétablissement des options.
		int theErr = ::tcsetattr( mSerialFD, TCSANOW, &mOriginalOptions);
		if (theErr < 0)
		{
			throw DCLPlatformUnknownError( errno );
		}

		// Fermeture.
		theErr = ::close( mSerialFD );
		if (theErr < 0)
		{
			throw DCLPlatformUnknownError( errno );
		}
		
		mSerialFD = -1;
	}
}

// -------------------------------------------------------------------------- //
//  * WaitForIncomingRequest( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::WaitForIncomingRequest( void )
{
	if (mRequestPresent)
	{
		// On attend d'être déconnecté.
		mWaitRequestMutex->Acquire();
		mWaitRequestMutex->Release();
	}

	Boolean theResult = Select();
	
	if (theResult)
	{
		mRequestPresent = true;
		mWaitRequestMutex->Acquire();	// On attendra la prochaine fois.
	}
		
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * Select( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::Select( void )
{
	// On fait juste un appel à select en attendant des données (de la
	// part du Newton).
	int readyPorts;
	
	do {
		// On tente de bloquer la méthode ici.
		mResetMutex->Acquire();
		mResetMutex->Release();

		fd_set portSet;

		FD_ZERO(&portSet);
		FD_SET(mSerialFD,&portSet);

		// Ne pas mettre le descripteur maximal plus un est une erreur classique
		readyPorts = ::select(
									mSerialFD + 1 /* nfds */,
									&portSet /* readfds */,
									nil /* writefds */,
									nil /* exceptfsd */,
									nil /* timeout */ );

	} while( mReset );

	return (readyPorts > 0);
}

// -------------------------------------------------------------------------- //
//  * DoAccept( void )
// -------------------------------------------------------------------------- //
TDCLPipe*
TDCLFDSerialPort::DoAccept( void )
{
	TDCLPipe* theResult = nil;
	
	// Si une requête est présente, on l'accepte.
	if (mRequestPresent)
	{
		mIsConnected = true;
        switch ( mPipeMode )
        {
            case kModePlain:
                theResult = new TDCLBufferedPipe( new TFDSerialPortPipe( this ) );
                break;
            case kModeMNP:
                theResult = new TDCLMNPPipe( new TFDSerialPortPipe( this ) );
                break;
            case kModeHammer:
                theResult = new TDCLHammerPipe( new TFDSerialPortPipe( this ) );
                break;
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * DoRefuse( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::DoRefuse( void )
{
	// On ferme et on rouvre.
	Reset();
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long 
TDCLFDSerialPort::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * Disconnected( TPipe* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Disconnected( TDCLPipe* /* inPipe */ )
{
	mIsConnected = false;

	// Select fera la réinitialisation au temps voulu.
	Reset();
}

// -------------------------------------------------------------------------- //
//  * TFDSerialPortPipe( TDCLFDSerialPort* )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TFDSerialPortPipe::TFDSerialPortPipe(
												TDCLFDSerialPort* inCommLayer )
	:
		TSyncPipe(inCommLayer->GetThreadsIntf(), inCommLayer),
		mSerialFD( inCommLayer->GetSerialFD() ),
		mDisconnected( false ),
		mLog( NULL )
{
	INITLOG();
}

// -------------------------------------------------------------------------- //
//  * ~TFDSerialPortPipe( void )
// -------------------------------------------------------------------------- //
TDCLFDSerialPort::TFDSerialPortPipe::~TFDSerialPortPipe( void ) 
{
	if(!mDisconnected)
	{
		try {
			Disconnect();
		} catch ( ... ) {
			// Silence!
		}
	}
	ENDLOG();
}

// -------------------------------------------------------------------------- //
//  * Read( void*, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	int readBytes = 0;

	// Nombre d'octets voulus.
	size_t wantedBytes = (size_t) *ioCount;

	if ( mTimeout > 0 )
	{
		fd_set set;
		struct timeval timeout;

		/* Initialize the file descriptor set. */
		FD_ZERO (&set);
		FD_SET (mSerialFD, &set);

		/* Initialize the timeout data structure. */
		timeout.tv_sec = mTimeout;
		timeout.tv_usec = 0;

		/* select returns 0 if timeout, 1 if input available, -1 if error. */
		int readyPorts = ::select( mSerialFD + 1, &set, NULL, NULL, &timeout );
		if ( readyPorts == 0 )
		{
			throw DCLTimeout;
		} else if (readyPorts == 1 ) {
			// Nombre d'octets effectivement lus.
			readBytes = ::read( mSerialFD, outBuffer, wantedBytes );
		} else {
			throw DCLPlatformUnknownError( readyPorts );
		}
	} else {
		// Nombre d'octets effectivement lus.
		readBytes = ::read( mSerialFD, outBuffer, wantedBytes );
	}

	if (readBytes < 0)
	{
		int theErr;
		if (readBytes == -1)
		{
			theErr = errno;
		} else {
			theErr = readBytes;
		}
		throw DCLPlatformUnknownError( theErr );
	}

	// On met à jour avant de lancer l'exception.
	*ioCount = (KUInt32) readBytes;
	if ((KUInt32) readBytes != wantedBytes)
	{
		throw DCLUnknownError;
	}
	LOGREAD( outBuffer, readBytes );
}

// -------------------------------------------------------------------------- //
//  * Write( const void*, KUInt32* )
// -------------------------------------------------------------------------- //
void 
TDCLFDSerialPort::TFDSerialPortPipe::Write(
										const void* inBuffer, KUInt32* ioCount )
{
	// Nombre d'octets voulus.
	size_t wantedBytes = (size_t) *ioCount;
	int sentBytes = ::write( mSerialFD, inBuffer, wantedBytes );

	if (sentBytes < 0)
	{
		throw DCLPlatformUnknownError( errno );
	}

	// On met à jour avant de lancer l'exception.
	*ioCount = (KUInt32) sentBytes;
	if ((KUInt32) sentBytes != wantedBytes)
	{
		throw DCLUnknownError;
	}
	LOGWRITE( inBuffer, sentBytes );
}

// -------------------------------------------------------------------------- //
//  * BytesAvailable( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::TFDSerialPortPipe::BytesAvailable( void ) 
{
	timeval timeout;
	fd_set portSet;

	FD_ZERO( &portSet );
	FD_SET( mSerialFD, &portSet );

	timeout.tv_sec = 0;		// secondes
	timeout.tv_usec = 0;	// microsecondes

	// Délai de temporisation à 0 pour savoir si on peut lire maintenant.
	int readyPorts = ::select( mSerialFD + 1, &portSet, NULL, NULL, &timeout );

	if (readyPorts < 0)
	{
		throw DCLPlatformNotAvailable( errno );
	}

	return (readyPorts > 0);
}

// -------------------------------------------------------------------------- //
//  * WaitForIncomingData( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLFDSerialPort::TFDSerialPortPipe::WaitForIncomingData( void )
{
	Boolean theResult = false;
	if (!mDisconnected)
	{
		// On attend des données.
		theResult = ((TDCLFDSerialPort*) GetCommLayer())->Select();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * DoDisconnect( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::DoDisconnect( void ) 
{
	if(!mDisconnected) 
	{
		mDisconnected = true;
		
		// On envoie les derniers octets.
		if (::tcdrain(mSerialFD) == -1)
		{
			throw DCLPlatformUnknownError( errno );
		}
	}

	// On déconnecte au niveau de la couche de communication (qui fermera
	// derrière nous).
	((TDCLFDSerialPort*) GetCommLayer())->Disconnected(this);
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long
TDCLFDSerialPort::TFDSerialPortPipe::GetTimeout( void )
{
	return mTimeout;
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::TFDSerialPortPipe::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

// -------------------------------------------------------------------------- //
//  * OpenAndConfigure( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::OpenAndConfigure( void )
{
	// Ouverture (non bloquant, lecture et écriture).
	int theFD = ::open( mDevPath, O_RDWR | O_NOCTTY | O_NONBLOCK );
	
	if (theFD == -1)
	{
		throw DCLPlatformUnknownError( errno );
	}
	
#if	TARGET_OS_CYGWIN
	//hli: TIOEXCL not known under cygwin
#else
	// On empêche d'autres appels à open avec TIOCEXCL (cf tty(4))
	if (::ioctl(theFD, TIOCEXCL) == -1)
	{
		// Fermeture.
		(void) ::close( theFD );
		
		throw DCLPlatformUnknownError( errno );
	}
#endif

	// Configuration de l'interface.
	// On veut qu'elle soit bloquante.
	int flags = ::fcntl(theFD, F_GETFL, 0 /* ignored */);
#if	TARGET_OS_CYGWIN
	//hli: O_ASYNC not known under cygwin
	flags &= ~ O_NONBLOCK;
#else
	flags &= ~ (O_NONBLOCK | O_ASYNC);
#endif
	flags |= O_APPEND;
	int theErr = ::fcntl(theFD, F_SETFL, flags);
	if (theErr < 0)
	{
		// Fermeture.
		(void) ::close( theFD );
		throw DCLPlatformUnknownError( errno );
	}
	
	// Enregistrement des options courantes pour pouvoir les remettre après.
	if (::tcgetattr(theFD, &mOriginalOptions) == -1)
	{
		// Fermeture.
		(void) ::close( theFD );
		throw DCLPlatformUnknownError( errno );
	}
	
	// Nouvelles options.
	struct termios theOptions = mOriginalOptions;

	// 8 bits, pas de parité, un bit d'arrêt, pas de CTS/RTS (?)
	theOptions.c_iflag = IGNBRK | INPCK;
	theOptions.c_oflag = 0;
	theOptions.c_cflag = CS8 | CREAD | HUPCL | CLOCAL & ~PARENB & ~PARODD & ~CSTOPB;
	theOptions.c_lflag = 0;
	
	// Vitesse.
	// (cfsetispeed & cfsetospeed sont des macros sur certaines plateformes)
    switch(mBaud) {
        #ifdef B2400
        case 2400:
            theErr = cfsetospeed(&theOptions, B2400);
            break;
        #endif
        #ifdef B4800
        case 4800:
            theErr = cfsetospeed(&theOptions, B4800);
            break;
        #endif
        #ifdef B9600
        case 9600:
            theErr = cfsetospeed(&theOptions, B9600);
            break;
        #endif
        #ifdef B19200
        case 19200:
            theErr = cfsetospeed(&theOptions, B19200);
            break;
        #endif
        #ifdef B38400
        case 38400:
            theErr = cfsetospeed(&theOptions, B38400);
            break;
        #endif
        #ifdef B57600
        case 57600:
            theErr = cfsetospeed(&theOptions, B57600);
            break;
        #endif
        #ifdef B115200
        case 115200:
            theErr = cfsetospeed(&theOptions, B115200);
            break;
        #endif
        #ifdef B230400
        case 230400:
            theErr = cfsetospeed(&theOptions, B230400);
            break;
        #endif
        default:
			throw DCLPlatformUnknownError( errno );
        }

	if (theErr < 0)
	{
		(void) ::close( theFD );
		throw DCLPlatformUnknownError( errno );
	}
	
	// B0 copies the speed from cfsetospeed above
	theErr = cfsetispeed( &theOptions, B0 );
	if (theErr < 0)
	{
		(void) ::close( theFD );
		throw DCLPlatformUnknownError( errno );
	}
	
	// Accès bloquant jusqu'à ce qu'un caractère soit lu ou qu'une temporisation
	// d'une seconde soit arrivée.
	theOptions.c_cc[VMIN] = 1;
	theOptions.c_cc[VTIME] = 10;

	// On règle les options.
	theErr = ::tcsetattr( theFD, TCSANOW, &theOptions );
	if (theErr < 0)
    {
		// Fermeture
		(void) ::close( theFD );
		
		throw DCLPlatformUnknownError( errno );
	}
	
	// On enregistre le descripteur de fichier.
	mSerialFD = theFD;
}

// -------------------------------------------------------------------------- //
//  * Reset( void )
// -------------------------------------------------------------------------- //
void
TDCLFDSerialPort::Reset( void )
{
	// Bloquons la méthode Select.
	mResetMutex->Acquire();
	
	// On marque qu'on est en train de réinitialiser.
	mReset = true;
	
	// On ferme le port en ignorant les erreurs.
	(void) ::close( mSerialFD );
	
	// On le rouvre.
	OpenAndConfigure();
	
	// On n'est plus en train de réinitialiser.
	mReset = false;
		
	// On n'a plus de requête présente.
	mRequestPresent = false;
	mWaitRequestMutex->Release();

	// Débloquage de la méthode Select.
	mResetMutex->Release();
}

// ======================================================== //
// Blinding speed can compensate for a lot of deficiencies. //
//                 -- David Nichols                         //
// ======================================================== //
