// ==============================
// File:			TDCLInspectorCommand.h
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
// The Original Code is TDCLInspectorCommand.h.
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

#ifndef __TDCLINSPECTORCOMMAND__
#define __TDCLINSPECTORCOMMAND__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>

class TDCLStream;
class TDCLPipe;

typedef void (*ProgressFuncPtr)(void* inRefCon, double inProgress);

class TDCLInspectorCommand: public TDCLDockCmdGeneric
{
public:
	enum {
		// Newton -> Desktop
		kDRequestToConnect			= FOUR_CHAR_CODE('cnnt'),
		kDStackFrame				= FOUR_CHAR_CODE('fstk'),
		kDText						= FOUR_CHAR_CODE('text'),
		kDException					= FOUR_CHAR_CODE('eerr'),
		kDExceptionRef				= FOUR_CHAR_CODE('eref'),
		kDCode						= FOUR_CHAR_CODE('code'),
		kDCodeBlock					= FOUR_CHAR_CODE('lscb'),

		// Desktop -> Newton
		kDConnectOk					= FOUR_CHAR_CODE('okln'),
		kDEnterBreakloop			= FOUR_CHAR_CODE('eext'),
		kDExitBreakloop				= FOUR_CHAR_CODE('bext'),
		kDLoadPackage				= FOUR_CHAR_CODE('pkg '),
		kDDeletePackage				= FOUR_CHAR_CODE('pkgX'),
		
		// Desktop -> Newton -> Desktop
		kDFunctionObject			= FOUR_CHAR_CODE('fobj'),
		kDResult					= FOUR_CHAR_CODE('rslt'),
	};

									TDCLInspectorCommand( KUInt32 inCommand, KUInt32 inLength = 0, const void* inData = nil );

	KUInt32							SendHeader( TDCLStream* inPipe );
	static TDCLInspectorCommand*	ReceiveCommand( TDCLStream* inPipe, ProgressFuncPtr inProgressFuncPtr = nil, void* inRefCon = nil );
	void							SendCommand( TDCLStream* inPipe, ProgressFuncPtr inProgressFuncPtr = nil, void* inRefCon = nil );
};

#endif
