// ==============================
// Fichier:			TMutex.cp
// Projet:			K
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Crée le:			26/5/2005
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
// The Original Code is TMutex.cp.
// 
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2005 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org>
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TMutex.cp,v 1.2 2005/06/07 05:53:33 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TMutex.h"

// -------------------------------------------------------------------------- //
//  * TMutex( void )
// -------------------------------------------------------------------------- //
TMutex::TMutex( void )
{
}

// -------------------------------------------------------------------------- //
//  * ~TMutex( void )
// -------------------------------------------------------------------------- //
TMutex::~TMutex( void )
{
}

// -------------------------------------------------------------------------- //
//  * Lock( void )
// -------------------------------------------------------------------------- //
void
TMutex::Lock( void )
{
	mMutex.lock();
}

// -------------------------------------------------------------------------- //
//  * Unlock( void )
// -------------------------------------------------------------------------- //
void
TMutex::Unlock( void )
{
	mMutex.unlock();
}

// -------------------------------------------------------------------------- //
//  * TryLock( void )
// -------------------------------------------------------------------------- //
Boolean
TMutex::TryLock( void )
{
	return mMutex.try_lock();
}

// ========================================= //
// egrep -n '^[a-z].*\(' $ | sort -t':' +2.0 //
// ========================================= //
