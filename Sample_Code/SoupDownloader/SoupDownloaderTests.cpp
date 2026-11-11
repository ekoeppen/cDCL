#include "SoupDownloadSession.h"

#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdDesktopInfo.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPassword.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/Streams/TDCLMemStream.h>

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <unistd.h>

namespace
{
using Cmd = TDCLDockCommand;
void
Check( bool ok, const char* message )
{
	if ( !ok )
		throw std::runtime_error( message );
}
template<class F>
void
Throws( F f )
{
	bool threw = false;
	try
	{
		f();
	}
	catch ( const std::exception& )
	{
		threw = true;
	}
	Check( threw, "Expected failure" );
}
struct Temp
{
	std::filesystem::path root;
	Temp()
	{
		std::string pattern = ( std::filesystem::temp_directory_path() / "cdcl-soup-XXXXXX" ).string();
		char* path = mkdtemp( pattern.data() );
		if ( !path )
			throw std::runtime_error( "mkdtemp failed" );
		root = path;
	}
	~Temp()
	{
		std::error_code e;
		std::filesystem::remove_all( root, e );
	}
};

// Exercise the real wire decoder too (including endian conversion and padding).
std::unique_ptr<Cmd>
Wire( Cmd& command )
{
	TDCLMemStream wire;
	command.SendCommand( &wire );
	wire.SetCursor( 0, TDCLRandomAccessStream::kFromStart );
	return std::unique_ptr<Cmd>( Cmd::ReceiveCommand( &wire ) );
}
struct Harness
{
	Temp temp;
	SoupDownloadSession session;
	TDCLMemStream pipe;
	KUInt32 consumed = 0;
	explicit Harness( const std::string& name = "Names" ) : session( name, temp.root / "export" )
	{
		session.Start( pipe );
		auto compression = Expect( Cmd::kDSetVBOCompression );
		Check( dynamic_cast<TDCLDockCmdSingleLong&>( *compression ).GetLong() == 0, "Compression not disabled" );
		// cvbo is fire-and-forget: gsto must already be sent, without a dres.
		Expect( Cmd::kDGetStoreNames );
		Silent();
	}
	std::filesystem::path
	Output() const
	{
		return temp.root / "export";
	}
	std::unique_ptr<Cmd>
	Expect( KUInt32 code )
	{
		Check( pipe.GetBufferSize() > consumed, "Expected outbound command" );
		TDCLMemStream input( static_cast<const KUInt8*>( pipe.GetBuffer() ) + consumed,
		                     pipe.GetBufferSize() - consumed );
		auto command = std::unique_ptr<Cmd>( Cmd::ReceiveCommand( &input ) );
		consumed += static_cast<KUInt32>( input.GetCursor() );
		Check( command->GetCommand() == code, "Wrong outbound command" );
		return command;
	}
	void
	Silent()
	{
		Check( pipe.GetBufferSize() == consumed, "Unexpected outbound command" );
	}
	void
	Feed( Cmd& command )
	{
		auto wire = Wire( command );
		session.Handle( *wire, pipe );
	}
	void
	NoData( KUInt32 code )
	{
		TDCLDockCmdNoData command( code );
		Feed( command );
	}
	void
	Result( KSInt32 error = 0 )
	{
		TDCLDockCmdSingleLong command( Cmd::kDResult, error );
		Feed( command );
	}
	void
	Object( KUInt32 code, const TDCLNSRef& object )
	{
		TDCLDockCmdNSObject command( code, object );
		Feed( command );
	}
	void
	Stores( unsigned count )
	{
		auto array = TDCLNSRef::MakeArray();
		for ( unsigned i = 0; i < count; ++i )
		{
			auto store = TDCLNSRef::MakeFrame();
			store.ToFrame().Set( "name", TDCLNSRef::MakeString( "Internal" ) );
			store.ToFrame().Set( "kind", TDCLNSRef::MakeString( "Internal" ) );
			store.ToFrame().Set( "signature", TDCLNSRef::MakeInt( i + 100 ) );
			array.ToArray().Add( store );
		}
		Object( Cmd::kDStoreNames, array );
	}
	void
	SelectStore( unsigned signature )
	{
		auto selected = Expect( Cmd::kDSetCurrentStore );
		auto frame = dynamic_cast<TDCLDockCmdNSObject&>( *selected ).GetObject();
		Check( frame.ToFrame().Get( "signature" ).ToInt() == static_cast<KSInt32>( signature ),
		       "Wrong store selected" );
		Result();
		Expect( Cmd::kDGetSoupNames );
	}
	void
	Names( bool found )
	{
		auto names = TDCLNSRef::MakeArray();
		auto signatures = TDCLNSRef::MakeArray();
		names.ToArray().Add( TDCLNSRef::MakeString( found ? "Names" : "Other" ) );
		signatures.ToArray().Add( TDCLNSRef::MakeInt( 42 ) );
		TDCLDockCmdTwoNSObjects command( Cmd::kDSoupNames, names, signatures );
		Feed( command );
	}
	void
	BeginEntries()
	{
		Names( true );
		auto selected = Expect( Cmd::kDSetCurrentSoup );
		// CString isn't specialized by ReceiveCommand, so check the raw wire bytes.
		const auto* bytes = static_cast<const KUInt8*>( pipe.GetBuffer() );
		const KUInt8 expected[] = { 0, 'N', 0, 'a', 0, 'm', 0, 'e', 0, 's', 0, 0 };
		Check( consumed >= sizeof( expected ), "Missing soup name" );
		Check( std::equal( std::begin( expected ), std::end( expected ), bytes + consumed - sizeof( expected ) ),
		       "Soup name must be null-terminated UTF-16BE" );
		Result();
		Expect( Cmd::kDGetSoupInfo );
		Object( Cmd::kDSoupInfo, TDCLNSRef::MakeFrame() );
		Expect( Cmd::kDGetIndexDescription );
		Object( Cmd::kDIndexDescription, TDCLNSRef::MakeArray() );
		Expect( Cmd::kDSendSoup );
	}
};

std::string
Read( const std::filesystem::path& path )
{
	std::ifstream input( path, std::ios::binary );
	Check( input.good(), "Missing output file" );
	return { std::istreambuf_iterator<char>( input ), std::istreambuf_iterator<char>() };
}

void
MultiStore()
{
	Harness h;
	h.NoData( Cmd::kDHello );
	h.Silent();
	h.Stores( 3 );
	h.SelectStore( 100 );
	h.BeginEntries();
	// Opaque payload, deliberately not decodable by cDCL: export must be lossless.
	const KUInt8 raw[] = { 2, 255, 0, 1, 9 };
	TDCLDockCmdNSObject entry( Cmd::kDEntry, sizeof( raw ), raw );
	h.Feed( entry );
	h.Feed( entry );
	h.Silent();
	Check( Read( h.Output() / "store-000001/entry-000001.nsof" ) ==
	           std::string( reinterpret_cast<const char*>( raw ), sizeof( raw ) ),
	       "Payload changed or padding exported" );
	Check( Read( h.Output() / "store-000001/entry-000002.nsof" ) ==
	           std::string( reinterpret_cast<const char*>( raw ), sizeof( raw ) ),
	       "Second entry lost" );
	Check( !std::filesystem::exists( h.Output() / "store-000001/COMPLETE" ), "Completed before bsdn" );
	h.NoData( Cmd::kDHello );
	h.Silent();
	h.NoData( Cmd::kDBackupSoupDone );
	h.SelectStore( 101 );
	h.Names( false );
	Check( !std::filesystem::exists( h.Output() / "store-000002" ), "Missing soup wasn't skipped" );
	h.SelectStore( 102 );
	h.BeginEntries();
	h.NoData( Cmd::kDBackupSoupDone ); // empty soup on third store
	Check( h.session.Complete() && h.session.ExportedStores() == 2, "Wrong completion state" );
	Check( Read( h.Output() / "store-000003/COMPLETE" ) == "Entries: 0\n", "Empty soup failed" );
	Check( Read( h.Output() / "store-000001/COMPLETE" ) == "Entries: 2\n", "Wrong entry count" );
	Check( std::filesystem::exists( h.Output() / "stores.nsof" ), "Missing stores manifest" );
	Check( std::filesystem::exists( h.Output() / "store-000001/soup-signatures.nsof" ), "Missing signatures" );
	Check( std::filesystem::exists( h.Output() / "store-000001/soup-info.nsof" ), "Missing metadata" );
	Check( std::filesystem::exists( h.Output() / "store-000001/indexes.nsof" ), "Missing indexes" );
	Check( std::filesystem::exists( h.Output() / "COMPLETE" ), "Missing global completion marker" );
	h.Silent();
}
void
FailureCases()
{
	{
		Harness h;
		h.Stores( 1 );
		h.SelectStore( 100 );
		Throws( [&] { h.Names( false ); } );
		Check( !h.session.Complete(), "Missing soup succeeded" );
	}
	{
		Harness h;
		Throws( [&] { h.Stores( 0 ); } );
	}
	{
		Harness h;
		Throws( [&] { h.Result( -28022 ); } );
	}
	{
		Harness h;
		Throws( [&] { h.Result(); } );
	} // no success dres is expected after cvbo
	{
		Harness h;
		h.Stores( 1 );
		h.SelectStore( 100 );
		h.BeginEntries();
		Throws( [&] { h.NoData( Cmd::kDOperationCanceled2 ); } );
		h.Expect( Cmd::kDOpCanceledAck );
		Check( !std::filesystem::exists( h.Output() / "COMPLETE" ), "Canceled export marked complete" );
	}
	{
		Harness h;
		Throws( [&] { h.NoData( Cmd::kDDisconnect ); } );
	}
	{
		Harness h;
		Throws( [&] { h.NoData( Cmd::kDBackupSoupDone ); } );
	}
	{
		Harness h;
		h.Stores( 1 );
		h.SelectStore( 100 );
		h.BeginEntries();
		Throws( [&] { h.Result( -1 ); } );
		Check( !std::filesystem::exists( h.Output() / "store-000001/COMPLETE" ), "Error marked complete" );
	}
	{
		Temp t;
		auto path = t.root / "not-a-directory";
		std::ofstream( path ) << "keep";
		Throws( [&] { SoupDownloadSession session( "Names", path ); } );
		Check( Read( path ) == "keep", "Non-directory output was modified" );
	}
	{
		Temp t;
		Throws( [&] { SoupDownloadSession empty( "", t.root / "out" ); } );
		Throws( [&] { SoupDownloadSession longName( std::string( 25, 'a' ), t.root / "out" ); } );
		Throws( [&] { SoupDownloadSession truncated( "Names\xc3", t.root / "out" ); } );
	}
	{
		Harness h;
		h.Stores( 1 );
		h.SelectStore( 100 );
		h.BeginEntries();
		std::filesystem::create_directory( h.Output() / "store-000001/entry-000001.nsof" );
		TDCLDockCmdNSObject entry( Cmd::kDEntry, TDCLNSRef::MakeFrame() );
		Throws( [&] { h.Feed( entry ); } );
		Check( !std::filesystem::exists( h.Output() / "COMPLETE" ), "Disk failure marked complete" );
	}
}
void
ReuseDirectory()
{
	Harness h;
	h.Stores( 1 );
	h.SelectStore( 100 );
	h.BeginEntries();
	TDCLDockCmdNSObject entry( Cmd::kDEntry, TDCLNSRef::MakeFrame() );
	h.Feed( entry );
	h.Feed( entry );
	h.NoData( Cmd::kDBackupSoupDone );
	Check( h.session.Complete(), "Initial export failed" );
	std::ofstream( h.Output() / "notes.txt" ) << "keep root";
	std::ofstream( h.Output() / "store-000001/notes.txt" ) << "keep store";
	std::ofstream( h.Output() / "store-000001/entry-custom.nsof" ) << "keep custom";
	std::filesystem::create_directory( h.Output() / "store-000002" );
	std::ofstream( h.Output() / "store-000002/COMPLETE" ) << "old store";

	h.session = SoupDownloadSession( "Names", h.Output() );
	Check( !std::filesystem::exists( h.Output() / "COMPLETE" ), "Old root completion retained" );
	Check( !std::filesystem::exists( h.Output() / "store-000001/COMPLETE" ), "Old store completion retained" );
	Check( !std::filesystem::exists( h.Output() / "store-000002/COMPLETE" ), "Absent store completion retained" );
	h.session.Start( h.pipe );
	h.Expect( Cmd::kDSetVBOCompression );
	h.Expect( Cmd::kDGetStoreNames );
	h.Stores( 1 );
	h.SelectStore( 100 );
	h.BeginEntries();
	Check( !std::filesystem::exists( h.Output() / "store-000001/entry-000001.nsof" ), "Old first entry retained" );
	Check( !std::filesystem::exists( h.Output() / "store-000001/entry-000002.nsof" ), "Stale higher entry retained" );
	h.NoData( Cmd::kDBackupSoupDone ); // empty soup replacing two old entries
	Check( h.session.Complete(), "Reused-directory export failed" );
	Check( Read( h.Output() / "store-000001/COMPLETE" ) == "Entries: 0\n", "Old count retained" );
	Check( Read( h.Output() / "notes.txt" ) == "keep root", "Unrelated root file modified" );
	Check( Read( h.Output() / "store-000001/notes.txt" ) == "keep store", "Unrelated store file modified" );
	Check( Read( h.Output() / "store-000001/entry-custom.nsof" ) == "keep custom", "Custom NSOF file removed" );
	h.Silent();

	Temp temp;
	SoupDownloadSession existingEmptyDirectory( "Names", temp.root );
	Check( std::filesystem::exists( temp.root / "soup-name.nsof" ), "Existing empty directory rejected" );
}
void
UnicodeName()
{
	Harness h( "N\xc3\xa4m" );
	h.Stores( 1 );
	h.SelectStore( 100 );
	auto names = TDCLNSRef::MakeArray();
	names.ToArray().Add( TDCLNSRef::MakeString( "N\xe4m" ) ); // MakeString(char*) takes Latin-1.
	TDCLDockCmdTwoNSObjects response( Cmd::kDSoupNames, names, TDCLNSRef::MakeArray() );
	h.Feed( response );
	h.Expect( Cmd::kDSetCurrentSoup );
	const KUInt8 expected[] = { 0, 'N', 0, 0xe4, 0, 'm', 0, 0 };
	const auto* bytes = static_cast<const KUInt8*>( h.pipe.GetBuffer() );
	Check( std::equal( std::begin( expected ), std::end( expected ), bytes + h.consumed - sizeof( expected ) ),
	       "UTF-8 soup name was not converted to UTF-16BE" );
}
void
DesktopInfoEndian()
{
	TDCLNSArray apps;
	const KUInt32 challenge[] = { UByteSex_ToBigEndian( KUInt32( 0x12345678 ) ),
	                              UByteSex_ToBigEndian( KUInt32( 0x90abcdef ) ) };
	TDCLDockCmdDesktopInfo command( apps, challenge[0], challenge[1], true, Cmd::kSettingUp_SType, Cmd::kWin_DType );
	TDCLMemStream wire;
	command.SendCommand( &wire );
	wire.SetCursor( 16, TDCLRandomAccessStream::kFromStart );
	Check( wire.GetLong() == Cmd::kProtocolVersion, "Desktop protocol version is not big-endian" );
	Check( wire.GetLong() == Cmd::kWin_DType, "Desktop type is not big-endian" );
	KUInt32 receivedChallenge[2];
	KUInt32 length = sizeof( receivedChallenge );
	wire.Read( receivedChallenge, &length );
	Check( std::memcmp( challenge, receivedChallenge, sizeof( challenge ) ) == 0, "Challenge bytes changed" );
	Check( wire.GetLong() == Cmd::kSettingUp_SType, "Session type is not big-endian" );
	Check( wire.GetLong() == 1, "Selective sync flag is not big-endian" );
	TDCLDockCmdPassword password( receivedChallenge );
	auto received = Wire( password );
	Check( dynamic_cast<TDCLDockCmdPassword&>( *received ).VerifyPassword( challenge ),
	       "Desktop challenge/password representation mismatch" );
}
void
EndianResults()
{
	for ( KSInt32 value : { KSInt32( 0 ), KSInt32( 0x12345678 ), KSInt32( -28022 ) } )
	{
		TDCLDockCmdSingleLong original( Cmd::kDResult, value );
		auto received = Wire( original );
		Check( static_cast<KSInt32>( dynamic_cast<TDCLDockCmdSingleLong&>( *received ).GetLong() ) == value,
		       "Single-long receive endian regression" );
	}
}
} // namespace

int
main( int argc, char** argv )
{
	try
	{
		// Private PTY-peer helper: encrypt an arbitrary challenge with the empty
		// Dock password. Its stdout must contain exactly eight binary bytes,
		// which also detects accidental DES diagnostic output.
		if ( argc == 3 && std::string( argv[1] ) == "--encrypt-challenge" )
		{
			std::string hex = argv[2];
			Check( hex.size() == 16 && hex.find_first_not_of( "0123456789abcdefABCDEF" ) == std::string::npos,
			       "Expected a 16-digit challenge" );
			KUInt32 challenge[] = {
			    UByteSex_ToBigEndian( static_cast<KUInt32>( std::stoul( hex.substr( 0, 8 ), nullptr, 16 ) ) ),
			    UByteSex_ToBigEndian( static_cast<KUInt32>( std::stoul( hex.substr( 8, 8 ), nullptr, 16 ) ) ) };
			TDCLDockCmdPassword password( challenge );
			TDCLMemStream wire;
			password.SendCommand( &wire );
			return std::fwrite( static_cast<const KUInt8*>( wire.GetBuffer() ) + 16, 1, 8, stdout ) == 8 ? 0 : 1;
		}
		Check( argc == 1, "Unexpected test arguments" );
		DesktopInfoEndian();
		EndianResults();
		MultiStore();
		ReuseDirectory();
		UnicodeName();
		FailureCases();
		std::cout << "SoupDownloader tests passed\n";
		return 0;
	}
	catch ( const std::exception& e )
	{
		std::cerr << e.what() << '\n';
	}
	catch ( const TDCLException& e )
	{
		std::cerr << e.GetExceptionName() << '\n';
	}
	return 1;
}
