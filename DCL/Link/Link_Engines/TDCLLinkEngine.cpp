// ==============================
// Fichier:			TDCLLinkEngine.cp
// Projet:			Desktop Connection Library
//
// Créé le:			10/4/2003
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
// The Original Code is TDCLLinkEngine.cp.
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
// $Id: TDCLLinkEngine.cp,v 1.3 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLLinkEngine.h>

// -------------------------------------------------------------------------- //
//  * TDCLLinkEngine( TDCLFullDockLink* )
// -------------------------------------------------------------------------- //
TDCLLinkEngine::TDCLLinkEngine( TDCLFullDockLink* inLink )
	:
		mLink( inLink )
{
	// Kilroy was here.
}

// -------------------------------------------------------------------------- //
//  * ~TDCLLinkEngine( void )
// -------------------------------------------------------------------------- //
TDCLLinkEngine::~TDCLLinkEngine( void )
{
	// This space for rent.
}

// -------------------------------------------------------------------------- //
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLLinkEngine::ProcessDockCommand(
					TDCLDockCommand* /* inCommand */,
					Boolean* outProcessed )
{
	*outProcessed = false;
	return true;
}

// -------------------------------------------------------------------------- //
//  * ProcessAppCommand( TDCLAppCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLLinkEngine::ProcessAppCommand(
					TDCLAppCommand* /* inCommand */,
					Boolean* outProcessed )
{
	*outProcessed = false;
	return true;
}

// -------------------------------------------------------------------------- //
//  * WorkIsOver( void )
// -------------------------------------------------------------------------- //
void
TDCLLinkEngine::WorkIsOver( void )
{
	// This space for rent.
}

// --------------------------------------------------------------------------------	//
//  * GetAppCommandMask( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLLinkEngine::GetAppCommandMask( void )
{
	return (KUInt32) (TDCLEvent::kAnyAppCommandMask &~ TDCLEvent::kIdleAppCommand);
}

// ======================================================================= //
// Real programmers don't draw flowcharts.  Flowcharts are, after all, the //
// illiterate's form of documentation.  Cavemen drew flowcharts; look how  //
// much good it did them.                                                  //
// ======================================================================= //
