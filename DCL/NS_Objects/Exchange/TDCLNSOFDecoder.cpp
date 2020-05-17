// ==============================
// Fichier:			TDCLNSOFDecoder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			22/10/2002
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
// The Original Code is TDCLNSOFDecoder.cp.
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
// $Id: TDCLNSOFDecoder.cp,v 1.3 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/KDCLNSErrors.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/NS_Objects/Objects/TDCLNSBinary.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSOFDecoder( TDCLStream* inStream )
// ------------------------------------------------------------------------- //
TDCLNSOFDecoder::TDCLNSOFDecoder( TDCLStream* inStream )
	:
		TDCLNSDecoder( inStream )
{
	// Lecture & vérification de la version.
	KUInt8 theVersion = inStream->GetByte();
	if (theVersion != kNSOFVersion)
	{
		throw DCLNS( kNSErrUnknownStreamFormat );
	}
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSOFDecoder( void )
// ------------------------------------------------------------------------- //
TDCLNSOFDecoder::~TDCLNSOFDecoder( void )
{
}

// ------------------------------------------------------------------------- //
//  * GetNextObject( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSOFDecoder::GetNextObject( void )
{
	// Lecture du type.
	TDCLStream* theStream = GetInputStream();
	KUInt8 theType = theStream->GetByte();
	
	// Suivant le type, lecture de l'objet (ou récupération de l'objet dans
	// la liste de ceux déjà lus).
	TDCLNSRef theResult;
	
	switch (theType)
	{
		case kImmediate:
			theResult = TDCLNSRef::Ref( theStream->GetXLong() );
			break;
			
		case kCharacter:
			theResult = TDCLNSRef::MakeChar( theStream->GetByte() );
			break;
			
		case kUnicodeCharacter:
			theResult = TDCLNSRef::MakeChar( theStream->GetShort() );
			break;

		case kBinaryObject:
			theResult = TDCLNSBinary::FromNSOF( this );
			break;

		case kArray:
			theResult = TDCLNSArray::FromNSOF( this, false );
			break;

		case kPlainArray:
			theResult = TDCLNSArray::FromNSOF( this, true );
			break;

		case kFrame:
			theResult = TDCLNSFrame::FromNSOF( this );
			break;

		case kSymbol:
			theResult = TDCLNSSymbol::FromNSOF( this );
			break;

		case kString:
			theResult = TDCLNSString::FromNSOF( this );
			break;

		case kNIL:
			// theResult vaut déjà NIL.
			break;

		case kSmallRect:
			theResult = TDCLNSRef( TDCLNSFrame::FromNSOFAsRect( this ) );
			break;

		case kLargeBinary:
			theResult = TDCLNSRef( TDCLNSBinary::FromNSOFAsLargeBinary( this ) );
			break;

		case kPrecedent:
			// L'objet a déjà été lu.
		    {
				KUInt32 theID = theStream->GetXLong();
				theResult = GetObject( theID );
			}
			break;

		default:
			throw DCLNS( kNSErrUnknownStreamFormat );
	}
	
	return theResult;
}

// ============================================================================= //
// Ever wondered about the origins of the term "bugs" as applied to computer     //
// technology?  U.S. Navy Capt. Grace Murray Hopper has firsthand explanation.   //
// The 74-year-old captain, who is still on active duty, was a pioneer in        //
// computer technology during World War II.  At the C.W. Post Center of Long     //
// Island University, Hopper told a group of Long Island public school adminis-  //
// trators that the first computer "bug" was a real bug--a moth.  At Harvard     //
// one August night in 1945, Hopper and her associates were working on the       //
// "granddaddy" of modern computers, the Mark I.  "Things were going badly;      //
// there was something wrong in one of the circuits of the long glass-enclosed   //
// computer," she said.  "Finally, someone located the trouble spot and, using   //
// ordinary tweezers, removed the problem, a two-inch moth.  From then on, when  //
// anything went wrong with a computer, we said it had bugs in it."  Hopper      //
// said that when the veracity of her story was questioned recently, "I referred //
// them to my 1945 log book, now in the collection of the Naval Surface Weapons  //
// Center, and they found the remains of that moth taped to the page in          //
// question."                                                                    //
//                 [actually, the term "bug" had even earlier usage in           //
//                 regard to problems with radio hardware.  Ed.]                 //
// ============================================================================= //
