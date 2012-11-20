// ==============================
// Fichier:			TDCLDockCmdLoadPkg.cp
// Projet:			Desktop Connection Library
//
// Créé le:			15/08/2001
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
// The Original Code is TDCLDockCmdLoadPkg.cp.
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
// $Id: TDCLDockCmdLoadPkg.cp,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdLoadPkg.h>

// ANSI C
#include <stdlib.h>

// K
#include <K/Math/UTInt64.h>

// DCL
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLFullException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Streams/TDCLStream.h>

// ------------------------------------------------------------------------- //
//  * TDCLDockCmdLoadPkg( TDCLFile* )
// ------------------------------------------------------------------------- //
TDCLDockCmdLoadPkg::TDCLDockCmdLoadPkg( TDCLFSItemRef inPackage )
	:
		TDCLDockCommand( kDLoadPackage ),
		mPackage( inPackage ),
		mPackageFile( nil )
{
	mPackageFile = (TDCLFile*) mPackage;
}

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt32
TDCLDockCmdLoadPkg::GetLength( void ) const
{
	// Récupération de la taille du paquet à partir du fichier.
	// Si un paquet fait plus de 0x60000000, ça va merder sur le Newton.
	// Donc on lance une exception dans ce cas.
	KUInt64 theResult = mPackageFile->GetLength();
	if (theResult >= 0x60000000)
	{
		throw DCLFull;
	}
	
	return UTInt64::GetLo( theResult );
}

// ------------------------------------------------------------------------- //
//  * SendBody( TDCLPipe*, KUInt32, ProgressFuncPtr, void* )
// ------------------------------------------------------------------------- //
void
TDCLDockCmdLoadPkg::SendBody(
					TDCLStream* inStream,
					KUInt32 inLength,
					ProgressFuncPtr inProgressFuncPtr,
					void* inRefCon )
{
	// Send the file in little chunks.
	KUInt32 remaining = inLength;				// Number of bytes left.
	
	char* theBuffer = new char [ kChunkSize ];	// Buffer for a chunk.
	if (theBuffer == nil)
	{
		throw DCLMemError;
	}
	
	// Ouverture du fichier.
	mPackageFile->Open( true /* lecture seule */ );
		
	try {
		do {
			KUInt32 packetSize = kChunkSize;		// Size of the packet. At most size
												// of the chunk.
			if (remaining < kChunkSize)
			{
				packetSize = remaining;			// There are less bytes available.
			}
			
			// Lecture du fichier.
			mPackageFile->Read( theBuffer, &packetSize );
			
			if (packetSize == 0)
			{
				// Abnormal EOF
				throw DCLPosition;
			}
		
			// Then, I output it to the pipe.
			inStream->Write( theBuffer, &packetSize );
		
			// On ajuste la taille qui reste.
			remaining -= packetSize;

			// On prévient si inProgressFuncPtr n'est pas nil.
			if (inProgressFuncPtr)
			{
				inStream->FlushOutput();
				(*inProgressFuncPtr)(
						inRefCon,
						((double) (16 + inLength - remaining))
							/ ((double) (16 + inLength)) );
			}
		} while ( remaining );
		
	} catch (...) {
		// Libération de la mémoire.
		delete [] theBuffer;

		// Fermeture du fichier.
		mPackageFile->Close();
		
		// On relance l'exception.	
		throw;
	}
	
	// Fermeture du fichier.
	mPackageFile->Close();		
	
	// Libération de la mémoire.
	delete [] theBuffer;
}

// =============================================================================== //
// Eudaemonic research proceeded with the casual mania peculiar to this part of    //
// the world.  Nude sunbathing on the back deck was combined with phone calls to   //
// Advanced Kinetics in Costa Mesa, American Laser Systems in Goleta, Automation   //
// Industries in Danbury, Connecticut, Arenberg Ultrasonics in Jamaica Plain,      //
// Massachusetts, and Hewlett Packard in Sunnyvale, California, where Norman       //
// Packard's cousin, David, presided as chairman of the board. The trick was to    //
// make these calls at noon, in the hope that out-to-lunch executives would return //
// them at their own expense.  Eudaemonic Enterprises, for all they knew, might be //
// a fast-growing computer company branching out of the Silicon Valley.  Sniffing  //
// the possibility of high-volume sales, these executives little suspected that    //
// they were talking on the other end of the line to a naked physicist crazed      //
// over roulette.                                                                  //
//                 -- Thomas Bass, "The Eudaemonic Pie"                            //
// =============================================================================== //
