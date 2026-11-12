// ==============================
// File: PackageLoader.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <filesystem>

#include <DCL/Server/TDCLSimpleServer.h>
#include <DCL/Package/TDCLPackage.h>
#include <K/Unicode/UUTF16CStr.h>

#include "TPackageLoaderApp.h"
#include "TPackageLoaderLink.h"
#include "TDCLClientSocket.h"

#if TARGET_OS_WIN32
#include <DCL/Interfaces/Win32/TDCLWin32File.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
typedef TDCLWin32File TDCLPlatformFile;
typedef TDCLWin32Files TDCLPlatformFiles;
#else
#include <DCL/Interfaces/POSIX/TDCLPOSIXFile.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
typedef TDCLPOSIXFile TDCLPlatformFile;
typedef TDCLPOSIXFiles TDCLPlatformFiles;
#endif

enum class TransportMode {
	Unspecified,
	Listen,
	Connect,
	Serial
};

static TPackageLoaderApp* sActiveApp = nullptr;
static TDCLSimpleServer* sActiveServer = nullptr;
static std::atomic<bool> sInterrupted( false );

static void SignalHandler( int /* inSignal */ )
{
	if (sInterrupted.exchange( true ))
	{
		std::_Exit( 130 );
	}

	std::cout << "\nInterrupted by user, shutting down..." << std::endl;

	// Launch watchdog: if clean shutdown takes more than 1 second, force exit.
	std::thread watchdog( []() {
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
		std::_Exit( 130 );
	} );
	watchdog.detach();

	if (sActiveApp)
	{
		sActiveApp->SignalCompletion();
	}
	if (sActiveServer)
	{
		sActiveServer->Stop();
	}
}

static void PrintUsage( const char* inProgName )
{
	std::cout << "Usage: " << inProgName << " [options] <package.pkg>\n\n"
			  << "Install a Newton package file over TCP/IP or Serial.\n\n"
			  << "Connection Modes (choose one; default is --listen 3679):\n"
			  << "  -l, --listen [port]         Listen on TCP port for incoming connection (default: 3679)\n"
			  << "  -c, --connect <host[:port]> Connect to Newton as TCP client (default port: 3679)\n"
			  << "  -s, --serial <device>       Connect via serial port with MNP framing\n"
			  << "                              (e.g., /dev/cu.usbserial-10, /dev/ttyUSB0)\n\n"
			  << "Serial Options:\n"
			  << "  -b, --baud <rate>           Serial baud rate (default: 38400)\n"
			  << "                              Supported: 2400, 4800, 9600, 19200, 38400,\n"
			  << "                                         57600, 115200, 230400\n\n"
			  << "General Options:\n"
			  << "  -t, --timeout <seconds>     Connection / operation timeout in seconds (default: 30)\n"
			  << "  -v, --verbose               Enable verbose protocol logging\n"
			  << "  -h, --help                  Show this help text\n";
}

static bool IsAllDigits( const std::string& str )
{
	return !str.empty() && std::all_of( str.begin(), str.end(), []( unsigned char c ) { return std::isdigit( c ); } );
}

int main( int argc, char** argv )
{
	const char* progName = argv[0];
	TransportMode mode = TransportMode::Unspecified;
	unsigned short tcpPort = TDCLSocket::kIANAAssignedPort;
	std::string connectHost;
	std::string serialDevice;
	long baudRate = 38400;
	long timeout = 30;
	bool verbose = false;
	std::string packagePath;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help")
		{
			PrintUsage( progName );
			return 0;
		}
		else if (arg == "-v" || arg == "--verbose")
		{
			verbose = true;
		}
		else if (arg == "-l" || arg == "--listen")
		{
			if (mode != TransportMode::Unspecified && mode != TransportMode::Listen)
			{
				std::cerr << "Error: Options --listen, --connect, and --serial are mutually exclusive." << std::endl;
				return 1;
			}
			mode = TransportMode::Listen;
			if (i + 1 < argc && IsAllDigits( argv[i + 1] ))
			{
				tcpPort = (unsigned short) std::stoi( argv[++i] );
			}
		}
		else if (arg == "-c" || arg == "--connect")
		{
			if (mode != TransportMode::Unspecified && mode != TransportMode::Connect)
			{
				std::cerr << "Error: Options --listen, --connect, and --serial are mutually exclusive." << std::endl;
				return 1;
			}
			if (i + 1 >= argc || argv[i + 1][0] == '-')
			{
				std::cerr << "Error: Missing host for option " << arg << std::endl;
				return 1;
			}
			mode = TransportMode::Connect;
			std::string target = argv[++i];
			// Parse host[:port]
			size_t colonPos = target.rfind( ':' );
			if (colonPos != std::string::npos && colonPos + 1 < target.size()
				&& IsAllDigits( target.substr( colonPos + 1 ) ))
			{
				connectHost = target.substr( 0, colonPos );
				tcpPort = (unsigned short) std::stoi( target.substr( colonPos + 1 ) );
			}
			else
			{
				connectHost = target;
			}
		}
		else if (arg == "-s" || arg == "--serial")
		{
			if (mode != TransportMode::Unspecified && mode != TransportMode::Serial)
			{
				std::cerr << "Error: Options --listen, --connect, and --serial are mutually exclusive." << std::endl;
				return 1;
			}
			if (i + 1 >= argc || argv[i + 1][0] == '-')
			{
				std::cerr << "Error: Missing device for option " << arg << std::endl;
				return 1;
			}
			mode = TransportMode::Serial;
			serialDevice = argv[++i];
		}
		else if (arg == "-b" || arg == "--baud")
		{
			if (i + 1 >= argc || !IsAllDigits( argv[i + 1] ))
			{
				std::cerr << "Error: Missing or invalid baud rate." << std::endl;
				return 1;
			}
			baudRate = std::stol( argv[++i] );
			const std::vector<long> validBauds = { 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400 };
			if (std::find( validBauds.begin(), validBauds.end(), baudRate ) == validBauds.end())
			{
				std::cerr << "Error: Unsupported baud rate: " << baudRate << std::endl;
				return 1;
			}
		}
		else if (arg == "-t" || arg == "--timeout")
		{
			if (i + 1 >= argc || !IsAllDigits( argv[i + 1] ))
			{
				std::cerr << "Error: Missing or invalid timeout value." << std::endl;
				return 1;
			}
			timeout = std::stol( argv[++i] );
			if (timeout <= 0)
			{
				std::cerr << "Error: Timeout must be greater than 0." << std::endl;
				return 1;
			}
		}
		else if (!arg.empty() && arg[0] == '-')
		{
			std::cerr << "Error: Unknown option: " << arg << std::endl;
			PrintUsage( progName );
			return 1;
		}
		else
		{
			if (!packagePath.empty())
			{
				std::cerr << "Error: Only one package file may be specified." << std::endl;
				return 1;
			}
			packagePath = arg;
		}
	}

	if (packagePath.empty())
	{
		std::cerr << "Error: No package file specified." << std::endl;
		PrintUsage( progName );
		return 1;
	}

	if (mode == TransportMode::Unspecified)
	{
		mode = TransportMode::Listen;
	}

	// Validate that package file exists
	if (!std::filesystem::exists( packagePath ))
	{
		std::cerr << "Error: File does not exist: " << packagePath << std::endl;
		return 1;
	}

	// Initialize application and inspect package
	TPackageLoaderApp theApp( verbose );
	IDCLThreads* theThreads = theApp.GetThreadsIntf();

	TDCLPlatformFiles theFilesIntf;
	TDCLFile* thePkgFile = new TDCLPlatformFile( &theFilesIntf, packagePath.c_str() );
	TDCLFSItemRef thePkgRef( thePkgFile );

	if (!thePkgFile->IsPackage())
	{
		std::cerr << "Error: '" << packagePath << "' is not a valid Newton package file." << std::endl;
		return 1;
	}

	// IsPackage() opens the file temporarily and closes it again.  Reopen it
	// before querying its length; the Win32 implementation requires an open
	// handle for GetLength().
	thePkgFile->Open( true /* read only */ );
	KUInt64 pkgSize = thePkgFile->GetLength();
	std::string pkgDisplayName = std::filesystem::path( packagePath ).filename().string();
	try
	{
		TDCLPackage pkgInfo( thePkgFile );
		const KUInt16* u16Name = pkgInfo.GetPackageName();
		if (u16Name)
		{
			char nameBuf[256] = {0};
			UUTF16CStr::ToISO88591( u16Name, (KUInt8*) nameBuf, sizeof(nameBuf) - 1 );
			if (nameBuf[0] != '\0')
			{
				pkgDisplayName = nameBuf;
			}
		}
		std::cout << "Package: \"" << pkgDisplayName << "\" (" << pkgSize << " bytes, "
				  << pkgInfo.GetNumParts() << " part" << (pkgInfo.GetNumParts() == 1 ? "" : "s") << ")"
				  << std::endl;
	}
	catch (...)
	{
		std::cout << "Package: \"" << pkgDisplayName << "\" (" << pkgSize << " bytes)" << std::endl;
	}

	theApp.SetPackage( thePkgRef, pkgDisplayName );

	// Configure comm layer
	TDCLCommLayer* theCommLayer = nullptr;
	if (mode == TransportMode::Listen)
	{
		std::cout << "Listening for Newton connection on TCP port " << tcpPort << "..." << std::endl;
		theCommLayer = new TDCLPlatformSocket( theThreads, tcpPort, 1, timeout );
	}
	else if (mode == TransportMode::Connect)
	{
		std::cout << "Connecting to Newton at " << connectHost << ":" << tcpPort << "..." << std::endl;
		theCommLayer = new TDCLClientSocket( theThreads, connectHost, tcpPort, timeout );
	}
	else if (mode == TransportMode::Serial)
	{
#if TARGET_OS_WIN32
		std::cerr << "Error: Serial connections are currently not supported on Windows." << std::endl;
		return 1;
#else
		std::cout << "Opening serial port " << serialDevice << " at " << baudRate
				  << " baud (MNP enabled)..." << std::endl;
		theCommLayer = new TDCLFDSerialPort( theThreads, serialDevice.c_str(), baudRate, true /* MNP */ );
#endif
	}

	TDCLSimpleServer theServer( &theApp, theCommLayer );
	TPackageLoaderLink theLink( &theApp, verbose );
	theServer.SetLink( &theLink );

	sActiveApp = &theApp;
	sActiveServer = &theServer;
	std::signal( SIGINT, SignalHandler );
	std::signal( SIGTERM, SignalHandler );

	try
	{
		theServer.Start();
		theApp.WaitForCompletion();
		theServer.Stop();

		while (theServer.GetState() != TDCLSimpleServer::kStopped)
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
		}
		theLink.WaitForThreadToStop( 1000 );
	}
	catch (const TDCLException& e)
	{
		std::cerr << "Error: " << e.GetExceptionName() << std::endl;
		theLink.WaitForThreadToStop( 1000 );
		delete theCommLayer;
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		theLink.WaitForThreadToStop( 1000 );
		delete theCommLayer;
		return 1;
	}

	if (sInterrupted)
	{
		theLink.WaitForThreadToStop( 500 );
		delete theCommLayer;
		return 130;
	}

	theLink.WaitForThreadToStop( 1000 );
	delete theCommLayer;
	return theApp.IsSuccess() ? 0 : 1;
}
