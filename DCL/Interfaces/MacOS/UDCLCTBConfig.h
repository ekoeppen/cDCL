// ==============================
// Fichier:			UDCLCTBConfig.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is UDCLCTBConfig.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: UDCLCTBConfig.h,v 1.3 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _UDCLCTBCONFIG_H
#define _UDCLCTBCONFIG_H

#include <DCL/Headers/DCLDefinitions.h>

// Vérification de la cible.
#if !TARGET_OS_MACOS || TARGET_OS_OPENSTEP
	#error "La Communication Toolbox n'est disponible que sur MacOS"
#endif

// DCL
#include <DCL/Exceptions/TDCLException.h>

///
/// Classe pour les dialogues de configuration d'outils CTB.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class UDCLCTBConfig
{
public:
	///
	/// Affiche le dialogue de configuration pour les outils CTB et récupère
	/// le résultat.
	///
	/// \param outToolName			en sortie, nom de l'outil CTB (CString)
	/// \param outConfigString		en sortie, chaîne de configuration pour l'outil (idem)
	/// \throws	TDCLException si une erreur est survenue.
	///
	static void	GetCommToolConfig( char* outToolName, char* outConfigString );
};

#endif
		// _UDCLCTBCONFIG_H

// ======================================================================= //
// As part of the conversion, computer specialists rewrote 1,500 programs; //
// a process that traditionally requires some debugging.                   //
//                 -- USA Today, referring to the Internal Revenue Service //
//                    conversion to a new computer system.                 //
// ======================================================================= //