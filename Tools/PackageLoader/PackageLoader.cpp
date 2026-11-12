// ==============================
// File: PackageLoader.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <charconv>
#include <cerrno>
#include <cstring>
#include <atomic>
#include <chrono>
#include <limits>
#include <mutex>
#include <thread>
#include <filesystem>
#ifdef _WIN32
#include <DCL/Interfaces/POSIX/Compat/getopt.h>
#else
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#endif

#include <DCL/Server/TDCLSimpleServer.h>
#include <DCL/Package/TDCLPackage.h>
#include <K/Unicode/UUTF16CStr.h>

#include "TPackageLoaderApp.h"
#include "TPackageLoaderLink.h"
#include "TDCLClientSocket.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
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

static std::atomic<bool> sInterrupted( false );

static void TriggerInterruptedShutdown( TPackageLoaderApp* inApp, TDCLSimpleServer* inServer )
{
	std::cout << "\nInterrupted by user, shutting down..." << std::endl;

	// Launch watchdog: if clean shutdown takes more than 1 second, force exit.
	std::thread watchdog( []() {
		std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
		std::_Exit( 130 );
	} );
	watchdog.detach();

	if (inApp)
	{
		inApp->SignalCompletion();
	}
	if (inServer)
	{
		inServer->Stop();
	}
}

#ifdef _WIN32
static std::mutex sActiveMutex;
static TPackageLoaderApp* sActiveApp = nullptr;
static TDCLSimpleServer* sActiveServer = nullptr;

static BOOL WINAPI ConsoleCtrlHandler( DWORD inCtrlType )
{
	if (inCtrlType != CTRL_C_EVENT && inCtrlType != CTRL_BREAK_EVENT && inCtrlType != CTRL_CLOSE_EVENT)
	{
		return FALSE;
	}
	if (sInterrupted.exchange( true ))
	{
		std::_Exit( 130 );
	}
	std::lock_guard<std::mutex> lock( sActiveMutex );
	TriggerInterruptedShutdown( sActiveApp, sActiveServer );
	return TRUE;
}

class TSignalGuard
{
public:
	TSignalGuard( TPackageLoaderApp& inApp, TDCLSimpleServer& inServer )
	{
		std::lock_guard<std::mutex> lock( sActiveMutex );
		sActiveApp = &inApp;
		sActiveServer = &inServer;
		::SetConsoleCtrlHandler( ConsoleCtrlHandler, TRUE );
	}
	~TSignalGuard( void )
	{
		::SetConsoleCtrlHandler( ConsoleCtrlHandler, FALSE );
		std::lock_guard<std::mutex> lock( sActiveMutex );
		sActiveApp = nullptr;
		sActiveServer = nullptr;
	}
};
#else
static int sSignalPipe[2] = { -1, -1 };

static void SignalHandler( int /* inSignal */ )
{
	if (sInterrupted.exchange( true ))
	{
		::_exit( 130 );
	}
	if (sSignalPipe[1] >= 0)
	{
		const char byte = 1;
		ssize_t unused = ::write( sSignalPipe[1], &byte, 1 );
		(void)unused;
	}
}

class TSignalGuard
{
public:
	TSignalGuard( TPackageLoaderApp& inApp, TDCLSimpleServer& inServer )
	{
		if (::pipe( sSignalPipe ) == 0)
		{
			int flags = ::fcntl( sSignalPipe[1], F_GETFL, 0 );
			if (flags >= 0)
			{
				(void)::fcntl( sSignalPipe[1], F_SETFL, flags | O_NONBLOCK );
			}
			mWatcher = std::thread( [&inApp, &inServer]() {
				char byte = 0;
				ssize_t n = 0;
				do
				{
					n = ::read( sSignalPipe[0], &byte, 1 );
				} while (n < 0 && errno == EINTR);
				if (n == 1 && byte == 1)
				{
					TriggerInterruptedShutdown( &inApp, &inServer );
				}
			} );
		}

		struct sigaction sa;
		(void)::memset( &sa, 0, sizeof(sa) );
		sa.sa_handler = SignalHandler;
		sigemptyset( &sa.sa_mask );
		::sigaction( SIGINT, &sa, &mOldSigInt );
		::sigaction( SIGTERM, &sa, &mOldSigTerm );
		mInstalled = true;
	}

	~TSignalGuard( void )
	{
		if (mInstalled)
		{
			::sigaction( SIGINT, &mOldSigInt, nullptr );
			::sigaction( SIGTERM, &mOldSigTerm, nullptr );
		}
		if (sSignalPipe[1] >= 0)
		{
			const char byte = 0;
			ssize_t unused = ::write( sSignalPipe[1], &byte, 1 );
			(void)unused;
		}
		if (mWatcher.joinable())
		{
			mWatcher.join();
		}
		if (sSignalPipe[0] >= 0)
		{
			::close( sSignalPipe[0] );
			sSignalPipe[0] = -1;
		}
		if (sSignalPipe[1] >= 0)
		{
			::close( sSignalPipe[1] );
			sSignalPipe[1] = -1;
		}
	}

private:
	struct sigaction	mOldSigInt{};
	struct sigaction	mOldSigTerm{};
	std::thread			mWatcher;
	bool				mInstalled = false;
};
#endif

static void PrintUsage( const char* inProgName )
{
	std::cout << "Usage: " << inProgName << " [options] <package.pkg>\n\n"
			  << "Install a Newton package file over TCP/IP or Serial.\n\n"
			  << "Connection Modes (choose one; default is -l):\n"
			  << "  -l            Listen on TCP port for incoming connection\n"
			  << "  -c <host>     Connect to Newton as TCP client\n"
			  << "  -s <device>   Connect via serial port with MNP framing\n"
			  << "                (e.g., /dev/cu.usbserial-10, /dev/ttyUSB0)\n\n"
			  << "TCP Options:\n"
			  << "  -p <port>     TCP port for -l or -c (default: " << TDCLSocket::kIANAAssignedPort << ")\n\n"
			  << "Serial Options:\n"
			  << "  -b <rate>     Serial baud rate (default: 38400)\n"
			  << "                Supported: 2400, 4800, 9600, 19200, 38400,\n"
			  << "                           57600, 115200, 230400\n\n"
			  << "General Options:\n"
			  << "  -t <seconds>  Connection / operation timeout in seconds (default: 30)\n"
			  << "  -v            Enable verbose protocol logging\n"
			  << "  -h            Show this help text\n";
}

static bool ParseLongInRange( const char* inStr, long inMin, long inMax, long& outValue )
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

	int opt;
	while ((opt = ::getopt( argc, argv, "lc:p:s:b:t:vh" )) != -1)
	{
		switch (opt)
		{
			case 'l':
				if (mode != TransportMode::Unspecified && mode != TransportMode::Listen)
				{
					std::cerr << "Error: Options -l, -c, and -s are mutually exclusive." << std::endl;
					return 1;
				}
				mode = TransportMode::Listen;
				break;

			case 'c':
				if (mode != TransportMode::Unspecified && mode != TransportMode::Connect)
				{
					std::cerr << "Error: Options -l, -c, and -s are mutually exclusive." << std::endl;
					return 1;
				}
				mode = TransportMode::Connect;
				connectHost = optarg;
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
					std::cerr << "Error: Options -l, -c, and -s are mutually exclusive." << std::endl;
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

	if (optind >= argc)
	{
		std::cerr << "Error: No package file specified." << std::endl;
		PrintUsage( progName );
		return 1;
	}
	if (argc - optind > 1)
	{
		std::cerr << "Error: Only one package file may be specified." << std::endl;
		return 1;
	}
	std::string packagePath = argv[optind];

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

	TDCLPlatformFiles* theFilesIntf = static_cast<TDCLPlatformFiles*>( theApp.GetFilesIntf() );
	TDCLFile* thePkgFile = new TDCLPlatformFile( theFilesIntf, packagePath.c_str() );
	TDCLFSItemRef thePkgRef( thePkgFile );

	if (!thePkgFile->IsPackage())
	{
		std::cerr << "Error: '" << packagePath << "' is not a valid Newton package file." << std::endl;
		return 1;
	}

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
	std::unique_ptr<TDCLCommLayer> theCommLayer;
	if (mode == TransportMode::Listen)
	{
		std::cout << "Listening for Newton connection on TCP port " << tcpPort << "..." << std::endl;
		theCommLayer = std::make_unique<TDCLPlatformSocket>( tcpPort, 1, timeout );
	}
	else if (mode == TransportMode::Connect)
	{
		std::cout << "Connecting to Newton at " << connectHost << ":" << tcpPort << "..." << std::endl;
		theCommLayer = std::make_unique<TDCLClientSocket>( connectHost, tcpPort, timeout );
	}
	else if (mode == TransportMode::Serial)
	{
#ifdef _WIN32
		std::cerr << "Error: Serial connections are currently not supported on Windows." << std::endl;
		return 1;
#else
		std::cout << "Opening serial port " << serialDevice << " at " << baudRate
				  << " baud (MNP enabled)..." << std::endl;
		theCommLayer = std::make_unique<TDCLFDSerialPort>( serialDevice.c_str(), baudRate, true /* MNP */ );
		theCommLayer->SetTimeout( timeout );
#endif
	}

	TDCLSimpleServer theServer(
		&theApp,
		std::move( theCommLayer ),
		std::make_unique<TPackageLoaderLink>( &theApp, verbose ) );

	TSignalGuard signalGuard( theApp, theServer );

	try
	{
		theServer.Start();
		theApp.WaitForCompletion();
		theServer.Stop();
		(void)theServer.Wait();
	}
	catch (const TDCLException& e)
	{
		std::cerr << "Error: " << e.GetExceptionName() << std::endl;
		theServer.Kill();
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		theServer.Kill();
		return 1;
	}

	if (sInterrupted)
	{
		theServer.Kill();
		return 130;
	}

	theServer.Kill();
	return theApp.IsSuccess() ? 0 : 1;
}
