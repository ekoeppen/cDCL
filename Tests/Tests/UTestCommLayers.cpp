// ==============================
// Fichier:			UTestCommLayers.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// Modifié par:		Eckhart Koeppen (eck@40hz.org)
//
// Créé le:			17/4/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot.
//					© 2026 by Eckhart Koeppen.
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

// //  * StartAndStopTwiceFDSerialPort( const char* )
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
	(void)theServer.Wait();

	(void)::fprintf( inApplication->GetLogFile(), "Second test\n" );

	inApplication->ResetTest();

	theServer.Start();

	inApplication->WaitUntilServerIsDown();
	(void)theServer.Wait();

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

