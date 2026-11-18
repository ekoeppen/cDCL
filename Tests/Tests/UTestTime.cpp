// ==============================
// Fichier:			UTestTime.cpp
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
#include <DCL/Interfaces/UDCLTime.h>

#include <doctest/doctest.h>

#include <ctime>

TEST_SUITE("Time")
{
	TEST_CASE("UDCLTime epochs and system time")
	{
		CHECK(::time(NULL) != (time_t) -1);

		// 2004-04-15 in Newton epochs (seconds since 1993, seconds/minutes since 1904).
		const KUInt32 sec1993Before = UDCLTime::TimeInSecondsSince1993();
		const KUInt32 sec1904 = UDCLTime::TimeInSecondsSince1904();
		const KUInt32 min1904 = UDCLTime::TimeInMinutesSince1904();
		const KUInt32 sec1993After = UDCLTime::TimeInSecondsSince1993();

		CHECK(sec1993Before >= (KUInt32) 0x153AA500);
		CHECK(sec1993After >= sec1993Before);
		CHECK(sec1904 >= (KUInt32) 0xBCA3DF00);
		CHECK(min1904 >= (KUInt32) 0x0325B650);
		CHECK(sec1904 >= sec1993Before + (KUInt32) 0xA7693A00);
		CHECK(sec1904 <= sec1993After + (KUInt32) 0xA7693A00);
		CHECK(min1904 >= (sec1993Before + (KUInt32) 0xA7693A00) / 60);
		CHECK(min1904 <= (sec1993After + (KUInt32) 0xA7693A00) / 60);
	}
}
