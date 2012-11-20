// ==============================
// Fichier:			UDCLTime.cp
// Projet:			Desktop Connection Library
// 
// CrŽŽ le:			15/4/2004
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
// The Original Code is UDCLTime.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: UDCLTime.cp,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/UDCLTime.h>

// ANSI C
#include <time.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //
const KUInt32 UDCLTime::kSecondsBetween1904And1993 = ((KUInt32) 0xA7693A00);
const KUInt32 UDCLTime::kMinutesBetween1904And1993 = ((KUInt32) 0x02CA4980);

// -------------------------------------------------------------------------- //
//  * TimeInSecondsSince1993( void )
// -------------------------------------------------------------------------- //
KUInt32
UDCLTime::TimeInSecondsSince1993( void )
{
	tm january1st1993;
	january1st1993.tm_sec = 0;
	january1st1993.tm_min = 0;
	january1st1993.tm_hour = 0;
	january1st1993.tm_mday = 1;
	january1st1993.tm_mon = 0;
	january1st1993.tm_year = 1993;
	january1st1993.tm_wday = 0;
	january1st1993.tm_yday = 0;
	january1st1993.tm_isdst = 0;
	
	time_t theDate = ::mktime( &january1st1993 );
	time_t now = ::time(NULL);
	
	return (KUInt32) ::difftime( now, theDate );
}

// -------------------------------------------------------------------------- //
//  * TimeInSecondsSince1904( void )
// -------------------------------------------------------------------------- //
KUInt32
UDCLTime::TimeInSecondsSince1904( void )
{
	return TimeInSecondsSince1993() + kSecondsBetween1904And1993;
}

// -------------------------------------------------------------------------- //
//  * TimeInMinutesSince1904( void )
// -------------------------------------------------------------------------- //
KUInt32
UDCLTime::TimeInMinutesSince1904( void )
{
	return (TimeInSecondsSince1993() / 60) + kMinutesBetween1904And1993;
}

// =================== //
// You are false data. //
// =================== //
