// ==============================
// Fichier:			TDCLDockCmdNoData.h
// Projet:			Desktop Connection Library
//
// Créé le:			02/02/2001
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
// The Original Code is TDCLDockCmdNoData.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdNoData.h,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCMDNODATA__
#define __TDCLDOCKCMDNODATA__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour une commande Dock sans données.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdNoData
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLDockCmdNoData( void );

	///
	/// Constructeur à partir d'un identifiant de commande.
	///
	/// \param inCommand	identifiant de commande.
	///
	TDCLDockCmdNoData( KUInt32 inCommand );

protected:
	///
	/// Détermine la taille des données de la commande (ici 0).
	///
	/// \return \c 0
	///
	virtual	KUInt32			GetLength( void ) const;
};

#endif
		// __TDCLDOCKCMDNODATA__

// ============================================================================= //
//         After sifting through the overwritten remaining blocks of Luke's home //
// directory, Luke and PDP-1 sped away from /u/lars, across the surface of the   //
// Winchester riding Luke's flying read/write head.  PDP-1 had Luke stop at the  //
// edge of the cylinder overlooking /usr/spool/uucp.                             //
//         "Unix-to-Unix Copy Program;" said PDP-1.  "You will never find a more //
// wretched hive of bugs and flamers.  We must be cautious."                     //
//                 -- DECWARS                                                    //
// ============================================================================= //
