// ==============================
// Fichier:			TDCLBufferedPipe.cp
// Projet:			Desktop Connection Library
//
// Créé le:			26/10/2002
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
// The Original Code is TDCLBufferedPipe.cp.
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
// $Id: TDCLBufferedPipe.cp,v 1.6 2004/11/24 14:08:53 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLBufferedPipe.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Tests/KDebug.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLMemError.h>

// #undef KDEBUG_ENABLED
// #define KDEBUG_ENABLED 1

// ------------------------------------------------------------------------- //
//	* TDCLBufferedPipe( TDCLCommLayer*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLBufferedPipe::TDCLBufferedPipe(
			TDCLPipe* inSubPipe,
			KUInt32 inBufferCapacity /* = kDefaultBufferCapacity */ )
	:
		TDCLPipeProxy( inSubPipe ),
		mBuffer( nil ),
		mBufferSize( 0 ),
		mBufferCapacity( inBufferCapacity )
{
	mBuffer = ::malloc( inBufferCapacity );
	if ((inBufferCapacity > 0) && (mBuffer == nil))
	{
		throw DCLMemError;
	}
}

// ------------------------------------------------------------------------- //
//	* ~TDCLBufferedPipe( void )
// ------------------------------------------------------------------------- //
TDCLBufferedPipe::~TDCLBufferedPipe( void )
{
	if (mBuffer)
	{
		::free( mBuffer );
	}
}

// ------------------------------------------------------------------------- //
//	* FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLBufferedPipe::FlushOutput( void )
{
	KDEBUG( "FlushOutput" );

	// Ecriture des données.
	KUInt32 actualAmountWritten = mBufferSize;
	if (actualAmountWritten > 0)
	{
		try {
			GetSubPipe()->Write( mBuffer, &actualAmountWritten );
		} catch (...) {
			if (actualAmountWritten != mBufferSize)
			{
				// Déplacement des données.
				(void) ::memmove(
							(void*) mBuffer,
							&((const KUInt8*) mBuffer)[actualAmountWritten],
							mBufferSize - actualAmountWritten );
				mBufferSize -= actualAmountWritten;
			}
			throw;	// Rethrow
		}

		// Tout a été écrit avec succès.
		mBufferSize = 0;
	}

	// On vide la mémoire tampon du flux fils.
	GetSubPipe()->FlushOutput();
}

// ------------------------------------------------------------------------- //
//  * Read( void* outBuffer, KUInt32* ioCount )
// ------------------------------------------------------------------------- //
void
TDCLBufferedPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	GetSubPipe()->Read( outBuffer, ioCount );
}

// ------------------------------------------------------------------------- //
//	* Write( const void*, KUInt32* )
// ------------------------------------------------------------------------- //
void
TDCLBufferedPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	KDEBUG1( "Write( inBuffer, ioCount = %u)", (unsigned int) *ioCount );

	// On remplit la mémoire tampon.
	KUInt32 toWrite = *ioCount;
	const KUInt8* theBuffer = (const KUInt8*) inBuffer;
	try {
		while (toWrite > 0)
		{
			// Remplissage de la mémoire tampon.
			KUInt32 copyAmount = toWrite;
			if (copyAmount + mBufferSize > mBufferCapacity)
			{
				copyAmount = mBufferCapacity - mBufferSize;
			}

			(void) ::memcpy(
				&((KUInt8*) mBuffer)[mBufferSize], theBuffer, copyAmount );

			mBufferSize += copyAmount;

			// Envoi des données si la mémoire tampon est pleine.
			if (mBufferSize == mBufferCapacity)
			{
				KUInt32 actualAmountWritten = mBufferSize;
				try {
					GetSubPipe()->Write( mBuffer, &actualAmountWritten );
				} catch (...) {
					if (mBufferSize != actualAmountWritten)
					{
						// Déplacement des données.
						(void) ::memmove(
									(void*) mBuffer,
									&((const KUInt8*) mBuffer)
													[actualAmountWritten],
									mBufferSize - actualAmountWritten );
						mBufferSize -= actualAmountWritten;
					}
					throw;
				}

				mBufferSize = 0;
			}

			toWrite -= copyAmount;
			theBuffer += copyAmount;
		}
	} catch ( ... ) {
		// Mise à jour de ce qu'on a réussi à écrire.
		*ioCount -= toWrite;
		throw;	// On relance.
	}
}

// ============================================================================= //
// A certain monk had a habit of pestering the Grand Tortue (the only one who    //
// had ever reached the Enlightenment 'Yond Enlightenment), by asking whether    //
// various objects had Buddha-nature or not.  To such a question Tortue          //
// invariably sat silent.  The monk had already asked about a bean, a lake,      //
// and a moonlit night.  One day he brought to Tortue a piece of string, and     //
// asked the same question.  In reply, the Grand Tortue grasped the loop         //
// between his feet and, with a few simple manipulations, created a complex      //
// string which he proferred wordlessly to the monk.  At that moment, the monk   //
// was enlightened.                                                              //
//                                                                               //
// From then on, the monk did not bother Tortue.  Instead, he made string after  //
// string by Tortue's method; and he passed the method on to his own disciples,  //
// who passed it on to theirs.                                                   //
// ============================================================================= //
