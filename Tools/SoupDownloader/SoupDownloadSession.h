#pragma once

#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <filesystem>
#include <string>

class TDCLDockCommand;
class TDCLStream;

// One response at a time; transport and worker ownership belong to the link.
// Entries are written verbatim, without decoding or accumulating a whole soup.
class SoupDownloadSession
{
public:
	SoupDownloadSession( const std::string& soup, const std::filesystem::path& output );
	void Start( TDCLStream& pipe );
	void Handle( TDCLDockCommand& command, TDCLStream& pipe );
	bool
	Complete() const
	{
		return state == State::Complete;
	}
	unsigned
	ExportedStores() const
	{
		return exportedStores;
	}

private:
	enum class State
	{
		New,
		Stores,
		Store,
		Names,
		Soup,
		Info,
		Indexes,
		Entries,
		Complete
	};
	void NextStore( TDCLStream& pipe );
	void Save( const std::filesystem::path& path, TDCLDockCommand& command );
	void WriteText( const std::filesystem::path& path, const std::string& text );

	State state = State::New;
	TDCLNSRef soupName;
	TDCLNSRef stores;
	std::filesystem::path output;
	std::filesystem::path storeOutput;
	KUInt32 storeIndex = 0;
	unsigned exportedStores = 0;
	unsigned entries = 0;
};
