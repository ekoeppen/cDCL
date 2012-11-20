// ==============================
// Fichier:			TDCLApplication.cp
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLApplication.cp.
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
// $Id: TDCLApplication.cp,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLApplication.h>

// DCL
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/IDCLThreads.h>

// ------------------------------------------------------------------------- //
//  * TDCLApplication( void )
// ------------------------------------------------------------------------- //
TDCLApplication::TDCLApplication( void )
	:
		mFilesIntf( nil ),
		mThreadsIntf( nil )
{
	// Kilroy was here.
}

// ------------------------------------------------------------------------- //
//  * ~TDCLApplication( void )
// ------------------------------------------------------------------------- //
TDCLApplication::~TDCLApplication( void )
{
	// On fait le ménage.
	if (mFilesIntf)
	{
		delete mFilesIntf;
	}
	
	if (mThreadsIntf)
	{
		delete mThreadsIntf;
	}
}

// ------------------------------------------------------------------------- //
//  * GetFilesIntf( void )
// ------------------------------------------------------------------------- //
IDCLFiles*
TDCLApplication::GetFilesIntf( void )
{
	if (mFilesIntf == nil)
	{
		mFilesIntf = CreateFilesIntf();
	}
	return mFilesIntf;
}

// ------------------------------------------------------------------------- //
//  * GetThreadsIntf( void )
// ------------------------------------------------------------------------- //
IDCLThreads*
TDCLApplication::GetThreadsIntf( void )
{
	if (mThreadsIntf == nil)
	{
		mThreadsIntf = CreateThreadsIntf();
	}
	return mThreadsIntf;
}

// ============================================================================= //
// Worthless.                                                                    //
//                 -- Sir George Bidell Airy, KCB, MA, LLD, DCL, FRS, FRAS       //
//                    (Astronomer Royal of Great Britain), estimating for the    //
//                    Chancellor of the Exchequer the potential value of the     //
//                    "analytical engine" invented by Charles Babbage, September //
//                    15, 1842.                                                  //
// ============================================================================= //
