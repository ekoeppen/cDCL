// ==============================
// Fichier:			TDCLNSFunction.h
// Projet:			Desktop Connection Library
// 
// Créé le:			8/4/2004
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
// The Original Code is TDCLNSFunction.h.
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
// $Id: TDCLNSFunction.h,v 1.3 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef _TDCLNSFUNCTION_H
#define _TDCLNSFUNCTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

///
/// Classe pour les fonctions.
/// Comme les chaînes sont des binaires, les fonctions sont des Frames.
/// Aucune variable ici ni méthode virtuelle.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLNSFunction
	:
		public TDCLNSFrame
{
public:
	///
	/// Constructeur par défaut.
	/// Crée une fonction nouvelle génération avec 0 argument.
	///
	TDCLNSFunction( void );
};

#endif
		// _TDCLNSFUNCTION_H

// ======================================================================= //
// The meat is rotten, but the booze is holding out.                       //
//                                                                         //
// Computer translation of "The spirit is willing, but the flesh is weak." //
// ======================================================================= //
