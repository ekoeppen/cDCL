// ==============================
// Fichier:			TDCLStyleRef.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			20/3/2004
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
// The Original Code is TDCLStyleRef.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLStyleRef.cp,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Data/Text/TDCLStyleRef.h>

// DCL
#include <DCL/Data/Text/TDCLFontSpecification.h>
#include <DCL/Data/Picture/TDCLPicture.h>
#include <DCL/Exceptions/TDCLClassCastException.h>

// -------------------------------------------------------------------------- //
//  * TDCLStyleRef( void )
// -------------------------------------------------------------------------- //
TDCLStyleRef::TDCLStyleRef( void )
	:
		mFontSpecification( nil ),
		mPicture( nil )
{
}

// -------------------------------------------------------------------------- //
//  * TDCLStyleRef( TDCLFontSpecification* )
// -------------------------------------------------------------------------- //
TDCLStyleRef::TDCLStyleRef( TDCLFontSpecification* inTextStyle )
	:
		mFontSpecification( inTextStyle ),
		mPicture( nil )
{
	if (inTextStyle)
	{
		inTextStyle->IncrementRef();
	}
}

// -------------------------------------------------------------------------- //
//  * TDCLStyleRef( TDCLPicture* )
// -------------------------------------------------------------------------- //
TDCLStyleRef::TDCLStyleRef( TDCLPicture* inPicture )
	:
		mFontSpecification( nil ),
		mPicture( inPicture )
{
	if (inPicture)
	{
		inPicture->IncrementRef();
	}
}

// -------------------------------------------------------------------------- //
//  * TDCLStyleRef( const TDCLStyleRef& )
// -------------------------------------------------------------------------- //
TDCLStyleRef::TDCLStyleRef( const TDCLStyleRef& inCopy )
	:
		mFontSpecification( inCopy.mFontSpecification ),
		mPicture( inCopy.mPicture )
{
	if (mFontSpecification)
	{
		mFontSpecification->IncrementRef();
	}
	if (mPicture)
	{
		mPicture->IncrementRef();
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLStyleRef( void )
// -------------------------------------------------------------------------- //
TDCLStyleRef::~TDCLStyleRef( void )
{
	if (mFontSpecification && mFontSpecification->DecrementRef())
	{
		delete mFontSpecification;
	}
	if (mPicture && mPicture->DecrementRef())
	{
		delete mPicture;
	}
}

// ------------------------------------------------------------------------- //
// 	* operator = ( const TDCLNSObject& )
// ------------------------------------------------------------------------- //
TDCLStyleRef&
TDCLStyleRef::operator = ( const TDCLStyleRef& inCopy )
{
	if (inCopy.mFontSpecification)
	{
		if (mPicture && mPicture->DecrementRef())
		{
			delete mPicture;
		}
		mPicture = nil;

		if (mFontSpecification != inCopy.mFontSpecification)
		{
			if (mFontSpecification && mFontSpecification->DecrementRef())
			{
				delete mFontSpecification;
			}
			mFontSpecification = inCopy.mFontSpecification;
			// mFontSpecification != nil ici.
			mFontSpecification->IncrementRef();
		}
	} else if (inCopy.mPicture) {
		if (mFontSpecification && mFontSpecification->DecrementRef())
		{
			delete mFontSpecification;
		}
		mFontSpecification = nil;

		if (mPicture != inCopy.mPicture)
		{
			if (mPicture && mPicture->DecrementRef())
			{
				delete mPicture;
			}
			mPicture = inCopy.mPicture;
			// mPicture != nil ici.
			mPicture->IncrementRef();
		}
	} else {
		if (mFontSpecification && mFontSpecification->DecrementRef())
		{
			delete mFontSpecification;
		}
		mFontSpecification = nil;

		if (mPicture && mPicture->DecrementRef())
		{
			delete mPicture;
		}
		mPicture = nil;
	}
	
	return *this;
}

// -------------------------------------------------------------------------- //
//  * operator TDCLFontSpecification& ( void ) const
// -------------------------------------------------------------------------- //
TDCLStyleRef::operator TDCLFontSpecification& ( void ) const
{
	if (mFontSpecification == nil)
	{
		throw DCLClassCast;
	}
	
	return *mFontSpecification;
}

// -------------------------------------------------------------------------- //
//  * operator TDCLPicture& ( void ) const
// -------------------------------------------------------------------------- //
TDCLStyleRef::operator TDCLPicture& ( void ) const
{
	if (mPicture == nil)
	{
		throw DCLClassCast;
	}
	
	return *mPicture;
}

// ============================ //
// panic: kernel trap (ignored) //
// ============================ //
