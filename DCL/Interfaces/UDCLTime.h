// ==============================
// Fichier:			UDCLTime.h
// Projet:			Desktop Connection Library
// 
// Créé le:			15/4/2004
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
// The Original Code is UDCLTime.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: UDCLTime.h,v 1.3 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef _UDCLTIME_H
#define _UDCLTIME_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour la gestion du temps.
/// Cette classe repose sur les fonctions de l'ANSI C.
/// On suppose que l'origine du temps de <time.h> est avant le 1er janvier
/// 1993.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class UDCLTime
{
public:
	///
	/// Temps en secondes depuis le 1er janvier 1904.
	///
	/// \return l'heure en secondes depuis le 1er janvier 1904.
	///
	static KUInt32 TimeInSecondsSince1904( void );

	///
	/// Temps en minutes depuis le 1er janvier 1993.
	///
	/// \return l'heure en minutes depuis le 1er janvier 1993.
	///
	static KUInt32 TimeInMinutesSince1904( void );

	///
	/// Temps en secondes depuis le 1er janvier 1993.
	///
	/// \return l'heure en secondes depuis le 1er janvier 1993.
	///
	static KUInt32 TimeInSecondsSince1993( void );

private:
	/// Secondes entre 1993 et 1904.
	static const KUInt32 kSecondsBetween1904And1993;
	
	/// Minutes entre 1993 et 1904.
	static const KUInt32 kMinutesBetween1904And1993;
};

#endif
		// _UDCLTIME_H

// =========== //
// HOLY MACRO! //
// =========== //
