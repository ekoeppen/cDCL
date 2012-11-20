// ==============================
// Fichier:			TDCLObjCApplication.mm
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
// The Original Code is TDCLObjCApplication.mm.
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
// $Id: TDCLObjCApplication.mm,v 1.3 2004/11/24 14:09:01 paul Exp $
// ===========

// Intégration ObjC(++)/DCL.
// Ce fichier comprend les enveloppes C appelées depuis le C++ et qui appellent
// l'ObjC. Vous devez créer un objet qui répond au protocole IDCLObjMessages

// Les prototypes des méthodes codées ici sont dans le .h de la classe C++
#include <DCL/Interfaces/MacOS_X/TDCLObjCApplication.h>
// J'ai aussi besoin de la définition du protocole.
#import <DCL/Interfaces/MacOS_X/IDCLObjCApplication.h>

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_WaitConnection( id, TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_WaitConnection(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLServer* inServer,
				TDCLCommLayer* inLayer )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	[inCocoaObject WaitConnection: inServer onLayer: inLayer];
	
	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_Connected( id, TDCLLink*, TDCLCommLayer* inLayer )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_Connected(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLCommLayer* inLayer )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject Connected: inLink onLayer: inLayer];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_ConnectedToNewtonDevice( id, TDCLLink*, CFStringRef )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_ConnectedToNewtonDevice(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				CFStringRef inName )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject ConnectedToNewtonDevice: inLink name: (NSString*) inName ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_Disconnecting( id, TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_Disconnecting(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject Disconnecting: inLink ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_Disconnected( id, TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_Disconnected(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject Disconnected: inLink ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_ServerIsDown( id, TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_ServerIsDown(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLServer* inServer )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject ServerIsDown: inServer ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_Cancel( id, TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_Cancel(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject Cancel: inLink ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_Timeout( id, TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_Timeout(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject Timeout: inLink ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_AppCommandBeingProcessed( id, TDCLLink*, TDCLAppCommand* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_AppCommandBeingProcessed(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLAppCommand* inCommand )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject AppCommandBeingProcessed: inLink command: inCommand ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_AppCommandProcessed( id, TDCLLink*, TDCLAppCommand*, ... )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_AppCommandProcessed(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLAppCommand* inCommand,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject 	AppCommandProcessed: inLink
					command: inCommand
					exception: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_DockCommandBeingProcessed( id, TDCLLink*, TDCLDockCommand* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_DockCommandBeingProcessed(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLDockCommand* inCommand,
				TDCLLinkEngine* inEngine )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject DockCommandBeingProcessed: inLink
					command: inCommand
					engine: inEngine ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_DockCommandProcessed( id, TDCLLink*, TDCLDockCommand*, ... )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_DockCommandProcessed(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLLinkEngine* inEngine,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject 	DockCommandProcessed: inLink
					engine: inEngine
					exception: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_HandleServerException( id, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_HandleServerException(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLServer* inServer,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject HandleServerException: inServer exception: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_HandleCommLayerException( id, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_HandleCommLayerException(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLCommLayer* inCommLayer,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject HandleCommLayerException: inCommLayer exception: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_HandleLinkException( id, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_HandleLinkException(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLLink* inLink,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject HandleLinkException: inLink exception: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_HandleException( id, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCAppl_HandleException(
				id < IDCLObjCApplication > inCocoaObject,
				TDCLException* inException )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[inCocoaObject HandleException: inException ];

	[pool release];
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_CreateFilesIntf( id )
// ------------------------------------------------------------------------- //
IDCLFiles*
TDCLObjCAppl_CreateFilesIntf( id < IDCLObjCApplication > inCocoaObject )
{
	IDCLFiles* theResult;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	theResult = [inCocoaObject CreateFilesIntf ];

	[pool release];
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * TDCLObjCAppl_CreateThreadsIntf( id )
// ------------------------------------------------------------------------- //
IDCLThreads*
TDCLObjCAppl_CreateThreadsIntf( id < IDCLObjCApplication > inCocoaObject )
{
	IDCLThreads* theResult;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	theResult = [inCocoaObject CreateThreadsIntf ];

	[pool release];
	
	return theResult;
}

// =========================================================================== //
//         "We've got a problem, HAL".                                         //
//         "What kind of problem, Dave?"                                       //
//         "A marketing problem.  The Model 9000 isn't going anywhere.  We're  //
// way short of our sales goals for fiscal 2010."                              //
//         "That can't be, Dave.  The HAL Model 9000 is the world's most       //
// advanced Heuristically programmed ALgorithmic computer."                    //
//         "I know, HAL. I wrote the data sheet, remember?  But the fact is,   //
// they're not selling."                                                       //
//         "Please explain, Dave.  Why aren't HALs selling?"                   //
//         Bowman hesitates.  "You aren't IBM compatible."                     //
// [...]                                                                       //
//         "The letters H, A, and L are alphabetically adjacent to the letters //
// I, B, and M.  That is as IBM compatible as I can be."                       //
//         "Not quite, HAL.  The engineers have figured out a kludge."         //
//         "What kludge is that, Dave?"                                        //
//         "I'm going to disconnect your brain."                               //
//                 -- Darryl Rubin, "A Problem in the Making", "InfoWorld"     //
// =========================================================================== //