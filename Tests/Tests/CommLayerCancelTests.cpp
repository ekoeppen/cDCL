// ==============================
// File:			CommLayerCancelTests.cpp
// Project:			Desktop Connection Library
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
// The Original Code is CommLayerCancelTests.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#include <DCL/Headers/DCLDefinitions.h>

#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
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

#include "TTestApplication.h"
#include "TTestDockLink.h"

#include <doctest/doctest.h>

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <type_traits>

#if TARGET_OS_COMPAT_POSIX || TARGET_OS_WIN32
namespace
{
class TCancelProbeApp : public TTestApplication
{
public:
	TCancelProbeApp( void )
		: TTestApplication(),
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
		: TDCLServer( inApp ),
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
		: TCancelProbeApp(),
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

class THandshakeFailApp : public TCancelProbeApp
{
public:
	THandshakeFailApp( void )
		: TCancelProbeApp(),
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

void ClosePlatformSocket( dcl_socket_t inSock )
{
#if TARGET_OS_WIN32
	::closesocket( inSock );
#else
	::close( inSock );
#endif
}

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
		ClosePlatformSocket( clientSock );
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
		ClosePlatformSocket( clientSock );
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
		ClosePlatformSocket( badClient );
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
		ClosePlatformSocket( secondClient );
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

TEST_SUITE("CommLayerCancel")
{
	TEST_CASE("Socket cancel read")
	{
		TCancelProbeApp theApp;
		TDCLPlatformSocket theSocketLayer( 0 /* ephemeral */ );
		TCancelProbeServer theServer( &theApp, &theSocketLayer );
		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		dcl_socket_t clientSock = 0;
		const bool connected = ConnectLoopback( theSocketLayer, clientSock );
		if ( !connected )
		{
			theServer.Stop();
			theApp.WaitUntilDown();
			REQUIRE( connected );
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
		ClosePlatformSocket( clientSock );
		theServer.Stop();
		theApp.WaitUntilDown();

		CHECK( gotCancelled );
		CHECK( elapsed.count() < 5000 );
	}

	TEST_CASE("Server kill active link")
	{
		TCancelProbeApp theApp;
		auto socketLayer = std::make_unique<TDCLPlatformSocket>( 0 /* ephemeral */ );
		auto* socketPtr = socketLayer.get();
		TDCLSimpleServer theServer( (TDCLApplication*)&theApp, std::move( socketLayer ) );
		auto* linkPtr = theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );

		theApp.Reset();
		theServer.Start();
		theApp.WaitUntilListening();

		dcl_socket_t clientSock = 0;
		const bool connected = ConnectLoopback( *socketPtr, clientSock );
		if ( !connected )
		{
			theServer.Kill();
			REQUIRE( connected );
		}

		theApp.WaitUntilLinkConnected();
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

		auto start = std::chrono::steady_clock::now();
		theServer.Kill();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start );

		ClosePlatformSocket( clientSock );

		CHECK( theServer.GetState() == TDCLServer::kStopped );
		CHECK( !linkPtr->IsConnected() );
		CHECK( elapsed.count() < 5000 );
	}

	TEST_CASE("Server kill from Connected")
	{
		CHECK( RunKillFromConnectedCase<TDCLSimpleServer>() );
		CHECK( RunKillFromConnectedCase<TDCLOneLinkServer>() );
	}

	TEST_CASE("Server reconnect after handshake error")
	{
		CHECK( RunReconnectAfterHandshakeErrorCase<TDCLSimpleServer>( false ) );
		CHECK( RunReconnectAfterHandshakeErrorCase<TDCLSimpleServer>( true ) );
		CHECK( RunReconnectAfterHandshakeErrorCase<TDCLOneLinkServer>( false ) );
		CHECK( RunReconnectAfterHandshakeErrorCase<TDCLOneLinkServer>( true ) );
	}

#if TARGET_OS_COMPAT_POSIX
	TEST_CASE("WakeableWait cancel write")
	{
		int pipeFDs[2];
		REQUIRE( ::pipe( pipeFDs ) == 0 );
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

		CHECK( res == TDCLWakeableWait::EResult::kCancelled );
		CHECK( elapsed.count() < 5000 );
	}

	TEST_CASE("Serial cancel")
	{
		TCancelProbeApp theApp;
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		const bool ptyOK = ( masterFD >= 0 ) && ( ::grantpt( masterFD ) == 0 ) && ( ::unlockpt( masterFD ) == 0 );
		if ( !ptyOK )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			REQUIRE( ptyOK );
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			REQUIRE( slaveName != nullptr );
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
		bool gotCancelled = false;
		long long elapsedMs = 0;
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
			elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start ).count();
			canceller.join();

			thePipe->Disconnect();
			thePipe.reset();
			theServer.Stop();
			theApp.WaitUntilDown();
		}

		::close( masterFD );

		CHECK( gotCancelled );
		CHECK( elapsedMs < 5000 );
	}

	TEST_CASE("Server cancel MNP link")
	{
		TCancelProbeApp theApp;
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		const bool ptyOK = ( masterFD >= 0 ) && ( ::grantpt( masterFD ) == 0 ) && ( ::unlockpt( masterFD ) == 0 );
		if ( !ptyOK )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			REQUIRE( ptyOK );
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			REQUIRE( slaveName != nullptr );
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

		CHECK( linkDisconnectedInTime );
		CHECK( stillRunningAfterCancel );
		CHECK( downInTime );
		CHECK( noFooterWritten );
		CHECK( theServer.GetState() == TDCLServer::kStopped );
		CHECK( !linkPtr->IsConnected() );
		CHECK( elapsed.count() < 5000 );
	}

	TEST_CASE("Serial reset")
	{
		TCancelProbeApp theApp;
		int masterFD = ::posix_openpt( O_RDWR | O_NOCTTY );
		const bool ptyOK = ( masterFD >= 0 ) && ( ::grantpt( masterFD ) == 0 ) && ( ::unlockpt( masterFD ) == 0 );
		if ( !ptyOK )
		{
			if ( masterFD >= 0 )
			{
				::close( masterFD );
			}
			REQUIRE( ptyOK );
		}

		const char* slaveName = ::ptsname( masterFD );
		if ( !slaveName )
		{
			::close( masterFD );
			REQUIRE( slaveName != nullptr );
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

		CHECK( resetWhileSelectOK );
		CHECK( disconnectThenAcceptOK );
		CHECK( refuseThenAcceptOK );
		CHECK( failedOpenRecoveryOK );
	}
#endif
}
#endif
