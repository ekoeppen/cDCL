// ==============================
// Fichier:			TDCLFontSpecification.cp
// Projet:			Desktop Connection Library
// 
// CrŽŽ le:			21/3/2004
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
// The Original Code is TDCLFontSpecification.cp.
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
// $Id: TDCLFontSpecification.cp,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Data/Text/TDCLFontSpecification.h>

// DCL
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TDCLFontSpecification( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLFontSpecification::TDCLFontSpecification( const TDCLNSRef& inFontSpec )
	:
		mFontSpec( inFontSpec )
{
}

// -------------------------------------------------------------------------- //
//  * TDCLFontSpecification( const TDCLNSRef&, KUInt32, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLFontSpecification::TDCLFontSpecification(
				const TDCLNSRef& inFontName,
				KUInt32 inSize,
				KUInt32 inFace )
{
	// CrŽation d'une structure.
	CreateFontSpec( inFontName, inSize, inFace );
}

// -------------------------------------------------------------------------- //
//  * TDCLFontSpecification( const TDCLFontSpecification& )
// -------------------------------------------------------------------------- //
TDCLFontSpecification::TDCLFontSpecification(
										const TDCLFontSpecification& inCopy )
	:
		TRefCounter(),
		mFontSpec( inCopy.mFontSpec.Clone() )
{
	// This space for rent.
}

// -------------------------------------------------------------------------- //
//  * GetFamily( void ) const
// -------------------------------------------------------------------------- //
TDCLNSRef
TDCLFontSpecification::GetFamily( void ) const
{
	TDCLNSRef theResult;
	
	// Deux cas.
	if (mFontSpec.IsInt())
	{
		KUInt32 theFamilyNumber = ((KUInt32) mFontSpec.ToInt()) & kFamilyMask;
		switch (theFamilyNumber)
		{
			case ktsSystem:
				theResult = KDCLSYM::kSYMespy;
				break;

			case ktsFancy:
				theResult = KDCLSYM::kSYMnewYork;
				break;

			case ktsSimple:
				theResult = KDCLSYM::kSYMgeneva;
				break;

			case ktsHWFont:
				theResult = KDCLSYM::kSYMhandwriting;
				break;
		}
	} else {
		theResult = mFontSpec.ToFrame().Get( KDCLSYM::kSYMfamily );
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetSize( void ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLFontSpecification::GetSize( void ) const
{
	KUInt32 theResult;
	
	// Deux cas.
	if (mFontSpec.IsInt())
	{
		theResult = (((KUInt32) mFontSpec.ToInt()) & kSizeMask) >> kSizeShift;
	} else {
		theResult = (KUInt32) mFontSpec.ToFrame()
									.Get( KDCLSYM::kSYMsize ).ToInt();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetFace( void ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLFontSpecification::GetFace( void ) const
{
	KUInt32 theResult;
	
	// Deux cas.
	if (mFontSpec.IsInt())
	{
		theResult = (((KUInt32) mFontSpec.ToInt()) & kFaceMask) >> kFaceShift;
	} else {
		theResult = (KUInt32) mFontSpec.ToFrame()
									.Get( KDCLSYM::kSYMface ).ToInt();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SetFamily( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
void
TDCLFontSpecification::SetFamily( const TDCLNSRef& inFamilyName )
{
	// Est-ce une des quatre polices de la ROM?
	KUInt32 familyNumber = 0;
	Boolean isROMFont = true;
	if (inFamilyName == KDCLSYM::kSYMespy)
	{
		familyNumber = ktsSystem;
	} else if (inFamilyName == KDCLSYM::kSYMnewYork) {
		familyNumber = ktsFancy;
	} else if (inFamilyName == KDCLSYM::kSYMgeneva) {
		familyNumber = ktsSimple;
	} else if (inFamilyName == KDCLSYM::kSYMhandwriting) {
		familyNumber = ktsHWFont;
	} else {
		isROMFont = false;
	}
	
	// Quatre cas.
	if (isROMFont)
	{
		if (mFontSpec.IsInt())
		{
			// Entier -> Entier.
			KUInt32 thePackedFontSpec = (KUInt32) mFontSpec.ToInt();
			thePackedFontSpec &= kFaceMask | kSizeMask | kSignExtension;
			thePackedFontSpec |= familyNumber;
			mFontSpec = TDCLNSRef::MakeInt( (KSInt32) thePackedFontSpec );
		} else {
			// Conversion en entier.
			TDCLNSFrame& theFontFrame = mFontSpec.ToFrame();
			KUInt32 theSize =
				(KUInt32) theFontFrame.Get( KDCLSYM::kSYMsize ).ToInt();
			KUInt32 theFace =
				(KUInt32) theFontFrame.Get( KDCLSYM::kSYMfamily ).ToInt();
			
			mFontSpec = TDCLNSRef::MakeInt(
				(KSInt32)
					(familyNumber
					| (theSize << kSizeShift)
					| (theFace << kFaceShift)));
		}
	} else {
		if (mFontSpec.IsInt())
		{
			// Conversion en structure.
			KUInt32 thePackedFontSpec = (KUInt32) mFontSpec.ToInt();
			mFontSpec = TDCLNSRef::MakeFrame();
			TDCLNSFrame& theFontFrame = mFontSpec.ToFrame();
			
			theFontFrame.Set( KDCLSYM::kSYMfamily, inFamilyName );
			theFontFrame.Set(
					KDCLSYM::kSYMsize,
					TDCLNSRef::MakeInt(
						(KSInt32)
							(thePackedFontSpec & kSizeMask) >> kSizeShift ));
			theFontFrame.Set(
					KDCLSYM::kSYMface,
					TDCLNSRef::MakeInt(
						(KSInt32)
							(thePackedFontSpec & kFaceMask) >> kFaceShift ));
		} else {
			// Structure -> Structure.
			mFontSpec.ToFrame().Set( KDCLSYM::kSYMfamily, inFamilyName );
		}
	}
}

// -------------------------------------------------------------------------- //
//  * SetSize( KUInt32 )
// -------------------------------------------------------------------------- //
void
TDCLFontSpecification::SetSize( KUInt32 inSize )
{
	// Deux cas.
	if (mFontSpec.IsInt())
	{
		KUInt32 thePackedFontSpec = (KUInt32) mFontSpec.ToInt();
		thePackedFontSpec &= kFaceMask | kFamilyMask | kSignExtension;
		thePackedFontSpec |= inSize << kSizeShift;
		mFontSpec = TDCLNSRef::MakeInt( (KSInt32) thePackedFontSpec );
	} else {
		mFontSpec.ToFrame().Set(
			KDCLSYM::kSYMsize, TDCLNSRef::MakeInt( (KSInt32) inSize ) );
	}
}

// -------------------------------------------------------------------------- //
//  * SetFace( KUInt32 )
// -------------------------------------------------------------------------- //
void
TDCLFontSpecification::SetFace( KUInt32 inFace )
{
	// Deux cas.
	if (mFontSpec.IsInt())
	{
		KUInt32 thePackedFontSpec = (KUInt32) mFontSpec.ToInt();
		thePackedFontSpec &= kSizeMask | kFamilyMask;
		thePackedFontSpec |= inFace << kFaceShift;
		// Extension.
		if (thePackedFontSpec & kSignBit)
		{
			thePackedFontSpec |= kSignExtension;
		}
		mFontSpec = TDCLNSRef::MakeInt( (KSInt32) thePackedFontSpec );
	} else {
		mFontSpec.ToFrame().Set(
			KDCLSYM::kSYMface, TDCLNSRef::MakeInt( (KSInt32) inFace ) );
	}
}

// -------------------------------------------------------------------------- //
//  * CreateFontSpec( const TDCLNSRef&, KUInt32, KUInt32 )
// -------------------------------------------------------------------------- //
void
TDCLFontSpecification::CreateFontSpec(
				const TDCLNSRef& inFontName,
				KUInt32 inSize,
				KUInt32 inFace )
{
	// Est-ce une des quatre polices de la ROM?
	KUInt32 familyNumber = 0;
	Boolean isROMFont = true;
	if (inFontName == KDCLSYM::kSYMespy)
	{
		familyNumber = ktsSystem;
	} else if (inFontName == KDCLSYM::kSYMnewYork) {
		familyNumber = ktsFancy;
	} else if (inFontName == KDCLSYM::kSYMgeneva) {
		familyNumber = ktsSimple;
	} else if (inFontName == KDCLSYM::kSYMhandwriting) {
		familyNumber = ktsHWFont;
	} else {
		isROMFont = false;
	}
	
	// Deux cas.
	if (isROMFont)
	{
		mFontSpec = TDCLNSRef::MakeInt(
			(KSInt32)
				(familyNumber
				| (inSize << kSizeShift)
				| (inFace << kFaceShift)));
	} else {
		mFontSpec = TDCLNSRef::MakeFrame();
		TDCLNSFrame& theFrame = mFontSpec.ToFrame();
	
		theFrame.Set( KDCLSYM::kSYMfamily, inFontName );
		theFrame.Set( KDCLSYM::kSYMsize, TDCLNSRef::MakeInt( (KSInt32) inSize ) );
		theFrame.Set( KDCLSYM::kSYMface, TDCLNSRef::MakeInt( (KSInt32) inFace ) );
	}
}

// ================================================================= //
// Bringing computers into the home won't change either one, but may //
// revitalize the corner saloon.                                     //
// ================================================================= //
