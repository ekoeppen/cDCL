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
#include <string>
#include <thread>

// DCL
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Server/TDCLOneLinkServer.h>
#include <DCL/Server/TDCLSimpleServer.h>

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
	// Création de l'application.
	TCADApplication theApplication;

	// Test à proprement parler.
	ConnectAndDisconnect(
		std::make_unique<TDCLPlatformSocket>(),
		&theApplication );
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
	// Création de l'application.
	TCADApplication theApplication;

	// Test à proprement parler.
	ConnectAndDisconnect(
		std::make_unique<TDCLFDSerialPort>( inDevPath ),
		&theApplication );
#endif
}

//  * StartAndStopTwiceBSDSocket( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::StartAndStopTwiceBSDSocket( void )
{
#if !TARGET_OS_COMPAT_POSIX && !TARGET_OS_WIN32
	(void)::printf( "Test not available for this platform\n" );
#else
	// Création de l'application.
	TSASTApplication theApplication;

	// Création de la couche de communication
	TDCLPlatformSocket theCommLayer;

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
	// Création de l'application.
	TSASTApplication theApplication;

	// Création de la couche de communication
	TDCLFDSerialPort theCommLayer( inDevPath );

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
	TCancelProbeApp( void )
		:
			TTestApplication(),
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
			mAcceptedPipe(),
			mRefuseCount( 0 ),
			mRefusedCount( 0 )
	{
	}

	void SetRefuseCount( int inCount )
	{
		std::lock_guard<std::mutex> lock( mMutex );
		mRefuseCount = inCount;
		mRefusedCount = 0;
	}

	bool WaitForRefusedCountFor( int inExpected, std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		return mCV.wait_for( lock, inTimeout, [this, inExpected] {
			return mRefusedCount >= inExpected;
		} );
	}

	std::unique_ptr<TDCLPipe> WaitForAcceptedPipe( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mAcceptedPipe != nil; } );
		return std::move( mAcceptedPipe );
	}

	std::unique_ptr<TDCLPipe> WaitForAcceptedPipeFor( std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		if (!mCV.wait_for( lock, inTimeout, [this] { return mAcceptedPipe != nil; } ))
		{
			return nil;
		}
		return std::move( mAcceptedPipe );
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
		bool shouldRefuse = false;
		{
			std::lock_guard<std::mutex> lock( mMutex );
			if (mRefuseCount > 0)
			{
				--mRefuseCount;
				shouldRefuse = true;
			}
		}
		if (shouldRefuse)
		{
			inCommLayer->Refuse();
			{
				std::lock_guard<std::mutex> lock( mMutex );
				++mRefusedCount;
			}
			mCV.notify_all();
			return GetState();
		}
		std::unique_ptr<TDCLPipe> thePipe = inCommLayer->Accept();
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mAcceptedPipe = std::move( thePipe );
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
	std::unique_ptr<TDCLPipe> mAcceptedPipe;
	int mRefuseCount;
	int mRefusedCount;
};

class TKillOnConnectApp : public TCancelProbeApp
{
public:
	TKillOnConnectApp( void )
		:
			TCancelProbeApp(),
			mTargetServer( nil )
	{
	}

	void SetTargetServer( TDCLServer* inServer )
	{
		mTargetServer = inServer;
	}

	virtual void Connected( TDCLLink* inLink, TDCLCommLayer* inCommLayer ) override
	{
		TCancelProbeApp::Connected( inLink, inCommLayer );
		if (mTargetServer)
		{
			mTargetServer->Kill();
		}
	}

private:
	TDCLServer* mTargetServer;
};

bool ConnectLoopback( TDCLPlatformSocket& inSocketLayer, dcl_socket_t& outClientSock )
{
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof( addr );
	if ( ::getsockname( inSocketLayer.GetServerSocket(), (struct sockaddr*)&addr, &addrLen ) != 0 )
	{
		return false;
	}
	dcl_socket_t clientSock = ::socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in connectAddr = addr;
	connectAddr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	if ( ::connect( clientSock, (struct sockaddr*)&connectAddr, sizeof( connectAddr ) ) != 0 )
	{
#if TARGET_OS_WIN32
		::closesocket( clientSock );
#else
		::close( clientSock );
#endif
		return false;
	}
	outClientSock = clientSock;
	return true;
}

template <typename ServerT>
bool RunKillFromConnectedCase( void )
{
	TKillOnConnectApp theApp;
	auto socketLayer = std::make_unique<TDCLPlatformSocket>( 0 /* ephemeral */ );
	auto* socketPtr = socketLayer.get();
	dcl_socket_t clientSock = 0;

	auto drive = [&]( ServerT& ioServer ) -> bool {
		theApp.SetTargetServer( &ioServer );
		auto* linkPtr = ioServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		if constexpr ( std::is_same_v<ServerT, TDCLOneLinkServer> )
		{
			(void)ioServer.AddCommLayer( std::move( socketLayer ) );
		}
		theApp.Reset();
		ioServer.Start();
		theApp.WaitUntilListening();

		if ( !ConnectLoopback( *socketPtr, clientSock ) )
		{
			ioServer.Kill();
			return false;
		}

		theApp.WaitUntilLinkConnected();
		(void)linkPtr->Wait();
		const bool stopped = ( ioServer.GetState() == TDCLServer::kStopped );
		const bool disconnected = !linkPtr->IsConnected();
#if TARGET_OS_WIN32
		::closesocket( clientSock );
#else
		::close( clientSock );
#endif
		return stopped && disconnected;
	};

	if constexpr ( std::is_same_v<ServerT, TDCLOneLinkServer> )
	{
		ServerT theServer( (TDCLApplication*)&theApp );
		return drive( theServer );
	}
	else
	{
		ServerT theServer( (TDCLApplication*)&theApp, std::move( socketLayer ) );
		return drive( theServer );
	}
}

class THandshakeFailApp : public TCancelProbeApp
{
public:
	THandshakeFailApp( void )
		:
			TCancelProbeApp(),
			mLinkExceptions( 0 ),
			mKillOnException( nil )
	{
	}

	void SetKillOnException( TDCLServer* inServer )
	{
		mKillOnException = inServer;
	}

	bool WaitUntilExceptionsFor( unsigned inCount, std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mExcMutex );
		return mExcCV.wait_for( lock, inTimeout, [&] { return mLinkExceptions >= inCount; } );
	}

	virtual void HandleLinkException( TDCLLink*, TDCLException* ) override
	{
		{
			std::lock_guard<std::mutex> lock( mExcMutex );
			++mLinkExceptions;
		}
		mExcCV.notify_all();
		if (mKillOnException)
		{
			mKillOnException->Kill();
		}
	}

private:
	std::mutex mExcMutex;
	std::condition_variable mExcCV;
	unsigned mLinkExceptions;
	TDCLServer* mKillOnException;
};

template <typename ServerT>
bool RunReconnectAfterHandshakeErrorCase( bool inKillInException )
{
	THandshakeFailApp theApp;
	auto socketLayer = std::make_unique<TDCLPlatformSocket>( 0 /* ephemeral */ );
	auto* socketPtr = socketLayer.get();

	auto runWithServer = [&]( ServerT& ioServer ) -> bool {
		if (inKillInException)
		{
			theApp.SetKillOnException( &ioServer );
		}
		auto* linkPtr = ioServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		if constexpr ( std::is_same_v<ServerT, TDCLOneLinkServer> )
		{
			(void)ioServer.AddCommLayer( std::move( socketLayer ) );
		}
		theApp.Reset();
		ioServer.Start();
		theApp.WaitUntilListening();

		dcl_socket_t badClient = 0;
		if ( !ConnectLoopback( *socketPtr, badClient ) )
		{
			ioServer.Kill();
			return false;
		}
		// Send an unexpected Dock command header ("newtdockXXXX", length 0) so DoConnect throws DCLUnexpDockCmd.
		const char badCmd[16] = { 'n', 'e', 'w', 't', 'd', 'o', 'c', 'k', 'X', 'X', 'X', 'X', 0, 0, 0, 0 };
		(void)::send( badClient, badCmd, sizeof( badCmd ), 0 );

		const bool gotException = theApp.WaitUntilExceptionsFor( 1, std::chrono::milliseconds( 5000 ) );
		(void)linkPtr->Wait();
#if TARGET_OS_WIN32
		::closesocket( badClient );
#else
		::close( badClient );
#endif
		if ( !gotException )
		{
			ioServer.Kill();
			return false;
		}
		if ( inKillInException )
		{
			return ( ioServer.GetState() == TDCLServer::kStopped ) && !linkPtr->IsConnected();
		}

		// A second connection must not hang waiting on the exited link.
		theApp.Reset();
		dcl_socket_t secondClient = 0;
		if ( !ConnectLoopback( *socketPtr, secondClient ) )
		{
			ioServer.Kill();
			return false;
		}
		theApp.WaitUntilLinkConnected();
		ioServer.Kill();
#if TARGET_OS_WIN32
		::closesocket( secondClient );
#else
		::close( secondClient );
#endif
		return ( ioServer.GetState() == TDCLServer::kStopped ) && !linkPtr->IsConnected();
	};

	if constexpr ( std::is_same_v<ServerT, TDCLOneLinkServer> )
	{
		ServerT theServer( (TDCLApplication*)&theApp );
		return runWithServer( theServer );
	}
	else
	{
		ServerT theServer( (TDCLApplication*)&theApp, std::move( socketLayer ) );
		return runWithServer( theServer );
	}
}
} // namespace

void
UTestCommLayers::CancelCommLayers( void )
{
#if !TARGET_OS_COMPAT_POSIX && !TARGET_OS_WIN32
	(void)::printf( "Test not available for this platform\n" );
#else
	TCancelProbeApp theApp;

	// 1. Socket Read() cancellation on an idle accepted loopback TCP socket.
	{
		TDCLPlatformSocket theSocketLayer( 0 /* ephemeral */ );
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

		std::unique_ptr<TDCLPipe> thePipe = theServer.WaitForAcceptedPipe();
		thePipe->SetTimeout( 30 );
		TDCLPipe* thePipePtr = thePipe.get();
		std::thread canceller( [thePipePtr] {
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
			(void)thePipePtr->RequestStop();
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
		thePipe.reset();
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

		TDCLFDSerialPort theSerial( slaveName, 38400, (KUInt32)TDCLFDSerialPort::kModePlain );

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

			std::unique_ptr<TDCLPipe> thePipe = theServer.WaitForAcceptedPipe();
			char discard;
			KUInt32 oneByte = 1;
			thePipe->SetTimeout( 5 );
			thePipe->Read( &discard, &oneByte );

			thePipe->SetTimeout( 30 );
			TDCLPipe* thePipePtr = thePipe.get();
			std::thread canceller( [thePipePtr] {
				std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
				(void)thePipePtr->RequestStop();
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
			thePipe.reset();
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
		auto socketLayer = std::make_unique<TDCLPlatformSocket>( 0 /* ephemeral */ );
		auto* socketPtr = socketLayer.get();
		TDCLSimpleServer theServer( (TDCLApplication*)&theApp, std::move( socketLayer ) );
		auto* linkPtr = theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );

		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		struct sockaddr_in addr;
		socklen_t addrLen = sizeof( addr );
		if ( ::getsockname( socketPtr->GetServerSocket(), (struct sockaddr*)&addr, &addrLen ) != 0 )
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
		     && !linkPtr->IsConnected()
		     && ( elapsed.count() < 5000 ) )
		{
			(void)::printf( "Server kill active link: OK\n" );
		}
		else
		{
			(void)::printf( "Server kill active link: FAILED (state=%d, connected=%d, ms=%lld)\n",
			                (int)theServer.GetState(),
			                linkPtr->IsConnected() ? 1 : 0,
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

		TDCLSimpleServer theServer(
			(TDCLApplication*)&theApp,
			std::make_unique<TDCLFDSerialPort>( slaveName, 38400, (KUInt32)TDCLFDSerialPort::kModeMNP ) );
		auto* linkPtr = theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );

		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		const char synByte = 0x16;
		(void)::write( masterFD, &synByte, 1 );
		auto waitDeadline = std::chrono::steady_clock::now() + std::chrono::seconds( 5 );
		while ( !linkPtr->IsConnected() && ( std::chrono::steady_clock::now() < waitDeadline ) )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

		auto start = std::chrono::steady_clock::now();
		(void)linkPtr->RequestStop();
		(void)linkPtr->Wait();
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
		     && !linkPtr->IsConnected()
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
			                linkPtr->IsConnected() ? 1 : 0,
			                (long long)elapsed.count() );
		}
#else
		(void)::printf( "Server cancel MNP link: OK\n" );
#endif
	}

	// 6. Calling Kill() from Connected() on the link thread must succeed cleanly.
	{
		const bool ok =
			RunKillFromConnectedCase<TDCLSimpleServer>()
			&& RunKillFromConnectedCase<TDCLOneLinkServer>();
		(void)::printf( "Server kill from Connected: %s\n", ok ? "OK" : "FAILED" );
	}

	// 7. Handshake failure (DCLUnexpDockCmd) must release pipe ownership so
	//    subsequent connections and Kill() (including from HandleLinkException) do not hang.
	{
		const bool ok =
			RunReconnectAfterHandshakeErrorCase<TDCLSimpleServer>( false )
			&& RunReconnectAfterHandshakeErrorCase<TDCLSimpleServer>( true )
			&& RunReconnectAfterHandshakeErrorCase<TDCLOneLinkServer>( false )
			&& RunReconnectAfterHandshakeErrorCase<TDCLOneLinkServer>( true );
		(void)::printf( "Server reconnect after handshake error: %s\n", ok ? "OK" : "FAILED" );
	}

	// 8. POSIX serial port Reset() while Select() is waiting, after Refuse(),
	//    after pipe disconnect, and when OpenAndConfigure() throws.
#if TARGET_OS_COMPAT_POSIX
	{
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		if ( ( masterFD < 0 ) || ( ::grantpt( masterFD ) != 0 ) || ( ::unlockpt( masterFD ) != 0 ) )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			(void)::printf( "Serial reset: FAILED (pty)\n" );
			return;
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			(void)::printf( "Serial reset: FAILED (ptsname)\n" );
			return;
		}

		bool resetWhileSelectOK = false;
		bool refuseThenAcceptOK = false;
		bool disconnectThenAcceptOK = false;
		bool failedOpenRecoveryOK = false;

		{
			TDCLFDSerialPort theSerial( slaveName, 38400, (KUInt32)TDCLFDSerialPort::kModePlain );
			TCancelProbeServer theServer( &theApp, &theSerial );
			theApp.Reset();
			theServer.Start();
			theApp.WaitUntilListening();

			// 8a. Call DoRefuse() -> Reset() while Select() is actively blocked in WaitReady().
			std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
			theSerial.DoRefuse();

			const char b1 = 'B';
			(void)::write( masterFD, &b1, 1 );
			std::unique_ptr<TDCLPipe> pipe1 = theServer.WaitForAcceptedPipeFor( std::chrono::milliseconds( 2000 ) );
			if ( pipe1 )
			{
				char discard = 0;
				KUInt32 oneByte = 1;
				pipe1->SetTimeout( 5 );
				pipe1->Read( &discard, &oneByte );
				resetWhileSelectOK = ( discard == b1 );
				// 8b. Disconnect() -> Disconnected() -> Reset() while server stays running.
				TTestDockLink dummyLink( &theApp );
				(void)pipe1->Connected( &dummyLink );
				pipe1->Disconnect();
				pipe1.reset();
			}

			const char b2 = 'C';
			(void)::write( masterFD, &b2, 1 );
			std::unique_ptr<TDCLPipe> pipe2 = theServer.WaitForAcceptedPipeFor( std::chrono::milliseconds( 2000 ) );
			if ( pipe2 )
			{
				char discard = 0;
				KUInt32 oneByte = 1;
				pipe2->SetTimeout( 5 );
				pipe2->Read( &discard, &oneByte );
				disconnectThenAcceptOK = ( discard == b2 );
				TTestDockLink dummyLink( &theApp );
				(void)pipe2->Connected( &dummyLink );
				pipe2->Disconnect();
				pipe2.reset();
			}

			// 8c. Refuse an incoming connection via inCommLayer->Refuse() -> Reset(), then accept next.
			theServer.SetRefuseCount( 1 );
			const char b3 = 'D';
			(void)::write( masterFD, &b3, 1 );
			const bool refused = theServer.WaitForRefusedCountFor( 1, std::chrono::milliseconds( 2000 ) );
			std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
			const char b4 = 'E';
			(void)::write( masterFD, &b4, 1 );
			std::unique_ptr<TDCLPipe> pipe3 = theServer.WaitForAcceptedPipeFor( std::chrono::milliseconds( 2000 ) );
			if ( refused && pipe3 )
			{
				char discard = 0;
				KUInt32 oneByte = 1;
				pipe3->SetTimeout( 5 );
				pipe3->Read( &discard, &oneByte );
				refuseThenAcceptOK = ( discard == b4 );
				TTestDockLink dummyLink( &theApp );
				(void)pipe3->Connected( &dummyLink );
				pipe3->Disconnect();
				pipe3.reset();
			}

			theServer.Stop();
			(void)theApp.WaitUntilDownFor( std::chrono::milliseconds( 2000 ) );
		}

		// 8d. Failed reopen in Reset() (OpenAndConfigure() throws) must not wedge mReset or Stop().
		{
			const std::string linkPath = "/tmp/dcl_serial_reset_test_" + std::to_string( ::getpid() );
			(void)::unlink( linkPath.c_str() );
			if ( ::symlink( slaveName, linkPath.c_str() ) == 0 )
			{
				TDCLFDSerialPort theSerial( linkPath.c_str(), 38400, (KUInt32)TDCLFDSerialPort::kModePlain );
				TCancelProbeServer theServer( &theApp, &theSerial );
				theApp.Reset();
				theServer.Start();
				theApp.WaitUntilListening();

				std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
				(void)::unlink( linkPath.c_str() );

				bool threw = false;
				try
				{
					theSerial.DoRefuse();
				}
				catch ( TDCLException& )
				{
					threw = true;
				}

				theServer.Stop();
				const bool downInTime = theApp.WaitUntilDownFor( std::chrono::milliseconds( 2000 ) );
				failedOpenRecoveryOK = threw && downInTime;
			}
		}

		::close( masterFD );

		if ( resetWhileSelectOK && disconnectThenAcceptOK && refuseThenAcceptOK && failedOpenRecoveryOK )
		{
			(void)::printf( "Serial reset: OK\n" );
		}
		else
		{
			(void)::printf( "Serial reset: FAILED (select=%d, disc=%d, refuse=%d, failopen=%d)\n",
			                resetWhileSelectOK ? 1 : 0,
			                disconnectThenAcceptOK ? 1 : 0,
			                refuseThenAcceptOK ? 1 : 0,
			                failedOpenRecoveryOK ? 1 : 0 );
		}
	}
#else
	(void)::printf( "Serial reset: OK\n" );
#endif
#endif
}

// ------------------------------------------------------------------------- //
//  * ConnectAndDisconnect( TDCLCommLayer*, IDCLThreads* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::ConnectAndDisconnect( std::unique_ptr<TDCLCommLayer> inCommLayer, TCADApplication* inApplication )
{
	// Création du serveur.
	TDCLSimpleServer theServer(
		(TDCLApplication*)inApplication,
		std::move( inCommLayer ),
		std::make_unique<TTestDockLink>( inApplication ) );

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

	(void)::fprintf( inApplication->GetLogFile(), "Second test\n" );

	std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	inApplication->ResetTest();

	theServer.Start();

	inApplication->WaitUntilServerIsDown();

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
UTestCommLayers::TCADApplication::TCADApplication( void )
    : TTestApplication(), mDone( false )
{
}

// ------------------------------------------------------------------------- //
//  * ~TCADApplication( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TCADApplication::~TCADApplication( void )
{
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
	std::unique_lock<std::mutex> lock( mWaitMutex );
	mWaitCV.wait( lock, [ this ] { return mDone; } );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::Disconnected( TDCLLink* inLink )
{
	TDCLLogApplication::Disconnected( inLink );
	{
		std::lock_guard<std::mutex> lock( mWaitMutex );
		mDone = true;
	}
	mWaitCV.notify_all();
}

// ------------------------------------------------------------------------- //
//  * ExceptionOccurred( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TCADApplication::ExceptionOccurred( void )
{
	{
		std::lock_guard<std::mutex> lock( mWaitMutex );
		mDone = true;
	}
	mWaitCV.notify_all();
}

// ------------------------------------------------------------------------- //
//  * TSASTApplication( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TSASTApplication::TSASTApplication( void )
    : TTestApplication(), mDone( false )
{
}

// ------------------------------------------------------------------------- //
//  * ~TSASTApplication( void )
// ------------------------------------------------------------------------- //
UTestCommLayers::TSASTApplication::~TSASTApplication( void )
{
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

	{
		std::lock_guard<std::mutex> lock( mWaitMutex );
		mDone = true;
	}
	mWaitCV.notify_all();
}

// ------------------------------------------------------------------------- //
//  * WaitUntilServerIsDown( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::WaitUntilServerIsDown( void )
{
	std::unique_lock<std::mutex> lock( mWaitMutex );
	mWaitCV.wait( lock, [ this ] { return mDone; } );
}

// ------------------------------------------------------------------------- //
//  * ResetTest( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::ResetTest( void )
{
	std::lock_guard<std::mutex> lock( mWaitMutex );
	mDone = false;
}

// ------------------------------------------------------------------------- //
//  * ExceptionOccurred( void )
// ------------------------------------------------------------------------- //
void
UTestCommLayers::TSASTApplication::ExceptionOccurred( void )
{
	{
		std::lock_guard<std::mutex> lock( mWaitMutex );
		mDone = true;
	}
	mWaitCV.notify_all();
}

// ------------------------------------------------------------------------- //
//  * TestServerConfiguration( void )
// ------------------------------------------------------------------------- //
namespace {

class TConfigTestCommLayer : public TDCLCommLayer
{
public:
	TConfigTestCommLayer( void )
		: TDCLCommLayer( false, 0 ), mStartCount( 0 ), mStopCount( 0 ), mTimeout( kDefaultTimeout )
	{
	}

	virtual std::unique_ptr<TDCLPipe> Accept( void ) override { return nullptr; }
	virtual void Refuse( void ) override {}
	virtual void SetTimeout( long inTimeout ) override { mTimeout = inTimeout; }
	virtual long GetTimeout( void ) override { return mTimeout; }
	virtual void Disconnected( TDCLPipe* /* inPipe */ ) override {}

	int GetStartCount( void ) const { return mStartCount; }
	int GetStopCount( void ) const { return mStopCount; }

protected:
	virtual void DoStartListening( void ) override
	{
		++mStartCount;
	}

	virtual void DoStopListening( void ) override
	{
		++mStopCount;
	}

private:
	int mStartCount;
	int mStopCount;
	long mTimeout;
};

class TConfigTestApplication : public TTestApplication
{
public:
	TConfigTestApplication( void )
		: TTestApplication(),
		  mWaitingCount( 0 ),
		  mDownCount( 0 )
	{
	}

	virtual void WaitConnection( TDCLServer* /* inServer */, TDCLCommLayer* /* inLayer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mWaitingCount;
		}
		mCV.notify_all();
	}

	virtual void ServerIsDown( TDCLServer* /* inServer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mDownCount;
		}
		mCV.notify_all();
	}

	void WaitForListening( int inExpectedCount = 1 )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [&]() { return mWaitingCount >= inExpectedCount; } );
		mWaitingCount -= inExpectedCount;
	}

	void WaitUntilServerIsDown( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [&]() { return mDownCount >= 1; } );
		--mDownCount;
	}

private:
	std::mutex mMutex;
	std::condition_variable mCV;
	int mWaitingCount;
	int mDownCount;
};

template <typename Fn>
static bool
ExpectBadState( Fn&& inFn )
{
	try
	{
		inFn();
		return false;
	}
	catch ( const TDCLBadStateError& )
	{
		return true;
	}
	catch ( const TDCLException& anException )
	{
		return anException.GetExCode() == TDCLException::exBadStateError;
	}
}

template <typename Fn>
static bool
ExpectBadParam( Fn&& inFn )
{
	try
	{
		inFn();
		return false;
	}
	catch ( const TDCLBadParamError& )
	{
		return true;
	}
	catch ( const TDCLException& anException )
	{
		return anException.GetExCode() == TDCLException::exBadParamError;
	}
}

static void
Check( bool inCond, const char* inLabel )
{
	if ( inCond )
	{
		(void)::printf( "PASS: %s\n", inLabel );
	}
	else
	{
		(void)::printf( "FAIL: %s\n", inLabel );
	}
}

} // namespace

void
UTestCommLayers::TestServerConfiguration( void )
{
	TConfigTestApplication theApp;

	// 1. TDCLSimpleServer: SetLink allowed before Start(), rejected while running,
	//    after Stop(), across a restart, and after Kill().
	{
		auto layer = std::make_unique<TConfigTestCommLayer>();
		auto* layerPtr = layer.get();
		TDCLSimpleServer theServer( &theApp, std::move( layer ) );

		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		Check( layerPtr->GetStartCount() == 0, "TDCLSimpleServer SetLink pre-Start" );

		theServer.Start();
		theApp.WaitForListening();
		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLSimpleServer SetLink rejected while running" );

		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		(void)theServer.Wait();
		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLSimpleServer SetLink rejected after Stop" );

		theServer.Start();
		theApp.WaitForListening();
		theServer.Kill();
		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLSimpleServer SetLink rejected after Kill" );
	}

	// 2. TDCLOneLinkServer: SetLink & AddCommLayer pre-Start(), nil rejected,
	//    and both rejected while running, after Stop(), and after Kill().
	{
		TDCLOneLinkServer theServer( &theApp );

		Check( ExpectBadParam( [&]() { (void)theServer.AddCommLayer( nullptr ); } ),
			"TDCLOneLinkServer AddCommLayer(nil) rejected" );

		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		auto layer1 = std::make_unique<TConfigTestCommLayer>();
		auto layer2 = std::make_unique<TConfigTestCommLayer>();
		auto* layer1Ptr = layer1.get();
		auto* layer2Ptr = layer2.get();
		(void)theServer.AddCommLayer( std::move( layer1 ) );
		(void)theServer.AddCommLayer( std::move( layer2 ) );
		Check( (layer1Ptr->GetStartCount() == 0) && (layer2Ptr->GetStartCount() == 0),
			"TDCLOneLinkServer AddCommLayer does not start listening pre-Start" );

		theServer.Start();
		theApp.WaitForListening( 2 );
		Check( (layer1Ptr->GetStartCount() == 1) && (layer2Ptr->GetStartCount() == 1),
			"TDCLOneLinkServer Start starts all registered layers" );

		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLOneLinkServer SetLink rejected while running" );
		Check( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ),
			"TDCLOneLinkServer AddCommLayer rejected while running" );

		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		(void)theServer.Wait();
		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLOneLinkServer SetLink rejected after Stop" );
		Check( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ),
			"TDCLOneLinkServer AddCommLayer rejected after Stop" );

		theServer.Start();
		theApp.WaitForListening( 2 );
		Check( (layer1Ptr->GetStartCount() == 2) && (layer2Ptr->GetStartCount() == 2),
			"TDCLOneLinkServer restart still starts registered layers" );
		theServer.Kill();
		Check( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ),
			"TDCLOneLinkServer SetLink rejected after Kill" );
		Check( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ),
			"TDCLOneLinkServer AddCommLayer rejected after Kill" );
	}
}

// ======================================================================= //
// When we write programs that "learn", it turns out we do and they don't. //
// ======================================================================= //
