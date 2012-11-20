// ==============================
// Fichier:			TDCLNSFunction.cp
// Projet:			Desktop Connection Library
// 
// CrŽŽ le:			8/4/2004
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
// The Original Code is TDCLNSFunction.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLNSFunction.cp,v 1.3 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSFunction.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLNSFunction( void )
// -------------------------------------------------------------------------- //
TDCLNSFunction::TDCLNSFunction( void )
{
	SetIsFunction( true );
	SetKeysAreSorted( false );

	Set( KDCLSYM::kSYMclass, TDCLNSRef::kNewFuncClass );
	Set( KDCLSYM::kSYMinstructions, TDCLNSRef::kNILREF );
	Set( KDCLSYM::kSYMliterals, TDCLNSRef::kNILREF );
	Set( KDCLSYM::kSYMargFrame, TDCLNSRef::kNILREF );
	Set( KDCLSYM::kSYMnumArgs, TDCLNSRef::MakeInt( 0 ) );
}

// ============================================ //
// You scratch my tape, and I'll scratch yours. //
// ============================================ //
