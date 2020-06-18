// ==============================
// Fichier:			TDCLMacOS7File.cp
// Projet:			Desktop Connection Library
//
// Créé le:			09/02/2003
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
// The Original Code is TDCLMacOS7File.cp.
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
// $Id: TDCLMacOS7File.cp,v 1.8 2004/11/24 18:11:19 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7File.h>

// ANSI C
#include <stdlib.h>

// MacOS
#include <Files.h>
#include <Script.h>

// K
#include <K/Math/UTInt64.h>
#include <K/Unicode/UUTF16Conv.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLFullException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>
#include <DCL/Interfaces/MacOS/TDCLMacOS7Folder.h>

// ------------------------------------------------------------------------- //
//  * TDCLMacOS7File( TDCLMacFiles*, const FSSpec* )
// ------------------------------------------------------------------------- //
TDCLMacOS7File::TDCLMacOS7File(
				TDCLMacFiles* inFilesIntf,
				const FSSpec* inFSSpec )
	:
		TDCLMacFile
			(
				inFilesIntf,
				TDCLFSItemRef( new TDCLMacOS7Folder(
											inFilesIntf,
											inFSSpec->vRefNum,
											inFSSpec->parID )))
{
	// Copie du FSSpec
	mFileFSSpec = *inFSSpec;
}

// ------------------------------------------------------------------------- //
//  * TDCLMacOS7File( TDCLMacFiles*, TDCLFSItemRef, const FSSpec* )
// ------------------------------------------------------------------------- //
TDCLMacOS7File::TDCLMacOS7File(
					TDCLMacFiles* inFilesIntf,
					TDCLFSItemRef inFolder,
					const FSSpec* inFSSpec )
	:
		TDCLMacFile( inFilesIntf, inFolder )
{
	// Copie du FSSpec
	mFileFSSpec = *inFSSpec;
}

// ------------------------------------------------------------------------- //
//  * ~TDCLMacOS7File( void )
// ------------------------------------------------------------------------- //
TDCLMacOS7File::~TDCLMacOS7File()
{
	Close();
}

// ------------------------------------------------------------------------- //
//  * Open( Boolean )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Open( Boolean inReadOnly )
{
	// Tentons d'ouvrir le fichier.
	OSErr theErr = ::FSpOpenDF(
						&mFileFSSpec,
						inReadOnly ? fsRdPerm : fsRdWrPerm,
						&mRefNum );
	
	/*
		noErr 0 No error
		nsvErr –35 No such volume
		ioErr –36 I/O error
		bdNamErr –37 Bad filename
		tmfoErr –42 Too many files open
		fnfErr –43 File not found
		opWrErr –49 File already open for writing
		permErr –54 Attempt to open locked file for writing
		dirNFErr –120 Directory not found or incomplete pathname
		afpAccessDenied –5000 User does not have the correct access to the file
	*/
	switch (theErr)
	{	
		case noErr:
			break;
			
		case nsvErr:
		case bdNamErr:
		case fnfErr:
		case dirNFErr:
			throw DCLPlatformDoesntExist( theErr );
			break;
			
		case permErr:
			throw DCLPlatformReadOnly( theErr );
			break;
			
		case afpAccessDenied:
			throw DCLPlatformPermission( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
	
	// Puisque le fichier est ouvert, on l'indique à la classe mère.
	DoOpen( inReadOnly );
}

// ------------------------------------------------------------------------- //
//  * Create( OSType, OSType )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Create( OSType inCreator,
						OSType inFileType )
{
	// Tentons de créer le fichier.
	OSErr theErr = ::FSpCreate(
				&mFileFSSpec, inCreator, inFileType, smSystemScript );
	
	/*
		noErr 0 No error
		dirFulErr –33 File directory full
		dskFulErr –34 Disk is full
		nsvErr –35 No such volume
		ioErr –36 I/O error
		bdNamErr –37 Bad filename
		fnfErr –43 Directory not found or incomplete pathname
		wPrErr –44 Hardware volume lock
		vLckdErr –46 Software volume lock
		dupFNErr –48 Duplicate filename and version
		dirNFErr –120 Directory not found or incomplete pathname
		afpAccessDenied –5000 User does not have the correct access
		afpObjectTypeErr –5025 A directory exists with that name
	*/
	switch (theErr)
	{	
		case noErr:
			break;
		
		case dirFulErr:
		case dskFulErr:
			throw DCLPlatformFull( theErr );
			break;
		
		case nsvErr:
		case bdNamErr:
		case fnfErr:
		case dirNFErr:
			throw DCLPlatformDoesntExist( theErr );
			break;
		
		case wPrErr:
		case vLckdErr:
			throw DCLPlatformReadOnly( theErr );
			break;
		
		case dupFNErr:
		case afpObjectTypeErr:
			throw DCLPlatformExistsAlready( theErr );
			break;
			
		case afpAccessDenied:
			throw DCLPlatformPermission( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
}

// ------------------------------------------------------------------------- //
//  * SetMetaData( OSType, OSType )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::SetMetaData(
						OSType inCreator,
						OSType inFileType )
{
	DoSetMetaData( &mFileFSSpec, inCreator, inFileType );
}

// ------------------------------------------------------------------------- //
//  * Delete( void )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Delete( void )
{
	// Vérifions que le fichier n'est pas ouvert.
	if (IsOpen())
	{
		throw DCLBadStateError;
	}
	
	// Tentons de supprimer le fichier.
	OSErr theErr = ::FSpDelete( &mFileFSSpec );
	
	/*
		noErr 0 No error
		nsvErr -35 No such volume
		ioErr -36 I/O error
		bdNamErr -37 Bad filename
		fnfErr -43 File not found
		wPrErr -44 Hardware volume loc
		fLckdErr -45 File is locked
		vLckdErr -46 Software volume lock
		fBsyErr -47 File busy, directory not empty, or working directory control block open
		dirNFErr -120 Directory not found or incomplete pathname
		afpAccessDenied -5000 User does not have the correct access
	*/
	switch (theErr)
	{	
		case noErr:
			break;
		
		case nsvErr:
		case bdNamErr:
		case fnfErr:
		case dirNFErr:
			throw DCLPlatformDoesntExist( theErr );
			break;
		
		case wPrErr:
		case fLckdErr:
		case vLckdErr:
			throw DCLPlatformReadOnly( theErr );
			break;

		case fBsyErr:
		case afpAccessDenied:
			throw DCLPlatformPermission( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
}

// ------------------------------------------------------------------------- //
//  * MakeName( void ) const
// ------------------------------------------------------------------------- //
KUInt16*
TDCLMacOS7File::MakeName( void ) const
{
	// Allocation de l'espace nécessaire (avec malloc)
	unsigned int theSize = (unsigned int) mFileFSSpec.name[0];
	KUInt16* theResult = (KUInt16*)
		::malloc( theSize * sizeof( KUInt16 ) );
	
	// Conversion depuis MacRoman
	UUTF16Conv::FromMacRoman(
		(KUInt8*) &mFileFSSpec.name[1], theResult, theSize );
	
	// Ajout du terminateur.
	theResult[theSize] = 0;
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetLength( void )
// ------------------------------------------------------------------------- //
KUInt64
TDCLMacOS7File::GetLength( void )
{
	HParamBlockRec myHPBRec;
	
	myHPBRec.fileParam.ioNamePtr = (StringPtr) &mFileFSSpec.name;
	myHPBRec.fileParam.ioVRefNum = mFileFSSpec.vRefNum;
	myHPBRec.fileParam.ioFVersNum = 0;
	myHPBRec.fileParam.ioDirID = mFileFSSpec.parID;
	myHPBRec.fileParam.ioFDirIndex = 0;
	
	OSErr theErr = ::PBHGetFInfoSync( &myHPBRec );
	
	/*
		noErr				0		No error
		nsvErr				-35		No such volume
		ioErr				-36		I/O error
		bdNamErr			-37		Bad filename
		fnfErr				-43		File not found
		paramErr			-50		No default volume
		dirNFErr			-120	Directory not found or incomplete pathname
		afpAccessDenied		-5000	User does not have the correct access
		afpObjectTypeErr	-5025	Directory not found or incomplete pathname
	*/
	
	switch ( theErr )
	{
		case noErr:
			break;
		
		case fnfErr:
		case dirNFErr:
		case afpObjectTypeErr:
			throw DCLPlatformDoesntExist( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */

	return (KUInt32) myHPBRec.fileParam.ioFlLgLen;
}

// ------------------------------------------------------------------------- //
//  * SetCursor( KSInt64, ECursorMode )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::SetCursor( KSInt64 inPos, ECursorMode inMode )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	short posMode = fsFromMark;
	switch (inMode)
	{
		case kFromStart:
			posMode = fsFromStart;
			break;
			
		case kFromLEOF:
			posMode = fsFromLEOF;
			break;
		
		case kFromCursor:
			break;
	} /* switch (inMode) */
	
	OSErr theErr = ::SetFPos( mRefNum, posMode, UTInt64::ToKSInt32( inPos ) );
	
	/*
		noErr 0 No error
		ioErr –36 I/O error
		fnOpnErr –38 File not open
		eofErr –39 Logical end-of-file reached
		posErr –40 Attempt to position mark before start of file
		rfNumErr –51 Bad reference number
	*/
	
	switch ( theErr )
	{
		case noErr:
			break;
		
		case fnOpnErr:
		case rfNumErr:
			throw DCLPlatformBadStateError( theErr );
			break;
		
		case eofErr:
		case posErr:
			throw DCLPlatformPosition( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
	
	// Done.
}

// ------------------------------------------------------------------------- //
//  * GetCursor( void )
// ------------------------------------------------------------------------- //
KSInt64
TDCLMacOS7File::GetCursor( void )
{
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	long thePosition;
	OSErr theErr = ::GetFPos( mRefNum, &thePosition );

	/*
		noErr	0	No error
		ioErr	-36	I/O error
		fnOpnErr	-38	File not open
		rfNumErr	-51	Bad reference number
		gfpErr	-52	Error during GetFPos
	*/

	switch ( theErr )
	{
		case noErr:
			break;
		
		case fnOpnErr:
		case rfNumErr:
			throw DCLPlatformBadStateError( theErr );
			break;
		
		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */

	return (KSInt64) thePosition;
} 

// ------------------------------------------------------------------------- //
//  * Close( void )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Close( void )
{
	if (IsOpen())
	{
		OSErr theErr = ::FSClose( mRefNum );
	
		/*
			noErr 0 No error
			ioErr –36 I/O error
			fnOpnErr –38 File not open
			fnfErr –43 File not found
			rfNumErr –51 Bad reference number
		*/
	
		switch ( theErr )
		{
			case noErr:
				break;
		
			case fnfErr:
				throw DCLPlatformDoesntExist( theErr );
				break;
		
			default:
				throw DCLPlatformUnknownError( theErr );
		} /* switch (theErr) */
	
		// Done.

		DoClose();		
	}
}

// ------------------------------------------------------------------------- //
//  * Read( void*, long* )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Read( void* outBuffer, KUInt32* ioCount )
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	
	// J'appelle FSRead tout simplement.
	OSErr theErr = ::FSRead( mRefNum, (long*) ioCount, outBuffer );
	
	// Si j'ai trouvé une erreur autre que EOF, je lance une exception
	
	/*
		noErr 0 No error
		ioErr –36 I/O error
		fnOpnErr –38 File not open
		eofErr –39 Logical end-of-file reached
		posErr –40 Attempt to position mark before start of file
		fLckdErr -45 File is locked
		paramErr –50 Negative count
		rfNumErr –51 Bad reference number
		afpAccessDenied –5000 User does not have the correct access to the file
	*/
	
	switch (theErr)
	{	
		case noErr:
		case eofErr:
			break;
		
		case fLckdErr:
			throw DCLPlatformReadOnly( theErr );
			break;
		
		case afpAccessDenied:
			throw DCLPlatformPermission( theErr );
			break;
		
		case posErr:
			throw DCLPlatformPosition( theErr );
			break;

		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
	
	// Done.
}

// ------------------------------------------------------------------------- //
//  * Write( const void*, long* )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::Write( const void* outBuffer,
					KUInt32* ioCount )
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}
	
	// Vérifions que le fichier est ouvert en lecture/écriture.
	if (IsReadOnly())
	{
		throw DCLReadOnly;
	}

	// Appel à FSWrite
	OSErr theErr = ::FSWrite( mRefNum, (long*) ioCount, outBuffer );
	
	// Si j'ai trouvé une erreur autre que EOF, je lance une exception
	
	/*
		noErr 0 No error
		dskFulErr –34 Disk full
		ioErr –36 I/O error
		fnOpnErr –38 File not open
		posErr –40 Attempt to position mark before start of file
		wPrErr –44 Hardware volume lock
		fLckdErr –45 File is locked
		vLckdErr –46 Software volume lock
		paramErr –50 Negative count
		rfNumErr –51 Bad reference number
		wrPermErr –61 Read/write permission doesn’t allow writing
	*/
	
	switch (theErr)
	{	
		case noErr:
			break;
		
		case wPrErr:
		case fLckdErr:
		case vLckdErr:
		case wrPermErr:
			throw DCLPlatformReadOnly( theErr );
			break;
		
		case dskFulErr:
			throw DCLPlatformFull( theErr );
			break;
			
		case posErr:
			throw DCLPlatformPosition( theErr );
			break;

		default:
			throw DCLPlatformUnknownError( theErr );
	} /* switch (theErr) */
	
	// Done.
}

// ------------------------------------------------------------------------- //
//  * FlushOutput( void )
// ------------------------------------------------------------------------- //
void
TDCLMacOS7File::FlushOutput( void )
{
	// Vérifions que le fichier est ouvert.
	if (!IsOpen())
	{
		throw DCLBadStateError;
	}

	// Appel à PBFlushFileSync
	ParamBlockRec myParamBlock;	// Parameter block pour PBFlushFileSync.
	myParamBlock.fileParam.ioFRefNum = mRefNum;
	OSErr theErr = ::PBFlushFileSync( &myParamBlock );
	
	// Si j'ai trouvé une erreur, je lance une exception
	
	/*
		noErr		0	No error
		nsvErr		-35	Volume not found
		ioErr 		-36	I/O error
		fnOpnErr 	-38	File not open
		fnfErr		-43	File not found
		rfNumErr	-51	Bad reference number
		extFSErr	-58	External file system
	*/
	
	if (theErr)
	{	
		throw DCLPlatformUnknownError( theErr );
	}
	
	// Done.
}

// ------------------------------------------------------------------------- //
//  * GetVRefNum( void ) const
// ------------------------------------------------------------------------- //
short
TDCLMacOS7File::GetVRefNum( void ) const
{
	return mFileFSSpec.vRefNum;
}

// ------------------------------------------------------------------------- //
//  * IsOnDesktop( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLMacOS7File::IsOnDesktop( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetStringPath( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacOS7File::GetStringPath( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetIcon( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacOS7File::GetIcon( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetCreationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacOS7File::GetCreationDate( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetModificationDate( void ) const
// ------------------------------------------------------------------------- //
KUInt32
TDCLMacOS7File::GetModificationDate( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ------------------------------------------------------------------------- //
//  * GetKind( void ) const
// ------------------------------------------------------------------------- //
TDCLNSRef
TDCLMacOS7File::GetKind( void ) const
{
#warning TODO
	throw DCLNotImplementedError;
}

// ============================================================== //
// Unix will self-destruct in five seconds... 4... 3... 2... 1... //
// ============================================================== //