// ==============================
// Fichier:			TDCLNSEncoder.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			06/08/2002
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
// The Original Code is TDCLNSEncoder.cp.
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
// $Id: TDCLNSEncoder.cp,v 1.7 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>

// ------------------------------------------------------------------------- //
//  * TDCLNSEncoder( TDCLStream* )
// ------------------------------------------------------------------------- //
TDCLNSEncoder::TDCLNSEncoder( TDCLStream* inStream )
	:
		TDCLNSCodec( inStream )
{
	// Kilroy was here.
}

// ------------------------------------------------------------------------- //
//  * ~TDCLNSEncoder( void )
// ------------------------------------------------------------------------- //
TDCLNSEncoder::~TDCLNSEncoder( void )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//  * WasObjectEncoded( TDCLNSObject*, KUInt32*, KUIntPtr* )
// ------------------------------------------------------------------------- //
Boolean
TDCLNSEncoder::WasObjectEncoded(
					TDCLNSObject* inObject,
					KUInt32* outObjectID,
					KUIntPtr* outObjectCookie )
{
	Boolean theResult = false;

	KUInt32 indexObjects;				// index pour cette liste.
	KUInt32 nbObjects = GetListSize();	// nombre d'objets dans cette liste.
	for ( indexObjects = 0; indexObjects < nbObjects; indexObjects++ )
	{
		if (GetListItemObject( indexObjects ) == inObject)
		{
			if (outObjectID)
			{
				*outObjectID = indexObjects;
			}
			
			if (outObjectCookie)
			{
				*outObjectCookie = *GetListItemCookie( indexObjects );
			}
			
			theResult = true;
			break;
		}
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * AddObject( const TDCLNSRef& )
// ------------------------------------------------------------------------- //
void
TDCLNSEncoder::AddObject( const TDCLNSRef& inObject )
{
	// Cet objet est-il un immŽdiat?
	if (inObject.IsRealPtr())
	{
		TDCLNSObject* theObject = inObject.GetPointer();
	
		// Cet objet peut-il tre reprŽsentŽ par un ID?
		if (CanHavePrecedentID( theObject ))
		{
			// D'abord, on commence par regarder si cet objet n'a pas dŽjˆ
			// ŽtŽ encodŽ.
			
			KUInt32 theID;
			KUIntPtr theCookie;
			if (WasObjectEncoded( theObject, &theID, &theCookie ))
			{
				// L'objet a dŽjˆ ŽtŽ encodŽ. Ajout d'une rŽfŽrence.
				PutPrecedent( theID, theCookie );
			} else {
				// Ajout de l'objet.
				// Dans la liste
				theID = AddObjectToList( inObject );

				// Dans le flux
				PutObject( theObject, theID, GetListItemCookie( theID ) );
			}
		} else {
			// Ajout le flux avec l'ID -1
			PutObject( theObject, (KUInt32) -1, nil );
		}
	} else {
		PutRef( inObject );
	}
}

// ------------------------------------------------------------------------- //
//  * CanHavePrecedentID( TDCLNSObject* )
// ------------------------------------------------------------------------- //
Boolean
TDCLNSEncoder::CanHavePrecedentID( TDCLNSObject* /* inObject */ )
{
	return true;
}

// =============================== //
// You can't take damsel here now. //
// =============================== //
