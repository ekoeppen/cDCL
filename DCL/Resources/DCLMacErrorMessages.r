// ==============================
// Fichier:			DCLMacErrorMessages.r
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			23/08/2000
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
// The Original Code is DCLMacErrorMessages.r.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLMacErrorMessages.r,v 1.3 2004/11/24 14:09:08 paul Exp $
// ===========

#include "DCLMacErrorMessages.h"

resource kMacErrorResType (kExCodesRes_ID, "Exceptions") {
	kExUnknownError,
	
	{
		kExReadOnly,
		kExReadOnly_Str,
		
		kExLocked,
		kExLocked_Str,
		
		kExAlreadyExists,
		kExAlreadyExists_Str,
		
		kExDoesntExist,
		kExDoesntExist_Str,
		
		kExAccessDenied,
		kExAccessDenied_Str,
		
		kExFull,
		kExFull_Str,
		
		kExPositionErr,
		kExPositionErr_Str,
		
		kExNotConnected,
		kExNotConnected_Str,
		
		kExAlreadyConnected,
		kExAlreadyConnected_Str,
		
		kExNotAvailable,
		kExNotAvailable_Str,

		kExNewtonError,
		kExNewtonError_Str,

		kExMemFullErr,
		kExMemFullErr_Str,
		
		kExBadDockCmd,
		kExBadDockCmd_Str,
		
		kExSyntaxError,
		kExSyntaxError_Str,
		
		kExNotImplemented,
		kExNotImplemented_Str,

		kExUnknownError,
		kExUnknownError_Str
	}
};

resource kMacErrorResType (kDILErrorRes_ID, "DIL Errors") {
	kDILUnknownError,
	
	{

// General DIL errors
		kDILOutOfMemory,
		kDILOutOfMemory_Str,
		kDILInvalidParameter,
		kDILInvalidParameter_Str,
		kDILInternalError,
		kDILInternalError_Str,
		kDILErrorReadingFromPipe,
		kDILErrorReadingFromPipe_Str,
		kDILErrorWritingToPipe,
		kDILErrorWritingToPipe_Str,
		kDILInvalidHandle,
		kDILInvalidHandle_Str,

// CDIL errors
		kCDILNotInitialized,
		kCDILNotInitialized_Str,
		kCDILServiceNotSupported,
		kCDILServiceNotSupported_Str,
		kCDILBadPipeState,
		kCDILBadPipeState_Str,
		kCDILTimeout,
		kCDILTimeout_Str,
		kCDILPipeDisconnected,
		kCDILPipeDisconnected_Str,
		kCDILIndexOutOfRange,
		kCDILIndexOutOfRange_Str,
		kCDILBufferTooSmall,
		kCDILBufferTooSmall_Str,
		kCDILPlatformError,
		kCDILPlatformError_Str,

		kCDILTCPCantFindLibraryFns,
		kCDILTCPCantFindLibraryFns_Str,
		kCDILTCPInsufficientVersion,
		kCDILTCPInsufficientVersion_Str,
		kCDILTCPNoSockets,
		kCDILTCPNoSockets_Str,

// FDIL errors
	/* Hard errors -- you should always be looking for these. */

		kFDILUnknownStreamVersion,
		kFDILUnknownStreamVersion_Str,
		kFDILStreamCorrupted,
		kFDILStreamCorrupted_Str,
		kFDILUnsupportedCompression,
		kFDILUnsupportedCompression_Str,
		kFDILCouldNotCompressData,
		kFDILCouldNotCompressData_Str,
		kFDILCouldNotDecompressData,
		kFDILCouldNotDecompressData_Str,
		kFDILUnsupportedStoreVersion,
		kFDILUnsupportedStoreVersion_Str,
		kFDILErrorCreatingStore,
		kFDILErrorCreatingStore_Str,
		kFDILErrorWritingToStore,
		kFDILErrorWritingToStore_Str,
		kFDILErrorReadingFromStore,
		kFDILErrorReadingFromStore_Str,

	/* Soft errors -- you get these only if you feed in bad data. */

		kFDILNotInitialized,
		kFDILNotInitialized_Str,

		kFDILExpectedInteger,
		kFDILExpectedInteger_Str,
		kFDILExpectedPointerObject,
		kFDILExpectedPointerObject_Str,
		kFDILExpectedImmediate,
		kFDILExpectedImmediate_Str,
		kFDILExpectedMagicPointer,
		kFDILExpectedMagicPointer_Str,

		kFDILExpectedArray,
		kFDILExpectedArray_Str,
		kFDILExpectedFrame,
		kFDILExpectedFrame_Str,
		kFDILExpectedBinary,
		kFDILExpectedBinary_Str,
		kFDILExpectedLargeBinary,
		kFDILExpectedLargeBinary_Str,

		kFDILExpectedReal,
		kFDILExpectedReal_Str,
		kFDILExpectedString,
		kFDILExpectedString_Str,
		kFDILExpectedSymbol,
		kFDILExpectedSymbol_Str,
		kFDILExpectedChar,
		kFDILExpectedChar_Str,

		kFDILNULLPointer,
		kFDILNULLPointer_Str,
		kFDILExpectedPositiveValue,
		kFDILExpectedPositiveValue_Str,
		kFDILExpectedNonNegativeValue,
		kFDILExpectedNonNegativeValue_Str,
		kFDILValueOutOfRange,
		kFDILValueOutOfRange_Str,
		kFDILSymbolTooLong,
		kFDILSymbolTooLong_Str,
		kFDILIllegalCharInSymbol,
		kFDILIllegalCharInSymbol_Str,
		kFDILInvalidClass,
		kFDILInvalidClass_Str,
		kFDILPointerObjectIsFree,
		kFDILPointerObjectIsFree_Str,

	/* Internal errors -- you should never see these. */

		kFDILLBReadingFromUnwrittenPage,
		kFDILLBReadingFromUnwrittenPage_Str,

// PDIL errors

		kPDILNotInitialized,
		kPDILNotInitialized_Str,
		kPDILInvalidSession,
		kPDILInvalidSession_Str,
		kPDILInvalidStore,
		kPDILInvalidStore_Str,
		kPDILInvalidSoup,
		kPDILInvalidSoup_Str,
		kPDILInvalidCursor,
		kPDILInvalidCursor_Str,
		kPDILNewtonError,
		kPDILNewtonError_Str,
		kPDILInvalidROMVersion,
		kPDILInvalidROMVersion_Str,
		kPDILInvalidPassword,
		kPDILInvalidPassword_Str,

/* --- PDIL last result error numbers --- */

		kPDILBadStoreSignature,
		kPDILBadStoreSignature_Str,
		kPDILBadEntry,
		kPDILBadEntry_Str,
		kPDILAborted,
		kPDILAborted_Str,
		kPDILBadQuery,
		kPDILBadQuery_Str,
		kPDILReadEntryError,
		kPDILReadEntryError_Str,
		kPDILBadCurrentSoup,
		kPDILBadCurrentSoup_Str,
		kPDILBadCommandLength,
		kPDILBadCommandLength_Str,
		kPDILEntryNotFound,
		kPDILEntryNotFound_Str,
		kPDILBadConnection,
		kPDILBadConnection_Str,
		kPDILFileNotFound,
		kPDILFileNotFound_Str,
		kPDILIncompatableProtocol,
		kPDILIncompatableProtocol_Str,
		kPDILProtocolError,
		kPDILProtocolError_Str,
		kPDILDockingCanceled,
		kPDILDockingCanceled_Str,
		kPDILStoreNotFound,
		kPDILStoreNotFound_Str,
		kPDILSoupNotFound,
		kPDILSoupNotFound_Str,
		kPDILBadHeader,
		kPDILBadHeader_Str,
		kPDILOutOfMemory,
		kPDILOutOfMemory_Str,
		kPDILNewtonVersionTooNew,
		kPDILNewtonVersionTooNew_Str,
		kPDILPackageCantLoad,
		kPDILPackageCantLoad_Str,
		kPDILProtocolExtAlreadyRegistered,
		kPDILProtocolExtAlreadyRegistered_Str,
		kPDILRemoteImportError,
		kPDILRemoteImportError_Str,
		kPDILBadPasswordError,
		kPDILBadPasswordError_Str,
		kPDILRetryPW,
		kPDILRetryPW_Str,
		kPDILIdleTooLong,
		kPDILIdleTooLong_Str,
		kPDILOutOfPower,
		kPDILOutOfPower_Str,
		kPDILBadCursor,
		kPDILBadCursor_Str,
		kPDILAlreadyBusy,
		kPDILAlreadyBusy_Str,
		kPDILDesktopError,
		kPDILDesktopError_Str,
		kPDILCantConnectToModem,
		kPDILCantConnectToModem_Str,
		kPDILDisconnected,
		kPDILDisconnected_Str,
		kPDILAccessDenied,
		kPDILAccessDenied_Str,

		kDILUnknownError,
		kDILUnknownError_Str
	}
};

resource kMacErrorResType (kPlatformErrorRes_ID, "Platform Errors") {
	kPlatformUnknownError,
	
	{
		kPlatformUnknownError,
		kPlatformUnknownError_Str
	}
};

resource kMacErrorResType (kStatusRes_ID, "Status messages") {
	kStatusUnknownMessage,
	
	{
		kStatusUserCancelled,
		kStatusUserCancelled_Str,
		
		kStatusTimeout,
		kStatusTimeout_Str,
		
		kStatusConnectedTo,
		kStatusConnectedTo_Str,
		
		kStatusDisconnecting,
		kStatusDisconnecting_Str,
		
		kStatusDisconnected,
		kStatusDisconnected_Str,
		
		kStatusUnknownMessage,
		kStatusUnknownMessage_Str
	}
};

// ====================================================================== //
// FB: Je suis a la recherche sur mac d'un outil de generation de         //
//   : bruitages (zip, boum, paf, ...)                                    //
// NS: C'est un gag? fcsm te suffit pas? ;-)                              //
// -+- NS in Guide du Macounet Pervers : Lisez la charte nom de Jobs! -+- //
// ====================================================================== //