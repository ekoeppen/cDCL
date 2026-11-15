// ==============================
// Fichier:			TDCLPipe.cp
// Projet:			Desktop Connection Library
//
// Créé le:			26/10/2002
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
// The Original Code is TDCLPipe.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPipe.cp,v 1.4 2004/11/24 14:08:53 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipe.h>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>

// ------------------------------------------------------------------------- //
//  * TDCLPipe( TDCLCommLayer*, long )
// ------------------------------------------------------------------------- //
TDCLPipe::TDCLPipe(
		TDCLCommLayer*	inCommLayer )
		:
			TDCLStream(),
			mCommLayer( inCommLayer ),
			mLink( nil )
{
	// Défense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLPipe::FlushOutput( void )
{
	// Loi du 28 juillet 1881.
}

// ------------------------------------------------------------------------- //
//  * Disconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLPipe::Disconnect( void )
{
	mCommLayer->Disconnected( this );

	mLink = nil;
}

// ------------------------------------------------------------------------- //
//  * Idle( void )
// ------------------------------------------------------------------------- //
void
TDCLPipe::Idle( void )
{
	mCommLayer->Idle();
}

// ------------------------------------------------------------------------- //
//  * PeekByte( void )
// ------------------------------------------------------------------------- //
KUInt8
TDCLPipe::PeekByte( void )
{
	throw DCLPosition;
}

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink* )
// ------------------------------------------------------------------------- //
TDCLCommLayer*
TDCLPipe::Connected( TDCLLink* inLink )
{
	mLink = inLink;

	return mCommLayer;
}

// ===================================== //
// You're using a keyboard!  How quaint! //
// ===================================== //
