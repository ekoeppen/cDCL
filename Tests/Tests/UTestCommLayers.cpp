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
#include <mutex>
#include <thread>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Server/TDCLOneLinkServer.h>
#include <DCL/Server/TDCLSimpleServer.h>

#if TARGET_OS_WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
#include <DCL/Interfaces/Win32/TDCLWin32Threads.h>
typedef TDCLWin32Socket TDCLPlatformSocket;
typedef TDCLWin32Threads TDCLPlatformThreads;
#else
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
#include <DCL/Interfaces/POSIX/TDCLPThreads.h>
typedef TDCLBSDSocket TDCLPlatformSocket;
typedef TDCLPThreads TDCLPlatformThreads;
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

	virtual TDCLPipe* Accept( void ) { return nil; }
	virtual void Refuse( void ) {}
	virtual void SetTimeout( long inTimeout ) { mTimeout = inTimeout; }
	virtual long GetTimeout( void ) { return mTimeout; }
	virtual void Disconnected( TDCLPipe* /* inPipe */ ) {}

	int GetStartCount( void ) const { return mStartCount; }
	int GetStopCount( void ) const { return mStopCount; }

protected:
	virtual void DoStartListening( void )
	{
		++mStartCount;
	}

	virtual void DoStopListening( void )
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
	TConfigTestApplication( IDCLThreads* inThreadsIntf )
		: TTestApplication( inThreadsIntf ),
		  mWaitingCount( 0 ),
		  mDownCount( 0 )
	{
	}

	virtual void WaitConnection( TDCLServer* /* inServer */, TDCLCommLayer* /* inLayer */ )
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mWaitingCount;
		}
		mCV.notify_all();
	}

	virtual void ServerIsDown( TDCLServer* /* inServer */ )
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
	IDCLThreads* theThreadsIntf = new TDCLPlatformThreads();
	TConfigTestApplication theApp( theThreadsIntf );

	TTestDockLink theLink1( &theApp );
	TTestDockLink theLink2( &theApp );

	// 1. TDCLSimpleServer: SetLink allowed before Start(), rejected while running,
	//    after Stop(), and across a subsequent Start()/Stop() cycle.
	{
		TConfigTestCommLayer theLayer;
		TDCLSimpleServer theServer( &theApp, &theLayer );

		theServer.SetLink( &theLink1 );
		theServer.SetLink( &theLink2 );
		Check( theLayer.GetStartCount() == 0, "TDCLSimpleServer SetLink pre-Start" );

		theServer.Start();
		theApp.WaitForListening();
		Check( ExpectBadState( [&]() { theServer.SetLink( &theLink1 ); } ),
			"TDCLSimpleServer SetLink rejected while running" );

		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		while ( theServer.GetState() != TDCLServer::kStopped )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		Check( ExpectBadState( [&]() { theServer.SetLink( &theLink1 ); } ),
			"TDCLSimpleServer SetLink rejected after Stop" );

		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		theServer.Start();
		theApp.WaitForListening();
		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		while ( theServer.GetState() != TDCLServer::kStopped )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		Check( ExpectBadState( [&]() { theServer.SetLink( &theLink1 ); } ),
			"TDCLSimpleServer SetLink rejected after second Stop" );
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
	}

	// 2. TDCLOneLinkServer: SetLink & AddCommLayer pre-Start(), nil rejected,
	//    and both rejected while running and after Stop().
	{
		TConfigTestApplication theOneApp( new TDCLPlatformThreads() );
		TTestDockLink theOneLink1( &theOneApp );
		TTestDockLink theOneLink2( &theOneApp );
		TConfigTestCommLayer theLayer1;
		TConfigTestCommLayer theLayer2;
		TConfigTestCommLayer theLayer3;
		TDCLOneLinkServer theServer( &theOneApp );

		Check( ExpectBadParam( [&]() { theServer.AddCommLayer( nil ); } ),
			"TDCLOneLinkServer AddCommLayer(nil) rejected" );

		theServer.SetLink( &theOneLink1 );
		theServer.SetLink( &theOneLink2 );
		theServer.AddCommLayer( &theLayer1 );
		theServer.AddCommLayer( &theLayer2 );
		Check( (theLayer1.GetStartCount() == 0) && (theLayer2.GetStartCount() == 0),
			"TDCLOneLinkServer AddCommLayer does not start listening pre-Start" );

		theServer.Start();
		theOneApp.WaitForListening( 2 );
		Check( (theLayer1.GetStartCount() == 1) && (theLayer2.GetStartCount() == 1),
			"TDCLOneLinkServer Start starts all registered layers" );

		Check( ExpectBadState( [&]() { theServer.SetLink( &theOneLink1 ); } ),
			"TDCLOneLinkServer SetLink rejected while running" );
		Check( ExpectBadState( [&]() { theServer.AddCommLayer( &theLayer3 ); } ),
			"TDCLOneLinkServer AddCommLayer rejected while running" );

		theServer.Stop();
		theOneApp.WaitUntilServerIsDown();
		while ( theServer.GetState() != TDCLServer::kStopped )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		Check( ExpectBadState( [&]() { theServer.SetLink( &theOneLink1 ); } ),
			"TDCLOneLinkServer SetLink rejected after Stop" );
		Check( ExpectBadState( [&]() { theServer.AddCommLayer( &theLayer3 ); } ),
			"TDCLOneLinkServer AddCommLayer rejected after Stop" );

		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		theServer.Start();
		theOneApp.WaitForListening( 2 );
		Check( (theLayer1.GetStartCount() == 2) && (theLayer2.GetStartCount() == 2),
			"TDCLOneLinkServer restart still starts registered layers" );
		theServer.Stop();
		theOneApp.WaitUntilServerIsDown();
		while ( theServer.GetState() != TDCLServer::kStopped )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		Check( ExpectBadState( [&]() { theServer.SetLink( &theOneLink1 ); } ),
			"TDCLOneLinkServer SetLink rejected after second Stop" );
		Check( ExpectBadState( [&]() { theServer.AddCommLayer( &theLayer3 ); } ),
			"TDCLOneLinkServer AddCommLayer rejected after second Stop" );
		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
	}
}

// ======================================================================= //
// When we write programs that "learn", it turns out we do and they don't. //
// ======================================================================= //
