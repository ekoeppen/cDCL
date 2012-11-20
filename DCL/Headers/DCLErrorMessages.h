// ==============================
// Fichier:			DCLErrorMessages.h
// Projet:			Desktop Connection Library
//
// Créé le:			22/08/2000
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
// The Original Code is DCLErrorMessages.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLErrorMessages.h,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __DCLERRORMESSAGES__
#define __DCLERRORMESSAGES__

// Purpose: Error messages as consts
// On MacOS, you'd better use STR# resources as I coded it.
//	(this file can be processed by Rez, BTW)

// Exceptions
#define kExCodesRes_ID 1

#define kExReadOnly_Str			"File was opened read only"
#define kExLocked_Str			"File/Volume/Directory is locked"
#define kExAlreadyExists_Str	"File already exists"
#define kExDoesntExist_Str		"File not found"
#define	kExAccessDenied_Str		"Access denied"
#define	kExFull_Str				"Disk/Directory is full"
#define	kExPositionErr_Str		"Can't reach this position in file"
#define	kExNotConnected_Str		"Not connected"
#define	kExAlreadyConnected_Str	"Already connected"
#define	kExNotAvailable_Str		"The required feature is not available on your OS/machine"
#define	kExNewtonError_Str		"The Newton returned an error (%i)"
#define	kExMemFullErr_Str		"Not enough memory error"
#define	kExBadDockCmd_Str		"Not a valid dock message (check I'm connected to the Dock/Connection application)"
#define kExSyntaxError_Str		"Syntax Error"
#define kExNotImplemented_Str	"Internal error: unimplemented feature (sub-classes should override)"

#define	kExUnknownError_Str		"Unknown Exception (%i)"

// IDs exceptions values (also used for MacOS resources)
#define kExReadOnly				1
#define kExLocked				2
#define kExAlreadyExists		3
#define kExDoesntExist			4
#define kExAccessDenied			5
#define kExFull					6
#define kExPositionErr			8
#define kExNotConnected			9
#define kExAlreadyConnected		10
#define kExNotAvailable			11
#define kExNewtonError			12
#define kExMemFullErr			13
#define kExBadDockCmd			14
#define kExSyntaxError			15
#define kExNotImplemented		16
#define kExUnknownError			17

// DILs Errors
#define kDILErrorRes_ID 4

// General DIL errors
#define kDILOutOfMemory_Str						"Out of memory"
#define kDILInvalidParameter_Str				"Invalid parameter"
#define kDILInternalError_Str					"Internal error"
#define kDILErrorReadingFromPipe_Str			"Error reading from pipe"
#define kDILErrorWritingToPipe_Str				"Error writing to pipe"
#define kDILInvalidHandle_Str					"Invalid handle"

// CDIL errors
#define kCDILNotInitialized_Str					"CDIL - Not initialized"
#define kCDILServiceNotSupported_Str			"CDIL - Service not supported"
#define kCDILBadPipeState_Str					"CDIL - Bad pipe state"
#define kCDILTimeout_Str						"CDIL - Timeout"
#define kCDILPipeDisconnected_Str				"CDIL - Pipe disconnected"
#define kCDILIndexOutOfRange_Str				"CDIL - Index out of range"
#define kCDILBufferTooSmall_Str					"CDIL - Buffer too small"
#define kCDILPlatformError_Str					"CDIL - Platform error"

#define kCDILTCPCantFindLibraryFns_Str			"CDIL - TCP can't find library functions"
#define kCDILTCPInsufficientVersion_Str			"CDIL - TCP insufficient version"
#define kCDILTCPNoSockets_Str					"CDIL - TCP no sockets"

// FDIL errors
	/* Hard errors -- you should always be looking for these. */

#define kFDILUnknownStreamVersion_Str			"FDIL - Unknown stream version"
#define kFDILStreamCorrupted_Str				"FDIL - Stream corrupted"
#define kFDILUnsupportedCompression_Str			"FDIL - Unsupported compression"
#define kFDILCouldNotCompressData_Str			"FDIL - Could not compress data"
#define kFDILCouldNotDecompressData_Str			"FDIL - Could not decompress data"
#define kFDILUnsupportedStoreVersion_Str		"FDIL - Unsupported store version"
#define kFDILErrorCreatingStore_Str				"FDIL - Error creatingStore"
#define kFDILErrorWritingToStore_Str			"FDIL - Error writingToStore"
#define kFDILErrorReadingFromStore_Str			"FDIL - Error reading from store"

	/* Soft errors -- you get these only if you feed in bad data. */

#define kFDILNotInitialized_Str					"FDIL - Not Initialized"

#define kFDILExpectedInteger_Str				"FDIL - Expected integer"
#define kFDILExpectedPointerObject_Str			"FDIL - Expected pointer object"
#define kFDILExpectedImmediate_Str				"FDIL - Expected immediate"
#define kFDILExpectedMagicPointer_Str			"FDIL - Expected magic pointer"

#define kFDILExpectedArray_Str					"FDIL - Expected array"
#define kFDILExpectedFrame_Str					"FDIL - Expected frame"
#define kFDILExpectedBinary_Str					"FDIL - Expected binary"
#define kFDILExpectedLargeBinary_Str			"FDIL - Expected large binary"

#define kFDILExpectedReal_Str					"FDIL - Expected real"
#define kFDILExpectedString_Str					"FDIL - Expected string"
#define kFDILExpectedSymbol_Str					"FDIL - Expected symbol"
#define kFDILExpectedChar_Str					"FDIL - Expected char"

#define kFDILNULLPointer_Str					"FDIL - NULL pointer"
#define kFDILExpectedPositiveValue_Str			"FDIL - Expected positiveValue"
#define kFDILExpectedNonNegativeValue_Str		"FDIL - Expected non negative value"
#define kFDILValueOutOfRange_Str				"FDIL - Value out of range"
#define kFDILSymbolTooLong_Str					"FDIL - Symbol too long"
#define kFDILIllegalCharInSymbol_Str			"FDIL - Illegal char in symbol"
#define kFDILInvalidClass_Str					"FDIL - Invalid class"
#define kFDILPointerObjectIsFree_Str			"FDIL - Pointer object is free"

	/* Internal errors -- you should never see these. */

#define kFDILLBReadingFromUnwrittenPage_Str	"FDIL - LB reading from unwritten page"

// PDIL errors

#define kPDILNotInitialized_Str					"PDIL - Not initialized"
#define kPDILInvalidSession_Str					"PDIL - Invalid session"
#define kPDILInvalidStore_Str					"PDIL - Invalid store"
#define kPDILInvalidSoup_Str					"PDIL - Invalid soup"
#define kPDILInvalidCursor_Str					"PDIL - Invalid cursor"
#define kPDILNewtonError_Str					"PDIL - Newton error"
#define kPDILInvalidROMVersion_Str				"PDIL - Invalid ROM version"
#define kPDILInvalidPassword_Str				"PDIL - Invalid password"

/* --- PDIL last result error numbers --- */
/* !!! WARNING-- THIS MAY CHANGE SOON !!! */

#define kPDILBadStoreSignature_Str				"PDIL - Bad store signature"
#define kPDILBadEntry_Str						"PDIL - Bad entry"
#define kPDILAborted_Str						"PDIL - Aborted"
#define kPDILBadQuery_Str						"PDIL - Bad query"
#define kPDILReadEntryError_Str					"PDIL - Read entry error"
#define kPDILBadCurrentSoup_Str					"PDIL - Bad current soup"
#define kPDILBadCommandLength_Str				"PDIL - Bad command length"
#define kPDILEntryNotFound_Str					"PDIL - Entry not found"
#define kPDILBadConnection_Str					"PDIL - Bad connection"
#define kPDILFileNotFound_Str					"PDIL - File not found"
#define kPDILIncompatableProtocol_Str			"PDIL - Incompatable protocol"
#define kPDILProtocolError_Str					"PDIL - Protocol error"
#define kPDILDockingCanceled_Str				"PDIL - Docking canceled"
#define kPDILStoreNotFound_Str					"PDIL - Store not found"
#define kPDILSoupNotFound_Str					"PDIL - Soup not found"
#define kPDILBadHeader_Str						"PDIL - Bad header"
#define kPDILOutOfMemory_Str					"PDIL - Out of memory"
#define kPDILNewtonVersionTooNew_Str			"PDIL - Newton version too new"
#define kPDILPackageCantLoad_Str				"PDIL - Package cant load"
#define kPDILProtocolExtAlreadyRegistered_Str	"PDIL - Protocol ext already registered"
#define kPDILRemoteImportError_Str 				"PDIL - Remote import error"
#define kPDILBadPasswordError_Str				"PDIL - Bad password error"
#define kPDILRetryPW_Str						"PDIL - Retry Password"
#define kPDILIdleTooLong_Str					"PDIL - Idle too long"
#define kPDILOutOfPower_Str						"PDIL - Out of power"
#define kPDILBadCursor_Str						"PDIL - Bad cursor"
#define kPDILAlreadyBusy_Str					"PDIL - Already busy"
#define kPDILDesktopError_Str					"PDIL - Desktop error"
#define kPDILCantConnectToModem_Str				"PDIL - Cant connect to modem"
#define kPDILDisconnected_Str					"PDIL - Disconnected"
#define kPDILAccessDenied_Str					"PDIL - Access denied"

#define kDILUnknownError_Str					"Unknown Error"

// IDs
#define kDILOutOfMemory							-98001
#define kDILInvalidParameter					-98002
#define kDILInternalError						-98003
#define kDILErrorReadingFromPipe				-98004
#define kDILErrorWritingToPipe					-98005
#define kDILInvalidHandle						-98006

// CDIL errors
#define kCDILNotInitialized						-98201
#define kCDILServiceNotSupported				-98202
#define kCDILBadPipeState						-98203
#define kCDILTimeout							-98204
#define kCDILPipeDisconnected					-98205
#define kCDILIndexOutOfRange					-98206
#define kCDILBufferTooSmall						-98207
#define kCDILPlatformError						-98208

#define kCDILTCPCantFindLibraryFns				-98220
#define kCDILTCPInsufficientVersion				-98221
#define kCDILTCPNoSockets						-98222

// FDIL errors
	/* Hard errors -- you should always be looking for these. */

#define kFDILUnknownStreamVersion				-98401
#define kFDILStreamCorrupted					-98402
#define kFDILUnsupportedCompression				-98403
#define kFDILCouldNotCompressData				-98404
#define kFDILCouldNotDecompressData				-98405
#define kFDILUnsupportedStoreVersion			-98406
#define kFDILErrorCreatingStore					-98407
#define kFDILErrorWritingToStore				-98408
#define kFDILErrorReadingFromStore				-98409

	/* Soft errors -- you get these only if you feed in bad data. */

#define kFDILNotInitialized						-98419

#define kFDILExpectedInteger					-98420
#define kFDILExpectedPointerObject				-98421
#define kFDILExpectedImmediate					-98422
#define kFDILExpectedMagicPointer				-98423

#define kFDILExpectedArray						-98424
#define kFDILExpectedFrame						-98425
#define kFDILExpectedBinary						-98426
#define kFDILExpectedLargeBinary				-98427

#define kFDILExpectedReal						-98428
#define kFDILExpectedString						-98429
#define kFDILExpectedSymbol						-98430
#define kFDILExpectedChar						-98431

#define kFDILNULLPointer						-98440
#define kFDILExpectedPositiveValue				-98441
#define kFDILExpectedNonNegativeValue			-98442
#define kFDILValueOutOfRange					-98443
#define kFDILSymbolTooLong						-98444
#define kFDILIllegalCharInSymbol				-98445
#define kFDILInvalidClass						-98446
#define kFDILPointerObjectIsFree				-98447

	/* Internal errors -- you should never see these. */

#define kFDILLBReadingFromUnwrittenPage			-98450

// PDIL errors

#define kPDILNotInitialized						-98601
#define kPDILInvalidSession						-98602
#define kPDILInvalidStore						-98603
#define kPDILInvalidSoup						-98604
#define kPDILInvalidCursor						-98605
#define kPDILNewtonError						-98606
#define kPDILInvalidROMVersion					-98607
#define kPDILInvalidPassword					-98608

/* --- PDIL last result error numbers --- */

#define kPDILBadStoreSignature					-28001
#define kPDILBadEntry							-28002
#define kPDILAborted							-28003
#define kPDILBadQuery							-28004
#define kPDILReadEntryError						-28005
#define kPDILBadCurrentSoup						-28006
#define kPDILBadCommandLength					-28007
#define kPDILEntryNotFound						-28008
#define kPDILBadConnection						-28009
#define kPDILFileNotFound						-28010
#define kPDILIncompatableProtocol				-28011
#define kPDILProtocolError						-28012
#define kPDILDockingCanceled					-28013
#define kPDILStoreNotFound						-28014
#define kPDILSoupNotFound						-28015
#define kPDILBadHeader							-28016
#define kPDILOutOfMemory						-28017
#define kPDILNewtonVersionTooNew				-28018
#define kPDILPackageCantLoad					-28019
#define kPDILProtocolExtAlreadyRegistered		-28020
#define kPDILRemoteImportError 					-28021
#define kPDILBadPasswordError					-28022
#define kPDILRetryPW							-28023
#define kPDILIdleTooLong						-28024
#define kPDILOutOfPower							-28025
#define kPDILBadCursor							-28026
#define kPDILAlreadyBusy						-28027
#define kPDILDesktopError						-28028
#define kPDILCantConnectToModem					-28029
#define kPDILDisconnected						-28030
#define kPDILAccessDenied						-28031

#define kDILUnknownError						-1

// Platform Error
#define kPlatformErrorRes_ID		2

// Error messages depend on your platform.
// We only provide unknown here.
#define kPlatformUnknownError		0

#define	kPlatformUnknownError_Str	"Unknown platform error (%i)"

// Status messages
#define kStatusRes_ID 3

#define kStatusUserCancelled_Str	"User cancelled"
#define kStatusTimeout_Str			"Timeout!"
#define kStatusConnectedTo_Str		"Connected to %s's Newton device"
#define kStatusDisconnecting_Str	"Closing connection with your Newton device"
#define kStatusDisconnected_Str		"Disconnected"
#define kStatusUnknownMessage_Str	"Unknown Status Message"

// IDs
#define kStatusUserCancelled		1
#define	kStatusTimeout				2
#define kStatusConnectedTo			3
#define kStatusDisconnecting		4
#define kStatusDisconnected			5
#define kStatusUnknownMessage		6

#endif
		// __DCLERRORMESSAGES__

// ========================================================================= //
// Viens chez moi, je te montrerai mon e2fsck ...                            //
// -+- LF in Guide du linuxien pervers - jusqu'où irez vous avec nous ? -+-  //
// ========================================================================= //
