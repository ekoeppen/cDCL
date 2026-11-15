// ==============================
// Fichier:			DCLCocoaApplication.mm
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
// The Original Code is DCLCocoaApplication.mm.
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
// $Id: DCLCocoaApplication.mm,v 1.4 2004/11/24 14:09:01 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#import <DCL/Interfaces/MacOS_X/DCLCocoaApplication.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

// Cocoa & Foundation
#import <Cocoa/Cocoa.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSRunLoop.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// DCL
#import <DCL/Interfaces/MacOS_X/IDCLObjCApplication.h>
#include <DCL/Interfaces/MacOS_X/TDCLObjCApplication.h>

@implementation DCLCocoaApplication

// ------------------------------------------------------------------------- //
//  * initWithController: (id < IDCLObjCApplication >)
// ------------------------------------------------------------------------- //
- (id) initWithController: (id < IDCLObjCApplication >) inController
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// On enregistre la référence sur le contrôleur.
	mController = inController;

	// Récupération de la boucle courante.
	mRunLoop = [NSRunLoop currentRunLoop];
	
	// Création de l'application DCL avec nous comme intermédiaire.
	mDCLAppl = new TDCLObjCApplication( self );
	
	// Récupération des interfaces sur les fichiers et les processus
	// légers.
	mFilesIntf = [inController CreateFilesIntf];
	mThreadsIntf = [inController CreateThreadsIntf];

	// Lancement du processus léger.
	[NSThread detachNewThreadSelector: @selector(DummyThreadEntryPoint:)
                                toTarget: self
                                withObject: nil];	

	return self;
}

// ------------------------------------------------------------------------- //
//  * CreateInvocationWithSelector: (SEL)
// ------------------------------------------------------------------------- //
- (NSInvocation*) CreateInvocationWithSelector: (SEL) inSelector
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	NSMethodSignature* theSignature;
	NSInvocation* theResult;
	
	// Récupération de la signature.
	// Comme nous mettons nous-même en place le protocole, on se demande à
	// nous-même (y a-t-il une façon de demander au mandataire?)
	theSignature = [self methodSignatureForSelector: inSelector];
	
	// Création de l'invocation.
	theResult = [NSInvocation invocationWithMethodSignature: theSignature];
	
	// On règle le séelcteur.
	[theResult setSelector: inSelector];
	
	// On règle la cible.
	[theResult setTarget: mController];
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * SendInvocation: (NSInvocation*)
// ------------------------------------------------------------------------- //
- (void) SendInvocation: (NSInvocation*) inInvocation
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	NSTimer* theTimer = [NSTimer
			timerWithTimeInterval: 0
			invocation: inInvocation
			repeats: NO];
	
	[mRunLoop addTimer: theTimer forMode: NSDefaultRunLoopMode];
}

// ------------------------------------------------------------------------- //
//  * DummyThreadEntryPoint: (id)
// ------------------------------------------------------------------------- //
- (void) DummyThreadEntryPoint: (id) ignoredArgs
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif
#pragma unused( ignoredArgs )

	// Ramasse-miettes.
    NSAutoreleasePool* pool;
    pool = [[NSAutoreleasePool alloc] init];
    	
	// Nettoyage
	[pool release];
	
	return;
}

// ------------------------------------------------------------------------- //
//  * GetDCLAppl
// ------------------------------------------------------------------------- //
- (TDCLObjCApplication*) GetDCLAppl
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	return mDCLAppl;
}

// ------------------------------------------------------------------------- //
//  * WaitConnection: (TDCLServer*) onLayer: (TDCLCommLayer*)
// ------------------------------------------------------------------------- //
- (void) WaitConnection: (TDCLServer*) inServer
				onLayer: (TDCLCommLayer*) inLayer
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
					@selector(WaitConnection:onLayer:)];
	
	[theInvocation setArgument: &inServer atIndex: 2];
	[theInvocation setArgument: &inLayer atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * Connected: (TDCLLink*) onLayer: (TDCLCommLayer*)
// ------------------------------------------------------------------------- //
- (void) Connected:(TDCLLink*) inLink onLayer: (TDCLCommLayer*) inLayer
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(Connected:onLayer:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inLayer atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * ConnectedToNewtonDevice: (TDCLLink*) name: (NSString*)
// ------------------------------------------------------------------------- //
- (void) ConnectedToNewtonDevice: (TDCLLink*) inLink name: (NSString*) inName
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
						@selector(ConnectedToNewtonDevice:name:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inName atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * Disconnecting: (TDCLLink*)
// ------------------------------------------------------------------------- //
- (void) Disconnecting: (TDCLLink*) inLink
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(Disconnecting:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * Disconnected: (TDCLLink*)
// ------------------------------------------------------------------------- //
- (void) Disconnected:(TDCLLink*) inLink
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(Disconnected:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * ServerIsDown: (TDCLServer*)
// ------------------------------------------------------------------------- //
- (void) ServerIsDown:(TDCLServer*) inServer
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(ServerIsDown:)];
	
	[theInvocation setArgument: &inServer atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * Cancel: (TDCLLink*)
// ------------------------------------------------------------------------- //
- (void) Cancel: (TDCLLink*) inLink
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(Cancel:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * Timeout: (TDCLLink*)
// ------------------------------------------------------------------------- //
- (void) Timeout: (TDCLLink*) inLink
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(Timeout:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * AppCommandBeingProcessed: (TDCLLink*) command: (TDCLAppCommand*)
// ------------------------------------------------------------------------- //
- (void) AppCommandBeingProcessed: (TDCLLink*) inLink
					command: (TDCLAppCommand*) inCommand
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
						@selector(AppCommandBeingProcessed:command:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inCommand atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * AppCommandProcessed: (TDCLLink*) command: (TDCLAppCommand*) exception: ...
// ------------------------------------------------------------------------- //
- (void) AppCommandProcessed: (TDCLLink*) inLink
					command: (TDCLAppCommand*) inCommand
					exception: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
						@selector(AppCommandProcessed:command:exception:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inCommand atIndex: 3];
	[theInvocation setArgument: &inException atIndex: 4];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * DockCommandBeingProcessed: (TDCLLink*) command: (TDCLDockCommand*) ...
// ------------------------------------------------------------------------- //
- (void) DockCommandBeingProcessed: (TDCLLink*) inLink
					command: (TDCLDockCommand*) inCommand
					engine: (TDCLLinkEngine*) inEngine
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
						@selector(DockCommandBeingProcessed:command:engine:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inCommand atIndex: 3];
	[theInvocation setArgument: &inEngine atIndex: 4];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * DockCommandProcessed: (TDCLLink*) command: (TDCLLinkEngine*) ...
// ------------------------------------------------------------------------- //
- (void) DockCommandProcessed: (TDCLLink*) inLink
					engine: (TDCLLinkEngine*) inEngine
					exception: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
						@selector(DockCommandProcessed:engine:exception:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inEngine atIndex: 3];
	[theInvocation setArgument: &inException atIndex: 4];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * HandleServerException: (TDCLServer*) exception: (TDCLException*)
// ------------------------------------------------------------------------- //
- (void) HandleServerException: (TDCLServer*) inServer
					exception: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
				@selector(HandleServerException:exception:)];
	
	[theInvocation setArgument: &inServer atIndex: 2];
	[theInvocation setArgument: &inException atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * HandleCommLayerException: (TDCLCommLayer*) exception: (TDCLException*)
// ------------------------------------------------------------------------- //
- (void) HandleCommLayerException: (TDCLCommLayer*) inCommLayer
					exception: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector:
				@selector(HandleCommLayerException:exception:)];
	
	[theInvocation setArgument: &inCommLayer atIndex: 2];
	[theInvocation setArgument: &inException atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * HandleLinkException: (TDCLLink*) exception: (TDCLException*)
// ------------------------------------------------------------------------- //
- (void) HandleLinkException: (TDCLLink*) inLink
					exception: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(HandleLinkException:exception:)];
	
	[theInvocation setArgument: &inLink atIndex: 2];
	[theInvocation setArgument: &inException atIndex: 3];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * HandleException: (TDCLException*)
// ------------------------------------------------------------------------- //
- (void) HandleException: (TDCLException*) inException
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Création de l'invocation.
	NSInvocation* theInvocation
		= [self CreateInvocationWithSelector: @selector(HandleException:)];
	
	[theInvocation setArgument: &inException atIndex: 2];
	
	// On invoque.
	[self SendInvocation: theInvocation];
}

// ------------------------------------------------------------------------- //
//  * CreateFilesIntf
// ------------------------------------------------------------------------- //
- (IDCLFiles*) CreateFilesIntf
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	return mFilesIntf;
}

// ------------------------------------------------------------------------- //
//  * CreateThreadsIntf
// ------------------------------------------------------------------------- //
- (IDCLThreads*) CreateThreadsIntf
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	return mThreadsIntf;
}

@end

// =========================== //
// Memory fault -- brain fried //
// =========================== //