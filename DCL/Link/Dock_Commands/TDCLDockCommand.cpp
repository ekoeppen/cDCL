// ==============================
// Fichier:			TDCLDockCommand.cp
// Projet:			Desktop Connection Library
//
// Créé le:			31/01/2001
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
// The Original Code is TDCLDockCommand.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCommand.cp,v 1.9 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdArray.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPackageList.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPassword.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Streams/TDCLStream.h>

#undef KDEBUG_ENABLED
#define KDEBUG_ENABLED 1

// ------------------------------------------------------------------------- //
//  * ReceiveCommand( TDCLStream*, ProgressFuncPtr, void* )
// ------------------------------------------------------------------------- //
TDCLDockCommand*
TDCLDockCommand::ReceiveCommand(
						TDCLStream* inStream,
						ProgressFuncPtr inProgressFuncPtr /* = nil */,
						void* inRefCon /* = nil */ )
{
	TDCLDockCommand* theCommandObject = nil;
	
	KUInt32 theCommand;	// Command ID.
	
	// Lecture de l'entête, i.e. des 4 premiers longs.
	KUInt32 theHeader[4] = { 0L , 0L , 0L , 0L };
	KUInt32 length = sizeof( theHeader );
	inStream->Read( &theHeader, &length );

	// Ici, length == 0 devrait suffire.
	// Cependant, on a un problème plus haut dans nos sockets, d'où ce test
	// un peu (complètement) crado.	
	if (length == 0 || theHeader[0] == 0 || theHeader[1] == 0)
	{
		if (length != 0)
		{
			KDEBUG3(
				"length == %u, theHeader[0] == %.8X, theHeader[1] == %.8X",
				(unsigned int) length,
				(unsigned int) theHeader[0],
				(unsigned int) theHeader[1] );
		}
		// Ça veut juste dire que l'autre côté à déconnecté.
		// Rien d'anormal.
		// Je dis EOF et ça sera récupéré plus haut.
		throw DCLEOF;
	}

	// The four longs are big endian.

	// I check it's a dock command.
	if (( UByteSex_FromBigEndian(theHeader[0]) != 'newt') ||
		( UByteSex_FromBigEndian(theHeader[1]) != 'dock'))
	{
		throw DCLBadDockCmd;
	}

	// If the size is null, I create a no data command.
	KUInt32 theLength = UByteSex_FromBigEndian(theHeader[3]);
				 	// Length of the command.
	theCommand = UByteSex_FromBigEndian(theHeader[2]);
	
	KDEBUG2( ">> Commande %.8X de taille %i",
		(unsigned int) theCommand, (int) theLength );

	if (theLength == 0)
	{
		theCommandObject =
			(TDCLDockCommand*) new TDCLDockCmdNoData( theCommand );
	} else if (theLength == 0xFFFFFFFF) {
		// Cas spécial: kDBackupIDs
		if (theCommand != kDBackupIDs)
		{
			throw DCLBadDockCmd;
		}
		
		// Lecture des entiers de 16 bits jusqu'à avoir 0x8000.
		KUInt16* theData = (KUInt16*) ::malloc( sizeof( KSInt16 ) );
		KUInt32 nbShorts = 0;
		
		do {
			KUInt16 theShort = inStream->GetShort();
			theData[nbShorts++] = theShort;
			
			if (theShort == 0x8000)
			{
				break;
			}
			
			theData = (KUInt16*)
				::realloc( theData, (nbShorts + 1) * sizeof( KUInt16 ) );
		} while ( true );
		
		if (nbShorts & 0x1)
		{
			// Lecture de deux octets pour l'alignement.
			(void) inStream->GetShort();
		}
	} else {
		// I then read the remaining data.
		// The doc says it's padded to four bytes.
	
		KUInt32 thePaddedLength = theLength;
	
		if (thePaddedLength & 0x3)
		{
			thePaddedLength = ((thePaddedLength >> 2) + 1 ) << 2;
		}
	
		void* theData = ::malloc( thePaddedLength );
	
		if ((thePaddedLength > 0) && (theData == nil))
		{
			throw DCLMemError;
		}
	
		try {
			// Lecture de la commande.
			if (inProgressFuncPtr)
			{
				KUInt8* theBuffer = (KUInt8*) theData;
				KUInt32 readCount = 16;
				KUInt32 totalSize = 16 + thePaddedLength;
				
				while (readCount < totalSize)
				{
					KUInt32 toRead = 8192;
					if (toRead + readCount > totalSize)
					{
						toRead = totalSize - readCount;
					}
					
					(*inProgressFuncPtr)(
								inRefCon,
								((double) readCount) / ((double) totalSize) );
					inStream->Read( theBuffer, &toRead );
					readCount += toRead;
					theBuffer += toRead;
				}

				// On a tout lu.				
				(*inProgressFuncPtr)( inRefCon, (double) 1.0 );
			} else {
				inStream->Read( theData, &thePaddedLength );
			}

			// Finally, I create the command object and I return it.
			// I create the object depending on the command type.

			switch ( theCommand )
			{
				case kDNewtonName:
					// I create a TDCLDockCmdNewtonName object.
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdNewtonName( theLength, theData );
					break;

				case kDPassword:
					// I create a TDCLDockCmdPassword object.
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdPassword( theLength, theData );
					break;
				
				case kDNewtonInfo:
					// I create a TDCLDockCmdNewtonInfo object.
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdNewtonInfo( theLength, theData );
					break;
				
				case kDPackageIDList:
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdPackageList( theLength, theData );
					break;

				case kDHello:
					// Les commandes Hello sont sans données.
					// Si on se trouve ici, c'est que le Newton a renvoyé une
					// commande hello avec des données, ce qui n'est
					// normalement pas possible.
					// On dégage en exception, les données seront libérées
					// plus bas.
					throw DCLBadDockCmd;
				
				// NSObject commands (only those possibly sent by the Newton)
				// Some of these may one day be subclasses of
				// TDCLDockCmdNSObject
				case kDQuery:
				case kDDefaultStore:
				case kDAppNames:
				case kDImportParameterSlipResult:
				case kDPackageInfo:
				case kDCallResult:
				case kDRemovePackage:
				case kDGetPackageInfo:
				case kDImportParametersSlip:
				case kDGetPassword:
				case kDSetStoreName:
				case kDRefTest:					
				case kDSyncOptions:
				case kDSyncResults:
				case kDSetStoreGetNames:
				case kDRequestToBrowse:
				case kDSetPath:
				case kDSetDrive:
				case kDGetFileInfo:
				case kDInternalStore:
				case kDDevices:
				case kDFilters:
				case kDPath:
				case kDFilesAndFolders:
				case kDFileInfo:
				case kDImportFile:
				case kDTranslatorList:
				case kDSoupsChanged:
				case kDLoadPackageFile:
				case kDRestoreFile:
				case kDRestoreOptions:
				case kDRestorePackage:
				case kDStoreNames:
				case kDIndexDescription:
				case kDEntry:
				case kDSetCurrentStore:
				case kDAddEntry:
				case kDChangedEntry:
				case kDSoupInfo:
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdNSObject(
									theCommand, theLength, theData );
					break;
				
				case kDSoupNames:
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdTwoNSObjects(
									theCommand, theLength, theData );
					break;

				case kDSoupIDs:
				case kDChangedIDs:
					// Tableau
					theCommandObject = (TDCLDockCommand*)
						new TDCLDockCmdArray( theCommand, theLength, theData );
					break;

				default:
					// If I have 4 bytes of advertised data, I instantiate a
					// TDCLDockCmdSingleLong object.
					if ( theLength == sizeof( KUInt32 ) )
					{
						theCommandObject = (TDCLDockCommand*)
							new TDCLDockCmdSingleLong(
								theCommand, *((KUInt32*) theData) );
					} else {
						// I create a TDCLDockCmdGeneric
						theCommandObject = (TDCLDockCommand*)
							new TDCLDockCmdGeneric(
								theCommand, theLength, theData );
					}
			}
		
		} catch ( TDCLException theException )
		{
			::free( theData );
			throw;	// rethrow the exception
		}

		::free( theData );
	} // if (thePaddedLength == 0) ... else
	
	return theCommandObject;
}

// ------------------------------------------------------------------------- //
//  * GetResultCmd( TDCLPipe* )
// ------------------------------------------------------------------------- //
KSInt32
TDCLDockCommand::GetResultCmd( TDCLPipe* inPipe )
{
	KSInt32 theResult = 0;
	
	do	// Loop to grab all incoming commands.
	{
		// Get command.
		TDCLDockCommand* theNewtMessage = ReceiveCommand( inPipe );

		if (theNewtMessage->GetCommand() == TDCLDockCommand::kDHello)
		{
			// It's a kDHello, continue.
			delete theNewtMessage;	// I no longer need it.
			
		} else if (theNewtMessage->GetCommand() == TDCLDockCommand::kDResult) {
		
			// It's a kDResult, process it and get out.
			TDCLDockCmdSingleLong* theResultCmd =
						(TDCLDockCmdSingleLong*) theNewtMessage;
			theResult = (KSInt32) theResultCmd->GetLong();
		
			delete theNewtMessage;	// I no longer need it.
			break;	// Get out of the loop.
			
		} else {
			// It's not what I expected. Get out.
	
			delete theNewtMessage;
			throw DCLUnexpDockCmd;
		}
		
		// Do I have any command left?
	} while( inPipe->BytesAvailable() );
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * SendCommand( TDCLStream*, ProgressFuncPtr, void* )
// ------------------------------------------------------------------------- //
void
TDCLDockCommand::SendCommand(
						TDCLStream* inStream,
						ProgressFuncPtr inProgressFuncPtr /* = nil */,
						void* inRefCon /* = nil */ )
{
	KDEBUG2( "<< Commande %.8X de taille %i",
				(unsigned int) GetCommand(), (int) GetLength() );

	// Envoi de l'entête et récupération de la taille.
	KUInt32 theLength = SendHeader( inStream );
	
	// Si inProgressFuncPtr n'est pas \c nil, on indique ces 16 octets.
	if (inProgressFuncPtr)
	{
		(*inProgressFuncPtr)(
				inRefCon, ((double) 16) / ((double) (16 + theLength)) );
	}
	
	if (theLength)	// don't call SendBody if there is no length.
	{
		SendBody( inStream, theLength, inProgressFuncPtr, inRefCon );
	}
	
	// On complète avec des zéros
	theLength = 4 - (theLength & 0x3);
	if (theLength != 4)
	{
		KUInt32 zeroes = 0;
		inStream->Write( &zeroes, &theLength );
	}

	// Finally, I flush the output
	inStream->FlushOutput();

	// Si inProgressFuncPtr n'est pas \c nil, on indique que c'est fini.
	if (inProgressFuncPtr)
	{
		(*inProgressFuncPtr)( inRefCon, 1.0 );
	}
}

// ------------------------------------------------------------------------- //
//  * TDCLDockCommand( void )
// ------------------------------------------------------------------------- //
TDCLDockCommand::TDCLDockCommand( void )
	:
		mCommand ( 0 )
{
	// This space for rent.
}

// ------------------------------------------------------------------------- //
//  * TDCLDockCommand( KUInt32 )
// ------------------------------------------------------------------------- //
TDCLDockCommand::TDCLDockCommand( KUInt32 inCommand )
	:
		mCommand ( inCommand )
{
}

// ------------------------------------------------------------------------- //
//  * ~TDCLDockCommand( void )
// ------------------------------------------------------------------------- //
TDCLDockCommand::~TDCLDockCommand( void )
{
}

// ------------------------------------------------------------------------- //
//  * GetData( void )
// ------------------------------------------------------------------------- //
const void*
TDCLDockCommand::GetData( void )
{
	throw DCLNotImplementedError;
	
	return nil;	// Make compiler happy.
}

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLDockCommand::GetLength( void ) const
{
	throw DCLNotImplementedError;
	
	return 0;	// Make compiler happy.
}

// ------------------------------------------------------------------------- //
//  * SendHeader( TDCLStream* )
// ------------------------------------------------------------------------- //
KUInt32
TDCLDockCommand::SendHeader( TDCLStream* inStream )
{
	KUInt32 someLong;

	someLong = 'newt';
	inStream->PutLong( someLong );	// Cette méthode convertit en big endian
									// comme il faut.
	someLong = 'dock';
	inStream->PutLong( someLong );
	someLong = GetCommand();
	inStream->PutLong( someLong );
	someLong = GetLength();
	inStream->PutLong( someLong );

	return someLong;	// someLong contient la taille.
}

// ------------------------------------------------------------------------- //
//  * SendBody( TDCLStream*, KUInt32, ProgressFuncPtr, void* )
// ------------------------------------------------------------------------- //
void
TDCLDockCommand::SendBody(
					TDCLStream* inStream,
					KUInt32 inLength,
					ProgressFuncPtr inProgressFuncPtr,
					void* inRefCon )
{
	// Récupération des données.
	const KUInt8* theData = (const KUInt8*) GetData();
	KUInt32 theLength;
	
	// Si inProgressFuncPtr n'est pas nil, on envoie par paquets de 2 Ko.
	if (inProgressFuncPtr)
	{
		KUInt32 sentBytes = 0;
		while (sentBytes < inLength)
		{
			theLength = 2048;
			if (theLength + sentBytes > inLength)
			{
				theLength = inLength - sentBytes;
			}
			inStream->Write( theData, &theLength );
			inStream->FlushOutput();
			
			sentBytes += theLength;
			
			// On prévient.
			(*inProgressFuncPtr)(
					inRefCon,
					((double) (16 + sentBytes)) / ((double) (16 + inLength)) );
		}
	} else {
		theLength = inLength;
		inStream->Write( theData, &theLength );
	}
}

// ======================================================================= //
// One good reason why computers can do more work than people is that they //
// never have to stop and answer the phone.                                //
// ======================================================================= //	
