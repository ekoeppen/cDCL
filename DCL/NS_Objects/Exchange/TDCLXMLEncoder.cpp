// ==============================
// Fichier:			TDCLXMLEncoder.cp
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
// The Original Code is TDCLXMLEncoder.cp.
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
// $Id: TDCLXMLEncoder.cp,v 1.5 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLStream.h>

// ANSI C
#include <stdio.h>

// ------------------------------------------------------------------------- //
//  * Constantes
// ------------------------------------------------------------------------- //
const char*	TDCLXMLEncoder::kXMLHeader
			= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
const char*	TDCLXMLEncoder::kDOCTYPEElement
			= "<!DOCTYPE frame PUBLIC \"-//DCL Group//Canonical DTD 1.0//EN\" \"http://www.kallisys.com/DTDs/newtonscript.dtd\">\n";

// ------------------------------------------------------------------------- //
//  * TDCLXMLEncoder( void )
// ------------------------------------------------------------------------- //
TDCLXMLEncoder::TDCLXMLEncoder( TDCLStream* inStream )
	:
		TDCLNSEncoder( inStream ),
		mLevel( 0 )
{
	// Entête XML.
	inStream->PutString( kXMLHeader );
	inStream->PutString( kDOCTYPEElement );
}

// ------------------------------------------------------------------------- //
//  * ~TDCLXMLEncoder( void )
// ------------------------------------------------------------------------- //
TDCLXMLEncoder::~TDCLXMLEncoder( void )
{
}

// ------------------------------------------------------------------------- //
//  * PutObject( const TDCLNSObject*, KUInt32, KUIntPtr* )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::PutObject(
					const TDCLNSObject*	inObject,
					KUInt32 inObjectID,
					KUIntPtr* /* outCookie */ )
{
	inObject->ToXML( this, inObjectID );
}

// ------------------------------------------------------------------------- //
//  * PutPrecedent( KUInt32, KUIntPtr )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::PutPrecedent( KUInt32 inRefID, KUIntPtr /* inCookie */ )
{
	// <precedent id="7"/>
	char theString[32];	// Largement suffisant.
	(void) ::sprintf( theString, "<precedent idref=\"n%lu\"/>", inRefID );
	GetOutputStream()->PutString( theString );
}

// ------------------------------------------------------------------------- //
//  * PutRef( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::PutRef( const TDCLNSRef& inRef )
{
	inRef.ToXML( this );
}

// ------------------------------------------------------------------------- //
//  * CanHavePrecedentID( TDCLNSObject* )
// ------------------------------------------------------------------------- //
Boolean
TDCLXMLEncoder::CanHavePrecedentID( TDCLNSObject* inObject )
{
	return !(inObject->IsSymbol());
}

// ------------------------------------------------------------------------- //
//  * PutTabulations( void )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::PutTabulations( void )
{
	TDCLStream* theOutputStream = GetOutputStream();
	
	theOutputStream->PutByte( '\n' );

	KUInt32 nbTabs = mLevel;
	KUInt32 indexTabs;
	for (indexTabs = 0; indexTabs < nbTabs; indexTabs++)
	{
		theOutputStream->PutByte( '\t' );
	}
}

// ------------------------------------------------------------------------- //
//  * PrintUTF8WithEntities( const KUInt8* inString )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::PrintUTF8WithEntities( const KUInt8* inString )
{
	TDCLStream* theOutputStream = GetOutputStream();

	KUInt8 theChar = *inString;
	while (theChar != 0x00)
	{
		if (theChar == '&')
		{
			theOutputStream->PutString("&amp;");
		} else if (theChar == '<')
		{
			theOutputStream->PutString("&lt;");
		} else if (theChar == '>')
		{
			theOutputStream->PutString("&gt;");
		} else if (theChar == '"')
		{
			theOutputStream->PutString("&quot;");
		} else if (theChar == '\'')
		{
			theOutputStream->PutString("&apos;");
		} else {
			theOutputStream->PutByte( theChar );
		}
		
		inString++;
		theChar = *inString;
	}
}

// ------------------------------------------------------------------------- //
//  * Print8BitsWithEntities( const KUInt8* inString )
// ------------------------------------------------------------------------- //
void
TDCLXMLEncoder::Print8BitsWithEntities( const KUInt8* inString )
{
	TDCLStream* theOutputStream = GetOutputStream();

	KUInt8 theChar = *inString;
	while (theChar != 0x00)
	{
		if (theChar == '&')
		{
			theOutputStream->PutString("&amp;");
		} else if (theChar == '<')
		{
			theOutputStream->PutString("&lt;");
		} else if (theChar == '>')
		{
			theOutputStream->PutString("&gt;");
		} else if (theChar == '"')
		{
			theOutputStream->PutString("&quot;");
		} else if (theChar == '\'')
		{
			theOutputStream->PutString("&apos;");
		} else if (theChar & 0x80) {
			char theEscSeq[7]; // &#xZV;
			(void) ::sprintf( theEscSeq, "&#x%.2X;", theChar );
			theOutputStream->PutString( theEscSeq );
		} else {
			theOutputStream->PutByte( theChar );
		}
		
		inString++;
		theChar = *inString;
	}
}

// =================================================================== //
// We don't really understand it, so we'll give it to the programmers. //
// =================================================================== //
