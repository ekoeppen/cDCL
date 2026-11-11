#include "SoupDownloadSession.h"

#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/TDCLDockLink.h>
#include <DCL/Server/TDCLSimpleServer.h>
#include <K/Unicode/UUTF16CStr.h>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
enum class TransportMode
{
	Unspecified,
	Listen,
	Serial
};

volatile std::sig_atomic_t interrupted = 0;
void
OnSignal( int )
{
	interrupted = 1;
}

bool
ParseLongInRange( const char* inStr, long inMin, long inMax, long& outValue )
{
	if ( !inStr || inStr[0] == '\0' )
		return false;
	const char* end = inStr + std::strlen( inStr );
	long val = 0;
	auto [ptr, ec] = std::from_chars( inStr, end, val );
	if ( ec != std::errc() || ptr != end || val < inMin || val > inMax )
		return false;
	outValue = val;
	return true;
}

class DownloadApp final : public TDCLApplication
{
public:
	explicit DownloadApp( bool inVerbose )
	    : verbose( inVerbose )
	{
	}
	bool
	Done() const
	{
		return done.load();
	}
	bool
	Failed() const
	{
		return failed.load();
	}
	void
	Error( const char* message )
	{
		std::cerr << "SoupDownloader: " << message << '\n';
		failed = true;
		done = true;
	}
	void
	WaitConnection( TDCLServer*, TDCLCommLayer* ) override
	{
		std::cerr << "Waiting for Newton Dock connection...\n";
	}
	void
	Connected( TDCLLink*, TDCLCommLayer* ) override
	{
		if ( verbose )
			std::cerr << "Transport connected; starting Dock handshake...\n";
	}
	void
	ConnectedToNewtonDevice( TDCLLink*, const KUInt16* inName ) override
	{
		if ( verbose && inName )
		{
			char nameBuf[510];
			UUTF16CStr::ToUTF8( inName, reinterpret_cast<KUInt8*>( nameBuf ), sizeof( nameBuf ) );
			std::cerr << "Connected to Newton: \"" << nameBuf << "\"\n";
		}
		std::cerr << "Dock connected; exporting soup across all stores...\n";
	}
	void
	Disconnecting( TDCLLink* ) override
	{
		if ( verbose )
			std::cerr << "Disconnecting link...\n";
	}
	void
	Disconnected( TDCLLink* ) override
	{
		if ( verbose )
			std::cerr << "Link disconnected.\n";
		done = true;
	}
	void
	ServerIsDown( TDCLServer* ) override
	{
		if ( verbose )
			std::cerr << "Server is down.\n";
		done = true;
	}
	void
	Cancel( TDCLLink* ) override
	{
		Error( "Export canceled" );
	}
	void
	Timeout( TDCLLink* ) override
	{
		Error( "Connection timed out" );
	}
	void
	AppCommandBeingProcessed( TDCLLink*, TDCLAppCommand* ) override
	{
	}
	void
	AppCommandProcessed( TDCLLink*, TDCLAppCommand*, TDCLException* error ) override
	{
		if ( error )
			Error( error );
	}
	void
	DockCommandBeingProcessed( TDCLLink*, TDCLDockCommand*, TDCLLinkEngine* ) override
	{
	}
	void
	DockCommandProcessed( TDCLLink*, TDCLLinkEngine*, TDCLException* error ) override
	{
		if ( error )
			Error( error );
	}
	void
	HandleServerException( TDCLServer*, TDCLException* e ) override
	{
		Error( e );
	}
	void
	HandleCommLayerException( TDCLCommLayer*, TDCLException* e ) override
	{
		Error( e );
	}
	void
	HandleLinkException( TDCLLink*, TDCLException* e ) override
	{
		Error( e );
	}
	void
	HandleException( TDCLException* e ) override
	{
		Error( e );
	}

protected:
	IDCLFiles*
	CreateFilesIntf() override
	{
		return new TDCLPOSIXFiles;
	}

private:
	void
	Error( TDCLException* e )
	{
		if ( !e )
		{
			Error( "Unknown error" );
			return;
		}
		std::cerr << "SoupDownloader: " << e->GetExceptionName()
		          << " (code: " << static_cast<int>( e->GetExCode() )
		          << ", error: " << static_cast<int>( e->GetError() ) << ")\n";
		failed = true;
		done = true;
	}
	bool verbose = false;
	std::atomic<bool> done{ false };
	std::atomic<bool> failed{ false };
};

class DownloadLink final : public TDCLDockLink
{
public:
	DownloadLink( DownloadApp& app, SoupDownloadSession& session )
	    : TDCLDockLink( &app, kNoIcon ), app( app ), session( session )
	{
		const char* text = std::getenv( "NEWTON_PASSWORD" );
		if ( text && *text )
		{
			password.resize( std::char_traits<char>::length( text ) + 1 );
			UUTF16CStr::FromUTF8( reinterpret_cast<const KUInt8*>( text ), password.data() );
		}
	}

protected:
	const KUInt16*
	GetPassword() override
	{
		return password.empty() ? nullptr : password.data();
	}
	void
	Connected() override
	{
		TDCLDockLink::Connected();
		try
		{
			session.Start( *GetPipe() );
		}
		catch ( const std::exception& e )
		{
			app.Error( e.what() );
			Disconnect();
		}
	}
	EState
	ProcessDockCommand( TDCLDockCommand* command, bool* processed ) override
	{
		*processed = true;
		try
		{
			session.Handle( *command, *GetPipe() );
			if ( session.Complete() )
				return kDisconnecting;
			// Reading an entry can prefetch subsequent commands into the MNP
			// payload buffer. The serial watcher only notices new wire bytes;
			// schedule buffered commands too, keeping the event loop responsive.
			if ( GetPipe()->BytesAvailable() )
				DataPresent();
			return kRunning;
		}
		catch ( const std::exception& e )
		{
			app.Error( e.what() );
			return kDisconnecting;
		}
	}

private:
	DownloadApp& app;
	SoupDownloadSession& session;
	std::vector<KUInt16> password;
};

void
Usage( const char* name )
{
	std::cout << "Usage: " << name << " [options] <soup> <output>\n\n"
	          << "Export the named soup from every store as raw NSOF (Newton OS 2.x).\n"
	          << "Set NEWTON_PASSWORD if Dock requires a password.\n\n"
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
	          << "  -v            Enable verbose logging\n"
	          << "  -h            Show this help text\n";
}
} // namespace

int
main( int argc, char** argv )
{
	const char* progName = argv[0];
	TransportMode mode = TransportMode::Unspecified;
	unsigned short tcpPort = TDCLBSDSocket::kIANAAssignedPort;
	std::string device;
	long baud = 38400;
	long timeout = 30;
	bool verbose = false;

	int opt;
	while ( ( opt = ::getopt( argc, argv, "lp:s:b:t:vh" ) ) != -1 )
	{
		switch ( opt )
		{
		case 'l':
			if ( mode != TransportMode::Unspecified && mode != TransportMode::Listen )
			{
				std::cerr << "Error: Options -l and -s are mutually exclusive.\n";
				return 1;
			}
			mode = TransportMode::Listen;
			break;

		case 'p':
		{
			long portVal = 0;
			if ( !ParseLongInRange( optarg, 1, 65535, portVal ) )
			{
				std::cerr << "Error: Invalid TCP port: " << optarg << '\n';
				return 1;
			}
			tcpPort = static_cast<unsigned short>( portVal );
			break;
		}

		case 's':
			if ( mode != TransportMode::Unspecified && mode != TransportMode::Serial )
			{
				std::cerr << "Error: Options -l and -s are mutually exclusive.\n";
				return 1;
			}
			mode = TransportMode::Serial;
			device = optarg;
			break;

		case 'b':
		{
			if ( !ParseLongInRange( optarg, 1, std::numeric_limits<long>::max(), baud ) )
			{
				std::cerr << "Error: Invalid baud rate: " << optarg << '\n';
				return 1;
			}
			const std::vector<long> validBauds = { 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400 };
			if ( std::find( validBauds.begin(), validBauds.end(), baud ) == validBauds.end() )
			{
				std::cerr << "Error: Unsupported baud rate: " << baud << '\n';
				return 1;
			}
			break;
		}

		case 't':
			if ( !ParseLongInRange( optarg, 1, std::numeric_limits<long>::max(), timeout ) )
			{
				std::cerr << "Error: Invalid timeout value: " << optarg << '\n';
				return 1;
			}
			break;

		case 'v':
			verbose = true;
			break;

		case 'h':
			Usage( progName );
			return 0;

		default:
			Usage( progName );
			return 1;
		}
	}

	if ( argc - optind != 2 )
	{
		Usage( progName );
		return 1;
	}
	const std::string soup = argv[optind];
	const std::string output = argv[optind + 1];

	if ( mode == TransportMode::Unspecified )
		mode = TransportMode::Listen;

	try
	{
		SoupDownloadSession session( soup, output );
		DownloadApp app( verbose );
		std::unique_ptr<TDCLCommLayer> commLayer;
		if ( mode == TransportMode::Listen )
		{
			if ( !TDCLBSDSocket::IsAvailable() )
			{
				std::cerr << "SoupDownloader: BSD sockets are not available on this system.\n";
				return 1;
			}
			commLayer = std::make_unique<TDCLBSDSocket>(
			    tcpPort, TDCLBSDSocket::kDefaultPipeCount, timeout );
			if ( verbose )
				std::cerr << "Listening on TCP port " << tcpPort << "...\n";
		}
		else
		{
			auto port = std::make_unique<TDCLFDSerialPort>(
			    device.c_str(), baud, static_cast<KUInt32>( TDCLFDSerialPort::kModeMNP ) );
			port->SetTimeout( timeout );
			commLayer = std::move( port );
			if ( verbose )
				std::cerr << "Waiting on serial port " << device << " (" << baud << " baud, MNP)...\n";
		}
		auto link = std::make_unique<DownloadLink>( app, session );
		auto* linkPtr = link.get();
		TDCLSimpleServer server( &app, std::move( commLayer ), std::move( link ) );
		std::signal( SIGINT, OnSignal );
		std::signal( SIGTERM, OnSignal );
		// Never unwind live worker objects after an exception or shutdown timeout.
		try
		{
			server.Start();
			while ( !interrupted && !app.Done() )
				std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
			if ( interrupted )
				server.Kill();
			else
				server.Stop();
			(void)server.Wait();
			(void)linkPtr->Wait();
		}
		catch ( const TDCLException& e )
		{
			std::cerr << "SoupDownloader: " << e.GetExceptionName()
			          << " (code: " << static_cast<int>( e.GetExCode() )
			          << ", error: " << static_cast<int>( e.GetError() ) << ")\n";
			std::_Exit( 1 );
		}
		catch ( const std::exception& e )
		{
			std::cerr << "SoupDownloader: " << e.what() << '\n';
			std::_Exit( 1 );
		}
		if ( interrupted )
			return 130;
		if ( app.Failed() || !session.Complete() )
		{
			std::cerr << "Export incomplete; partial files retained in " << output << '\n';
			return 1;
		}
		std::cerr << "Export complete: " << session.ExportedStores() << " store(s) in " << output << '\n';
		return 0;
	}
	catch ( const TDCLException& e )
	{
		std::cerr << "SoupDownloader: " << e.GetExceptionName()
		          << " (code: " << static_cast<int>( e.GetExCode() )
		          << ", error: " << static_cast<int>( e.GetError() ) << ")\n";
	}
	catch ( const std::exception& e )
	{
		std::cerr << "SoupDownloader: " << e.what() << '\n';
	}
	return 1;
}
