// ==============================
// Fichier:			TDCLTextEncoder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			16/10/2002
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
// The Original Code is TDCLTextEncoder.cp.
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
// $Id: TDCLTextEncoder.cp,v 1.4 2004/11/24 14:09:06 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLTextEncoder.h>

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLTextEncoder( void )
// ------------------------------------------------------------------------- //
TDCLTextEncoder::TDCLTextEncoder( TDCLStream* inStream )
	:
		TDCLNSEncoder( inStream )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLTextEncoder( void )
// ------------------------------------------------------------------------- //
TDCLTextEncoder::~TDCLTextEncoder( void )
{
}

// ------------------------------------------------------------------------- //
//  * PutObject( const TDCLNSObject*, KUInt32, KUIntPtr* )
// ------------------------------------------------------------------------- //
void
TDCLTextEncoder::PutObject(
					const TDCLNSObject*	inObject,
					KUInt32 /* inObjectID */,
					KUIntPtr* /* outCookie */ )
{
	inObject->ToText( this );
}

// ------------------------------------------------------------------------- //
//  * PutPrecedent( KUInt32, KUIntPtr )
// ------------------------------------------------------------------------- //
void
TDCLTextEncoder::PutPrecedent( KUInt32 inRefID, KUIntPtr /* inCookie */ )
{
	// Le format est <<ID>>.
	// La taille maximale est 14 octets.
	char precedentStr[15];
	(void) ::sprintf( precedentStr, "<<%lu>>", (unsigned long) inRefID );

	GetOutputStream()->PutString( precedentStr );
}

// ------------------------------------------------------------------------- //
//  * PutRef( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLTextEncoder::PutRef( const TDCLNSRef& inRef )
{
	inRef.ToText( this );
}

// ------------------------------------------------------------------------- //
//  * CanHavePrecedentID( TDCLNSObject* )
// ------------------------------------------------------------------------- //
Boolean
TDCLTextEncoder::CanHavePrecedentID( TDCLNSObject* inObject )
{
	return !(inObject->IsSymbol());
}

// ===================================================================== //
// skldfjkljklsR%^&(IXDRTYju187pkasdjbasdfbuil                        //
// h;asvgy8p       23r1vyui135    2                                     //
// kmxsij90TYDFS$$b        jkzxdjkl bjnk ;j        nk;<[][;-==-<<<<<';[, //
//                 [hjioasdvbnuio;buip^&(FTSD$%*VYUI:buio;sdf}[asdf']    //
//                                 sdoihjfh(_YU*G&F^*CTY98y              //
//                                                                       //
//                                                                       //
// Now look what you've gone and done!  You've broken it!                //
// ===================================================================== //
