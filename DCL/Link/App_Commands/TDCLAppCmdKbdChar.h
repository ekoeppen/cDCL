// ==============================
// Fichier:			TDCLAppCmdKbdChar.h
// Projet:			Desktop Connection Library
//
// Créé le:			11/4/2003
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
// The Original Code is TDCLAppCmdKbdChar.h.
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
// $Id: TDCLAppCmdKbdChar.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLAPPCMDKBDCHAR_H
#define _TDCLAPPCMDKBDCHAR_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>

///
/// Classe pour une commande pour envoyer un seul caractère au Newton.
/// Ce caractère peut comprendre une touche et un code pour savoir si la
/// touche pomme était enfoncée (en théorie).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLAppCmdKbdChar
	:
		public TDCLAppCommand
{
public:
	///
	/// Constructeur à partir de la touche et de drapeaux pour indiquer
	/// les touches modificatrices.
	///
	/// \param inChar		caractère
	/// \param inFlags		état de la touche. Permet en théorie de dire que
	///						la touche commande est enfoncée, mais le Newton
	///						ne semble pas le gérer.
	///
	TDCLAppCmdKbdChar( KUInt16 inChar, KUInt16 inFlags );

private:
	/// \name Variables
	KUInt16			mData[2];	///< Données pour cette commande.
};

#endif
		// _TDCLAPPCMDKBDCHAR_H

// ==================================================================== //
// I am not now, nor have I ever been, a member of the demigodic party. //
//                 -- Dennis Ritchie                                    //
// ==================================================================== //
