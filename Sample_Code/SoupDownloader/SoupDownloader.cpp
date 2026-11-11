#include "SoupDownloadSession.h"

#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Interfaces/Common/TDCLStdThreads.h>
#include <DCL/Interfaces/POSIX/TDCLPOSIXFiles.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/TDCLDockLink.h>
#include <DCL/Server/TDCLSimpleServer.h>
#include <K/Unicode/UUTF16CStr.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
volatile std::sig_atomic_t interrupted = 0;
void
OnSignal( int )
{
	interrupted = 1;
}

class DownloadApp final : public TDCLApplication
{
public:
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
	}
	void
	ConnectedToNewtonDevice( TDCLLink*, const KUInt16* ) override
	{
		std::cerr << "Dock connected; exporting soup across all stores...\n";
	}
	void
	Disconnecting( TDCLLink* ) override
	{
	}
	void
	Disconnected( TDCLLink* ) override
	{
		done = true;
	}
	void
	ServerIsDown( TDCLServer* ) override
	{
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
			Error( error->GetExceptionName() );
	}
	void
	DockCommandBeingProcessed( TDCLLink*, TDCLDockCommand*, TDCLLinkEngine* ) override
	{
	}
	void
	DockCommandProcessed( TDCLLink*, TDCLLinkEngine*, TDCLException* error ) override
	{
		if ( error )
			Error( error->GetExceptionName() );
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
	IDCLThreads*
	CreateThreadsIntf() override
	{
		return new TDCLStdThreads;
	}

private:
	void
	Error( TDCLException* e )
	{
		Error( e ? e->GetExceptionName() : "Unknown error" );
	}
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
	bool
	Stopped()
	{
		return GetThreadState() == IDCLThreads::kStopped;
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
	ProcessDockCommand( TDCLDockCommand* command, Boolean* processed ) override
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

bool
WaitUntilStopped( TDCLSimpleServer& server, DownloadLink& link )
{
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 8 );
	while ( std::chrono::steady_clock::now() < deadline )
	{
		if ( server.GetState() == TDCLServer::kStopped && link.Stopped() )
			return true;
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}
	return server.GetState() == TDCLServer::kStopped && link.Stopped();
}

void
Usage( const char* name )
{
	std::cout << "Usage: " << name
	          << " --serial <device-or-pty> --soup <name> --output <directory> [--baud <rate>]\n"
	          << "Export the named soup from every store as raw NSOF (Newton OS 2.x).\n"
	          << "Default baud: 38400. Set NEWTON_PASSWORD if Dock requires a password.\n";
}
} // namespace

int
main( int argc, char** argv )
{
	std::string device, soup, output;
	long baud = 38400;
	for ( int i = 1; i < argc; ++i )
	{
		std::string arg = argv[i];
		if ( arg == "--help" || arg == "-h" )
		{
			Usage( argv[0] );
			return 0;
		}
		if ( i + 1 >= argc )
		{
			Usage( argv[0] );
			return 1;
		}
		if ( arg == "--serial" )
			device = argv[++i];
		else if ( arg == "--soup" )
			soup = argv[++i];
		else if ( arg == "--output" )
			output = argv[++i];
		else if ( arg == "--baud" )
		{
			try
			{
				std::string value = argv[++i];
				size_t used = 0;
				baud = std::stol( value, &used );
				if ( used != value.size() )
					throw std::invalid_argument( "baud" );
			}
			catch ( const std::exception& )
			{
				std::cerr << "Invalid baud rate\n";
				return 1;
			}
		}
		else
		{
			Usage( argv[0] );
			return 1;
		}
	}
	if ( device.empty() || soup.empty() || output.empty() )
	{
		Usage( argv[0] );
		return 1;
	}
	if ( baud != 2400 && baud != 4800 && baud != 9600 && baud != 19200 && baud != 38400 && baud != 57600 &&
	     baud != 115200 && baud != 230400 )
	{
		std::cerr << "Unsupported baud rate\n";
		return 1;
	}

	try
	{
		SoupDownloadSession session( soup, output );
		DownloadApp app;
		auto port = std::make_unique<TDCLFDSerialPort>( app.GetThreadsIntf(), device.c_str(), baud,
		                                                static_cast<KUInt32>( TDCLFDSerialPort::kModeMNP ) );
		// The link may notify its server while being destroyed: server outlives link.
		TDCLSimpleServer server( &app, port.get() );
		DownloadLink link( app, session );
		server.SetLink( &link );
		std::signal( SIGINT, OnSignal );
		std::signal( SIGTERM, OnSignal );
		// Never unwind live worker objects after an exception or shutdown timeout.
		try
		{
			server.Start();
			while ( !interrupted && !app.Done() )
				std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
			server.Stop();
			if ( !WaitUntilStopped( server, link ) )
			{
				std::cerr << "SoupDownloader: shutdown timed out\n";
				std::_Exit( 1 );
			}
		}
		catch ( const TDCLException& e )
		{
			std::cerr << "SoupDownloader: " << e.GetExceptionName() << '\n';
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
		std::cerr << "SoupDownloader: " << e.GetExceptionName() << '\n';
	}
	catch ( const std::exception& e )
	{
		std::cerr << "SoupDownloader: " << e.what() << '\n';
	}
	return 1;
}
