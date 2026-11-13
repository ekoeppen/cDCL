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
/// Example console application demonstrating TDCLFullDockLink over TCP/IP
/// (TDCLBSDSocket) or a POSIX serial port (TDCLFDSerialPort).
///

// POSIX
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

// ISO C++
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <charconv>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Package/TDCLPackage.h>
#include <DCL/Server/TDCLSimpleServer.h>

namespace
{

enum class TransportMode
{
	Unspecified,
	Listen,
	Serial
};

int gSignalPipe[2] = { -1, -1 };
std::atomic<bool> gStopRequested{ false };

void
NotifyMainLoop( char inByte )
{
	if (gSignalPipe[1] >= 0)
	{
		ssize_t unused = ::write( gSignalPipe[1], &inByte, 1 );
		(void)unused;
	}
}

void
HandleSignal( int /* inSignal */ )
{
	gStopRequested.store( true, std::memory_order_relaxed );
	NotifyMainLoop( 1 );
}

void
PrintUsage( const char* inProgName )
{
	std::cout << "Usage: " << inProgName << " [options] [package.pkg]\n\n"
			  << "Connect to a Newton device using TDCLFullDockLink over TCP/IP or Serial.\n"
			  << "If <package.pkg> is provided, it is installed automatically on connect.\n\n"
			  << "Connection Modes (choose one; default is -l):\n"
			  << "  -l            Listen on TCP port for incoming connection\n"
			  << "  -s <device>   Connect via serial port with MNP framing\n"
			  << "                (e.g., /dev/cu.usbserial-10, /dev/ttyUSB0)\n\n"
			  << "TCP Options:\n"
			  << "  -p <port>     TCP port for -l (default: " << TDCLBSDSocket::kIANAAssignedPort << ")\n\n"
			  << "Serial Options:\n"
			  << "  -b <rate>     Serial baud rate (default: 38400)\n"
			  << "                Supported: 2400, 4800, 9600, 19200, 38400,\n"
			  << "                           57600, 115200, 230400\n\n"
			  << "General Options:\n"
			  << "  -t <seconds>  Connection / operation timeout in seconds (default: 30)\n"
			  << "  -v            Enable verbose protocol logging\n"
			  << "  -h            Show this help text\n";
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
ParseLongInRange( const char* inStr, long inMin, long inMax, long& outValue )
{
	if (!inStr || inStr[0] == '\0')
	{
		return false;
	}
	const char* end = inStr + std::strlen( inStr );
	long val = 0;
	auto [ptr, ec] = std::from_chars( inStr, end, val );
	if (ec != std::errc() || ptr != end || val < inMin || val > inMax)
	{
		return false;
	}
	outValue = val;
	return true;
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
	TFullDockApp( const std::string& inAutoInstallPkg, bool inVerbose )
		:
			TDCLLogApplication( stdout ),
			mAutoInstallPkg( inAutoInstallPkg ),
			mAutoInstalled( false ),
			mVerbose( inVerbose ),
			mServerDown( false ),
			mPOSIXFiles( nullptr )
	{
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

	virtual void WaitConnection( TDCLServer* inServer, TDCLCommLayer* inCommLayer ) override
	{
		if (mVerbose)
		{
			TDCLLogApplication::WaitConnection( inServer, inCommLayer );
			(void)::fflush( mLogFile );
		}
	}

	virtual void Connected( TDCLLink* inLink, TDCLCommLayer* inCommLayer ) override
	{
		if (mVerbose)
		{
			TDCLLogApplication::Connected( inLink, inCommLayer );
			(void)::fflush( mLogFile );
		}
	}

	virtual void ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName ) override
	{
		char nameBuf[510];
		UUTF16CStr::ToUTF8( inName, reinterpret_cast<KUInt8*>( nameBuf ), sizeof( nameBuf ) );
		(void)::fprintf( mLogFile, "[%p] Connected to %s's Newton\n", static_cast<void*>( inLink ), nameBuf );
		(void)::fflush( mLogFile );

		TDCLFullDockLink* fullDockLink = static_cast<TDCLFullDockLink*>( inLink );
		if ( !mAutoInstallPkg.empty() && !mAutoInstalled && ( fullDockLink != nullptr ) )
		{
			mAutoInstalled = true;
			TDCLFSItemRef pkgRef( new TDCLPOSIXFile( GetPOSIXFiles(), mAutoInstallPkg.c_str() ) );
			(void)::fprintf( mLogFile, "Queueing package install: %s\n", mAutoInstallPkg.c_str() );
			(void)::fflush( mLogFile );
			if ( !fullDockLink->InstallPackage( pkgRef ) )
			{
				(void)::fprintf( mLogFile, "Failed to queue package install\n" );
				(void)::fflush( mLogFile );
			}
		}
	}

	virtual void AppCommandProcessed(
			TDCLLink* inLink,
			TDCLAppCommand* inCommand,
			TDCLException* inException ) override
	{
		if (inException)
		{
			(void)::fprintf(
				mLogFile,
				"[%p] App command [%p] failed: %s (code: %d, error: %d)\n",
				static_cast<void*>( inLink ),
				static_cast<void*>( inCommand ),
				inException->GetExceptionName(),
				static_cast<int>( inException->GetExCode() ),
				static_cast<int>( inException->GetError() ) );
			(void)::fflush( mLogFile );
		}
		else if (mVerbose)
		{
			TDCLLogApplication::AppCommandProcessed( inLink, inCommand, inException );
			(void)::fflush( mLogFile );
		}
	}

	virtual void DockCommandProcessed(
			TDCLLink* inLink,
			TDCLLinkEngine* inEngine,
			TDCLException* inException ) override
	{
		if (inException)
		{
			(void)::fprintf(
				mLogFile,
				"[%p] Dock command (engine [%p]) failed: %s (code: %d, error: %d)\n",
				static_cast<void*>( inLink ),
				static_cast<void*>( inEngine ),
				inException->GetExceptionName(),
				static_cast<int>( inException->GetExCode() ),
				static_cast<int>( inException->GetError() ) );
		}
		else
		{
			(void)::fprintf(
				mLogFile,
				"[%p] Dock command (engine [%p]) succeeded\n",
				static_cast<void*>( inLink ),
				static_cast<void*>( inEngine ) );
		}
		(void)::fflush( mLogFile );
	}

	virtual void Disconnecting( TDCLLink* inLink ) override
	{
		if (mVerbose)
		{
			TDCLLogApplication::Disconnecting( inLink );
			(void)::fflush( mLogFile );
		}
	}

	virtual void Disconnected( TDCLLink* inLink ) override
	{
		TDCLLogApplication::Disconnected( inLink );
		(void)::fflush( mLogFile );
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
		NotifyMainLoop( 2 );
	}

	virtual void HandleServerException( TDCLServer* inServer, TDCLException* inException ) override
	{
		if (inException)
		{
			(void)::fprintf(
				mLogFile,
				"[%p] Server exception: %s (code: %d, error: %d)\n",
				static_cast<void*>( inServer ),
				inException->GetExceptionName(),
				static_cast<int>( inException->GetExCode() ),
				static_cast<int>( inException->GetError() ) );
		}
		else
		{
			TDCLLogApplication::HandleServerException( inServer, inException );
		}
		(void)::fflush( mLogFile );
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mServerDown = true;
		}
		mCV.notify_all();
		NotifyMainLoop( 2 );
	}

	virtual void HandleCommLayerException( TDCLCommLayer* inCommLayer, TDCLException* inException ) override
	{
		if (inException)
		{
			(void)::fprintf(
				mLogFile,
				"[%p] Comm layer exception: %s (code: %d, error: %d)\n",
				static_cast<void*>( inCommLayer ),
				inException->GetExceptionName(),
				static_cast<int>( inException->GetExCode() ),
				static_cast<int>( inException->GetError() ) );
			(void)::fflush( mLogFile );
		}
	}

	virtual void HandleLinkException( TDCLLink* inLink, TDCLException* inException ) override
	{
		if (inException)
		{
			(void)::fprintf(
				mLogFile,
				"[%p] Link exception: %s (code: %d, error: %d)\n",
				static_cast<void*>( inLink ),
				inException->GetExceptionName(),
				static_cast<int>( inException->GetExCode() ),
				static_cast<int>( inException->GetError() ) );
			(void)::fflush( mLogFile );
		}
	}

protected:
	virtual IDCLFiles* CreateFilesIntf( void ) override
	{
		mPOSIXFiles = new TDCLPOSIXFiles();
		return mPOSIXFiles;
	}

private:
	std::string mAutoInstallPkg;
	bool mAutoInstalled;
	bool mVerbose;
	bool mServerDown;
	TDCLPOSIXFiles* mPOSIXFiles;
	std::mutex mMutex;
	std::condition_variable mCV;
};

} // namespace

int
main( int argc, char** argv )
{
	const char* progName = argv[0];
	TransportMode mode = TransportMode::Unspecified;
	unsigned short tcpPort = TDCLBSDSocket::kIANAAssignedPort;
	std::string serialDevice;
	long baudRate = 38400;
	long timeout = 30;
	bool verbose = false;
	std::string autoInstallPkg;

	int opt;
	while ((opt = ::getopt( argc, argv, "lp:s:b:t:vh" )) != -1)
	{
		switch (opt)
		{
			case 'l':
				if (mode != TransportMode::Unspecified && mode != TransportMode::Listen)
				{
					std::cerr << "Error: Options -l and -s are mutually exclusive." << std::endl;
					return 1;
				}
				mode = TransportMode::Listen;
				break;

			case 'p':
			{
				long portVal = 0;
				if (!ParseLongInRange( optarg, 1, 65535, portVal ))
				{
					std::cerr << "Error: Invalid TCP port: " << optarg << std::endl;
					return 1;
				}
				tcpPort = static_cast<unsigned short>( portVal );
				break;
			}

			case 's':
				if (mode != TransportMode::Unspecified && mode != TransportMode::Serial)
				{
					std::cerr << "Error: Options -l and -s are mutually exclusive." << std::endl;
					return 1;
				}
				mode = TransportMode::Serial;
				serialDevice = optarg;
				break;

			case 'b':
			{
				if (!ParseLongInRange( optarg, 1, std::numeric_limits<long>::max(), baudRate ))
				{
					std::cerr << "Error: Invalid baud rate: " << optarg << std::endl;
					return 1;
				}
				const std::vector<long> validBauds = { 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400 };
				if (std::find( validBauds.begin(), validBauds.end(), baudRate ) == validBauds.end())
				{
					std::cerr << "Error: Unsupported baud rate: " << baudRate << std::endl;
					return 1;
				}
				break;
			}

			case 't':
				if (!ParseLongInRange( optarg, 1, std::numeric_limits<long>::max(), timeout ))
				{
					std::cerr << "Error: Invalid timeout value: " << optarg << std::endl;
					return 1;
				}
				break;

			case 'v':
				verbose = true;
				break;

			case 'h':
				PrintUsage( progName );
				return 0;

			default:
				PrintUsage( progName );
				return 1;
		}
	}

	if (argc - optind == 1)
	{
		autoInstallPkg = argv[optind];
	}
	else if (argc - optind > 1)
	{
		std::cerr << "Error: Only one package file may be specified." << std::endl;
		return 1;
	}

	if (mode == TransportMode::Unspecified)
	{
		mode = TransportMode::Listen;
	}

	if (!autoInstallPkg.empty() && !FileExists( autoInstallPkg ))
	{
		std::cerr << "Error: Package file not found: " << autoInstallPkg << std::endl;
		return 1;
	}

	if (::pipe( gSignalPipe ) != 0)
	{
		::perror( "pipe" );
		return 1;
	}
	int pipeFlags = ::fcntl( gSignalPipe[1], F_GETFL, 0 );
	if (pipeFlags >= 0)
	{
		(void)::fcntl( gSignalPipe[1], F_SETFL, pipeFlags | O_NONBLOCK );
	}

	struct sigaction sa;
	::memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = HandleSignal;
	sigemptyset( &sa.sa_mask );
	::sigaction( SIGINT, &sa, nullptr );
	::sigaction( SIGTERM, &sa, nullptr );

	int exitCode = 0;

	try
	{
		TFullDockApp theApp( autoInstallPkg, verbose );

		if (!autoInstallPkg.empty())
		{
			TDCLFile* pkgFile = new TDCLPOSIXFile( theApp.GetPOSIXFiles(), autoInstallPkg.c_str() );
			TDCLFSItemRef pkgRef( pkgFile );
			TDCLPackage thePackage( pkgRef );
			KUInt8 nameBuf[256] = { 0 };
			UUTF16CStr::ToUTF8( thePackage.GetPackageName(), nameBuf, sizeof( nameBuf ) );
			std::cout << "Package: \"" << reinterpret_cast<const char*>( nameBuf ) << "\" ("
					  << pkgFile->GetLength() << " bytes, "
					  << thePackage.GetNumParts() << " part"
					  << (thePackage.GetNumParts() == 1 ? "" : "s") << ")" << std::endl;
		}

		std::unique_ptr<TDCLCommLayer> theCommLayer;
		if (mode == TransportMode::Listen)
		{
			if (!TDCLBSDSocket::IsAvailable())
			{
				std::cerr << "Error: BSD sockets are not available on this system." << std::endl;
				::close( gSignalPipe[0] );
				::close( gSignalPipe[1] );
				return 1;
			}
			theCommLayer = std::make_unique<TDCLBSDSocket>(
				tcpPort,
				TDCLBSDSocket::kDefaultPipeCount,
				timeout );
			std::cout << "Listening for Newton connection on TCP port " << tcpPort << "..." << std::endl;
		}
		else
		{
			auto serialPort = std::make_unique<TDCLFDSerialPort>(
				serialDevice.c_str(),
				baudRate,
				static_cast<KUInt32>( TDCLFDSerialPort::kModeMNP ) );
			serialPort->SetTimeout( timeout );
			theCommLayer = std::move( serialPort );
			std::cout << "Waiting for Newton connection on " << serialDevice
					  << " (" << baudRate << " baud, MNP)..." << std::endl;
		}

		auto theLink = std::make_unique<TDCLFullDockLink>(
			&theApp,
			TDCLFullDockLink::kFileBrowsingSet | TDCLFullDockLink::kInstallPackageSet,
			TDCLDockLink::kInstallIcon );
		TDCLFullDockLink* linkPtr = theLink.get();
		TDCLSimpleServer theServer( &theApp, std::move( theCommLayer ), std::move( theLink ) );

		const bool interactiveStdin = ( ::isatty( STDIN_FILENO ) != 0 );
		if (interactiveStdin)
		{
			PrintCommands();
		}

		theServer.Start();

		bool running = true;
		while (running && !gStopRequested.load( std::memory_order_relaxed ))
		{
			struct pollfd fds[2];
			fds[0].fd = gSignalPipe[0];
			fds[0].events = POLLIN;
			fds[0].revents = 0;
			nfds_t nfds = 1;

			if (interactiveStdin)
			{
				fds[1].fd = STDIN_FILENO;
				fds[1].events = POLLIN;
				fds[1].revents = 0;
				nfds = 2;
			}

			int ready = ::poll( fds, nfds, -1 );
			if (ready < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				break;
			}

			if (fds[0].revents & POLLIN)
			{
				break;
			}

			if (interactiveStdin && (fds[1].revents & (POLLIN | POLLHUP | POLLERR)))
			{
				std::string line;
				if (!std::getline( std::cin, line ))
				{
					break;
				}

				if ((line == "quit") || (line == "exit"))
				{
					running = false;
				}
				else if (line == "help")
				{
					PrintCommands();
				}
				else if (line == "disconnect")
				{
					if (!linkPtr->Disconnect())
					{
						(void)::printf( "Not connected.\n" );
					}
				}
				else if (line.rfind( "install ", 0 ) == 0)
				{
					std::string pkgPath = line.substr( 8 );
					while (!pkgPath.empty() && (pkgPath.front() == ' ' || pkgPath.front() == '\t'))
					{
						pkgPath.erase( pkgPath.begin() );
					}
					if (!FileExists( pkgPath ))
					{
						(void)::printf( "File not found: %s\n", pkgPath.c_str() );
					}
					else
					{
						TDCLFSItemRef pkgRef(
							new TDCLPOSIXFile( theApp.GetPOSIXFiles(), pkgPath.c_str() ) );
						if (!linkPtr->InstallPackage( pkgRef ))
						{
							(void)::printf( "Cannot install package: not connected.\n" );
						}
					}
				}
				else if (!line.empty())
				{
					(void)::printf( "Unknown command: %s\n", line.c_str() );
					PrintCommands();
				}
			}
		}

		(void)::printf( "\nStopping server...\n" );
		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		(void)theServer.Wait();
	}
	catch (TDCLException& ex)
	{
		(void)::fprintf(
			stderr,
			"DCL exception: %s (code: %d, error: %d)\n",
			ex.GetExceptionName(),
			static_cast<int>( ex.GetExCode() ),
			static_cast<int>( ex.GetError() ) );
		exitCode = 1;
	}
	catch (std::exception& ex)
	{
		(void)::fprintf( stderr, "Exception: %s\n", ex.what() );
		exitCode = 1;
	}

	::close( gSignalPipe[0] );
	::close( gSignalPipe[1] );

	return exitCode;
}
