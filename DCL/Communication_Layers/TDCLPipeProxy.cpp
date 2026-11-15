// ==============================
// Fichier:			TDCLPipeProxy.cp
// Projet:			Desktop Connection Library
//
// Créé le:			5/4/2003
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
// The Original Code is TDCLPipeProxy.cp.
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
// $Id: TDCLPipeProxy.cp,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipeProxy.h>

// -------------------------------------------------------------------------- //
//  * TDCLPipeProxy( TDCLPipe* )
// -------------------------------------------------------------------------- //
TDCLPipeProxy::TDCLPipeProxy( TDCLPipe* inSubPipe )
	:
		TDCLPipe( inSubPipe->GetCommLayer() ),
		mSubPipe( inSubPipe )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPipeProxy( void )
// -------------------------------------------------------------------------- //
TDCLPipeProxy::~TDCLPipeProxy( void )
{
	if (mSubPipe)
	{
		delete mSubPipe;
	}
}

// -------------------------------------------------------------------------- //
//  * BytesAvailable( void )
// -------------------------------------------------------------------------- //
Boolean
TDCLPipeProxy::BytesAvailable( void )
{
	return mSubPipe->BytesAvailable();
}

// -------------------------------------------------------------------------- //
//  * Disconnect( void )
// -------------------------------------------------------------------------- //
void
TDCLPipeProxy::Disconnect( void )
{
	mSubPipe->Disconnect();
}

// -------------------------------------------------------------------------- //
//  * Idle( void )
// -------------------------------------------------------------------------- //
void
TDCLPipeProxy::Idle( void )
{
	mSubPipe->Idle();
}

// -------------------------------------------------------------------------- //
//  * GetTimeout( void )
// -------------------------------------------------------------------------- //
long
TDCLPipeProxy::GetTimeout( void )
{
	return mSubPipe->GetTimeout();
}

// -------------------------------------------------------------------------- //
//  * SetTimeout( long )
// -------------------------------------------------------------------------- //
void
TDCLPipeProxy::SetTimeout( long inTimeout )
{
	mSubPipe->SetTimeout( inTimeout );
}

// -------------------------------------------------------------------------- //
//  * PeekByte( void )
// -------------------------------------------------------------------------- //
KUInt8
TDCLPipeProxy::PeekByte( void )
{
	return mSubPipe->PeekByte();
}

// -------------------------------------------------------------------------- //
//  * Connected( TDCLLink* )
// -------------------------------------------------------------------------- //
TDCLCommLayer*
TDCLPipeProxy::Connected( TDCLLink* inLink )
{
	return mSubPipe->Connected( inLink );
}

// -------------------------------------------------------------------------- //
//  * ClearDataPresent( void )
// -------------------------------------------------------------------------- //
void
TDCLPipeProxy::ClearDataPresent( void )
{
	mSubPipe->ClearDataPresent();
}

// ============================ //
// <<<<< EVACUATION ROUTE <<<<< //
// ============================ //
