// ==============================
// Fichier:			TDCLDockCmdSingleLong.h
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
// The Original Code is TDCLDockCmdSingleLong.h.
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
// $Id: TDCLDockCmdSingleLong.h,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCMDSINGLELONG__
#define __TDCLDOCKCMDSINGLELONG__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// K
#include <K/Defines/UByteSex.h>

///
/// Classe pour une commande du dock contenant un long (4 octets)
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdSingleLong
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLDockCmdSingleLong( void );
	
	///
	/// Constructeur à partir d'une commande et d'un long.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inLong		le long composant cette commande.
	///
	TDCLDockCmdSingleLong( KUInt32 inCommand, KUInt32 inLong = 0 );

	///
	/// Accesseur sur le long.
	///
	/// \return le long de cette commande.
	///
	inline KUInt32	GetLong( void ) const
		{
			return UByteSex_FromBigEndian( mLong );
		}
		
	///
	/// Sélecteur sur le long.
	///
	/// \param inNewLong nouvelle valeur pour le long de cette commande.
	///
	inline void	SetLong( KUInt32 inNewLong )
		{
			mLong = UByteSex_ToBigEndian( inNewLong );
		}
	
protected:
	///
	/// Méthode pour récupérer un pointeur sur les données afin de les
	/// envoyer au Newton.
	/// Retourne un pointeur sur le long.
	///
	/// \return un pointeur sur le long.
	///
	virtual const void*		GetData( void );
	
	///
	/// Méthode pour connaître la taille des données afin de les envoyer
	/// au Newton.
	/// Retourne 4.
	///
	/// \return la taille du long (4).
	///
	virtual	KUInt32			GetLength( void ) const;

private:
	KUInt32		mLong;		///< Le long qui constitue les données de cette
							///< commande.
};

#endif
		// __TDCLDOCKCMDSINGLELONG__

// ====================== //
// Equal bytes for women. //
// ====================== //
