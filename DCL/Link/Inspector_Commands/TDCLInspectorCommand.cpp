// ==============================
// File:			TDCLInspectorCommand.cpp
// Project:			Desktop Connection Library
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
// The Original Code is TDCLInspectorCommand.cpp.
//
// The Initial Developer of the Original Code is Eckhart Köppen
// Copyright (C) 2007 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Köppen <eck@users.sourceforce.net> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Defines/UByteSex.h>
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Streams/TDCLStream.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdDeletePkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdEnterBreak.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdException.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExitBreak.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdLoadPkg.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h>

#define LOG(...) {printf(__VA_ARGS__); putchar(10);}
#define LOGFN() puts(__PRETTY_FUNCTION__);

TDCLInspectorCommand::TDCLInspectorCommand(	KUInt32 inCommand,	KUInt32 inLength, const void* inData ):
	TDCLDockCmdGeneric( inCommand,	inLength, inData )
{
}

KUInt32 TDCLInspectorCommand::SendHeader( TDCLStream* inStream )
{
	KUInt32 someLong;

	someLong = 'newt';
	inStream->PutLong( someLong );
	someLong = 'ntp ';
	inStream->PutLong( someLong );
	someLong = GetCommand();
	inStream->PutLong( someLong );
	someLong = GetLength();
	inStream->PutLong( someLong );

	return someLong;
}

TDCLInspectorCommand* TDCLInspectorCommand::ReceiveCommand(	TDCLStream* inStream, ProgressFuncPtr inProgressFuncPtr /* = nil */, void* inRefCon /* = nil */ )
{
	// LOGFN();
	TDCLInspectorCommand* theCommandObject = nil;
	
	KUInt32 theCommand;	// Command ID.
	
	KUInt32 theHeader[4] = { 0L , 0L , 0L , 0L };
	KUInt32 length = sizeof( theHeader );
	inStream->Read( &theHeader, &length );

	if (length == 0 || theHeader[0] == 0 || theHeader[1] == 0)
	{
		if (length != 0)
		{
			LOG(
				"length == %u, theHeader[0] == %.8X, theHeader[1] == %.8X",
				(unsigned int) length,
				(unsigned int) theHeader[0],
				(unsigned int) theHeader[1] );
		}
		throw DCLEOF;
	}

	if (( UByteSex_FromBigEndian(theHeader[0]) != 'newt') ||
		( UByteSex_FromBigEndian(theHeader[1]) != 'ntp '))
	{
        KUInt32 c1, c2, c3;
        c1 = UByteSex_FromBigEndian(theHeader[0]);
        c2 = UByteSex_FromBigEndian(theHeader[1]);
        c3 = UByteSex_FromBigEndian(theHeader[2]);
		LOG( "Invalid command (%d bytes):", (unsigned int) length );
		LOG( "%4.4s %4.4s %4.4s", (char*) &c1, (char*) &c2, (char*) &c3 );
		throw DCLEOF;
	}

	KUInt32 theLength = UByteSex_FromBigEndian(theHeader[3]);
	theCommand = UByteSex_FromBigEndian(theHeader[2]);
	
	// LOG( ">> Command %4.4s, length %i", (char*) &theCommand, (int) theLength );

	if (theLength == 0)
	{
		theCommandObject = (TDCLInspectorCommand*) new TDCLDockCmdNoData( theCommand );
	} else if (theLength == 0xFFFFFFFF) {
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
			(void) inStream->GetShort();
		}
	} else {
		void* theData = ::malloc( theLength );
	
		if ( theData == nil ) {
			throw DCLMemError;
		}
	
		try {
			if (inProgressFuncPtr) {
				KUInt8* theBuffer = (KUInt8*) theData;
				KUInt32 readCount = 16;
				KUInt32 totalSize = 16 + theLength;
				
				while (readCount < totalSize)
				{
					KUInt32 toRead = 8192;
					if (toRead + readCount > totalSize)
					{
						toRead = totalSize - readCount;
					}
					
					(*inProgressFuncPtr)( inRefCon, ((double) readCount) / ((double) totalSize) );
					inStream->Read( theBuffer, &toRead );
					readCount += toRead;
					theBuffer += toRead;
				}

				(*inProgressFuncPtr)( inRefCon, (double) 1.0 );
			} else {
				inStream->Read( theData, &theLength );
			}

			if ( theLength == sizeof( KUInt32 ) ) {
				theCommandObject = (TDCLInspectorCommand*) new TDCLDockCmdSingleLong( theCommand, *((KUInt32*) theData) );
			} else {
				switch ( theCommand ) {
					case kDStackFrame:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdStackFrame( theLength, theData );
						break;
					case kDText:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdText( theLength, theData );
						break;
					case kDException:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdException( theLength, theData );
						break;
					case kDExceptionRef:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdExceptionRef( theLength, theData );
						break;
					case kDCode:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdCode( theLength, theData );
						break;
					case kDCodeBlock:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdCodeBlock( theLength, theData );
						break;
					case kDResult:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdResult( theLength, theData );
						break;
					case kDFunctionObject:
						theCommandObject = (TDCLInspectorCommand*) new TDCLInspectorCmdFuncObj( theLength, theData );
						break;
					default:
						break;
				}
			}
		
		} catch ( TDCLException theException )
		{
			::free( theData );
			throw;	// rethrow the exception
		}

		::free( theData );
	}
	
	return theCommandObject;
}

void TDCLInspectorCommand::SendCommand( TDCLStream* inStream, ProgressFuncPtr inProgressFuncPtr /* = nil */, void* inRefCon /* = nil */ )
{
	KUInt32 theLength = SendHeader( inStream );

	KDEBUG2( "<< Command %.8X, length %i", (unsigned int) GetCommand(), (int) GetLength() );
	if (inProgressFuncPtr)
	{
		(*inProgressFuncPtr)( inRefCon, ((double) 16) / ((double) (16 + theLength)) );
	}
	
	if (theLength)
	{
		SendBody( inStream, theLength, inProgressFuncPtr, inRefCon );
	}
	inStream->FlushOutput();

	if (inProgressFuncPtr)
	{
		(*inProgressFuncPtr)( inRefCon, 1.0 );
	}
}

