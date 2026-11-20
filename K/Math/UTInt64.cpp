// ==============================
// Fichier:			UTInt64.cp
// Projet:			K
// 
// Créé le:			10/9/2003
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
// The Original Code is UTInt64.cp.
// 
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2003-2004
// the Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: UTInt64.cp,v 1.5 2005/02/24 06:49:58 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include <K/Math/UTInt64.h>

// -------------------------------------------------------------------------- //
//  * GetHi( const KUInt64& )
// -------------------------------------------------------------------------- //
KUInt32
UTInt64::GetHi( const KUInt64& in64BitsWord )
{
	return (KUInt32) (in64BitsWord >> 32);
}

// -------------------------------------------------------------------------- //
//  * GetHi( const KSInt64& )
// -------------------------------------------------------------------------- //
KSInt32
UTInt64::GetHi( const KSInt64& in64BitsWord )
{
	return (KSInt32) (in64BitsWord >> 32);
}

// -------------------------------------------------------------------------- //
//  * GetLo( const KUInt64& )
// -------------------------------------------------------------------------- //
KUInt32
UTInt64::GetLo( const KUInt64& in64BitsWord )
{
	return (KUInt32) (in64BitsWord);
}

// -------------------------------------------------------------------------- //
//  * GetLo( const KSInt64& )
// -------------------------------------------------------------------------- //
KUInt32
UTInt64::GetLo( const KSInt64& in64BitsWord )
{
	return (KUInt32) (in64BitsWord);
}

// -------------------------------------------------------------------------- //
//  * ToKUInt32( const KSInt64& )
// -------------------------------------------------------------------------- //
KUInt32
UTInt64::ToKUInt32( const KSInt64& in64BitsWord )
{
	return (KUInt32) in64BitsWord;
}

// -------------------------------------------------------------------------- //
//  * ToKUInt32( const KUInt64& )
// -------------------------------------------------------------------------- //
KUInt32
UTInt64::ToKUInt32( const KUInt64& in64BitsWord )
{
	return (KUInt32) in64BitsWord;
}

// -------------------------------------------------------------------------- //
//  * ToKSInt32( const KSInt64& )
// -------------------------------------------------------------------------- //
KSInt32
UTInt64::ToKSInt32( const KSInt64& in64BitsWord )
{
	return (KSInt32) in64BitsWord;
}

// -------------------------------------------------------------------------- //
//  * SetHi( KUInt64*, const KUInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::SetHi( KUInt64* io64BitsWord, const KUInt32 in32BitsValue )
{
	*io64BitsWord &= (KUInt64) 0x00000000FFFFFFFFLLU;
	*io64BitsWord |= (((KUInt64) in32BitsValue) << 32);
}

// -------------------------------------------------------------------------- //
//  * SetHi( KSInt64*, const KSInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::SetHi( KSInt64* io64BitsWord, const KSInt32 in32BitsValue )
{
	*io64BitsWord &= (KSInt64) 0x00000000FFFFFFFFLLU;
	*io64BitsWord |= (((KSInt64) in32BitsValue) << 32);
}

// -------------------------------------------------------------------------- //
//  * SetLo( KUInt64*, const KUInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::SetLo( KUInt64* io64BitsWord, const KUInt32 in32BitsValue )
{
	*io64BitsWord &= (KUInt64) 0xFFFFFFFF00000000LLU;
	*io64BitsWord |= in32BitsValue;
}

// -------------------------------------------------------------------------- //
//  * SetLo( KSInt64*, const KUInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::SetLo( KSInt64* io64BitsWord, const KUInt32 in32BitsValue )
{
	*io64BitsWord &= (KUInt64) 0xFFFFFFFF00000000LLU;
	*io64BitsWord |= in32BitsValue;
}

// -------------------------------------------------------------------------- //
//  * Set( KUInt64*, const KUInt32, const KUInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::Set(
			KUInt64* out64BitsWord,
			const KUInt32 inHiHalf,
			const KUInt32 inLoHalf )
{
	*out64BitsWord = (((KUInt64) inHiHalf) << 32) | inLoHalf;
}

// -------------------------------------------------------------------------- //
//  * Set( KSInt64*, const KSInt32, const KUInt32 )
// -------------------------------------------------------------------------- //
void
UTInt64::Set(
			KSInt64* out64BitsWord,
			const KSInt32 inHiHalf,
			const KUInt32 inLoHalf )
{
	*out64BitsWord = (((KSInt64) inHiHalf) << 32) | inLoHalf;
}

// -------------------------------------------------------------------------- //
//  * CreateKUInt64( const KUInt32, const KUInt32 )
// -------------------------------------------------------------------------- //
KUInt64
UTInt64::CreateKUInt64( const KUInt32 inHiHalf, const KUInt32 inLoHalf )
{
	return (((KUInt64) inHiHalf) << 32) | inLoHalf;
}

// -------------------------------------------------------------------------- //
//  * CreateKSInt64( const KSInt32, const KUInt32 )
// -------------------------------------------------------------------------- //
KSInt64
UTInt64::CreateKSInt64( const KSInt32 inHiHalf, const KUInt32 inLoHalf )
{
	return (((KSInt64) inHiHalf) << 32) | inLoHalf;
}

// --------------------------------------------------------------------	//
//	* MultModulo( const KUInt64&, const KUInt64&, const KUInt64& )
// --------------------------------------------------------------------	//
// Multiplication avec modulo
// Retourne Un * Deux mod Module
KUInt64
UTInt64::MultModulo(
	const KUInt64& inArgOne,
	const KUInt64& inArgTwo,
	const KUInt64& inModule )
{
	// Russian Peasant modifié avec modulo.
	KUInt64 theLeftInt = inArgOne % inModule;
	KUInt64 theRightInt = inArgTwo % inModule;
	KUInt64 theResult = 0;
	
	while (theRightInt > 0)
	{
		// On divise theRightInt par deux
		bool theCarry = ((theRightInt & 0x0000000000000001LLU) != 0);
		theRightInt >>= 1;
		if (theCarry)
		{
			// Gestion de l'overflow.
			// theResult est < inModule
			// theLeftInt est < inModule
			// Donc theResult + theLeftInt est < 2 * inModule
			// Si c'est >= inModule, je retranche inModule
			theResult += theLeftInt;
			if ((theResult < theLeftInt) || (theResult >= inModule))
			{
				(void) (theResult -= inModule);
			}
		}
		// Et on multiplie l'entier de gauche par deux
		// Comme theLeftInt est < inModule, 
		// theLeftInt * 2 est < 2 * inModule
		theCarry = ((theLeftInt & 0x8000000000000000LLU) != 0);
		theLeftInt <<= 1;
		if (theCarry || (theLeftInt >= inModule))
		{
			(void) (theLeftInt -= inModule);
			// Gestion de l'overflow:
			// theLeftInt est en fait 0x100000000 + theLeftInt
			// Donc theLeftInt - inModule est:
			// theLeftInt + (0x100000000 - inModule)
			// Il suffit donc de faire theLeftInt -= inModule
		}
	}
		
	return theResult;
}

// --------------------------------------------------------------------	//
//	* ExpModulo( const KUInt64&, const KUInt64&, const KUInt64& )
// --------------------------------------------------------------------	//
// Exponentiation avec modulo
// Retourne Base ^ Exposant mod Module
KUInt64
UTInt64::ExpModulo(
	const KUInt64& inBase,
	const KUInt64& inExponent,
	const KUInt64& inModule )
{
	// Itération sur les bits de E.
	// le résultat est produit(base^(2^ki) modulo module) modulo module.
	// Donc, on calcule base^2ki à chaque itération, on multiplie
	// avec le résultat si le bit correspondant de E est à 1.
	
	KUInt64 basePuissance2KiReduite = inBase % inModule; // i = 0
	KUInt64 theResult = 1;
	KUInt64 theExponent = inExponent;
	
	while (theExponent != 0)
	{
		bool theCarry = ((theExponent & 0x0000000000000001LLU) != 0);
		theExponent >>= 1;
		if (theCarry)
		{
			theResult =
				MultModulo( basePuissance2KiReduite, theResult, inModule );
		}
		
		basePuissance2KiReduite =
			MultModulo( basePuissance2KiReduite, basePuissance2KiReduite, inModule );
	}
		
	return theResult;
}

// --------------------------------------------------------------------	//
//	* EuclideanDivision( KUInt64&, KUInt64&, const KUInt64&, const ... )
// --------------------------------------------------------------------	//
void
UTInt64::EuclideanDivision(
		KUInt64& outQuotient,
		KUInt64& outRemainder,
		const KUInt64& inDividend,
		const KUInt64& inDivisor )
{
	outQuotient = inDividend / inDivisor;
	outRemainder = inDividend - (outQuotient * inDivisor);
}

// =============================================== //
// There are never any bugs you haven't found yet. //
// =============================================== //
