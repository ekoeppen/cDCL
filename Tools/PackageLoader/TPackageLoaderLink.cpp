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
#include <memory>
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
	(void) RequestStop();
	(void) Wait();
}

void
TPackageLoaderLink::DoDisconnect( void )
{
	if (GetPipe())
	{
		if (!StopRequested() && !GetPipe()->PeerDisconnected())
		{
			Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDDisconnect ) );
		}
		try
		{
			TDCLLink::DoDisconnect();
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
	GetApplication()->Connected( this, GetPipe()->Connected( this ) );

	Log( "Waiting for Newton Dock greeting..." );

	std::unique_ptr<TDCLDockCommand> msg( TDCLDockCommand::ReceiveCommand( GetPipe() ) );
	if (!msg)
	{
		throw DCLEOF;
	}
	LogCommand( "RECV", msg.get() );

	KUInt32 cmd = msg->GetCommand();

	if (cmd == TDCLDockCommand::kDResult)
	{
		TDCLDockCmdSingleLong* resCmd = static_cast<TDCLDockCmdSingleLong*>( msg.get() );
		KSInt32 resCode = (KSInt32) resCmd->GetLong();
		std::string explanation = ExplainError( resCode );
		Log( "Newton sent unexpected result: " + explanation );
		if (resCode == -28004 || resCode == -28020)
		{
			throw DCLUserCancelled;
		}
		throw DCLNewton( resCode );
	}
	else if (cmd == TDCLDockCommand::kDOperationCanceled || cmd == TDCLDockCommand::kDOperationCanceled2)
	{
		Log( "Newton cancelled the operation, acknowledging..." );
		TDCLDockCmdNoData ack( TDCLDockCommand::kDOpCanceledAck );
		ack.SendCommand( GetPipe() );
		GetPipe()->FlushOutput();
		throw DCLUserCancelled;
	}
	else if (cmd == TDCLDockCommand::kDDisconnect)
	{
		Log( "Newton requested disconnect." );
		throw DCLUserCancelled;
	}
	else if (cmd != TDCLDockCommand::kDRequestToDock)
	{
		throw DCLUnexpDockCmd;
	}

	// Send kDInitiateDocking with session type kLoadPackage_SType
	Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDInitiateDocking ) + " (session type: kLoadPackage)" );
	TDCLDockCmdSingleLong initMsg( TDCLDockCommand::kDInitiateDocking, mType );
	initMsg.SendCommand( GetPipe() );
	GetPipe()->FlushOutput();

	// Await kDNewtonName, handling any retransmitted greetings or cancellation
	while (true)
	{
		msg.reset( TDCLDockCommand::ReceiveCommand( GetPipe() ) );
		if (!msg)
		{
			throw DCLEOF;
		}
		LogCommand( "RECV", msg.get() );
		cmd = msg->GetCommand();

		if (cmd == TDCLDockCommand::kDRequestToDock)
		{
			Log( "Retransmitting kDInitiateDocking..." );
			initMsg.SendCommand( GetPipe() );
			GetPipe()->FlushOutput();
			continue;
		}
		else if (cmd == TDCLDockCommand::kDHello)
		{
			continue;
		}
		else if (cmd == TDCLDockCommand::kDResult)
		{
			TDCLDockCmdSingleLong* resCmd = static_cast<TDCLDockCmdSingleLong*>( msg.get() );
			KSInt32 resCode = (KSInt32) resCmd->GetLong();
			throw DCLNewton( resCode );
		}
		else if (cmd == TDCLDockCommand::kDNewtonName)
		{
			break;
		}
		else
		{
			throw DCLUnexpDockCmd;
		}
	}

	// Populate device information from kDNewtonName
	TDCLDockCmdNewtonName* newtName = static_cast<TDCLDockCmdNewtonName*>( msg.get() );
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
		throw DCLMemError;
	}
	(void) ::memcpy( (void*) mNewtonName, newtName->GetNewtonName(), nameSize );
	msg.reset();

	// Send kDSetTimeout
	long theTimeout = GetPipe()->GetTimeout();
	if (theTimeout < 0)
	{
		theTimeout = kDefaultTimeout;
	}
	Log( std::string( "[DOCK SEND] " ) + FormatCmd( TDCLDockCommand::kDSetTimeout ) + " (" + std::to_string( theTimeout ) + "s)" );
	TDCLDockCmdSingleLong setTimeoutMsg( TDCLDockCommand::kDSetTimeout, (KUInt32) theTimeout );
	setTimeoutMsg.SendCommand( GetPipe() );
	GetPipe()->FlushOutput();

	// Await kDResult from SetTimeout
	msg.reset( TDCLDockCommand::ReceiveCommand( GetPipe() ) );
	if (!msg)
	{
		throw DCLEOF;
	}
	LogCommand( "RECV", msg.get() );
	if (msg->GetCommand() != TDCLDockCommand::kDResult)
	{
		throw DCLUnexpDockCmd;
	}
	TDCLDockCmdSingleLong* resultCmd = static_cast<TDCLDockCmdSingleLong*>( msg.get() );
	KSInt32 resultVal = (KSInt32) resultCmd->GetLong();
	msg.reset();
	if (resultVal != 0)
	{
		throw DCLNewton( resultVal );
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
		bool* outProcessed )
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
			GetApplication()->Cancel( this );
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
