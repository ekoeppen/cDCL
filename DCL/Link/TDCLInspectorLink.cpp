#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLInspectorLink.h>

// ANSI C
#include <stdlib.h>
#include <string.h>
#include <time.h>

// DCL
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Link/App_Commands/TDCLAppCmdFile.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdDeletePkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdEnterBreak.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdException.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdLoadPkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>

#define LOG(...) { printf( __VA_ARGS__ ); putchar( 10 ); }

TDCLInspectorLink::TDCLInspectorLink( TDCLApplication* inApplication )
	: TDCLLink( inApplication, TDCLDockCommand::kLoadPackage_SType )
{
	SetCommandMode( TDCLCommandFramer::EMode::kInspector );
	SetMNPKeepAlive( true );
}

TDCLInspectorLink::~TDCLInspectorLink( void )
{
}

void
TDCLInspectorLink::DoConnect( void )
{
	NotifyConnected();
	auto theNewtMessage = ReceiveCommand();
	if ( theNewtMessage->GetCommand() != TDCLInspectorCommand::kDRequestToConnect )
	{
		throw DCLUnexpDockCmd;
	}
	TDCLInspectorCommand response( TDCLInspectorCommand::kDConnectOk, 0 );
	SendCommand( &response );

	mNewtonName = (KUInt16*)::malloc( 2 );
	(void)::memcpy( (void*)mNewtonName, "\x00\x00", 2 );
}

void
TDCLInspectorLink::Connected( void )
{
	mApplication->ConnectedToNewtonDevice( this, mNewtonName );
	SetMNPKeepAlive( true );
}

TDCLLink::EState
TDCLInspectorLink::ProcessDockCommand( TDCLDockCommand* inCommand, Boolean* outProcessed )
{
	switch ( inCommand->GetCommand() )
	{
	case TDCLInspectorCommand::kDResult:
		break;
	case TDCLInspectorCommand::kDText:
	{
		auto* cmd = static_cast<TDCLInspectorCmdText*>( inCommand );
		LOG( "%.*s", cmd->GetLength(), (const char*)cmd->GetDataPtr() );
		break;
	}
	default:
		break;
	}
	*outProcessed = true;
	return kRunning;
}

TDCLLink::EState
TDCLInspectorLink::ProcessAppCommand( TDCLAppCommand* inCommand, Boolean* outProcessed )
{
	return TDCLLink::ProcessAppCommand( inCommand, outProcessed );
}
