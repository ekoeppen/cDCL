// ==============================
// Fichier:			TDCLMacDesktopFolder.h
// Projet:			Desktop Connection Library
//
// Créé le:			11/1/2003
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
// The Original Code is TDCLMacDesktopFolder.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLMacDesktopFolder.h,v 1.3 2004/11/24 14:09:00 paul Exp $
// ===========

#ifndef _TDCLMACDESKTOPFOLDER_H
#define _TDCLMACDESKTOPFOLDER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLRootFolder.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLMacFiles;

///
/// Classe de base pour le bureau sur MacOS et OS X.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLMacDesktopFolder
	:
		public TDCLRootFolder
{
public:
	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Retourne une structure avec name: "Desktop" et type: 'desktop.
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef	ToFrame( void );

protected:
	///
	/// Constructeur à partir de l'interface sur les fichiers.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	///
	TDCLMacDesktopFolder( TDCLMacFiles* inFilesIntf );

private:
	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	///
	virtual KUInt16*		MakeName( void ) const;

	/// \name Constantes
	static const char* const	kDesktopStr; ///< Nom du bureau ("Desktop")
};

#endif
		// _TDCLMACDESKTOPFOLDER_H

// =========================================================================== //
// The work [of software development] is becoming far easier (i.e. the tools   //
// we're using work at a higher level, more removed from machine, peripheral   //
// and operating system imperatives) than it was twenty years ago, and because //
// of this, knowledge of the internals of a system may become less accessible. //
// We may be able to dig deeper holes, but unless we know how to build taller  //
// ladders, we had best hope that it does not rain much.                       //
//                 -- Paul Licker                                              //
// =========================================================================== //