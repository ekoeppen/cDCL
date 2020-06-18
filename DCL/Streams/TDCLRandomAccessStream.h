// ==============================
// Fichier:			TDCLRandomAccessStream.h
// Projet:			Desktop Connection Library
//
// Créé le:			21/2/2004
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
// The Original Code is TDCLRandomAccessStream.h.
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
// $Id: TDCLRandomAccessStream.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLRANDOMACCESSSTREAM_H
#define _TDCLRANDOMACCESSSTREAM_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStream.h>

///
/// Classe pour un flux avec un retour arrière possible.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLRandomAccessStream
	:
		public TDCLStream
{
public:
	///
	/// Différents modes pour le curseur.
	/// Utilisés avec la méthode SetCursor.
	///
	enum ECursorMode {
		kFromStart,		///< depuis le début du fichier
		kFromLEOF,		///< depuis la fin du fichier
		kFromCursor		///< depuis la position courante du curseur
	};

	///
	/// Détermine la position du curseur dans le flux.
	///
	/// \return la position par rapport au début du flux.
	///
	virtual KSInt64 GetCursor( void ) = 0;

	///
	/// Déplace le curseur dans le flux. Peut lancer une exception
	/// pour indiquer que la fin du flux a été atteinte.
	///
	/// \param inPos	nouvelle position du curseur
	/// \param inMode	définit l'origine de \c inPos
	/// \throws TDCLPositionException si la fin du flux (ou le début) est
	///			dépassée.
	///
	virtual void SetCursor( KSInt64 inPos, ECursorMode inMode ) = 0;
};

#endif
		// _TDCLRANDOMACCESSSTREAM_H

// ========================================================================= //
// Windows Airlines:                                                         //
// The terminal is very neat and clean, the attendants all very attractive,  //
// the pilots very capable. The fleet of Learjets the carrier operates is    //
// immense.  Your jet takes off without a hitch, pushing above the clouds,   //
// and at 20,000 feet it explodes without warning.                           //
// ========================================================================= //
