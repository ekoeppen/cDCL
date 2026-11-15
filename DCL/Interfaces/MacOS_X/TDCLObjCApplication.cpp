// ==============================
// Fichier:			TDCLObjCApplication.cp
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
// The Original Code is TDCLObjCApplication.cp.
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
// $Id: TDCLObjCApplication.cp,v 1.4 2004/11/24 14:09:01 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/MacOS_X/TDCLObjCApplication.h>

// CoreFoundation
#include <CoreFoundation/CFString.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// ------------------------------------------------------------------------- //
//  * TDCLObjCApplication( id )
// ------------------------------------------------------------------------- //
TDCLObjCApplication::TDCLObjCApplication( id inCocoaObject )
	:
		mCocoaObject( inCocoaObject )
{
	// C'est tout je pense.
}

// ------------------------------------------------------------------------- //
//  * WaitConnection( TDCLServer*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::WaitConnection(
							TDCLServer* inServer,
							TDCLCommLayer* inLayer )
{
	TDCLObjCAppl_WaitConnection( mCocoaObject, inServer, inLayer );
}

// ------------------------------------------------------------------------- //
//  * Connected( TDCLLink*, TDCLCommLayer* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::Connected( TDCLLink* inLink, TDCLCommLayer* inLayer )
{
	TDCLObjCAppl_Connected( mCocoaObject, inLink, inLayer );
}

// ------------------------------------------------------------------------- //
//  * ConnectedToNewtonDevice( TDCLLink*, const KUInt16* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::ConnectedToNewtonDevice(
						TDCLLink* inLink,
						const KUInt16* inName )
{
	// Traduction du nom.
	// On suppose que le nom fait moins de 32768 caractères (généralement,
	// c'est le cas ;)
	// [Sinon, la fonction NewtonScript StrLen ne fonctionnerait pas.]
	CFStringRef theStringRef = ::CFStringCreateWithCharacters(
							kCFAllocatorDefault,
							(const UniChar*) inName,
							(long) UUTF16CStr::StrLen( inName ));
	
	TDCLObjCAppl_ConnectedToNewtonDevice( mCocoaObject, inLink, theStringRef );

	// Libération du nom.
	::CFRelease( theStringRef );
}

// ------------------------------------------------------------------------- //
//  * Disconnecting( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::Disconnecting( TDCLLink* inLink )
{
	TDCLObjCAppl_Disconnecting( mCocoaObject, inLink );
}

// ------------------------------------------------------------------------- //
//  * Disconnected( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::Disconnected( TDCLLink* inLink )
{
	TDCLObjCAppl_Disconnected( mCocoaObject, inLink );
}

// ------------------------------------------------------------------------- //
//  * ServerIsDown( TDCLServer* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::ServerIsDown( TDCLServer* inServer )
{
	TDCLObjCAppl_ServerIsDown( mCocoaObject, inServer );
}

// ------------------------------------------------------------------------- //
//  * Cancel( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::Cancel( TDCLLink* inLink )
{
	TDCLObjCAppl_Cancel( mCocoaObject, inLink );
}

// ------------------------------------------------------------------------- //
//  * Timeout( TDCLLink* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::Timeout( TDCLLink* inLink )
{
	TDCLObjCAppl_Timeout( mCocoaObject, inLink );
}

// ------------------------------------------------------------------------- //
//  * AppCommandBeingProcessed( TDCLLink*, TDCLAppCommand* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::AppCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand )
{
	TDCLObjCAppl_AppCommandBeingProcessed(
				mCocoaObject, inLink, inCommand );
}

// ------------------------------------------------------------------------- //
//  * AppCommandProcessed( TDCLLink*, TDCLAppCommand*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::AppCommandProcessed(
							TDCLLink* inLink,
							TDCLAppCommand* inCommand,
							TDCLException* inException )
{
	TDCLObjCAppl_AppCommandProcessed(
				mCocoaObject, inLink, inCommand, inException );
}

// ------------------------------------------------------------------------- //
//  * DockCommandBeingProcessed( TDCLLink*, TDCLDockCommand*, TDCLLinkEngine* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::DockCommandBeingProcessed(
							TDCLLink* inLink,
							TDCLDockCommand* inCommand,
							TDCLLinkEngine* inEngine )
{
	TDCLObjCAppl_DockCommandBeingProcessed(
				mCocoaObject, inLink, inCommand, inEngine );
}

// ------------------------------------------------------------------------- //
//  * DockCommandProcessed( TDCLLink*, TDCLLinkEngine*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::DockCommandProcessed(
							TDCLLink* inLink,
							TDCLLinkEngine* inEngine,
							TDCLException* inException )
{
	TDCLObjCAppl_DockCommandProcessed(
				mCocoaObject, inLink, inEngine, inException );
}

// ------------------------------------------------------------------------- //
//  * HandleServerException( TDCLServer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::HandleServerException(
							TDCLServer* inServer,
							TDCLException* inException )
{
	TDCLObjCAppl_HandleServerException( mCocoaObject, inServer, inException );
}

// ------------------------------------------------------------------------- //
//  * HandleCommLayerException( TDCLCommLayer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException )
{
	TDCLObjCAppl_HandleCommLayerException(
									mCocoaObject, inCommLayer, inException );
}

// ------------------------------------------------------------------------- //
//  * HandleLinkException( TDCLLink*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::HandleLinkException(
							TDCLLink* inLink,
							TDCLException* inException )
{
	TDCLObjCAppl_HandleLinkException( mCocoaObject, inLink, inException );
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TDCLObjCApplication::HandleException( TDCLException* inException )
{
	TDCLObjCAppl_HandleException( mCocoaObject, inException );
}

// ------------------------------------------------------------------------- //
//  * CreateFilesIntf( void )
// ------------------------------------------------------------------------- //
IDCLFiles*
TDCLObjCApplication::CreateFilesIntf( void )
{
	return TDCLObjCAppl_CreateFilesIntf( mCocoaObject );
}

// ------------------------------------------------------------------------- //
//  * CreateThreadsIntf( void )
// ------------------------------------------------------------------------- //
IDCLThreads*
TDCLObjCApplication::CreateThreadsIntf( void )
{
	return TDCLObjCAppl_CreateThreadsIntf( mCocoaObject );
}

// ============================================ //
// Our OS who art in CPU, UNIX be thy name.     //
//         Thy programs run, thy syscalls done, //
//         In kernel as it is in user!          //
// ============================================ //