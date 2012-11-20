// ==============================
// Fichier:			TMutex.cp
// Projet:			K
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// CrŽŽ le:			26/5/2005
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
// 
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TMutex.cp,v 1.2 2005/06/07 05:53:33 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TMutex.h"

// ANSI C & POSIX
#include <assert.h>
#include <errno.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TMutex( void )
// -------------------------------------------------------------------------- //
TMutex::TMutex( void )
{
	int err = ::pthread_mutex_init( &mMutex, NULL );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * ~TMutex( void )
// -------------------------------------------------------------------------- //
TMutex::~TMutex( void )
{
	int err = ::pthread_mutex_destroy( &mMutex );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * Lock( void )
// -------------------------------------------------------------------------- //
void
TMutex::Lock( void )
{
	int err = ::pthread_mutex_lock( &mMutex );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * Unlock( void )
// -------------------------------------------------------------------------- //
void
TMutex::Unlock( void )
{
	int err = ::pthread_mutex_unlock( &mMutex );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * TryLock( void )
// -------------------------------------------------------------------------- //
Boolean
TMutex::TryLock( void )
{
	int err = ::pthread_mutex_lock( &mMutex );
	if (err == EBUSY)
	{
		return true;
	} else {
		assert( err == 0 );
		return false;
	}
	(void) err;
}

// ========================================= //
// egrep -n '^[a-z].*\(' $ | sort -t':' +2.0 //
// ========================================= //
