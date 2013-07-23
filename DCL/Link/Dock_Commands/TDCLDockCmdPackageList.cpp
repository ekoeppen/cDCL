// ==============================
// Fichier:			TDCLDockCmdPackageList.cp
// Projet:			Desktop Connection Library
//
// Créé le:			10/5/2003
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
// The Original Code is TDCLDockCmdPackageList.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdPackageList.cp,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPackageList.h>

// ANSI C
#include <stdlib.h>

// K
#include <K/Defines/UByteSex.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// -------------------------------------------------------------------------- //
//  * TDCLDockCmdPackageList( KUInt32, const void* )
// -------------------------------------------------------------------------- //
TDCLDockCmdPackageList::TDCLDockCmdPackageList(
										KUInt32 inLength, const void* inData )
	:
		mPackageCount( 0 ),
		mInfos( nil )
{
	// Récupération du nombre de paquets.
	if (inLength < sizeof(KUInt32))
	{
		throw DCLUnknownError;
	}
	
	mPackageCount = UByteSex_FromBigEndian( ((KUInt32*) inData)[0] );
	
	// Analyse de la liste.
	mInfos =
		(SPackageInfo**) ::malloc( sizeof(SPackageInfo*) * mPackageCount );
	
	// On met tout à nil au cas où on sortirait avec une exception.
	KUInt32 indexInfo;
	for (indexInfo = 0; indexInfo < mPackageCount; indexInfo++)
	{
		mInfos[indexInfo] = nil;
	}
	
	KUInt32 inDataOffset = sizeof(KUInt32);
	for (indexInfo = 0; indexInfo < mPackageCount; indexInfo++)
	{
		if ((sizeof(SPackageInfo) + inDataOffset) > inLength)
		{
			throw DCLUnknownError;
		}
		
		const SPackageInfo* thisPackageInfo =
			(const SPackageInfo*) (((const char *) inData) + inDataOffset);
		
		KUInt32 theNameLength =
			UByteSex_FromBigEndian( thisPackageInfo->fNameLength );
		KUInt32 thisInfoSize = sizeof(SPackageInfo) + theNameLength;
		SPackageInfo* theInfo = (SPackageInfo*) ::malloc( thisInfoSize );
		mInfos[indexInfo] = theInfo;
		
		if ((thisInfoSize + inDataOffset) > inLength)
		{
			throw DCLUnknownError;
		}
		
		// Copie des données (avec traduction de l'indien).
		theInfo->fPackageSize =
			UByteSex_FromBigEndian( thisPackageInfo->fPackageSize );
		theInfo->fPackageID =
			UByteSex_FromBigEndian( thisPackageInfo->fPackageID );
		theInfo->fPackageVersion =
			UByteSex_FromBigEndian( thisPackageInfo->fPackageVersion );
		theInfo->fPackageFormat =
			UByteSex_FromBigEndian( thisPackageInfo->fPackageFormat );
		theInfo->fDeviceKind =
			UByteSex_FromBigEndian( thisPackageInfo->fDeviceKind );
		theInfo->fDeviceNumber =
			UByteSex_FromBigEndian( thisPackageInfo->fDeviceNumber );
		theInfo->fDeviceID =
			UByteSex_FromBigEndian( thisPackageInfo->fDeviceID );
		theInfo->fModificationDate =
			UByteSex_FromBigEndian( thisPackageInfo->fModificationDate );
		theInfo->fIsCopyProtected =
			UByteSex_FromBigEndian( thisPackageInfo->fIsCopyProtected );
		theInfo->fNameLength = theNameLength;
		
		// Copie du nom.
		KUInt32 indexName;
		for (indexName = 0; indexName < theNameLength; indexName += 2)
		{
			theInfo->fName[indexName] =
				UByteSex_FromBigEndian( thisPackageInfo->fName[indexName] );
		}
		
		// Suivant.
		inDataOffset += thisInfoSize;
	}
}

// -------------------------------------------------------------------------- //
//  * ~TDCLDockCmdPackageList( void )
// -------------------------------------------------------------------------- //
TDCLDockCmdPackageList::~TDCLDockCmdPackageList( void )
{
	// Itération sur les structures.
	if (mInfos)
	{
		KUInt32 indexInfo;
		for (indexInfo = 0; indexInfo < mPackageCount; indexInfo++)
		{
			if (mInfos[indexInfo])
			{
				::free(mInfos[indexInfo]);
			} else {
				break;
			}
		}
		
		::free(mInfos);
	}
}


// ================================ //
// Programming is an unnatural act. //
// ================================ //
