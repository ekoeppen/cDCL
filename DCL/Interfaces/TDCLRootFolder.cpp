// ==============================
// Fichier:			TDCLRootFolder.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			10/1/2003
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
// The Original Code is TDCLRootFolder.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLRootFolder.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLRootFolder.h>

// -------------------------------------------------------------------------- //
// TDCLRootFolder( IDCLFiles* )
// -------------------------------------------------------------------------- //
TDCLRootFolder::TDCLRootFolder( IDCLFiles* inFilesIntf )
	:
		TDCLFolder( inFilesIntf )
{
}

// -------------------------------------------------------------------------- //
// GetPath( void )
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLRootFolder::GetPath( void )
{
	return TDCLNSRef::MakeArray( 1, ToFrame() );
}

// -------------------------------------------------------------------------- //
// GetParentFolder( void )
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLRootFolder::GetParentFolder( void )
{
	return TDCLFSItemRef();
}

// -------------------------------------------------------------------------- //
// MakeParentFolder( void ) const
// -------------------------------------------------------------------------- //
TDCLFSItemRef
TDCLRootFolder::MakeParentFolder( void ) const
{
	return TDCLFSItemRef();
}

// ===================================================================== //
// Much of the excitement we get out of our work is that we don't really //
// know what we are doing.                                               //
//                 -- E. Dijkstra                                        //
// ===================================================================== //
