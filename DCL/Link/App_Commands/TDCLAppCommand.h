// ==============================
// Fichier:			TDCLAppCommand.h
// Projet:			Desktop Connection Library
//
// Créé le:			2/2/2003
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
// The Original Code is TDCLAppCommand.h.
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
// $Id: TDCLAppCommand.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLAPPCOMMAND_H
#define _TDCLAPPCOMMAND_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLEvent.h>

///
/// Classe de base pour une commande venant de l'application.
/// Cette classe ne sert désormais plus qu'à distinguer les
/// commandes venant de l'application. Elle ne diffère pas vraiment
/// de la classe TDCLEvent.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLAppCommand
	:
		public TDCLEvent
{
public:
	///
	/// Constructeur à partir d'un type, d'un ID et d'une donnée.
	/// La donnée n'est pas supprimée lorsque cet objet est supprimé (sauf
	/// si la sous-classe s'en occupe).
	///
	/// \param inCommandKind	Type de la commande.
	/// \param inCommandID		Identifiant de la commande
	/// \param inData			donnée associée à l'événement.
	///
	TDCLAppCommand(
				EEventKind inCommandKind,
				EEventID inCommandID,
				void* inData = nil );

	///
	/// Accesseur sur l'identifiant de la commande.
	///
	/// \return l'identifiant de la commande.
	///
	inline KUInt32	GetCommandID( void )
		{
			return GetEventID();
		}
};

#endif
		// _TDCLAPPCOMMAND_H

// ====================================== //
// VMS is like a nightmare about RXS-11M. //
// ====================================== //
