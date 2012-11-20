// ==============================
// Fichier:			PrecompiledHeader.h
// Projet:			Desktop Connection Library
//
// Créé le:			31/3/2003
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
// The Original Code is PrecompiledHeader.h.
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
// $Id: PrecompiledHeader.h,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef _PRECOMPILEDHEADER_H
#define _PRECOMPILEDHEADER_H

// Entêtes DCL & Co les plus utilisés/les plus stables.

// K
	// K Crypto
	#include <K/Crypto/UDES.h>
	// K Defines
	#include <K/Defines/KDefinitions.h>
	#include <K/Defines/UByteSex.h>
	// K Math
	#include <K/Math/TInt64.h>
	#include <K/Math/TSInt64.h>
	#include <K/Math/TUInt64.h>
	#include <K/Math/UTInt64.h>
	// K Misc
	#include <K/Misc/TRefCounter.h>
//	#include <K/Misc/TSmartPtr.h>	//<-- doit être chargé après les entêtes
									// Apple
	#include <K/Misc/UBase64.h>
	#include <K/Misc/UPStrings.h>
	// K Tests
	#include <K/Tests/KDebug.h>
	// K Unicode
	#include <K/Unicode/TMutableStringRef.h>
	#include <K/Unicode/TString.h>
	#include <K/Unicode/TStringRef.h>
	#include <K/Unicode/UUTF16Conv.h>
	#include <K/Unicode/UUTF16CStr.h>
	#include <K/Unicode/UnicodeDefinitions.h>
// DCL
	// Communication Layers
	#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
	#include <DCL/Communication_Layers/TDCLCommLayer.h>
	#include <DCL/Communication_Layers/TDCLMNPPipe.h>
	#include <DCL/Communication_Layers/TDCLPipe.h>
	#include <DCL/Communication_Layers/TDCLPipeProxy.h>
	#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>
	// Exceptions
		// Errors
		#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
		#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
		#include <DCL/Exceptions/Errors/TDCLMemError.h>
		#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
		#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
		#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
		// IO Exceptions
		#include <DCL/Exceptions/IO_Exceptions/TDCLDoesntExistException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLFullException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLPermissionException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLReadOnlyException.h>
		#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>
		// Link Exceptions
		#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
		#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
		#include <DCL/Exceptions/Link_Exceptions/TDCLBadAppCmdException.h>
		#include <DCL/Exceptions/Link_Exceptions/TDCLLinkException.h>
		// Thread Exceptions
		#include <DCL/Exceptions/Thread_Exceptions/TDCLSemaphoreBusyException.h>
		#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadActiveException.h>
		#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadException.h>
		#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadStoppedException.h>
	#include <DCL/Exceptions/TDCLException.h>
	#include <DCL/Exceptions/TDCLClassCastException.h>
	#include <DCL/Exceptions/TDCLNewtonException.h>
	#include <DCL/Exceptions/TDCLNSException.h>
	#include <DCL/Exceptions/TDCLNotAvailableException.h>
	#include <DCL/Exceptions/TDCLPackageException.h>
	#include <DCL/Exceptions/TDCLUserCancelledException.h>
	// Headers
	#include <DCL/Headers/DCLDefinitions.h>
	#include <DCL/Headers/DCLStrings.h>
	// Interfaces
	#include <DCL/Interfaces/IDCLFiles.h>
	#include <DCL/Interfaces/IDCLThreads.h>
	#include <DCL/Interfaces/TDCLApplication.h>
	#include <DCL/Interfaces/TDCLEvent.h>
	#include <DCL/Interfaces/TDCLFile.h>
	#include <DCL/Interfaces/TDCLFolder.h>
	#include <DCL/Interfaces/TDCLFSItem.h>
	#include <DCL/Interfaces/TDCLFSItemRef.h>
	#include <DCL/Interfaces/TDCLRootFolder.h>
	#include <DCL/Interfaces/TDCLThread.h>
	// Link
		// App Commands
		#include <DCL/Link/App_Commands/TDCLAppCommand.h>
		// Dock Commands
		#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
	#include <DCL/Link/TDCLDockLink.h>
	#include <DCL/Link/TDCLFullDockLink.h>
	#include <DCL/Link/TDCLLink.h>
	// NS Objects
		// Exchange
		#include <DCL/NS_Objects/Exchange/TDCLNSCodec.h>
		#include <DCL/NS_Objects/Exchange/TDCLNSDecoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLPkgDecoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLPkgEncoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLTextDecoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLTextEncoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLXMLDecoder.h>
		#include <DCL/NS_Objects/Exchange/TDCLXMLEncoder.h>
// ... 
#endif
		// _PRECOMPILEDHEADER_H

// ========================================================================== //
// WARNING!!!                                                                 //
// This machine is subject to breakdowns during periods of critical need.     //
//                                                                            //
// A special circuit in the machine called "critical detector" senses the     //
// operator's emotional state in terms of how desperate he/she is to use the  //
// machine.  The "critical detector" then creates a malfunction proportional  //
// to the desperation of the operator.  Threatening the machine with violence //
// only aggravates the situation.  Likewise, attempts to use another machine  //
// may cause it to malfunction.  They belong to the same union.  Keep cool    //
// and say nice things to the machine.  Nothing else seems to work.           //
//                                                                            //
// See also: flog(1), tm(1)                                                   //
// ========================================================================== //