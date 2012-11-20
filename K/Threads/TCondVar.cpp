// ==============================
// Fichier:			TCondVar.cp
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
// The Original Code is TCondVar.cp.
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
// $Id: TCondVar.cp,v 1.2 2005/06/07 05:53:32 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TCondVar.h"

// ANSI C & POSIX
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

// K
#include <K/Threads/TMutex.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * TCondVar( void )
// -------------------------------------------------------------------------- //
TCondVar::TCondVar( void )
{
	int err = ::pthread_cond_init( &mCondVar, NULL );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * ~TCondVar( void )
// -------------------------------------------------------------------------- //
TCondVar::~TCondVar( void )
{
	int err = ::pthread_cond_destroy( &mCondVar );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * Broadcast( void )
// -------------------------------------------------------------------------- //
void
TCondVar::Broadcast( void )
{
	int err = ::pthread_cond_broadcast( &mCondVar );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * Signal( void )
// -------------------------------------------------------------------------- //
void
TCondVar::Signal( void )
{
	int err = ::pthread_cond_signal( &mCondVar );
	assert( err == 0 );
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * TimedWait( TMutex*, const struct timespec* )
// -------------------------------------------------------------------------- //
Boolean
TCondVar::TimedWait( TMutex* inMutex, const struct timespec* inAbsTime )
{
	int err = ::pthread_cond_timedwait( &mCondVar, &inMutex->mMutex, inAbsTime );
	if (err == ETIMEDOUT)
	{
		return false;
	} else {
		assert( err == 0 );
		return true;
	}
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * TimedWaitRelative( TMutex*, const struct timespec* )
// -------------------------------------------------------------------------- //
Boolean
TCondVar::TimedWaitRelative( TMutex* inMutex, const struct timespec* inRelTime )
{
#if HAS_COND_TIMEDWAIT_RELATIVE_NP
	int err = ::pthread_cond_timedwait_relative_np(
					&mCondVar, &inMutex->mMutex, inRelTime );
#else
	struct timespec abs_time;
	struct timeval now;
	(void) gettimeofday( &now, NULL );
	abs_time.tv_nsec = (now.tv_usec * 1000) + inRelTime->tv_nsec;
	abs_time.tv_sec = now.tv_sec + (abs_time.tv_nsec / 1000000000);
	abs_time.tv_nsec %= 1000000000;
	int err = ::pthread_cond_timedwait(
					&mCondVar, &inMutex->mMutex, &abs_time );
#endif

	if (err == ETIMEDOUT)
	{
		return false;
	} else {
		assert( err == 0 );
		return true;
	}
	(void) err;
}

// -------------------------------------------------------------------------- //
//  * Wait( TMutex*, const struct timespec* )
// -------------------------------------------------------------------------- //
void
TCondVar::Wait( TMutex* inMutex )
{
	int err = ::pthread_cond_wait( &mCondVar, &inMutex->mMutex );
	assert( err == 0 );
	(void) err;
}

// =============================================================================== //
// Niklaus Wirth has lamented that, whereas Europeans pronounce his name correctly //
// (Ni-klows Virt), Americans invariably mangle it into (Nick-les Worth).  Which   //
// is to say that Europeans call him by name, but Americans call him by value.     //
// =============================================================================== //
