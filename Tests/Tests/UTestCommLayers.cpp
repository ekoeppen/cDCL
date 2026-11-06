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
#include <thread>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
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

#if TARGET_OS_OPENSTEP && !TARGET_OS_OPENSTEP_AMD64 && !TARGET_OS_OPENSTEP_ARM64
#include <DCL/Communication_Layers/MacOS_X/TDCLXADSP.h>
#include <DCL/Interfaces/POSIX/TDCLPThreads.h>
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

// ======================================================================= //
// When we write programs that "learn", it turns out we do and they don't. //
// ======================================================================= //
