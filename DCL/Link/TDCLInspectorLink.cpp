// ==============================
// File:			TDCLInspectorLink.cpp
// Project:			Desktop Connection Library
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is TDCLInspectorLink.cpp.
//
// The Initial Developer of the Original Code is Eckhart Köppen
// Copyright (C) 2007 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Köppen <eck@users.sourceforce.net> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLInspectorLink.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>

// K
#include <K/Misc/TSmartPtr.h>

// DCL
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Communication_Layers/TDCLMNPPipe.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Link/App_Commands/TDCLAppCmdFile.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdDeletePkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdEnterBreak.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdException.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExitBreak.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdLoadPkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>

#define LOG(...) {printf(__VA_ARGS__); putchar(10);}
#define LOGFN() {puts(__PRETTY_FUNCTION__);}

TDCLInspectorLink::TDCLInspectorLink( TDCLApplication* inApplication ):
	TDCLLink( inApplication, TDCLDockCommand::kLoadPackage_SType )
{
}

TDCLInspectorLink::~TDCLInspectorLink( void )
{
}

void TDCLInspectorLink::DoConnect( void )
{
	// LOGFN();
	GetApplication()->Connected( this, GetPipe()->Connected( this ) );	
	TSmartPtr<TDCLInspectorCommand> theNewtMessage;
	{
		theNewtMessage = TDCLInspectorCommand::ReceiveCommand( GetPipe() );
		if ( theNewtMessage->GetCommand() != TDCLInspectorCommand::kDRequestToConnect )	{
			throw DCLUnexpDockCmd;
		} else {
			TDCLInspectorCommand response( TDCLInspectorCommand::kDConnectOk, 0 );
			response.SendCommand( GetPipe() );
		}
		theNewtMessage.Delete();
	}

	// we don't have a name in the Inspector protocol, reset to an empty string
	mNewtonName = (KUInt16*) ::malloc( 2 );
	(void) ::memcpy( (void*) mNewtonName, "\x00\x00", 2 );
}

void TDCLInspectorLink::Connected( void )
{
	TSmartPtr<TDCLInspectorCommand> theNewtMessage;
	Boolean processed;

	// LOGFN();
	mApplication->ConnectedToNewtonDevice( this, mNewtonName );
	((TDCLMNPPipe *) GetPipe())->SetKeepAlive( true );
	while ( true )
	{
		theNewtMessage = TDCLInspectorCommand::ReceiveCommand( GetPipe() );
		ProcessDockCommand( theNewtMessage.Get(), &processed );
		theNewtMessage.Delete();
	}
}

TDCLLink::EState TDCLInspectorLink::ProcessDockCommand(
	TDCLDockCommand* inCommand,
	Boolean* outProcessed )
{
    TDCLInspectorCmdText* cmd;
    
	// LOGFN();
	switch ( inCommand->GetCommand() )
	{
		case TDCLInspectorCommand::kDResult:
			break;
		case TDCLInspectorCommand::kDText:
			cmd = (TDCLInspectorCmdText*) inCommand;
			LOG("%.*s", cmd->GetLength(), (char *) cmd->GetDataPtr());
			break;
		default:
			break;
	}
	TDCLLink::EState r = kRunning;
	*outProcessed = true;
	return r;
}

TDCLLink::EState TDCLInspectorLink::ProcessAppCommand(
	TDCLAppCommand* inCommand,
	Boolean* outProcessed )
{
	// LOGFN();
	TDCLLink::EState r = kRunning;
	r = TDCLLink::ProcessAppCommand( inCommand, outProcessed );
	return r;
}

TDCLDockCommand* TDCLInspectorLink::ReceiveCommand( void )
{
	// LOGFN();
	return TDCLInspectorCommand::ReceiveCommand( GetPipe() );
}
