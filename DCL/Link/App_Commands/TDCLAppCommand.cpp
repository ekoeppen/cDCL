// ==============================
// Fichier:			TDCLAppCommand.cp
// Projet:			Desktop Connection Library
//
// Créé le:			2/2/2003
// Tabulation:		4 espaces
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
// The Original Code is TDCLAppCommand.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLAppCommand.cp,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>

// -------------------------------------------------------------------------- //
//  * TDCLAppCommand( void )
// -------------------------------------------------------------------------- //
TDCLAppCommand::TDCLAppCommand(
				EEventKind inCommandKind,
				EEventID inCommandID,
				void* inData /* = nil */ )
	:
		TDCLEvent( inCommandKind, inCommandID, inData )
{
}

// ======================================= //
// Building translators is good clean fun. //
//                 -- T. Cheatham          //
// ======================================= //
