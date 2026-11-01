// ==============================
// File: TPackageLoaderLink.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>
#include "TPackageLoaderLink.h"
#include "TPackageLoaderApp.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <cstring>
#include <thread>
#include <chrono>

#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/TDCLUserCancelledException.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>

std::string
TPackageLoaderLink::FormatCmd( KUInt32 inCmd )
{
	char str[5];
	str[0] = static_cast<char>( (inCmd >> 24) & 0xFF );
	str[1] = static_cast<char>( (inCmd >> 16) & 0xFF );
	str[2] = static_cast<char>( (inCmd >> 8) & 0xFF );
	str[3] = static_cast<char>( inCmd & 0xFF );
	str[4] = '\0';

	std::string name;
	switch (inCmd)
	{
		case TDCLDockCommand::kDRequestToDock:        name = "kDRequestToDock"; break;
		case TDCLDockCommand::kDInitiateDocking:      name = "kDInitiateDocking"; break;
		case TDCLDockCommand::kDNewtonName:           name = "kDNewtonName"; break;
		case TDCLDockCommand::kDDesktopInfo:          name = "kDDesktopInfo"; break;
		case TDCLDockCommand::kDNewtonInfo:           name = "kDNewtonInfo"; break;
		case TDCLDockCommand::kDWhichIcons:           name = "kDWhichIcons"; break;
		case TDCLDockCommand::kDSetTimeout:           name = "kDSetTimeout"; break;
		case TDCLDockCommand::kDPassword:             name = "kDPassword"; break;
		case TDCLDockCommand::kDResult:               name = "kDResult"; break;
		case TDCLDockCommand::kDLoadPackage:          name = "kDLoadPackage"; break;
		case TDCLDockCommand::kDDisconnect:           name = "kDDisconnect"; break;
		case TDCLDockCommand::kDOperationCanceled:    name = "kDOperationCanceled"; break;
		case TDCLDockCommand::kDOperationCanceled2:   name = "kDOperationCanceled2"; break;
		case TDCLDockCommand::kDOpCanceledAck:        name = "kDOpCanceledAck"; break;
		case TDCLDockCommand::kDHello:                name = "kDHello"; break;
		default:                                      name = "unknown"; break;
	}

	std::ostringstream oss;
	oss << "'" << str << "' (" << name << ")";
	return oss.str();
}

std::string
TPackageLoaderLink::ExplainError( KSInt32 inCode )
{
	switch (inCode)
	{
		case 0:      return "Success";
		case -28001: return "Unable to establish dock connection";
		case -28004: return "Operation cancelled by Newton";
		case -28005: return "Protocol error";
		case -28020: return "Session cancelled";
		case -28022: return "Bad password";
		case -28023: return "Retry password";
		case -48002: return "Package already installed";
		case -48004: return "Package incompatible";
		case -48005: return "Out of storage space on Newton";
		default:
		{
			std::ostringstream oss;
			oss << "Newton error code " << inCode;
			return oss.str();
		}
	}
}

TPackageLoaderLink::TPackageLoaderLink( TPackageLoaderApp* inApplication, bool inVerbose )
	:
		TDCLLoadPackageLink( inApplication ),
		mLoaderApp( inApplication ),
		mVerbose( inVerbose ),
		mLastPercent( -1 )
{
}

TPackageLoaderLink::~TPackageLoaderLink( void )
{
	WaitForThreadToStop( 1000 );
	mPipe = nullptr;
	if (mNewtonName)
	{
		::free( const_cast<KUInt16*>( mNewtonName ) );
		mNewtonName = nullptr;
	}
}

void
TPackageLoaderLink::WaitForThreadToStop( unsigned int inTimeoutMs )
{
	auto start = std::chrono::steady_clock::now();
	while (GetThreadState() != IDCLThreads::kStopped)
	{
		if (inTimeoutMs > 0)
		{
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - start ).count();
			if (elapsed >= (long long) inTimeoutMs)
			{
				break;
			}
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
	}
}

void
TPackageLoaderLink::DoDisconnect( void )
{
	if (mPipe)
	{
		mApplication->Disconnecting( this );
		if (mPipe->PeerDisconnected())
		{
			// The Newton already closed the link; don't send kDDisconnect.
			Log( "Newton already closed the link, not sending 'disc'." );
			return;
		}
		try
		{
			Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDDisconnect ) );
			TDCLDockCmdNoData theDisconnectMessage( TDCLDockCommand::kDDisconnect );
			theDisconnectMessage.SendCommand( mPipe );
			mPipe->FlushOutput();
		}
		catch (...)
		{
			// Connection may already be closed
		}
	}
}

void
TPackageLoaderLink::Log( const std::string& inMessage )
{
	if (mLoaderApp)
	{
		mLoaderApp->Log( inMessage );
	}
}

void
TPackageLoaderLink::LogCommand( const char* inDirection, TDCLDockCommand* inCommand )
{
	if (inCommand && mVerbose)
	{
		std::ostringstream oss;
		oss << "[DOCK " << inDirection << "] " << FormatCmd( inCommand->GetCommand() );
		if (inCommand->GetCommand() == TDCLDockCommand::kDResult)
		{
			TDCLDockCmdSingleLong* resCmd = (TDCLDockCmdSingleLong*) inCommand;
			KSInt32 code = (KSInt32) resCmd->GetLong();
			oss << " -> code " << code << " (" << ExplainError( code ) << ")";
		}
		Log( oss.str() );
	}
}

void
TPackageLoaderLink::DoConnect( void )
{
	// Inform application of the active pipe
	mApplication->Connected( this, mPipe->Connected( this ) );

	Log( "Waiting for Newton Dock greeting..." );

	TDCLDockCommand* msg = TDCLDockCommand::ReceiveCommand( mPipe );
	if (msg == nullptr)
	{
		throw DCLEOF;
	}
	LogCommand( "RECV", msg );

	KUInt32 cmd = msg->GetCommand();

	if (cmd == TDCLDockCommand::kDResult)
	{
		TDCLDockCmdSingleLong* resCmd = (TDCLDockCmdSingleLong*) msg;
		KSInt32 resCode = (KSInt32) resCmd->GetLong();
		delete msg;
		std::string explanation = ExplainError( resCode );
		Log( "Newton sent unexpected result: " + explanation );
		if (resCode == -28004 || resCode == -28020)
		{
			throw TDCLUserCancelledException( __FILE__, __LINE__ );
		}
		throw TDCLNewtonException( __FILE__, __LINE__, resCode );
	}
	else if (cmd == TDCLDockCommand::kDOperationCanceled || cmd == TDCLDockCommand::kDOperationCanceled2)
	{
		delete msg;
		Log( "Newton cancelled the operation, acknowledging..." );
		TDCLDockCmdNoData ack( TDCLDockCommand::kDOpCanceledAck );
		ack.SendCommand( mPipe );
		mPipe->FlushOutput();
		throw TDCLUserCancelledException( __FILE__, __LINE__ );
	}
	else if (cmd == TDCLDockCommand::kDDisconnect)
	{
		delete msg;
		Log( "Newton requested disconnect." );
		throw TDCLUserCancelledException( __FILE__, __LINE__ );
	}
	else if (cmd != TDCLDockCommand::kDRequestToDock)
	{
		delete msg;
		throw DCLUnexpDockCmd;
	}
	delete msg;

	// Send kDInitiateDocking with session type kLoadPackage_SType
	Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDInitiateDocking ) + " (session type: kLoadPackage)" );
	TDCLDockCmdSingleLong initMsg( TDCLDockCommand::kDInitiateDocking, mType );
	initMsg.SendCommand( mPipe );
	mPipe->FlushOutput();

	// Await kDNewtonName, handling any retransmitted greetings or cancellation
	while (true)
	{
		msg = TDCLDockCommand::ReceiveCommand( mPipe );
		if (msg == nullptr)
		{
			throw DCLEOF;
		}
		LogCommand( "RECV", msg );
		cmd = msg->GetCommand();

		if (cmd == TDCLDockCommand::kDRequestToDock)
		{
			delete msg;
			Log( "Retransmitting kDInitiateDocking..." );
			initMsg.SendCommand( mPipe );
			mPipe->FlushOutput();
			continue;
		}
		else if (cmd == TDCLDockCommand::kDHello)
		{
			delete msg;
			continue;
		}
		else if (cmd == TDCLDockCommand::kDResult)
		{
			TDCLDockCmdSingleLong* resCmd = (TDCLDockCmdSingleLong*) msg;
			KSInt32 resCode = (KSInt32) resCmd->GetLong();
			delete msg;
			throw TDCLNewtonException( __FILE__, __LINE__, resCode );
		}
		else if (cmd == TDCLDockCommand::kDNewtonName)
		{
			break;
		}
		else
		{
			delete msg;
			throw DCLUnexpDockCmd;
		}
	}

	// Populate device information from kDNewtonName
	TDCLDockCmdNewtonName* newtName = (TDCLDockCmdNewtonName*) msg;
	mNewtonID         = newtName->GetNewtonUniqueID();
	mManufacturer     = newtName->GetManufacturer();
	mMachineType      = newtName->GetMachineType();
	mROMVersion       = newtName->GetROMVersion();
	mROMStage         = newtName->GetROMStage();
	mRAMSize          = newtName->GetRAMSize();
	mScreenHeight     = newtName->GetScreenHeight();
	mScreenWidth      = newtName->GetScreenWidth();
	mSystemUpdate     = newtName->GetSystemUpdate();
	mNOSVersion       = newtName->GetNOSVersion();
	mInternalStoreSig = newtName->GetInternalStoreSig();
	mVertScreenRes    = newtName->GetVertScreenRes();
	mHoriScreenRes    = newtName->GetHoriScreenRes();
	mScreenDepth      = newtName->GetScreenDepth();

	KUInt32 nameSize = (newtName->GetNameLength() + 1) * sizeof(KUInt16);
	mNewtonName = (KUInt16*) ::malloc( nameSize );
	if (mNewtonName == nullptr)
	{
		delete msg;
		throw DCLMemError;
	}
	(void) ::memcpy( (void*) mNewtonName, newtName->GetNewtonName(), nameSize );
	delete msg;

	// Send kDSetTimeout
	long theTimeout = mPipe->GetTimeout();
	if (theTimeout < 0)
	{
		theTimeout = kDefaultTimeout;
	}
	Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDSetTimeout ) + " (" + std::to_string( theTimeout ) + "s)" );
	TDCLDockCmdSingleLong setTimeoutMsg( TDCLDockCommand::kDSetTimeout, (KUInt32) theTimeout );
	setTimeoutMsg.SendCommand( mPipe );
	mPipe->FlushOutput();

	// Await kDResult from SetTimeout
	msg = TDCLDockCommand::ReceiveCommand( mPipe );
	if (msg == nullptr)
	{
		throw DCLEOF;
	}
	LogCommand( "RECV", msg );
	if (msg->GetCommand() != TDCLDockCommand::kDResult)
	{
		delete msg;
		throw DCLUnexpDockCmd;
	}
	TDCLDockCmdSingleLong* resultCmd = (TDCLDockCmdSingleLong*) msg;
	KSInt32 resultVal = (KSInt32) resultCmd->GetLong();
	delete msg;
	if (resultVal != 0)
	{
		throw TDCLNewtonException( __FILE__, __LINE__, resultVal );
	}

	Log( "Dock connection established successfully." );
}

void
TPackageLoaderLink::StartInstallingPackage( void )
{
	mLastPercent = -1;
	std::cout << "Transferring package to Newton..." << std::endl;
}

void
TPackageLoaderLink::InstallationProgress( double inProgress )
{
	int percent = (int)(inProgress * 100.0 + 0.5);
	if (percent > 100) percent = 100;
	if (percent < 0)   percent = 0;
	if (percent == mLastPercent) return;
	mLastPercent = percent;

	const int barWidth = 40;
	int pos = (int)(barWidth * inProgress);
	if (pos > barWidth) pos = barWidth;

	std::cout << "\r[";
	for (int i = 0; i < barWidth; ++i)
	{
		if (i < pos)       std::cout << "=";
		else if (i == pos) std::cout << ">";
		else               std::cout << " ";
	}
	std::cout << "] " << std::setw(3) << percent << "%" << std::flush;
	if (percent >= 100)
	{
		std::cout << std::endl;
	}
}

TDCLLink::EState
TPackageLoaderLink::ProcessDockCommand(
		TDCLDockCommand* inCommand,
		Boolean* outProcessed )
{
	LogCommand( "RECV (in session)", inCommand );
	TDCLLink::EState theResult = kRunning;

	switch (inCommand->GetCommand())
	{
		case TDCLDockCommand::kDDisconnect:
			Log( "Newton requested disconnect." );
			theResult = kDisconnected;
			*outProcessed = true;
			break;

		case TDCLDockCommand::kDOperationCanceled:
		case TDCLDockCommand::kDOperationCanceled2:
		{
			Log( "Newton cancelled the operation, acknowledging..." );
			TDCLDockCmdNoData ack( TDCLDockCommand::kDOpCanceledAck );
			ack.SendCommand( GetPipe() );
			GetPipe()->FlushOutput();
			theResult = kDisconnected;
			*outProcessed = true;
			break;
		}

		case TDCLDockCommand::kDHello:
			*outProcessed = true;
			break;

		default:
			*outProcessed = false;
			break;
	}

	return theResult;
}
