// ==============================
// File:			TDCLInspectorCmdStackFrame.cpp
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
// The Original Code is TDCLInspectorCmdStackFrame.cpp.
//
// The Initial Developer of the Original Code is Eckhart K�ppen
// Copyright (C) 2007 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Eckhart K�ppen <eck@users.sourceforce.net> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Streams/TDCLStream.h>

#undef KDEBUG_ENABLED
#define KDEBUG_ENABLED 1

#define LOG(...) {printf(__VA_ARGS__); putchar(10);}
#define LOGFN() puts(__PRETTY_FUNCTION__);

TDCLInspectorCmdStackFrame::TDCLInspectorCmdStackFrame(	KUInt32 inLength, const void* inData ):
	TDCLInspectorCommand( kDStackFrame, inLength, inData )
{
}
