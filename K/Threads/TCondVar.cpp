// ==============================
// Fichier:			TCondVar.cp
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
// The Original Code is TCondVar.cp.
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
// $Id: TCondVar.cp,v 1.2 2005/06/07 05:53:32 pguyot Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TCondVar.h"

#include <chrono>

// K
#include <K/Threads/TMutex.h>

// -------------------------------------------------------------------------- //
//  * TCondVar( void )
// -------------------------------------------------------------------------- //
TCondVar::TCondVar( void )
{
}

// -------------------------------------------------------------------------- //
//  * ~TCondVar( void )
// -------------------------------------------------------------------------- //
TCondVar::~TCondVar( void )
{
}

// -------------------------------------------------------------------------- //
//  * Broadcast( void )
// -------------------------------------------------------------------------- //
void
TCondVar::Broadcast( void )
{
	mCondVar.notify_all();
}

// -------------------------------------------------------------------------- //
//  * Signal( void )
// -------------------------------------------------------------------------- //
void
TCondVar::Signal( void )
{
	mCondVar.notify_one();
}

// -------------------------------------------------------------------------- //
//  * TimedWait( TMutex*, const struct timespec* )
// -------------------------------------------------------------------------- //
Boolean
TCondVar::TimedWait( TMutex* inMutex, const struct timespec* inAbsTime )
{
	auto dur = std::chrono::seconds(inAbsTime->tv_sec) +
	           std::chrono::nanoseconds(inAbsTime->tv_nsec);
	auto target_tp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(dur)));
	std::cv_status status = mCondVar.wait_until(inMutex->mMutex, target_tp);
	return (status == std::cv_status::no_timeout);
}

// -------------------------------------------------------------------------- //
//  * TimedWaitRelative( TMutex*, const struct timespec* )
// -------------------------------------------------------------------------- //
Boolean
TCondVar::TimedWaitRelative( TMutex* inMutex, const struct timespec* inRelTime )
{
	auto rel_dur = std::chrono::seconds(inRelTime->tv_sec) +
	               std::chrono::nanoseconds(inRelTime->tv_nsec);
	std::cv_status status = mCondVar.wait_for(inMutex->mMutex, rel_dur);
	return (status == std::cv_status::no_timeout);
}

// -------------------------------------------------------------------------- //
//  * Wait( TMutex* )
// -------------------------------------------------------------------------- //
void
TCondVar::Wait( TMutex* inMutex )
{
	mCondVar.wait( inMutex->mMutex );
}

// =============================================================================== //
// Niklaus Wirth has lamented that, whereas Europeans pronounce his name correctly //
// (Ni-klows Virt), Americans invariably mangle it into (Nick-les Worth).  Which   //
// is to say that Europeans call him by name, but Americans call him by value.     //
// =============================================================================== //
