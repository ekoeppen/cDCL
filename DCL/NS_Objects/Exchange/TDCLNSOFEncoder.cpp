// ==============================
// Fichier:			TDCLNSOFEncoder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			15/10/2002
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
// The Original Code is TDCLNSOFEncoder.cp.
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
// $Id: TDCLNSOFEncoder.cp,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSOFEncoder( TDCLStream* )
// ------------------------------------------------------------------------- //
TDCLNSOFEncoder::TDCLNSOFEncoder( TDCLStream* inStream )
	:
		TDCLNSEncoder( inStream )
{
	// J'indique la version.
	inStream->PutByte( kNSOFVersion );
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSOFEncoder( void )
// ------------------------------------------------------------------------- //
TDCLNSOFEncoder::~TDCLNSOFEncoder( void )
{
	// Défense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * PutObject( const TDCLNSObject*, KUInt32, KUIntPtr* )
// ------------------------------------------------------------------------- //
void
TDCLNSOFEncoder::PutObject(
					const TDCLNSObject*	inObject,
					KUInt32 /* inObjectID */,
					KUIntPtr* /* outCookie */ )
{
	inObject->ToNSOF( this );
}

// ------------------------------------------------------------------------- //
//  * PutPrecedent( KUInt32, KUIntPtr )
// ------------------------------------------------------------------------- //
void
TDCLNSOFEncoder::PutPrecedent( KUInt32 inRefID, KUIntPtr /* inCookie */ )
{
	// On met le type precedent.
	GetOutputStream()->PutByte( KDCLNSOF::kPrecedent );
	
	// Ensuite, la référence.
	GetOutputStream()->PutXLong( inRefID );
}

// ------------------------------------------------------------------------- //
//  * PutRef( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSOFEncoder::PutRef( const TDCLNSRef& inRef )
{
	inRef.ToNSOF( this );
}

// ================================================== //
// ASCII a stupid question, you get an EBCDIC answer. //
// ================================================== //
