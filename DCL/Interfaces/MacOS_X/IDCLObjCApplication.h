// ==============================
// Fichier:			IDCLObjCApplication.h
// Projet:			Desktop Connection Library
//
// Créé le:			30/3/2003
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
// The Original Code is IDCLObjCApplication.h.
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
// $Id: IDCLObjCApplication.h,v 1.4 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _IDCLOBJCAPPLICATION_H
#define _IDCLOBJCAPPLICATION_H

#ifdef __MWERKS__
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

#import <Cocoa/Cocoa.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// Pré-déclarations
class IDCLFiles;
class IDCLThreads;
class IDCLMisc;
class TDCLAppCommand;
class TDCLCommLayer;
class TDCLException;
class TDCLLink;
class TDCLServer;
class TDCLDockCommand;
class TDCLLinkEngine;

///
/// Protocole Obj-C pour l'application.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
/// \see	DCLCocoaApplication
///
@protocol IDCLObjCApplication
- (void) WaitConnection: (TDCLServer*) inServer onLayer: (TDCLCommLayer*) inLayer;
- (void) Connected:(TDCLLink*) inLink  onLayer: (TDCLCommLayer*) inLayer;
- (void) ConnectedToNewtonDevice:(TDCLLink*) inLink name: (NSString*) inName;
- (void) Disconnecting:(TDCLLink*) inLink;
- (void) Disconnected:(TDCLLink*) inLink;
- (void) ServerIsDown:(TDCLServer*) inServer;
- (void) Cancel:(TDCLLink*) inLink;
- (void) Timeout:(TDCLLink*) inLink;
- (void) AppCommandBeingProcessed: (TDCLLink*) inLink command: (TDCLAppCommand*) inCommand;
- (void) AppCommandProcessed: (TDCLLink*) inLink command: (TDCLAppCommand*) inCommand exception: (TDCLException*) inException;
- (void) DockCommandBeingProcessed: (TDCLLink*) inLink command: (TDCLDockCommand*) inCommand engine: (TDCLLinkEngine*) inEngine;
- (void) DockCommandProcessed: (TDCLLink*) inLink engine: (TDCLLinkEngine*) inEngine exception: (TDCLException*) inException;
- (void) HandleServerException: (TDCLServer*) inServer exception: (TDCLException*) inException;
- (void) HandleCommLayerException: (TDCLCommLayer*) inLink exception: (TDCLException*) inException;
- (void) HandleLinkException: (TDCLLink*) inLink exception: (TDCLException*) inException;
- (void) HandleException:(TDCLException*) inException;
- (IDCLFiles*) CreateFilesIntf;
- (IDCLThreads*) CreateThreadsIntf;
@end


#endif
		// _IDCLOBJCAPPLICATION_H

// ================================================================= //
// 101 USES FOR A DEAD MICROPROCESSOR                                //
//         (1)  Scarecrow for centipedes                             //
//         (2)  Dead cat brush                                       //
//         (3)  Hair barrettes                                       //
//         (4)  Cleats                                               //
//         (5)  Self-piercing earrings                               //
//         (6)  Fungus trellis                                       //
//         (7)  False eyelashes                                      //
//         (8)  Prosthetic dog claws                                 //
//         .                                                         //
//         .                                                         //
//         .                                                         //
//         (99)  Window garden harrow (pulled behind Tonka tractors) //
//         (100) Killer velcro                                       //
//         (101) Currency                                            //
// ================================================================= //