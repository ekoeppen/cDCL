// ==============================
// Fichier:			TDCLNSDecoder.cp
// Projet:			Desktop Connection Library
//
// Créé le:			07/08/2002
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
// The Original Code is TDCLNSDecoder.cp.
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
// $Id: TDCLNSDecoder.cp,v 1.5 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSDecoder.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSDecoder( void )
// ------------------------------------------------------------------------- //
TDCLNSDecoder::TDCLNSDecoder( TDCLStream* inStream )
	:
		TDCLNSCodec( inStream )
{
	// Défense d'afficher.
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSDecoder( void )
// ------------------------------------------------------------------------- //
TDCLNSDecoder::~TDCLNSDecoder( void )
{
	// Loi du 29 juillet 1881
}

// ------------------------------------------------------------------------- //
//  * GetObject( KUInt32 inID )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSDecoder::GetObject( KUInt32 inID )
{
	if (inID >= GetListSize())
	{
		throw DCLNS( kNSErrObjectCorrupted );
	}
	
	return GetListItemObject( inID );
}

// ------------------------------------------------------------------------- //
//  * GetObject( KUInt32 inID )
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLNSDecoder::GetNextObjectWithCookie(
					KUIntPtr inCookie,
					KUInt32* ioID /* = nil */ )
{
	KUInt32 theListSize = GetListSize();
	KUInt32 theID = 0;
	if (ioID)
	{
		theID = *ioID;

		if ((theID > 0) && (theID >= theListSize))
		{
			throw DCLNS( kNSErrObjectCorrupted );
		}	
	}
	
	TDCLNSRef theResult;
	
	while (theID < theListSize )
	{
		if (*GetListItemCookie( theID ) == inCookie)
		{
			// Trouvé.
			if (ioID)
			{
				*ioID = theID;
			}
			theResult = GetListItemObject( theID );
			break;
		} else {
			theID++;
		}
	}
	
	return theResult;
}

// ================================ //
// New crypt.  See /usr/news/crypt. //
// ================================ //
