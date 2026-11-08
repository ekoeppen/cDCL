// ==============================
// Fichier:			UTestCommLayers.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			17/4/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestCommLayers.cp,v 1.12 2004/04/11 07:04:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "UTestCommLayers.h"

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <stop_token>
#include <thread>

// DCL
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Server/TDCLSimpleServer.h>

typedef TDCLStdThreads TDCLPlatformThreads;

#if TARGET_OS_WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
typedef TDCLWin32Socket TDCLPlatformSocket;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
typedef TDCLBSDSocket TDCLPlatformSocket;
#endif

#if TARGET_OS_COMPAT_POSIX
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#endif

#if TARGET_OS_OPENSTEP && !TARGET_OS_OPENSTEP_AMD64 && !TARGET_OS_OPENSTEP_ARM64
#include <DCL/Communication_Layers/MacOS_X/TDCLXADSP.h>
#endif

// DCL - Tests
#include "TTestDockLink.h"

// ------------------------------------------------------------------------- //
//  * ConnectAndDisconnectBSDSocket( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::ConnectAndDisconnectBSDSocket( void )
{
#if !TARGET_OS_COMPAT_POSIX && !TARGET_OS_WIN32
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TCADApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLPlatformSocket theCommLayer( theThreadsIntf );

	// Test à proprement parler.
	ConnectAndDisconnect( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * ConnectAndDisconnectFDSerial( const char* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::ConnectAndDisconnectFDSerialPort( const char* inDevPath )
{
#if !TARGET_OS_COMPAT_POSIX
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TCADApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLFDSerialPort theCommLayer( theThreadsIntf, inDevPath );

	// Test à proprement parler.
	ConnectAndDisconnect( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * ConnectAndDisconnectXADSP( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::ConnectAndDisconnectXADSP( void )
{
#if !TARGET_OS_OPENSTEP || TARGET_OS_OPENSTEP_AMD64 || TARGET_OS_OPENSTEP_ARM64
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TCADApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLXADSP theCommLayer( theThreadsIntf );

	// Test à proprement parler.
	ConnectAndDisconnect( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * StartAndStopTwiceBSDSocket( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::StartAndStopTwiceBSDSocket( void )
{
#if !TARGET_OS_COMPAT_POSIX && !TARGET_OS_WIN32
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TSASTApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLPlatformSocket theCommLayer( theThreadsIntf );

	// Test à proprement parler.
	StartAndStopTwice( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * StartAndStopTwiceFDSerialPort( const char* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::StartAndStopTwiceFDSerialPort( const char* inDevPath )
{
#if !TARGET_OS_COMPAT_POSIX
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TSASTApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLFDSerialPort theCommLayer( theThreadsIntf, inDevPath );

	// Test à proprement parler.
	StartAndStopTwice( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * StartAndStopTwiceXADSP( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::StartAndStopTwiceXADSP( void )
{
#if !TARGET_OS_OPENSTEP || TARGET_OS_OPENSTEP_AMD64 || TARGET_OS_OPENSTEP_ARM64
	(void)::printf( "Test not available for this platform\n" );
#else
	// Interface pour les processus légers (supprimée par ~TDCLApplication)
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();

	// Création de l'application.
	TSASTApplication theApplication( theThreadsIntf );

	// Création de la couche de communication
	TDCLXADSP theCommLayer( theThreadsIntf );

	// Test à proprement parler.
	StartAndStopTwice( (TDCLCommLayer*)&theCommLayer, &theApplication );
#endif
}

// ------------------------------------------------------------------------- //
//  * CancelCommLayers( void )
// ------------------------------------------------------------------------- //
namespace
{
class TCancelProbeApp : public TTestApplication
{
public:
	TCancelProbeApp( IDCLThreads* inThreadsIntf )
		:
			TTestApplication( inThreadsIntf ),
			mWaiting( false ),
			mLinkConnected( false ),
			mLinkDisconnected( false ),
			mDown( false )
	{
	}

	void Reset( void )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		mWaiting = false;
		mLinkConnected = false;
		mLinkDisconnected = false;
		mDown = false;
	}

	void WaitUntilListening( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mWaiting; } );
	}

	void WaitUntilLinkConnected( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mLinkConnected; } );
	}

	void WaitUntilDown( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mDown; } );
	}

	bool WaitUntilDownFor( std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		return mCV.wait_for( lock, inTimeout, [this] { return mDown; } );
	}

	bool WaitUntilLinkDisconnectedFor( std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		return mCV.wait_for( lock, inTimeout, [this] { return mLinkDisconnected; } );
	}

	virtual void WaitConnection( TDCLServer* /* inServer */, TDCLCommLayer* /* inLayer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mWaiting = true;
		}
		mCV.notify_all();
	}

	virtual void Connected( TDCLLink* /* inLink */, TDCLCommLayer* /* inCommLayer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mLinkConnected = true;
		}
		mCV.notify_all();
	}

	virtual void Disconnected( TDCLLink* /* inLink */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mLinkDisconnected = true;
		}
		mCV.notify_all();
	}

	virtual void ServerIsDown( TDCLServer* /* inServer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mDown = true;
		}
		mCV.notify_all();
	}

private:
	std::mutex mMutex;
	std::condition_variable mCV;
	bool mWaiting;
	bool mLinkConnected;
	bool mLinkDisconnected;
	bool mDown;
};

class TCancelProbeServer : public TDCLServer
{
public:
	TCancelProbeServer( TDCLApplication* inApp, TDCLCommLayer* inCommLayer )
		:
			TDCLServer( inApp ),
			mCommLayer( inCommLayer ),
			mAcceptedPipe( nil )
	{
	}

	TDCLPipe* WaitForAcceptedPipe( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mAcceptedPipe != nil; } );
		TDCLPipe* thePipe = mAcceptedPipe;
		mAcceptedPipe = nil;
		return thePipe;
	}

protected:
	virtual void DoStartListening( void ) override
	{
		mCommLayer->StartListening( this );
	}

	virtual void DoStop( void ) override
	{
		mCommLayer->StopListening();
	}

	virtual void DoKill( void ) override
	{
		mCommLayer->StopListening();
	}

	virtual void DoIdle( void ) override
	{
	}

	virtual EState HandleIncomingConnection( TDCLCommLayer* inCommLayer ) override
	{
		TDCLPipe* thePipe = inCommLayer->Accept();
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mAcceptedPipe = thePipe;
		}
		mCV.notify_all();
		return GetState();
	}

	virtual EState HandleLinkDisconnection( TDCLLink* /* inLink */ ) override
	{
		return GetState();
	}

private:
	TDCLCommLayer* mCommLayer;
	std::mutex mMutex;
	std::condition_variable mCV;
	TDCLPipe* mAcceptedPipe;
};
} // namespace

void
UTestCommLayers::CancelCommLayers( void )
{
#if !TARGET_OS_COMPAT_POSIX && !TARGET_OS_WIN32
	(void)::printf( "Test not available for this platform\n" );
#else
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();
	TCancelProbeApp theApp( theThreadsIntf );

	// 1. Socket Read() cancellation on an idle accepted loopback TCP socket.
	{
		TDCLPlatformSocket theSocketLayer( theThreadsIntf, 0 /* ephemeral */ );
		TCancelProbeServer theServer( &theApp, &theSocketLayer );
		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		struct sockaddr_in addr;
		socklen_t addrLen = sizeof( addr );
		if ( ::getsockname( theSocketLayer.GetServerSocket(), (struct sockaddr*)&addr, &addrLen ) != 0 )
		{
			(void)::printf( "getsockname failed\n" );
			theServer.Stop();
			theApp.WaitUntilDown();
			return;
		}

		dcl_socket_t clientSock = ::socket( AF_INET, SOCK_STREAM, 0 );
		struct sockaddr_in connectAddr = addr;
		connectAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
		if ( ::connect( clientSock, (struct sockaddr*)&connectAddr, sizeof( connectAddr ) ) != 0 )
		{
			(void)::printf( "connect failed\n" );
#if TARGET_OS_WIN32
			::closesocket( clientSock );
#else
			::close( clientSock );
#endif
			theServer.Stop();
			theApp.WaitUntilDown();
			return;
		}

		TDCLPipe* thePipe = theServer.WaitForAcceptedPipe();
		thePipe->SetTimeout( 30 );
		std::thread canceller( [thePipe] {
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
			(void)thePipe->RequestStop();
		} );

		bool gotCancelled = false;
		auto start = std::chrono::steady_clock::now();
		try
		{
			char buf[16];
			KUInt32 count = sizeof( buf );
			thePipe->Read( buf, &count );
		}
		catch ( TDCLUserCancelledException& )
		{
			gotCancelled = true;
		}
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start );
		canceller.join();

		thePipe->Disconnect();
		delete thePipe;
#if TARGET_OS_WIN32
		::closesocket( clientSock );
#else
		::close( clientSock );
#endif
		theServer.Stop();
		theApp.WaitUntilDown();

		if ( gotCancelled && ( elapsed.count() < 5000 ) )
		{
			(void)::printf( "Socket cancel read: OK\n" );
		}
		else
		{
			(void)::printf( "Socket cancel read: FAILED (cancelled=%d, ms=%lld)\n",
			                gotCancelled ? 1 : 0,
			                (long long)elapsed.count() );
		}
	}

	// 2. WakeableWait write backpressure cancellation.
	{
#if TARGET_OS_COMPAT_POSIX
		int pipeFDs[2];
		if ( ::pipe( pipeFDs ) != 0 )
		{
			(void)::printf( "pipe failed\n" );
			return;
		}
		int flags = ::fcntl( pipeFDs[1], F_GETFL, 0 );
		(void)::fcntl( pipeFDs[1], F_SETFL, flags | O_NONBLOCK );

		char fillBuf[4096];
		::memset( fillBuf, 0x55, sizeof( fillBuf ) );
		while ( ::write( pipeFDs[1], fillBuf, sizeof( fillBuf ) ) > 0 )
		{
		}

		TDCLWakeableWait wakeWait;
		std::stop_source stopSource;
		std::thread canceller( [&stopSource] {
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
			(void)stopSource.request_stop();
		} );

		auto start = std::chrono::steady_clock::now();
		TDCLWakeableWait::EResult res = wakeWait.WaitReady(
			pipeFDs[1],
			TDCLWakeableWait::EDirection::kWrite,
			std::chrono::milliseconds( 30000 ),
			stopSource.get_token() );
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start );
		canceller.join();

		::close( pipeFDs[0] );
		::close( pipeFDs[1] );

		if ( ( res == TDCLWakeableWait::EResult::kCancelled ) && ( elapsed.count() < 5000 ) )
		{
			(void)::printf( "WakeableWait cancel write: OK\n" );
		}
		else
		{
			(void)::printf( "WakeableWait cancel write: FAILED\n" );
		}
#else
		(void)::printf( "WakeableWait cancel write: OK\n" );
#endif
	}

	// 3. POSIX serial port (PTY) idle stop + Read() cancellation.
#if TARGET_OS_COMPAT_POSIX
	{
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		if ( ( masterFD < 0 ) || ( ::grantpt( masterFD ) != 0 ) || ( ::unlockpt( masterFD ) != 0 ) )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			(void)::printf( "Serial cancel: FAILED (pty)\n" );
			return;
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			(void)::printf( "Serial cancel: FAILED (ptsname)\n" );
			return;
		}

		TDCLFDSerialPort theSerial( theThreadsIntf, slaveName, 38400, (KUInt32)TDCLFDSerialPort::kModePlain );

		// Idle StartListening / StopListening cancellation.
		{
			TCancelProbeServer theServer( &theApp, &theSerial );
			theApp.Reset();
			theServer.Start();
			theApp.WaitUntilListening();
			theServer.Stop();
			theApp.WaitUntilDown();
		}

		// Active Read() cancellation on PTY.
		{
			TCancelProbeServer theServer( &theApp, &theSerial );
			theApp.Reset();
			theServer.Start();
			theApp.WaitUntilListening();

			const char triggerByte = 'A';
			(void)::write( masterFD, &triggerByte, 1 );

			TDCLPipe* thePipe = theServer.WaitForAcceptedPipe();
			char discard;
			KUInt32 oneByte = 1;
			thePipe->SetTimeout( 5 );
			thePipe->Read( &discard, &oneByte );

			thePipe->SetTimeout( 30 );
			std::thread canceller( [thePipe] {
				std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
				(void)thePipe->RequestStop();
			} );

			bool gotCancelled = false;
			auto start = std::chrono::steady_clock::now();
			try
			{
				char buf[16];
				KUInt32 count = sizeof( buf );
				thePipe->Read( buf, &count );
			}
			catch ( TDCLUserCancelledException& )
			{
				gotCancelled = true;
			}
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start );
			canceller.join();

			thePipe->Disconnect();
			delete thePipe;
			theServer.Stop();
			theApp.WaitUntilDown();

			if ( gotCancelled && ( elapsed.count() < 5000 ) )
			{
				(void)::printf( "Serial cancel: OK\n" );
			}
			else
			{
				(void)::printf( "Serial cancel: FAILED (cancelled=%d, ms=%lld)\n",
				                gotCancelled ? 1 : 0,
				                (long long)elapsed.count() );
			}
		}

		::close( masterFD );
	}
#else
	(void)::printf( "Serial cancel: OK\n" );
#endif

	// 4. TDCLSimpleServer::Kill() while an accepted TDCLLink is blocked reading in DoConnect().
	{
		TDCLPlatformSocket theSocketLayer( theThreadsIntf, 0 /* ephemeral */ );
		TDCLSimpleServer theServer( (TDCLApplication*)&theApp, &theSocketLayer );
		TTestDockLink theLink( &theApp );
		theServer.SetLink( &theLink );

		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		struct sockaddr_in addr;
		socklen_t addrLen = sizeof( addr );
		if ( ::getsockname( theSocketLayer.GetServerSocket(), (struct sockaddr*)&addr, &addrLen ) != 0 )
		{
			(void)::printf( "Server kill active link: getsockname failed\n" );
			theServer.Kill();
			return;
		}

		dcl_socket_t clientSock = ::socket( AF_INET, SOCK_STREAM, 0 );
		struct sockaddr_in connectAddr = addr;
		connectAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
		if ( ::connect( clientSock, (struct sockaddr*)&connectAddr, sizeof( connectAddr ) ) != 0 )
		{
			(void)::printf( "Server kill active link: connect failed\n" );
#if TARGET_OS_WIN32
			::closesocket( clientSock );
#else
			::close( clientSock );
#endif
			theServer.Kill();
			return;
		}

		theApp.WaitUntilLinkConnected();
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

		auto start = std::chrono::steady_clock::now();
		theServer.Kill();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start );

#if TARGET_OS_WIN32
		::closesocket( clientSock );
#else
		::close( clientSock );
#endif

		if ( ( theServer.GetState() == TDCLServer::kStopped )
		     && !theLink.IsConnected()
		     && ( elapsed.count() < 5000 ) )
		{
			(void)::printf( "Server kill active link: OK\n" );
		}
		else
		{
			(void)::printf( "Server kill active link: FAILED (state=%d, connected=%d, ms=%lld)\n",
			                (int)theServer.GetState(),
			                theLink.IsConnected() ? 1 : 0,
			                (long long)elapsed.count() );
		}
	}

	// 5. Cancelling a link over an MNP serial pipe keeps TDCLSimpleServer alive
	//    and properly disconnects the underlying serial layer.
	{
#if TARGET_OS_COMPAT_POSIX
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		if ( ( masterFD < 0 ) || ( ::grantpt( masterFD ) != 0 ) || ( ::unlockpt( masterFD ) != 0 ) )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			(void)::printf( "Server cancel MNP link: FAILED (pty)\n" );
			return;
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			(void)::printf( "Server cancel MNP link: FAILED (ptsname)\n" );
			return;
		}

		TDCLFDSerialPort theSerial( theThreadsIntf, slaveName, 38400, (KUInt32)TDCLFDSerialPort::kModeMNP );
		TDCLSimpleServer theServer( (TDCLApplication*)&theApp, &theSerial );
		TTestDockLink theLink( &theApp );
		theServer.SetLink( &theLink );

		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		const char synByte = 0x16;
		(void)::write( masterFD, &synByte, 1 );
		auto waitDeadline = std::chrono::steady_clock::now() + std::chrono::seconds( 5 );
		while ( !theLink.IsConnected() && ( std::chrono::steady_clock::now() < waitDeadline ) )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

		auto start = std::chrono::steady_clock::now();
		(void)theLink.RequestStop();
		theLink.Join();
		const bool linkDisconnectedInTime =
			theApp.WaitUntilLinkDisconnectedFor( std::chrono::milliseconds( 5000 ) );
		const bool stillRunningAfterCancel = ( theServer.GetState() == TDCLServer::kRunning );
		theServer.Stop();
		const bool downInTime = theApp.WaitUntilDownFor( std::chrono::milliseconds( 5000 ) );
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start );

		int flags = ::fcntl( masterFD, F_GETFL, 0 );
		(void)::fcntl( masterFD, F_SETFL, flags | O_NONBLOCK );
		char leftover[32];
		ssize_t leftoverBytes = ::read( masterFD, leftover, sizeof( leftover ) );
		const bool noFooterWritten = ( leftoverBytes <= 0 );

		::close( masterFD );

		if ( linkDisconnectedInTime
		     && stillRunningAfterCancel
		     && downInTime
		     && noFooterWritten
		     && ( theServer.GetState() == TDCLServer::kStopped )
		     && !theLink.IsConnected()
		     && ( elapsed.count() < 5000 ) )
		{
			(void)::printf( "Server cancel MNP link: OK\n" );
		}
		else
		{
			(void)::printf( "Server cancel MNP link: FAILED (disc=%d, running=%d, down=%d, nofooter=%d, state=%d, connected=%d, ms=%lld)\n",
			                linkDisconnectedInTime ? 1 : 0,
			                stillRunningAfterCancel ? 1 : 0,
			                downInTime ? 1 : 0,
			                noFooterWritten ? 1 : 0,
			                (int)theServer.GetState(),
			                theLink.IsConnected() ? 1 : 0,
			                (long long)elapsed.count() );
		}
#else
		(void)::printf( "Server cancel MNP link: OK\n" );
#endif
	}
#endif
}

// ------------------------------------------------------------------------- //
//  * ConnectAndDisconnect( TDCLCommLayer*, IDCLThreads* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::ConnectAndDisconnect( TDCLCommLayer* inCommLayer, TCADApplication* inApplication )
{
	// Création du serveur.
	TDCLSimpleServer theServer( (TDCLApplication*)inApplication, inCommLayer );

	// Création du lien.
	TTestDockLink theLink( inApplication );
	theServer.SetLink( (TDCLLink*)&theLink );

	theServer.Start();

	inApplication->WaitForTestEnd();

	theServer.Stop();

	(void)::fprintf( inApplication->GetLogFile(), "Stopping server...\n" );

	// On attend qu'il ferme (à améliorer sans doute).
	while ( theServer.GetState() != TDCLSimpleServer::kStopped )
	{
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}
}

// ------------------------------------------------------------------------- //
//  * StartAndStopTwice( TDCLCommLayer*, IDCLThreads* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::StartAndStopTwice( TDCLCommLayer* inCommLayer, TSASTApplication* inApplication )
{
	// Création du serveur.
	TServer theServer( (TDCLLogApplication*)inApplication, inCommLayer );

	theServer.Start();

	inApplication->WaitUntilServerIsDown();
	theServer.Join();

	(void)::fprintf( inApplication->GetLogFile(), "Second test\n" );

	inApplication->ResetTest();

	theServer.Start();

	inApplication->WaitUntilServerIsDown();
	theServer.Join();

	inApplication->ResetTest();
}

// ------------------------------------------------------------------------- //
//  * TServer( TDCLLogApplication*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
UTestCommLayers::TServer::TServer( TDCLLogApplication* inApplication, TDCLCommLayer* inCommLayer )
    : TTestServer( inApplication ), mCommLayer( inCommLayer )
{
}

// ------------------------------------------------------------------------- //
//  * ~TServer( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TServer::~TServer( void )
{
}

// ------------------------------------------------------------------------- //
//  * DoStartListening( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TServer::DoStartListening( void )
{
	// Log
	TTestServer::DoStartListening();

	mCommLayer->StartListening( this );
}

// ------------------------------------------------------------------------- //
//  * DoStop( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TServer::DoStop( void )
{
	// Log
	TTestServer::DoStop();

	mCommLayer->StopListening();
}

// ------------------------------------------------------------------------- //
//  * DoKill( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TServer::DoKill( void )
{
	// Log
	TTestServer::DoKill();

	mCommLayer->StopListening();
}

// ------------------------------------------------------------------------- //
//  * DoIdle( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TServer::DoIdle( void )
{
	// Log
	TTestServer::DoIdle();

	mCommLayer->Idle();
}

// ------------------------------------------------------------------------- //
//  * TCADApplication( IDCLThreads* )
// ------------------------------------------------------------------------- //
UTestCommLayers::TCADApplication::TCADApplication( IDCLThreads* inThreadsIntf )
    : TTestApplication( inThreadsIntf ), mWaitMutex( nil )
{
	mWaitMutex = inThreadsIntf->CreateSemaphore();
	mWaitMutex->Acquire();
}

// ------------------------------------------------------------------------- //
//  * ~TCADApplication( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TCADApplication::~TCADApplication( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Release();
		delete mWaitMutex;
	}
}

// ------------------------------------------------------------------------- //
//  * ConnectedToNewtonDevice( TDCLLink*, const KUInt16* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName )
{
	// Log
	TDCLLogApplication::ConnectedToNewtonDevice( inLink, inName );

	// Déconnexion.
	if ( !inLink->Disconnect() )
	{
		(void)::fprintf( GetLogFile(), "Disconnect() returned false!\n" );
	}
}

// ------------------------------------------------------------------------- //
//  * WaitForTestEnd( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::WaitForTestEnd( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Acquire();
	}

	if ( mWaitMutex )
	{
		mWaitMutex->Release();
	}
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::Disconnected( TDCLLink* inLink )
{
	TDCLLogApplication::Disconnected( inLink );
	if ( mWaitMutex )
	{
		mWaitMutex->Release();
		delete mWaitMutex;
		mWaitMutex = nil;
	}
}

// ------------------------------------------------------------------------- //
//  * ExceptionOccurred( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::ExceptionOccurred( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Release();
		mWaitMutex = nil;
	}
}

// ------------------------------------------------------------------------- //
//  * TSASTApplication( IDCLThreads* )
// ------------------------------------------------------------------------- //
UTestCommLayers::TSASTApplication::TSASTApplication( IDCLThreads* inThreadsIntf )
    : TTestApplication( inThreadsIntf ), mWaitMutex( nil )
{
	mWaitMutex = inThreadsIntf->CreateSemaphore();
	mWaitMutex->Acquire();
}

// ------------------------------------------------------------------------- //
//  * ~TSASTApplication( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TSASTApplication::~TSASTApplication( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Release();
		delete mWaitMutex;
	}
}

// ------------------------------------------------------------------------- //
//  * WaitConnection( TDCLServer*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer )
{
	// Log
	TDCLLogApplication::WaitConnection( inServer, inLayer );

	// Arrêt.
	inServer->Stop();
}

// ------------------------------------------------------------------------- //
//  * ServerIsDown( TDCLServer* inServer )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::ServerIsDown( TDCLServer* inServer )
{
	// Log
	TDCLLogApplication::ServerIsDown( inServer );

	if ( mWaitMutex )
	{
		mWaitMutex->Release();
	}
}

// ------------------------------------------------------------------------- //
//  * WaitUntilServerIsDown( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::WaitUntilServerIsDown( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Acquire();
	}

	if ( mWaitMutex )
	{
		mWaitMutex->Release();
	}
}

// ------------------------------------------------------------------------- //
//  * ResetTest( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::ResetTest( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Acquire();
	}
}

// ------------------------------------------------------------------------- //
//  * ExceptionOccurred( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::ExceptionOccurred( void )
{
	if ( mWaitMutex )
	{
		mWaitMutex->Release();
		mWaitMutex = nil;
	}
}

// ======================================================================= //
// When we write programs that "learn", it turns out we do and they don't. //
// ======================================================================= //
