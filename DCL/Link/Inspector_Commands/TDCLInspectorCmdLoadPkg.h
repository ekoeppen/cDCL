// ==============================
// File:			TDCLInspectorCmdLoadPkg.h
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
// The Original Code is TDCLInspectorCmdLoadPkg.h.
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

#ifndef __TDCLINSPECTORCMDLOADPKG__
#define __TDCLINSPECTORCMDLOADPKG__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>

class TDCLInspectorCmdLoadPkg: public TDCLInspectorCommand
{
public:
									TDCLInspectorCmdLoadPkg( KUInt32 inLength = 0, const void* inData = nil );
};

#endif
