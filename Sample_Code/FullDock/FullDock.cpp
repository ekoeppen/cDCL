// ==============================
// File:			FullDock.cpp
// Project:			Desktop Connection Library - FullDock Sample
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
// The Original Code is FullDock.cpp.
//
// ***** END LICENSE BLOCK *****

///
/// \file
/// Example console application demonstrating TDCLFullDockLink over a POSIX
/// serial port (TDCLFDSerialPort).
///

// POSIX
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

// ISO C++
#include <atomic>
#include <cerrno>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Server/TDCLSimpleServer.h>

namespace
{

int gSignalPipe[2] = { -1, -1 };
std::atomic<bool> gStopRequested{ false };

void
HandleSignal( int /* inSignal */ )
{
	gStopRequested.store( true, std::memory_order_relaxed );
	if ( gSignalPipe[1] >= 0 )
	{
		const char byte = 1;
		(void)::write( gSignalPipe[1], &byte, 1 );
	}
}

void
PrintUsage( const char* inProgName )
{
	(void)::fprintf(
		stderr,
		"Usage: %s <serial-device> [options]\n"
		"\n"
		"Options:\n"
		"  --baud <rate>       Serial baud rate (default: 38400)\n"
		"  --mnp               Use MNP framing (default)\n"
		"  --plain             Use plain serial framing\n"
		"  --install <pkg>     Install package automatically once connected\n"
		"  --help              Show this help\n",
		inProgName );
}

void
PrintCommands( void )
{
	(void)::printf(
		"Commands:\n"
		"  install <path>      Install a .pkg file on the connected Newton\n"
		"  disconnect          Disconnect the current Newton session\n"
		"  quit                Stop the server and exit\n"
		"  help                Show this list\n" );
	(void)::fflush( stdout );
}

bool
FileExists( const std::string& inPath )
{
	struct stat st;
	return ( ::stat( inPath.c_str(), &st ) == 0 ) && S_ISREG( st.st_mode );
}

class TFullDockApp : public TDCLLogApplication
{
public:
	TFullDockApp( const std::string& inAutoInstallPkg )
		:
			TDCLLogApplication( stdout ),
			mAutoInstallPkg( inAutoInstallPkg ),
			mAutoInstalled( false ),
			mServerDown( false ),
			mLink( nil ),
			mPOSIXFiles( nil )
	{
	}

	void SetLink( TDCLFullDockLink* inLink )
	{
		mLink = inLink;
	}

	TDCLPOSIXFiles* GetPOSIXFiles( void )
	{
		(void)GetFilesIntf();
		return mPOSIXFiles;
	}

	void WaitUntilServerIsDown( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [this] { return mServerDown; } );
	}

	virtual void ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName ) override
	{
		char nameBuf[510];
		UUTF16CStr::ToISO88591( inName, reinterpret_cast<KUInt8*>( nameBuf ), sizeof( nameBuf ) );
		(void)::fprintf( mLogFile, "[%p] Connected to %s's Newton\n", static_cast<void*>( inLink ), nameBuf );
		(void)::fflush( mLogFile );

		if ( !mAutoInstallPkg.empty() && !mAutoInstalled && ( mLink != nil ) )
		{
			mAutoInstalled = true;
			TDCLFSItemRef pkgRef( new TDCLPOSIXFile( GetPOSIXFiles(), mAutoInstallPkg.c_str() ) );
			(void)::fprintf( mLogFile, "Queueing package install: %s\n", mAutoInstallPkg.c_str() );
			(void)::fflush( mLogFile );
			if ( !mLink->InstallPackage( pkgRef ) )
			{
				(void)::fprintf( mLogFile, "Failed to queue package install\n" );
				(void)::fflush( mLogFile );
			}
		}
	}

	virtual void ServerIsDown( TDCLServer* inServer ) override
	{
		TDCLLogApplication::ServerIsDown( inServer );
		(void)::fflush( mLogFile );
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mServerDown = true;
		}
		mCV.notify_all();
	}

	virtual void HandleServerException( TDCLServer* inServer, TDCLException* inException ) override
	{
		TDCLLogApplication::HandleServerException( inServer, inException );
		(void)::fflush( mLogFile );
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mServerDown = true;
		}
		mCV.notify_all();
	}

protected:
	virtual IDCLFiles* CreateFilesIntf( void ) override
	{
		mPOSIXFiles = new TDCLPOSIXFiles();
		return mPOSIXFiles;
	}

	virtual IDCLThreads* CreateThreadsIntf( void ) override
	{
		return new TDCLStdThreads();
	}

private:
	std::string mAutoInstallPkg;
	bool mAutoInstalled;
	bool mServerDown;
	TDCLFullDockLink* mLink;
	TDCLPOSIXFiles* mPOSIXFiles;
	std::mutex mMutex;
	std::condition_variable mCV;
};

} // namespace

int
main( int argc, char** argv )
{
	std::string devicePath;
	long baudRate = 38400;
	KUInt32 serialMode = TDCLFDSerialPort::kModeMNP;
	std::string autoInstallPkg;

	for ( int i = 1; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( ( arg == "--help" ) || ( arg == "-h" ) )
		{
			PrintUsage( argv[0] );
			return 0;
		}
		else if ( arg == "--baud" )
		{
			if ( i + 1 >= argc )
			{
				(void)::fprintf( stderr, "Missing value for --baud\n" );
				return 1;
			}
			baudRate = ::atol( argv[++i] );
			if ( baudRate <= 0 )
			{
				(void)::fprintf( stderr, "Invalid baud rate\n" );
				return 1;
			}
		}
		else if ( arg == "--mnp" )
		{
			serialMode = TDCLFDSerialPort::kModeMNP;
		}
		else if ( arg == "--plain" )
		{
			serialMode = TDCLFDSerialPort::kModePlain;
		}
		else if ( arg == "--install" )
		{
			if ( i + 1 >= argc )
			{
				(void)::fprintf( stderr, "Missing value for --install\n" );
				return 1;
			}
			autoInstallPkg = argv[++i];
		}
		else if ( arg[0] == '-' )
		{
			(void)::fprintf( stderr, "Unknown option: %s\n", arg.c_str() );
			PrintUsage( argv[0] );
			return 1;
		}
		else if ( devicePath.empty() )
		{
			devicePath = arg;
		}
		else
		{
			(void)::fprintf( stderr, "Unexpected argument: %s\n", arg.c_str() );
			PrintUsage( argv[0] );
			return 1;
		}
	}

	if ( devicePath.empty() )
	{
		PrintUsage( argv[0] );
		return 1;
	}

	if ( !autoInstallPkg.empty() && !FileExists( autoInstallPkg ) )
	{
		(void)::fprintf( stderr, "Package file not found: %s\n", autoInstallPkg.c_str() );
		return 1;
	}

	if ( ::pipe( gSignalPipe ) != 0 )
	{
		::perror( "pipe" );
		return 1;
	}

	struct sigaction sa;
	::memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = HandleSignal;
	::sigaction( SIGINT, &sa, nullptr );
	::sigaction( SIGTERM, &sa, nullptr );

	int exitCode = 0;

	try
	{
		TFullDockApp theApp( autoInstallPkg );
		TDCLFDSerialPort theSerialPort(
			theApp.GetThreadsIntf(),
			devicePath.c_str(),
			baudRate,
			serialMode );

		TDCLSimpleServer theServer( &theApp, &theSerialPort );
		TDCLFullDockLink theLink(
			&theApp,
			TDCLFullDockLink::kFileBrowsingSet | TDCLFullDockLink::kInstallPackageSet,
			TDCLDockLink::kInstallIcon );
		theApp.SetLink( &theLink );
		theServer.SetLink( &theLink );

		(void)::printf( "Starting FullDock on %s (%ld baud)...\n", devicePath.c_str(), baudRate );
		PrintCommands();

		theServer.Start();

		const bool interactiveStdin = ( ::isatty( STDIN_FILENO ) != 0 );
		bool running = true;
		while ( running && !gStopRequested.load( std::memory_order_relaxed ) )
		{
			struct pollfd fds[2];
			fds[0].fd = gSignalPipe[0];
			fds[0].events = POLLIN;
			fds[0].revents = 0;
			nfds_t nfds = 1;

			if ( interactiveStdin )
			{
				fds[1].fd = STDIN_FILENO;
				fds[1].events = POLLIN;
				fds[1].revents = 0;
				nfds = 2;
			}

			int ready = ::poll( fds, nfds, -1 );
			if ( ready < 0 )
			{
				if ( errno == EINTR )
				{
					continue;
				}
				break;
			}

			if ( fds[0].revents & POLLIN )
			{
				break;
			}

			if ( interactiveStdin && ( fds[1].revents & ( POLLIN | POLLHUP | POLLERR ) ) )
			{
				std::string line;
				if ( !std::getline( std::cin, line ) )
				{
					break;
				}

				if ( ( line == "quit" ) || ( line == "exit" ) )
				{
					running = false;
				}
				else if ( line == "help" )
				{
					PrintCommands();
				}
				else if ( line == "disconnect" )
				{
					if ( !theLink.Disconnect() )
					{
						(void)::printf( "Not connected.\n" );
					}
				}
				else if ( line.rfind( "install ", 0 ) == 0 )
				{
					std::string pkgPath = line.substr( 8 );
					while ( !pkgPath.empty() && ( pkgPath.front() == ' ' || pkgPath.front() == '\t' ) )
					{
						pkgPath.erase( pkgPath.begin() );
					}
					if ( !FileExists( pkgPath ) )
					{
						(void)::printf( "File not found: %s\n", pkgPath.c_str() );
					}
					else
					{
						TDCLFSItemRef pkgRef(
							new TDCLPOSIXFile( theApp.GetPOSIXFiles(), pkgPath.c_str() ) );
						if ( !theLink.InstallPackage( pkgRef ) )
						{
							(void)::printf( "Cannot install package: not connected.\n" );
						}
					}
				}
				else if ( !line.empty() )
				{
					(void)::printf( "Unknown command: %s\n", line.c_str() );
					PrintCommands();
				}
			}
		}

		(void)::printf( "\nStopping server...\n" );
		theServer.Stop();
		theApp.WaitUntilServerIsDown();
	}
	catch ( TDCLException& ex )
	{
		(void)::fprintf(
			stderr,
			"DCL exception: %s (code: %d, error: %d)\n",
			ex.GetExceptionName(),
			static_cast<int>( ex.GetExCode() ),
			static_cast<int>( ex.GetError() ) );
		exitCode = 1;
	}
	catch ( std::exception& ex )
	{
		(void)::fprintf( stderr, "Exception: %s\n", ex.what() );
		exitCode = 1;
	}

	::close( gSignalPipe[0] );
	::close( gSignalPipe[1] );

	return exitCode;
}
