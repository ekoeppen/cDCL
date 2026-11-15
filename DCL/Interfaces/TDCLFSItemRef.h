// ==============================
// Fichier:			TDCLFSItemRef.h
// Projet:			Desktop Connection Library
//
// Créé le:			9/4/2003
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
// The Original Code is TDCLFSItemRef.h.
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
// $Id: TDCLFSItemRef.h,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef _TDCLFSITEMREF_H
#define _TDCLFSITEMREF_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLClassCastException.h>

// Pré-déclarations
class TDCLFSItem;
class TDCLFile;
class TDCLFolder;

///
/// Classe pour une référence sur un objet de type TDCLFSItem.
/// Ceci permet d'éviter les fuites de mémoire.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFSItemRef
{
public:
	///
	/// Constructeur à partir d'un objet TDCLFSItem.
	///
	/// \param inItem	objet TDCLFSItem à référencer
	///
	TDCLFSItemRef( TDCLFSItem* inItem = nil );

	///
	/// Destructeur.
	///
	~TDCLFSItemRef( void );

	///
	/// Constructeur par copie.
	/// Incrémente la référence.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFSItemRef( const TDCLFSItemRef& inCopy );

	///
	/// Opérateur d'assignation.
	/// Idem.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFSItemRef& operator = ( const TDCLFSItemRef& inCopy );

	///
	/// Opérateur de déréférencement.
	/// Retourne l'objet TDCLFSItem
	///
	inline TDCLFSItem& operator * () const
		{
			return *mFSItem;
		}

	///
	/// Opérateur de déréférencement.
	/// Retourne l'objet TDCLFSItem
	///
	inline TDCLFSItem* operator -> () const
		{
			return mFSItem;
		}

	///
	/// Retourne l'objet TDCLFSItem sous forme de fichier
	/// (vérifie que c'est un fichier)
	///
	/// \throws TDCLClassCastException si l'objet n'est pas un fichier.
	///
	operator TDCLFile* () const;

	///
	/// Retourne l'objet TDCLFSItem sous forme de dossier
	/// (vérifie que c'est un dossier)
	///
	/// \throws TDCLClassCastException si l'objet n'est pas un dossier.
	///
	operator TDCLFolder* () const;

	///
	/// Accesseur sur l'objet TDCLFSItem.
	///
	inline TDCLFSItem* GetFSItem( void ) const
		{
			return mFSItem;
		}

private:
	/// \name Variables
	TDCLFSItem*		mFSItem;	///< Objet référencé.
};

#endif
		// _TDCLFSITEMREF_H

// =================================== //
// The world is not octal despite DEC. //
// =================================== //
