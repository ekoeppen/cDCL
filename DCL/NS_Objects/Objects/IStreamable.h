// ==============================
// Fichier:			IStreamable.h
// Projet:			Desktop Connection Library
//
// Créé le:			16/10/2002
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
// The Original Code is IStreamable.h.
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
// $Id: IStreamable.h,v 1.5 2006/02/12 01:23:56 pguyot Exp $
// ===========

#ifndef __ISTREAMABLE__
#define __ISTREAMABLE__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>

class TDCLNSEncoder;
class TDCLXMLEncoder;
class TDCLNSOFEncoder;
class TDCLPkgEncoder;
class TDCLStream;
class TDCLTextEncoder;

///
/// Classe pour les objets qui peuvent être écrits et lus
/// dans/à partir d'un flux avec un encodeur/décodeur.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class IStreamable
{
	friend class TDCLXMLEncoder;
	friend class TDCLNSOFEncoder;
	friend class TDCLTextEncoder;
	friend class TDCLPkgEncoder;

public:
	///
	/// Destructor.
	///
	virtual ~IStreamable( void ) {};

private:
	///
	/// Crée une représentation XML de l'objet.
	/// Comprend la balise de début, les données et la balise de fin.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	/// \param inObjectID		ID de l'objet dans ce flux.
	///
	virtual void ToXML(
				TDCLXMLEncoder* inEncoder,
				KUInt32			inObjectID ) const = 0;

	///
	/// Crée une représentation NSOF de l'objet.
	/// Comprend le type et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToNSOF( TDCLNSOFEncoder* inEncoder ) const = 0;

	///
	/// Crée une représentation sous forme de texte de l'objet.
	/// Comprend les séparateurs et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToText( TDCLTextEncoder* inEncoder ) const = 0;

	///
	/// Crée une représentation de l'objet pour mettre dans un
	/// paquet.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder	encodeur pour le paquet.
	/// \param ioOffset		en entrée et en sortie, décalage par rapport
	///						au paquet.
	///
	virtual void ToPkg( TDCLPkgEncoder* inEncoder, KUInt32* ioOffset ) const = 0;
};

#endif
		// __ISTREAMABLE__

// ===================================== //
// God is real, unless declared integer. //
// ===================================== //
