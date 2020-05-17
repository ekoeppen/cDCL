// ==============================
// Fichier:			TDCLMNPPipe.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/01/2002
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
// The Original Code is TDCLMNPPipe.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// Filip Zawadiak and Nicolas Zinovieff. Portions created by the Initial
// Developers are Copyright (C) 1998-2004 the Initial Developers. All Rights
// Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Filip Zawadiak <philz@vyx.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
//
// Copyright (c) 2001 Filip Zawadiak <philz@vyx.net>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ===========
// $Id: TDCLMNPPipe.cp,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLMNPPipe.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>

// Commentaire intéressant de Philz:
// Just keep in mind that MNP implementation in lpkg is very primitive.
// I just wanted to have minimal functioning MNP support. It has one
// frame window size, which could harm download performance, especially
// on high speed links :-(

// ------------------------------------------------------------------------- //
//  Constantes
// ------------------------------------------------------------------------- //
const KUInt16 TDCLMNPPipe::kCRC16Table[256] =
{
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

// Caractères pour la transmission MNP.
const KUInt8 TDCLMNPPipe::SYN = 0x16;
const KUInt8 TDCLMNPPipe::DLE = 0x10;
const KUInt8 TDCLMNPPipe::STX = 0x02;
const KUInt8 TDCLMNPPipe::ETX = 0x03;
const KUInt8 TDCLMNPPipe::LA = 0x05;
const KUInt8 TDCLMNPPipe::LT = 0x04;
const KUInt8 TDCLMNPPipe::kDefaultCredits = 8;

const KUInt8 TDCLMNPPipe::kMNPHeader[24] =
{
	23,
	1,2,1,
	6,1,0,0,0,0,255,
	2,1,2,
	3,1,TDCLMNPPipe::kDefaultCredits,
	4,2,64,0,
	8,1,3
};

const KUInt8 TDCLMNPPipe::kMNPFooter[8] =
{
	7,2,1,1,255,2,1,0
};


// ------------------------------------------------------------------------- //
//  * TDCLMNPPipe( TDCLPipe* )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::TDCLMNPPipe(
			TDCLPipe* inSubPipe,
			KUInt32 inOutputMaxBufferSize /* = kDefaultOutputMaxBufferSize */ )
	:
		TDCLPipeProxy( inSubPipe ),
		mInputBuffer( nil ),
		mInputBufferCapacity( kInputFrameIncrement ),
		mInputBufferSize( 0 ),
		mInputBufferCrsr( 0 ),
		mOutputBuffer( nil ),
		mOutputBufferCapacity( inOutputMaxBufferSize ),
		mOutputBufferCrsr( 0 ),
		mInputPacketIndex( 0 ),
		mOutputPacketIndex( 0 ),
		mSendKeepAlive( false )
{
	// Création des mémoires tampon.
	mInputBuffer = (KUInt8*) ::malloc( mInputBufferCapacity );
	mOutputBuffer = (KUInt8*) ::malloc( inOutputMaxBufferSize );
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMNPPipe( void )
// ------------------------------------------------------------------------- //
TDCLMNPPipe::~TDCLMNPPipe( void )
{
	// Nettoyage des mémoires tampon.
	if (mInputBuffer)
	{
		::free( mInputBuffer );
	}

	if (mOutputBuffer)
	{
		::free( mOutputBuffer );
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void* outBuffer, KUInt32* ioCount )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	KUInt32 bytesLeft = *ioCount;
	KUInt32 bytesCopied = 0;
	while ( bytesLeft > 0 )
	{
		// Faut-il lire une nouvelle structure?
		if (mInputBufferCrsr == mInputBufferSize)
		{
			// On vide la mémoire tampon.
			mInputBufferCrsr = 0;
			mInputBufferSize = 0;

			// Et on la re-remplit.
			ReceiveData();
		}

		// Copie des données.
		KUInt32 copyCount = bytesLeft;
		if (copyCount + mInputBufferCrsr > mInputBufferSize)
		{
			copyCount = mInputBufferSize - mInputBufferCrsr;
		}

		(void) ::memcpy( (void*) &((KUInt8*) outBuffer)[bytesCopied],
				(const void*) &mInputBuffer[mInputBufferCrsr],
				copyCount );

		// Mise à jour des curseurs.
		mInputBufferCrsr += copyCount;
		bytesLeft -= copyCount;
		bytesCopied += copyCount;
		*ioCount = bytesCopied;
	}
}

// ------------------------------------------------------------------------- //
//	* Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	KUInt32 bytesLeft = *ioCount;
	KUInt32 bytesCopied = 0;
	while ( bytesLeft > 0 )
	{
		// Copie des données.
		KUInt32 copyCount = bytesLeft;
		if (copyCount + mOutputBufferCrsr > mOutputBufferCapacity)
		{
			copyCount = mOutputBufferCapacity - mOutputBufferCrsr;
		}

		(void) ::memcpy( (void*) &mOutputBuffer[mOutputBufferCrsr],
				(const void*) &((const KUInt8*) inBuffer)[bytesCopied],
				copyCount );

		// Mise à jour des curseurs.
		mOutputBufferCrsr += copyCount;
		bytesLeft -= copyCount;
		bytesCopied += copyCount;
		*ioCount = bytesCopied;

		// Faut-il écrire la structure?
		if (mOutputBufferCrsr == mOutputBufferCapacity)
		{
			SendData();
		}
	}
}

// ------------------------------------------------------------------------- //
//	* ReceiveData( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::ReceiveData( void )
{
	KUInt8* theInputFrame;
	KUInt32 theInputFrameSize;
	ReadFrame( &theInputFrame, &theInputFrameSize );

	if ( theInputFrameSize == 4 && theInputFrame[1] == LA )
	{
		if ( mSendKeepAlive )
		{
			// confirm LA frames again (NTK protocol), give credit
			SendACK();
		}
	}
	else
	{
		mInputPacketIndex = theInputFrame[2];
		SendACK();

		// Copie des donnÈes.
		KUInt32 toCopy = theInputFrameSize - 3;
		if ((mInputBufferSize + toCopy) > mInputBufferCapacity)
		{
			mInputBufferCapacity = mInputBufferSize + toCopy;
			mInputBuffer = (KUInt8*)
					::realloc(
							mInputBuffer,
							mInputBufferCapacity );
		}

		(void) ::memcpy(
					&mInputBuffer[mInputBufferSize],
					(const void*) &theInputFrame[3],
					toCopy );
		mInputBufferSize += toCopy;
	}

	// LibÈration.
	::free( theInputFrame );
}

// ------------------------------------------------------------------------- //
//	* SendData( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SendData( void )
{
	// La structure de sortie comprend les données précédées de 3 octets.
	KUInt32 theOutputFrameSize = mOutputBufferCrsr + 3;
	KUInt8* theOutputFrame = (KUInt8*) ::malloc( theOutputFrameSize );

	// Les 3 octets du début.
	theOutputFrame[0] = 2;	// Taille de l'entête.
	theOutputFrame[1] = LT;
	theOutputFrame[2] = mOutputPacketIndex;

	// Copie des données.
	(void) ::memcpy(
			&theOutputFrame[3],
			(const void*) mOutputBuffer,
			mOutputBufferCrsr );

	WriteFrame( theOutputFrame, theOutputFrameSize );
	mOutputPacketIndex++;

	// On vide la mémoire tampon de sortie.
	mOutputBufferCrsr = 0;

	// Libération de la structure.
	::free( theOutputFrame );
}

// ------------------------------------------------------------------------- //
//	* FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::FlushOutput( void )
{
	SendData();
	GetSubPipe()->FlushOutput();
}

// ------------------------------------------------------------------------- //
//	* SendACK( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::SendACK( void )
{
    KUInt8 frame[4];

    frame[0] = sizeof( frame ) - 1;
    frame[1] = LA;
    frame[2] = mInputPacketIndex;
    frame[3] = kDefaultCredits;

    WriteFrame( frame, sizeof( frame ) );
}

// ------------------------------------------------------------------------- //
//	* BytesAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLMNPPipe::BytesAvailable( void )
{
	Boolean theResult;

	if (mInputBufferCrsr < mInputBufferSize)
	{
		theResult = true;
	} else {
		theResult = GetSubPipe()->BytesAvailable();
	}

	return theResult;
}

// ------------------------------------------------------------------------- //
//	* ReadFrame( KUInt8**, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::ReadFrame( KUInt8** outFrame, KUInt32* outSize )
{
	// Récupération de la connexion dont on est le mandataire.
	TDCLPipe* theSubPipe = GetSubPipe();

	KUInt8* theFrame = (KUInt8*) ::malloc( kInputFrameIncrement );
	KUInt32 theFrameCapacity = kInputFrameIncrement;
	KUInt32 theFrameSize = 0;

	KUInt8 theCurrentByte;
	KUInt32 theCount = sizeof( theCurrentByte );
	KUInt16 theCRC = 0;

	// Passage des caractères SYN au début.
	do {
		theSubPipe->Read( &theCurrentByte, &theCount );
	} while ( theCurrentByte == SYN );

	if (theCurrentByte != DLE)
	{
		throw DCLUnknownError;
	}
	// Lecture du suivant. Est-ce un STX?
	theSubPipe->Read( &theCurrentByte, &theCount );
	if (theCurrentByte != STX)
	{
		throw DCLUnknownError;
	}

	do {
		if (theFrameSize == theFrameCapacity)
		{
			// Agrandissement de la taille de la structure.
			theFrameCapacity += kInputFrameIncrement;
			theFrame =
				(KUInt8*) ::realloc( theFrame, theFrameCapacity );
		}

		theSubPipe->Read( &theCurrentByte, &theCount );
		if (theCurrentByte == DLE)
		{
			theSubPipe->Read( &theCurrentByte, &theCount );
			if (theCurrentByte == ETX)
			{
				theCRC = UpdateCRC16( theCRC, theCurrentByte );
				theSubPipe->Read( &theCurrentByte, &theCount );
				KUInt16 theOriginalCRC = theCurrentByte;
				theSubPipe->Read( &theCurrentByte, &theCount );
				theOriginalCRC |= theCurrentByte << 8;
				if (theOriginalCRC == theCRC)
				{
					break;
				} else {
					// Nettoyage.
					::free( theFrame );
					throw DCLUnknownError;
				}
			} else if (theCurrentByte == DLE) {
				theFrame[theFrameSize++] = theCurrentByte;
				theCRC = UpdateCRC16( theCRC, theCurrentByte );
			} else {
				// Nettoyage.
				::free( theFrame );
				throw DCLUnknownError;
			}
		} else {
			theFrame[theFrameSize++] = theCurrentByte;
			theCRC = UpdateCRC16( theCRC, theCurrentByte );
		}
	} while ( true );

	// Retour du résultat.
	*outFrame = theFrame;
	*outSize = theFrameSize;
}

// ------------------------------------------------------------------------- //
//	* WriteFrame( const KUInt8*, KUInt32 )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::WriteFrame( const KUInt8* inFrame, KUInt32 inSize )
{
	// Récupération de la connexion dont on est le mandataire.
	TDCLPipe* theSubPipe = GetSubPipe();

	// Entête (SYN, DLE, STX)
	KUInt8 theHeader[3] = { SYN, DLE, STX };
	KUInt32 theCount = sizeof( theHeader );
	theSubPipe->Write( theHeader, &theCount );

	// Écriture des données, en remplaçant les DLE par DLE, DLE.
	KUInt32 indexFrame;
	KUInt8 theCurrentByte;
	theCount = sizeof( theCurrentByte );
	KUInt16 theCRC = 0;

	for (indexFrame = 0; indexFrame < inSize; indexFrame++)
	{
		theCurrentByte = inFrame[indexFrame];

		if (theCurrentByte == DLE)
		{
			// On écrit deux fois DLE.
			theSubPipe->Write( &theCurrentByte, &theCount );
		}
		theSubPipe->Write( &theCurrentByte, &theCount );
		theCRC = UpdateCRC16( theCRC, theCurrentByte );
	}

	// DLE, ETX puis CRC
	theCurrentByte = DLE;
	theSubPipe->Write( &theCurrentByte, &theCount );
	theCurrentByte = ETX;
	theSubPipe->Write( &theCurrentByte, &theCount );
	theCRC = UpdateCRC16( theCRC, theCurrentByte );

	// CRC
	theCurrentByte = (KUInt8) (theCRC & 0x00FF);
	theSubPipe->Write( &theCurrentByte, &theCount );
	theCurrentByte = (KUInt8) ((theCRC & 0xFF00) >> 8);
	theSubPipe->Write( &theCurrentByte, &theCount );
}

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink* inLink )
// ------------------------------------------------------------------------- //
TDCLCommLayer*
TDCLMNPPipe::Connected( TDCLLink* inLink )
{
	TDCLCommLayer* theResult = TDCLPipeProxy::Connected( inLink );

	// Echange de l'entête MNP (?)
	KUInt8* theInputFrame;
	KUInt32 theInputFrameSize;
	ReadFrame( &theInputFrame, &theInputFrameSize );
	::free( theInputFrame );

	WriteFrame( kMNPHeader, sizeof( kMNPHeader ) );
	mInputPacketIndex = 1;
	ReadFrame( &theInputFrame, &theInputFrameSize );
	mOutputPacketIndex = 1;

	// Nettoyage
	::free( theInputFrame );

	return theResult;
}

// ------------------------------------------------------------------------- //
//  * Disconnect( void )
// ------------------------------------------------------------------------- //
void
TDCLMNPPipe::Disconnect( void )
{
	// Envoi de la séquence MNP de fin.
	WriteFrame( kMNPFooter, sizeof( kMNPFooter ) );

	TDCLPipeProxy::Disconnect();
}

// ------------------------------------------------------------------------- //
//	* UpdateCRC16( KUInt16, KUInt8 )
// ------------------------------------------------------------------------- //
KUInt16
TDCLMNPPipe::UpdateCRC16( KUInt16 inCRC, KUInt8 inByte )
{
	return (KUInt16)(((inCRC >> 8) & 0xFF)
			^ kCRC16Table[ (inCRC & 0xFF) ^ inByte ]);
}

// ========================================================================== //
// ===  ALL USERS PLEASE NOTE  ========================                       //
//                                                                            //
// Compiler optimizations have been made to macro expand LET into a WITHOUT-  //
// INTERRUPTS special form so that it can PUSH things into a stack in the     //
// LET-OPTIMIZATION area, SETQ the variables and then POP them back when it's //
// done.  Don't worry about this unless you use multiprocessing.              //
// Note that LET *could* have been defined by:                                //
//                                                                            //
//         (LET ((LET '`(LET ((LET ',LET))                                    //
//                         ,LET)))                                            //
//         `(LET ((LET ',LET))                                                //
//                 ,LET))                                                     //
//                                                                            //
// This is believed to speed up execution by as much as a factor of 1.01 or   //
// 3.50 depending on whether you believe our friendly marketing               //
// representatives.  This code was written by a new programmer here (we       //
// snatched him away from Itty Bitti Machines where we was writting COUGHBOL  //
// code) so to give him confidence we trusted his vows of "it works pretty    //
// well" and installed it.                                                    //
// ========================================================================== //
