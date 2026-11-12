// ==============================
// File: TPackageLoaderApp.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>
#include "TPackageLoaderApp.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

#include <K/Unicode/UUTF16CStr.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Link/TDCLLink.h>
#include <DCL/Link/TDCLLoadPackageLink.h>

typedef TDCLStdThreads TDCLPlatformThreads;

#if TARGET_OS_WIN32
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
typedef TDCLWin32Files TDCLPlatformFiles;
#else
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
typedef TDCLPOSIXFiles TDCLPlatformFiles;
#endif

TPackageLoaderApp::TPackageLoaderApp( bool inVerbose )
	:
		mVerbose( inVerbose ),
		mSuccess( false ),
		mDone( false )
{
}

void
TPackageLoaderApp::SetPackage( TDCLFSItemRef inPackageRef, std::string inPackageName )
{
	mPackageRef = inPackageRef;
	mPackageName = std::move( inPackageName );
}

std::string
TPackageLoaderApp::Timestamp( void )
{
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( now.time_since_epoch() ) % 1000;
	std::time_t t = std::chrono::system_clock::to_time_t( now );
	std::tm tm;
#if TARGET_OS_WIN32
	localtime_s( &tm, &t );
#else
	localtime_r( &t, &tm );
#endif
	char timeBuf[32];
	std::snprintf( timeBuf, sizeof(timeBuf), "[%02d:%02d:%02d.%03d] ",
		tm.tm_hour, tm.tm_min, tm.tm_sec, (int) ms.count() );
	return timeBuf;
}

void
TPackageLoaderApp::Log( const std::string& inMessage )
{
	if (mVerbose)
	{
		std::cout << Timestamp() << inMessage << std::endl;
	}
}

void
TPackageLoaderApp::WaitForCompletion( void )
{
	std::unique_lock<std::mutex> lock( mWaitMutex );
	mWaitCond.wait( lock, [this]() { return mDone; } );
}

void
TPackageLoaderApp::SignalCompletion( void )
{
	{
		std::lock_guard<std::mutex> lock( mWaitMutex );
		mDone = true;
	}
	mWaitCond.notify_all();
}

void
TPackageLoaderApp::WaitConnection( TDCLServer* /* inServer */, TDCLCommLayer* /* inLayer */ )
{
	Log( "Waiting for connection..." );
}

void
TPackageLoaderApp::Connected( TDCLLink* /* inLink */, TDCLCommLayer* /* inLayer */ )
{
	Log( "Transport connection established." );
	std::cout << "Connection established with Newton." << std::endl;
}

void
TPackageLoaderApp::ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName )
{
	char nameBuf[512] = {0};
	if (inName)
	{
		UUTF16CStr::ToISO88591( inName, (KUInt8*) nameBuf, sizeof(nameBuf) - 1 );
	}

	Log( std::string( "Identified Newton device: \"" ) + nameBuf + "\"" );
	std::cout << "Connected to Newton: \"" << nameBuf << "\"" << std::endl;
	std::cout << "Initiating installation of '" << mPackageName << "'..." << std::endl;

	TDCLLoadPackageLink* loadPkgLink = static_cast<TDCLLoadPackageLink*>( inLink );
	if (!loadPkgLink->InstallPackage( mPackageRef ))
	{
		std::cerr << "Error: Failed to post package installation request." << std::endl;
		mSuccess = false;
		mErrorMessage = "Failed to post package installation request.";
		inLink->Disconnect();
		SignalCompletion();
	}
}

void
TPackageLoaderApp::Disconnecting( TDCLLink* /* inLink */ )
{
	Log( "Disconnecting link..." );
}

void
TPackageLoaderApp::Disconnected( TDCLLink* /* inLink */ )
{
	Log( "Link disconnected." );
	std::cout << "Disconnected from Newton." << std::endl;
	SignalCompletion();
}

void
TPackageLoaderApp::ServerIsDown( TDCLServer* /* inServer */ )
{
	Log( "Server is down." );
	SignalCompletion();
}

void
TPackageLoaderApp::Cancel( TDCLLink* /* inLink */ )
{
	Log( "Session was cancelled by Newton." );
	std::cout << "Operation cancelled by Newton." << std::endl;
	mSuccess = false;
	mErrorMessage = "Operation cancelled by Newton.";
	SignalCompletion();
}

void
TPackageLoaderApp::Timeout( TDCLLink* /* inLink */ )
{
	Log( "Session timed out." );
	std::cerr << "Connection timed out." << std::endl;
	mSuccess = false;
	mErrorMessage = "Connection timed out.";
	SignalCompletion();
}

void
TPackageLoaderApp::AppCommandBeingProcessed( TDCLLink* /* inLink */, TDCLAppCommand* /* inCommand */ )
{
	Log( "Processing app command (installing package)..." );
}

void
TPackageLoaderApp::AppCommandProcessed(
		TDCLLink* inLink,
		TDCLAppCommand* /* inCommand */,
		TDCLException* inException )
{
	if (inException)
	{
		mSuccess = false;
		mErrorMessage = inException->GetExceptionName();
		Log( std::string( "App command failed: " ) + mErrorMessage );
		std::cerr << "Package installation failed: " << mErrorMessage << std::endl;
	}
	else
	{
		mSuccess = true;
		Log( "App command completed successfully." );
		std::cout << "Package installed successfully!" << std::endl;
	}

	inLink->Disconnect();
}

void
TPackageLoaderApp::DockCommandBeingProcessed( TDCLLink* /* inLink */, TDCLDockCommand* /* inCommand */, TDCLLinkEngine* /* inEngine */ )
{
	Log( "Dock command being processed..." );
}

void
TPackageLoaderApp::DockCommandProcessed( TDCLLink* /* inLink */, TDCLLinkEngine* /* inEngine */, TDCLException* inException )
{
	if (inException)
	{
		Log( std::string( "Dock command exception: " ) + inException->GetExceptionName() );
	}
}

void
TPackageLoaderApp::HandleServerException( TDCLServer* /* inServer */, TDCLException* inException )
{
	mSuccess = false;
	if (inException)
	{
		mErrorMessage = inException->GetExceptionName();
		Log( std::string( "Server exception: " ) + mErrorMessage );
		std::cerr << "Server error: " << mErrorMessage << std::endl;
	}
	SignalCompletion();
}

void
TPackageLoaderApp::HandleCommLayerException( TDCLCommLayer* /* inCommLayer */, TDCLException* inException )
{
	mSuccess = false;
	if (inException)
	{
		mErrorMessage = inException->GetExceptionName();
		Log( std::string( "Communication layer exception: " ) + mErrorMessage );
		std::cerr << "Communication error: " << mErrorMessage << std::endl;
	}
	SignalCompletion();
}

void
TPackageLoaderApp::HandleLinkException( TDCLLink* /* inLink */, TDCLException* inException )
{
	mSuccess = false;
	if (inException)
	{
		mErrorMessage = inException->GetExceptionName();
		Log( std::string( "Link exception: " ) + mErrorMessage );
		std::cerr << "Link error: " << mErrorMessage << std::endl;
	}
	SignalCompletion();
}

void
TPackageLoaderApp::HandleException( TDCLException* inException )
{
	mSuccess = false;
	if (inException)
	{
		mErrorMessage = inException->GetExceptionName();
		Log( std::string( "Unhandled exception: " ) + mErrorMessage );
		std::cerr << "General error: " << mErrorMessage << std::endl;
	}
	SignalCompletion();
}

IDCLFiles*
TPackageLoaderApp::CreateFilesIntf( void )
{
	return new TDCLPlatformFiles();
}

IDCLThreads*
TPackageLoaderApp::CreateThreadsIntf( void )
{
	return new TDCLPlatformThreads();
}
