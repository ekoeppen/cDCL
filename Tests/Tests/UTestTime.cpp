// ==============================
// Fichier:			UTestTime.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			15/4/2004
// Tabulation:		4 espaces
// 
// Copyright:		© 2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestTime.cp,v 1.4 2004/04/15 23:06:34 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "UTestTime.h"

// ANSI C
#include <time.h>

// DCL
#include <DCL/Interfaces/UDCLTime.h>

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * CheckReturnValues( void )
// -------------------------------------------------------------------------- //
void
UTestTime::CheckReturnValues( void )
{
	// On vérifie que time(3) ne retourne pas -1.
	if (::time(NULL) == (time_t) -1)
	{
		(void) ::printf( "time(3) retourne -1\n" );
	}

	// On vérifie que l'heure en secondes depuis 1993 est supérieure à
	// l'heure quand j'ai écrit ce test.
	KUInt32 theTime = UDCLTime::TimeInSecondsSince1993();
	if (theTime < ((KUInt32) 0x407E2400))
	{
		(void) ::printf(
			"TimeInSecondsSince1993 semble incorrect! (%.8X)\n",
			(unsigned int) theTime );
	}

	theTime = UDCLTime::TimeInSecondsSince1904();
	if (theTime < ((KUInt32) 0xE7E75E00))
	{
		(void) ::printf(
			"TimeInSecondsSince1904 semble incorrect! (%.8X)\n",
			(unsigned int) theTime );
	}

	theTime = UDCLTime::TimeInMinutesSince1904();
	if (theTime < ((KUInt32) 0x03DD74C4))
	{
		(void) ::printf(
			"TimeInMinutesSince1904 semble incorrect! (%.8X)\n",
			(unsigned int) theTime );
	}
}

// ====================== //
// Equal bytes for women. //
// ====================== //
