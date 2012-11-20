// ==============================
// Fichier:			TDCLNSRef.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			30/06/2002
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
// The Original Code is TDCLNSRef.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLNSRef.cp,v 1.11 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Unicode/UUTF16Conv.h>

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLTextEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSFunction.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * Constantes
// ------------------------------------------------------------------------- //
const TDCLNSRef TDCLNSRef::kNILREF = TDCLNSRef( TDCLNSRef::kNILRefValue );
const TDCLNSRef TDCLNSRef::kTRUEREF = TDCLNSRef( TDCLNSRef::kTRUERefValue );
const TDCLNSRef TDCLNSRef::kNewFuncClass = TDCLNSRef( 50 );

// ------------------------------------------------------------------------- //
// 	*ÊTDCLNSRef( void )
// ------------------------------------------------------------------------- //
TDCLNSRef::TDCLNSRef( void )
	:
		mRef( kNILRefValue )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
// 	*ÊTDCLNSRef( KUIntPtr )
// ------------------------------------------------------------------------- //
TDCLNSRef::TDCLNSRef( KUIntPtr inRef )
	:
		mRef( inRef )
{
	if (IsRealPtr())
	{
		TDCLNSObject* thePointer = GetPointer();
		thePointer->IncrementRef();
	}
}

// ------------------------------------------------------------------------- //
// 	*ÊTDCLNSRef( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
TDCLNSRef::TDCLNSRef( const TDCLNSRef& inCopy )
	:
		mRef( inCopy.mRef )
{
	if (IsRealPtr())
	{
		TDCLNSObject* thePointer = GetPointer();
		thePointer->IncrementRef();
	}
}

// ------------------------------------------------------------------------- //
// 	*ÊTDCLNSRef( TDCLNSObject* )
// ------------------------------------------------------------------------- //
TDCLNSRef::TDCLNSRef( TDCLNSObject* inObject )
	:
		mRef( 0 )
{
	inObject->IncrementRef();
	mRef = ((KUIntPtr) inObject) + kPointerTag;
}

// ------------------------------------------------------------------------- //
// 	*ÊTDCLNSRef( TDCLNSObject& )
// ------------------------------------------------------------------------- //
TDCLNSRef::TDCLNSRef( TDCLNSObject& inObject )
	:
		mRef( 0 )
{
	inObject.IsOnStack();
	inObject.IncrementRef();
	mRef = ((KUIntPtr) &inObject) + kPointerTag;
}

// ------------------------------------------------------------------------- //
// 	*Ê~TDCLNSRef( void )
// ------------------------------------------------------------------------- //
TDCLNSRef::~TDCLNSRef( void )
{
	if (IsRealPtr())
	{
		TDCLNSObject* thePointer = GetPointer();
		if (thePointer->DecrementRef())
		{
			delete thePointer;
		}
	}
}

// ------------------------------------------------------------------------- //
// 	*Êoperator = ( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
TDCLNSRef&
TDCLNSRef::operator = ( const TDCLNSRef& inCopy )
{
	if (mRef != inCopy.mRef)
	{
		if (IsRealPtr())
		{
			TDCLNSObject* thePointer = GetPointer();
			if (thePointer->DecrementRef())
			{
				delete thePointer;
			}
		}

		mRef = inCopy.mRef;
		if (IsRealPtr())
		{
			TDCLNSObject* thePointer = GetPointer();
			thePointer->IncrementRef();
		}
	}
	
	return *this;
}

// ------------------------------------------------------------------------- //
// 	*Êoperator = ( TDCLNSObject& )
// ------------------------------------------------------------------------- //
TDCLNSRef&
TDCLNSRef::operator = ( TDCLNSObject& inObject )
{
	KUIntPtr objectRef = ((KUIntPtr) &inObject) + kPointerTag;
	if (mRef != objectRef)
	{
		if (IsRealPtr())
		{
			TDCLNSObject* thePointer = GetPointer();
			if (thePointer->DecrementRef())
			{
				delete thePointer;
			}
		}

		mRef = objectRef;
		inObject.IncrementRef();
	}
	
	return *this;
}

// ------------------------------------------------------------------------- //
// 	*Êoperator == ( const TDCLNSObject& ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSRef::operator == ( const TDCLNSRef& inAlter ) const
{
	Boolean theResult = false;
	
	if (inAlter.mRef == mRef)
	{
		theResult = true;
	} else {
		if (
			(IsRealPtr() && inAlter.IsRealPtr())
			&& (IsSymbol() && inAlter.IsSymbol()))
		{
			TDCLNSSymbol* thisPtr = (TDCLNSSymbol*) GetPointer();
			TDCLNSSymbol* alterPtr = (TDCLNSSymbol*) inAlter.GetPointer();
			if (thisPtr->Compare( *alterPtr ) == 0)
			{
				theResult = true;
			}
		}
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
// 	*ÊRef( KUIntPtr )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::Ref( KUIntPtr inRef )
{
	return TDCLNSRef( inRef );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeInt( KSInt32 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeInt( KSInt32 inInt )
{
	// Copie du signe.
	KUIntPtr theRef = kIntegerTag;
	
	if (inInt < 0)
	{
		theRef |= 0x80000000;
		// Inversion du signe.
		inInt = 0 - inInt;
	}
	
	// VŽrification que l'entier est dans les bonnes bornes.
	if (inInt & 0x60000000)
	{
		throw DCLNS( kNSErrOutOfRange );
	}
	
	theRef |= ((inInt & 0x1FFFFFFF) << kTagShift);
		// 29 bits + le signe -> 30.
	
	return TDCLNSRef( theRef );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeChar( KUInt16 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeChar( KUInt16 inChar )
{
	return TDCLNSRef(
			(KUIntPtr) (inChar << kImmedBitsShift) | kImmedChar | kImmedTag );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeBoolean( Boolean )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeBoolean( Boolean inBoolean )
{
	if (inBoolean)
	{
		return TDCLNSRef( kTRUERefValue );
	} else {
		return TDCLNSRef( kNILRefValue );
	}
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeMagicPtr( KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeMagicPtr( KUInt32 inValue )
{
	// VŽrification que la valeur est dans les bonnes bornes.
	if (inValue & 0xC0000000)
	{
		throw DCLNS( kNSErrOutOfRange );
	}
	
	return TDCLNSRef( (inValue << kTagShift) | kMagicPtrTag );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeMagicPtr( KUInt32, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeMagicPtr( KUInt32 inTable, KUInt32 inIndex )
{
	// VŽrification que les valeurs sont dans les bonnes bornes.
	
	// 12 bits pour l'index.
	if (inIndex & 0xFFFFF000)
	{
		throw DCLNS( kNSErrOutOfRange );
	}
	// 18 bits pour la table
	if (inTable & 0xFFFC0000)
	{
		throw DCLNS( kNSErrOutOfRange );
	}
	
	return TDCLNSRef(
				(((inTable << kMagicPtrTShift) | inIndex)
					<< kTagShift) | kMagicPtrTag);
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeFrame( void )
{
	return TDCLNSRef( new TDCLNSFrame() );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeFunction( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeFunction( void )
{
	return TDCLNSRef( new TDCLNSFunction() );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeArray( KUInt32, const TDCLNSRef& )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeArray(
					KUInt32 inSize /* = 0 */,
					const TDCLNSRef& inRef /* = TDCLNSRef::kNILREF */ )
{
	return TDCLNSRef( new TDCLNSArray( inSize, inRef ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeSymbol( const char* )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeSymbol( const char* inString )
{
	return TDCLNSRef( new TDCLNSSymbol( inString ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeSymbol( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeSymbol( const KUInt16* inString )
{
	return TDCLNSRef( new TDCLNSSymbol( inString ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeString( const char* )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeString( const char* inString )
{
	return TDCLNSRef( new TDCLNSString( inString ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeString( const KUInt16* )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeString( const KUInt16* inString )
{
	return TDCLNSRef( new TDCLNSString( inString ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeString( const KUInt16*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeString( const KUInt16* inString, KUInt32 inSize )
{
	return TDCLNSRef( new TDCLNSString( inString, inSize ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeBinary( const void*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeBinary(
				const void* inData,
				KUInt32 inSize )
{
	return TDCLNSRef( new TDCLNSBinary( inData, inSize ) );
}

// ------------------------------------------------------------------------- //
// 	*ÊMakeBinary( const void*, KUInt32, const TDCLNSRef& )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::MakeBinary(
				const void* inData,
				KUInt32 inSize,
				const TDCLNSRef& inClass )
{
	return TDCLNSRef( new TDCLNSBinary( inData, inSize, inClass ) );
}

// ------------------------------------------------------------------------- //
//  * ClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::ClassOf( void ) const
{
	TDCLNSRef theResult;

	if (IsRealPtr())
	{
		TDCLNSObject* thePointer = GetPointer();
		theResult = thePointer->ClassOf();
	} else if (IsInt()) {
		theResult = KDCLSYM::kSYMint;
	} else if (IsChar()) {
		theResult = KDCLSYM::kSYMchar;
	} else if (mRef == kTRUERefValue) {
		theResult = KDCLSYM::kSYMboolean;
	} else {
		theResult = KDCLSYM::kSYMweird_immediate;
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * PrimClassOf( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::PrimClassOf( void ) const
{
	return KDCLSYM::kSYMimmediate;
}

// ------------------------------------------------------------------------- //
//  * SetClass( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::SetClass( const TDCLNSRef& inClass )
{
	if (IsRealPtr())
	{
		TDCLNSObject* thePointer = GetPointer();
		thePointer->SetClass( inClass );
	} else {
		throw DCLNS( kNSErrObjectPointerOfNonPtr );
	}
	
	return *this;
}

// ------------------------------------------------------------------------- //
// 	*ÊToInt( void )
// ------------------------------------------------------------------------- //
KSInt32
TDCLNSRef::ToInt( void ) const
{
	if (!IsInt())
	{
		throw DCLNS( kNSErrNotAnInteger );
	}
	
	// Valeur absolue
	KSInt32 theResult = (KSInt32) ((mRef >> kTagShift) & 0x1FFFFFFF);

	// RŽcupŽration du signe
	if (mRef & 0x80000000)
	{
		theResult = 0 - theResult;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
// 	*ÊToChar( void )
// ------------------------------------------------------------------------- //
KUInt16
TDCLNSRef::ToChar( void ) const
{
	if (!IsChar())
	{
		throw DCLNS( kNSErrNotACharacter );
	}
	
	// Valeur absolue
	return (KUInt16) (mRef >> kImmedBitsShift);
}

// ------------------------------------------------------------------------- //
// 	*ÊToMagicPtr( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLNSRef::ToMagicPtr( void ) const
{
	if (!IsMagicPtr())
	{
		throw DCLNS( kNSErrNotAMagicPtr );
	}
	
	// Valeur absolue
	KUInt32 theResult = (KUInt32) (mRef >> kTagShift);
	
	return theResult;
}

// ------------------------------------------------------------------------- //
// 	*ÊGetMagicPtrIndex( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLNSRef::GetMagicPtrIndex( void ) const
{
	if (!IsMagicPtr())
	{
		throw DCLNS( kNSErrNotAMagicPtr );
	}
	
	// Valeur absolue
	KUInt32 theResult = ((KUInt32) (mRef >> kTagShift)) & kMagicPtrIMask;
	
	return theResult;
}

// ------------------------------------------------------------------------- //
// 	*ÊGetMagicPtrTable( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLNSRef::GetMagicPtrTable( void ) const
{
	if (!IsMagicPtr())
	{
		throw DCLNS( kNSErrNotAMagicPtr );
	}
	
	// Valeur absolue
	KUInt32 theResult = (KUInt32) (mRef >> (kTagShift + kMagicPtrTShift));
	
	return theResult;
}

// ------------------------------------------------------------------------- //
// 	*ÊClone( void )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::Clone( void ) const
{
	if (IsRealPtr())
	{
		return TDCLNSRef(GetPointer()->Clone());
	} else {
		return *this;
	}
}

// ------------------------------------------------------------------------- //
// 	*ÊToSymbol( void )
// ------------------------------------------------------------------------- //
TDCLNSSymbol&
TDCLNSRef::ToSymbol( void ) const
{
	if (!IsSymbol())
	{
		throw DCLNS( kNSErrNotASymbol );
	}
	
	return *((TDCLNSSymbol*) GetPointer());
}

// ------------------------------------------------------------------------- //
// 	*ÊToFrame( void )
// ------------------------------------------------------------------------- //
TDCLNSFrame&
TDCLNSRef::ToFrame( void ) const
{
	if (!IsFrame())
	{
		throw DCLNS( kNSErrNotAFrame );
	}
	
	return *((TDCLNSFrame*) GetPointer());
}

// ------------------------------------------------------------------------- //
// 	*ÊToArray( void )
// ------------------------------------------------------------------------- //
TDCLNSArray&
TDCLNSRef::ToArray( void ) const
{
	if (!IsArray())
	{
		throw DCLNS( kNSErrNotAnArray );
	}
	
	return *((TDCLNSArray*) GetPointer());
}

// ------------------------------------------------------------------------- //
// 	*ÊToString( void )
// ------------------------------------------------------------------------- //
TDCLNSString&
TDCLNSRef::ToString( void ) const
{
	if (!IsString())
	{
		throw DCLNS( kNSErrNotAString );
	}
	
	return *((TDCLNSString*) GetPointer());
}

// ------------------------------------------------------------------------- //
// 	*ÊToBinary( void )
// ------------------------------------------------------------------------- //
TDCLNSBinary&
TDCLNSRef::ToBinary( void ) const
{
	if (!IsBinary())
	{
		throw DCLNS( kNSErrNotABinaryObject );
	}
	
	return *((TDCLNSBinary*) GetPointer());
}

// ------------------------------------------------------------------------- //
// 	*Êoperator & ( const TDCLNSRef& ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator & ( const TDCLNSRef& inAlter ) const
{
	return TDCLNSRef( ToString().Catenate( inAlter.ToString() ) );
}

// ------------------------------------------------------------------------- //
// 	*Êoperator & ( const char* ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator & ( const char* inAlter ) const
{
	return TDCLNSRef( ToString().Catenate( inAlter ) );
}

// ------------------------------------------------------------------------- //
// 	*Êoperator & ( const KUInt16* ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator & ( const KUInt16* inAlter ) const
{
	return TDCLNSRef( ToString().Catenate( inAlter ) );
}

// ------------------------------------------------------------------------- //
// 	*Êoperator && ( const TDCLNSRef& ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator && ( const TDCLNSRef& inAlter ) const
{
	return TDCLNSRef( ToString().CatenateWithSpace( inAlter.ToString() ) );
}

// ------------------------------------------------------------------------- //
// 	*Êoperator && ( const char* ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator && ( const char* inAlter ) const
{
	return TDCLNSRef( ToString().CatenateWithSpace( inAlter ) );
}

// ------------------------------------------------------------------------- //
// 	*Êoperator && ( const KUInt16* ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSRef::operator && ( const KUInt16* inAlter ) const
{
	return TDCLNSRef( ToString().CatenateWithSpace( inAlter ) );
}

// ------------------------------------------------------------------------- //
//  * ToXML( TDCLXMLEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSRef::ToXML( TDCLXMLEncoder* inEncoder ) const
{
	// RŽcupŽration du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Si on n'est pas un pointeur, mettre la valeur directement.
	if (!IsRealPtr())
	{
		// On gre les cas spŽciaux.
		// 1 - caractre
		if (IsChar())
		{
			theOutputStream->PutString( "<char>" );

			// Il faut le transformer en unicode.
			KUInt8 theUTF8Char[4];
			size_t theOutputSize = 3;
			size_t theInputSize = 1;
			KUInt16 theChar = UByteSex_ToBigEndian( ToChar() );
			
			// assert( ... != kInputExhausted)
			(void) UUTF16Conv::ToUTF8(
				&theChar, &theInputSize, theUTF8Char, &theOutputSize );

			theUTF8Char[theOutputSize] = '\0';
			inEncoder->PrintUTF8WithEntities( theUTF8Char );

			theOutputStream->PutString( "</char>" );
		} else if (IsNIL()) {
			theOutputStream->PutString( "<nil/>" );
		} else if (IsTRUE()) {
			theOutputStream->PutString( "<true/>" );
		} else if (IsInt()) {
			char theOutputStr[32]; 	// Suffisant.
			(void) ::sprintf( theOutputStr, "<int>%li</int>", ToInt() );
			theOutputStream->PutString( theOutputStr );
		} else if (IsMagicPtr()) {
			char theOutputStr[32]; 	// Suffisant.
			(void) ::sprintf(
				theOutputStr, "<magic_ptr>%lu</magic_ptr>", ToMagicPtr() );
			theOutputStream->PutString( theOutputStr );
		} else {
			// weird_immediate
			char theOutputStr[64]; 	// Suffisant.
			(void) ::sprintf(
				theOutputStr, "<weird_immediate>%lu</weird_immediate>", mRef );
			theOutputStream->PutString( theOutputStr );
		}
	} else {
		inEncoder->AddObject( *this );
	}
}

// ------------------------------------------------------------------------- //
//  * ToNSOF( TDCLNSOFEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSRef::ToNSOF( TDCLNSOFEncoder* inEncoder ) const
{
	// Newton Formats, page 59 sqq
	
	// RŽcupŽration du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// On ne doit pas tre un pointeur ici.
	if (!IsRealPtr())
	{
		do {
			// On gre les cas spŽciaux qui diminuent la taille des donnŽes
			// NSOF.
				
			// 1 - caractre
			if (IsChar())
			{
				// Extraction du caractre
				KUInt16 theChar = (KUInt16) (mRef >> kImmedBitsShift);
				
				// Est-ce un caractre unicode?
				if (theChar & 0xFF00)
				{
					theOutputStream->PutByte( KDCLNSOF::kUnicodeCharacter );
					theOutputStream->PutShort( theChar );
				} else {
					theOutputStream->PutByte( KDCLNSOF::kCharacter );
					theOutputStream->PutByte( (KUInt8) theChar );
				}
				break;
			}

			// 2 - NIL
			if (IsNIL())
			{
				theOutputStream->PutByte( KDCLNSOF::kNIL );
				break;
			}

			// Sinon, on met la rŽfŽrence directement.
			theOutputStream->PutByte( KDCLNSOF::kImmediate );
			theOutputStream->PutXLong( mRef );
		} while (0);
	} else {
		inEncoder->AddObject( *this );
	}
}

// ------------------------------------------------------------------------- //
//  * ToText( TDCLTextEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSRef::ToText( TDCLTextEncoder* inEncoder ) const
{
	// RŽcupŽration du flux de sortie.
//	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Si on n'est pas un pointeur, mettre la valeur directement.
	if (!IsRealPtr())
	{
	} else {
		// RŽcupŽrer l'objet associŽ.
		TDCLNSObject* theObject = GetPointer();

		// Ajout via l'encodeur.
		inEncoder->AddObject( theObject );
	}
}

// ------------------------------------------------------------------------- //
//  * ToPkg( TDCLPkgEncoder* ) const
// ------------------------------------------------------------------------- //
void
TDCLNSRef::ToPkg( TDCLPkgEncoder* inEncoder ) const
{
	// RŽcupŽration du flux de sortie.
	TDCLStream* theOutputStream = inEncoder->GetOutputStream();
	
	// Si on n'est pas un pointeur, mettre la valeur directement.
	if (!IsRealPtr())
	{
		theOutputStream->PutLong( mRef );
	} else {
		// RŽcupŽrer l'objet associŽ.
		TDCLNSObject* theObject = GetPointer();

		// Ajout via l'encodeur.
		inEncoder->AddObject( theObject );
	}
}

// =============================================================== //
// The primary function of the design engineer is to make things   //
// difficult for the fabricator and impossible for the serviceman. //
// =============================================================== //

