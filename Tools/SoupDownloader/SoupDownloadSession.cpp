#include "SoupDownloadSession.h"

#include <DCL/Link/Dock_Commands/TDCLDockCmdCString.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <K/Unicode/UUTF16Conv.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
using Cmd = TDCLDockCommand;

void
Send( KUInt32 command, TDCLStream& pipe )
{
	TDCLDockCmdNoData( command ).SendCommand( &pipe );
}

std::string
Number( unsigned value )
{
	std::ostringstream text;
	text << std::setfill( '0' ) << std::setw( 6 ) << value;
	return text.str();
}

bool
NumberedName( const std::string& name, const std::string& prefix, const std::string& suffix = "" )
{
	if ( name.size() < prefix.size() + 6 + suffix.size() || name.compare( 0, prefix.size(), prefix ) != 0 ||
	     name.compare( name.size() - suffix.size(), suffix.size(), suffix ) != 0 )
		return false;
	for ( size_t i = prefix.size(); i < name.size() - suffix.size(); ++i )
		if ( name[i] < '0' || name[i] > '9' )
			return false;
	return true;
}

void
EnsureDirectory( const std::filesystem::path& path )
{
	if ( !std::filesystem::create_directory( path ) && !std::filesystem::is_directory( path ) )
		throw std::runtime_error( "Output path is not a directory: " + path.string() );
}

TDCLDockCmdNSObject&
Object( TDCLDockCommand& command )
{
	auto* object = dynamic_cast<TDCLDockCmdNSObject*>( &command );
	if ( !object )
		throw std::runtime_error( "Expected a Dock NSOF payload" );
	return *object;
}

void
CheckResult( TDCLDockCommand& command )
{
	auto* result = dynamic_cast<TDCLDockCmdSingleLong*>( &command );
	if ( command.GetCommand() != Cmd::kDResult || !result )
		throw std::runtime_error( "Expected Dock result" );
	auto error = static_cast<KSInt32>( result->GetLong() );
	if ( error )
		throw std::runtime_error( "Newton error " + std::to_string( error ) );
}
} // namespace

SoupDownloadSession::SoupDownloadSession( const std::string& soup, const std::filesystem::path& destination )
    : output( destination )
{
	if ( soup.empty() || soup.find( '\0' ) != std::string::npos )
		throw std::runtime_error( "Soup name must be nonempty and contain no NUL" );
	std::vector<KUInt16> utf16( soup.size() + 1 );
	size_t inputSize = soup.size();
	size_t length = utf16.size() - 1;
	auto conversion = UUTF16Conv::FromUTF8( soup.data(), &inputSize, utf16.data(), &length );
	if ( conversion != UUTF16Conv::kInputExhausted || inputSize != soup.size() )
		throw std::runtime_error( "Invalid UTF-8 soup name" );
	utf16[length] = 0;
	if ( length == 0 || length >= 25 )
		throw std::runtime_error( "Soup name must contain 1-24 UTF-16 code units" );
	soupName = TDCLNSRef::MakeString( utf16.data() );
	EnsureDirectory( output );
	// Reusing a directory starts a new export, not a resume. Invalidate old
	// completion markers, including stores no longer present on this Newton.
	std::filesystem::remove( output / "COMPLETE" );
	for ( const auto& entry : std::filesystem::directory_iterator( output ) )
	{
		if ( NumberedName( entry.path().filename().string(), "store-" ) && !entry.is_symlink() && entry.is_directory() )
			std::filesystem::remove( entry.path() / "COMPLETE" );
	}
	TDCLDockCmdNSObject name( Cmd::kDEntry, soupName );
	Save( output / "soup-name.nsof", name );
}

void
SoupDownloadSession::WriteText( const std::filesystem::path& path, const std::string& text )
{
	auto partial = path;
	partial += ".partial";
	std::ofstream file( partial, std::ios::binary );
	file.exceptions( std::ios::failbit | std::ios::badbit );
	file.write( text.data(), static_cast<std::streamsize>( text.size() ) );
	file.close();
	// Publish a completion marker only after its full contents were written.
	std::filesystem::rename( partial, path );
}

void
SoupDownloadSession::Save( const std::filesystem::path& path, TDCLDockCommand& command )
{
	auto& object = Object( command );
	std::ofstream file( path, std::ios::binary );
	file.exceptions( std::ios::failbit | std::ios::badbit );
	file.write( static_cast<const char*>( object.GetNSOFData() ), object.GetNSOFSize() );
	file.close();
}

void
SoupDownloadSession::Start( TDCLStream& pipe )
{
	if ( state != State::New )
		throw std::runtime_error( "Export already started" );
	TDCLDockCmdSingleLong( Cmd::kDSetVBOCompression, 0 ).SendCommand( &pipe );
	// cvbo only updates the Newton's setting; it does not return a dres.
	Send( Cmd::kDGetStoreNames, pipe );
	state = State::Stores;
}

void
SoupDownloadSession::NextStore( TDCLStream& pipe )
{
	if ( storeIndex == stores.ToArray().GetLength() )
	{
		if ( !exportedStores )
			throw std::runtime_error( "Soup not found on any store" );
		WriteText( output / "COMPLETE", "Exported stores: " + std::to_string( exportedStores ) + "\n" );
		state = State::Complete;
		return;
	}
	auto store = stores.ToArray().Get( storeIndex++ );
	if ( !store.IsFrame() )
		throw std::runtime_error( "Invalid store frame" );
	storeOutput = output / ( "store-" + Number( storeIndex ) );
	TDCLDockCmdNSObject( Cmd::kDSetCurrentStore, store ).SendCommand( &pipe );
	state = State::Store;
}

void
SoupDownloadSession::Handle( TDCLDockCommand& command, TDCLStream& pipe )
{
	const auto code = command.GetCommand();
	if ( code == Cmd::kDHello )
		return;
	if ( code == Cmd::kDOperationCanceled || code == Cmd::kDOperationCanceled2 )
	{
		Send( Cmd::kDOpCanceledAck, pipe );
		throw std::runtime_error( "Export canceled on Newton" );
	}
	if ( code == Cmd::kDDisconnect )
		throw std::runtime_error( "Newton disconnected before export completed" );
	// An error may replace any normal data response.
	if ( code == Cmd::kDResult )
		CheckResult( command );

	switch ( state )
	{
	case State::Stores:
		if ( code != Cmd::kDStoreNames )
			break;
		stores = Object( command ).GetObject();
		if ( !stores.IsArray() )
			throw std::runtime_error( "Invalid store list" );
		Save( output / "stores.nsof", command );
		NextStore( pipe );
		return;
	case State::Store:
		CheckResult( command );
		Send( Cmd::kDGetSoupNames, pipe );
		state = State::Names;
		return;
	case State::Names:
	{
		if ( code != Cmd::kDSoupNames )
			break;
		auto* response = dynamic_cast<TDCLDockCmdTwoNSObjects*>( &command );
		if ( !response )
			throw std::runtime_error( "Invalid soup names response" );
		auto objects = response->GetObjects();
		auto names = objects.ToArray().Get( 0 );
		if ( !names.IsArray() )
			throw std::runtime_error( "Invalid soup names array" );
		bool found = false;
		for ( KUInt32 i = 0; i < names.ToArray().GetLength(); ++i )
		{
			if ( names.ToArray().Get( i ).ToString().StrEqual( soupName.ToString() ) )
			{
				found = true;
				break;
			}
		}
		if ( !found )
		{
			NextStore( pipe );
			return;
		}
		if ( std::filesystem::is_symlink( storeOutput ) )
			throw std::runtime_error( "Store output directory must not be a symlink" );
		EnsureDirectory( storeOutput );
		// Counters restart at one. Remove only previous generated entry files
		// so a shorter or empty soup cannot retain stale higher-numbered entries.
		for ( const auto& entry : std::filesystem::directory_iterator( storeOutput ) )
		{
			if ( NumberedName( entry.path().filename().string(), "entry-", ".nsof" ) && !entry.is_directory() )
				std::filesystem::remove( entry.path() );
		}
		TDCLDockCmdNSObject store( Cmd::kDEntry, stores.ToArray().Get( storeIndex - 1 ) );
		Save( storeOutput / "store.nsof", store );
		TDCLDockCmdNSObject signatures( Cmd::kDEntry, objects.ToArray().Get( 1 ) );
		TDCLDockCmdNSObject soupNames( Cmd::kDEntry, names );
		Save( storeOutput / "soup-names.nsof", soupNames );
		Save( storeOutput / "soup-signatures.nsof", signatures );
		TDCLDockCmdCString( Cmd::kDSetCurrentSoup, soupName.ToString() ).SendCommand( &pipe );
		state = State::Soup;
		return;
	}
	case State::Soup:
		CheckResult( command );
		Send( Cmd::kDGetSoupInfo, pipe );
		state = State::Info;
		return;
	case State::Info:
		if ( code != Cmd::kDSoupInfo )
			break;
		Save( storeOutput / "soup-info.nsof", command );
		Send( Cmd::kDGetIndexDescription, pipe );
		state = State::Indexes;
		return;
	case State::Indexes:
		if ( code != Cmd::kDIndexDescription )
			break;
		Save( storeOutput / "indexes.nsof", command );
		entries = 0;
		Send( Cmd::kDSendSoup, pipe );
		state = State::Entries;
		return;
	case State::Entries:
		if ( code == Cmd::kDEntry )
		{
			Save( storeOutput / ( "entry-" + Number( ++entries ) + ".nsof" ), command );
			return;
		}
		if ( code != Cmd::kDBackupSoupDone )
			break;
		WriteText( storeOutput / "COMPLETE", "Entries: " + std::to_string( entries ) + "\n" );
		++exportedStores;
		NextStore( pipe );
		return;
	default:
		break;
	}
	throw std::runtime_error( "Unexpected Dock command " + std::to_string( code ) );
}
