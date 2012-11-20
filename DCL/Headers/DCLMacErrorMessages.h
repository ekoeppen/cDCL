// ==============================
// Fichier:			DCLMacErrorMessages.h
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			22/08/2000
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
// The Original Code is DCLMacErrorMessages.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLMacErrorMessages.h,v 1.5 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __DCLMACERRORMESSAGES__
#define __DCLMACERRORMESSAGES__

#ifndef __DCLERRORMESSAGES__
	#ifdef rez
		#include "DCLErrorMessages.h"
	#else
		#include <DCLErrorMessages.h>
	#endif
#endif

#define kMacErrorResType 'ErrM'

// More MacOS errors that could arise in the DCL.

// It's more readable if I put Rez & C definition one after the other rather
// than mixing them.
// The principle is the following: each resource has an idea depending on the
// kind of errors we have.

// Everything is not aligned

#ifdef rez

type kMacErrorResType {
	longint;				// The error code for the unknown error message.
	integer = $$Countof(ErrorMsgsArray);
	
	array ErrorMsgsArray {
		longint;			// The error code
		cstring;			// The error string
	};
};

#else

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

///
/// Structure pour un message d'erreur avec un ID et une cha”ne C.
///
typedef struct {
	long			fID;			///< Identifiant de l'erreur.
	char			fString[1];		///< Cha”ne de caractre.
} SErrorMessage;

///
/// Structure pour une liste de messages d'erreurs.
///
typedef struct {
	long			fUnknownErrorID;	///< Identifiant pour les erreurs.
	short			fCount;				///< Nombre de message d'erreur.
	SErrorMessage	fMessages[1];		///< Messages d'erreur.
} SMacErrorRes;

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#endif

#endif
		// __DCLMACERRORMESSAGES__

// ======================================================================= //
// Ç C'est l'histoire d'un gars qui veut la machine la plus puissante du   //
// monde sous Windows 95 en Žmulation sous Wine qui tourne sur une station //
// FreeBSD avec bibliotheque de compatibilitŽ Linux. È                     //
//     -+- ST in Guide du linuxien pervers : "A quoi sert Unix ?" -+-      //
// ======================================================================= //