// ==============================
// Fichier:			KDCLPkg.h
// Projet:			Desktop Connection Library
// 
// Créé le:			25/2/2004
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
// The Original Code is KDCLPkg.h.
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
// $Id: KDCLPkg.h,v 1.5 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _KDCLPKG_H
#define _KDCLPKG_H

#include <K/Defines/KDefinitions.h>

///
/// Classe pour les constantes spécifiques à l'encodage/au décodage des paquets.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class KDCLPkg
{
public:
	/// Constantes pour le format paquets
	enum {
		kObjFlagHeader	= 0x40,		///< Drapeau toujours à un dans l'entête.
		kObjSlotted		= 0x01,		///< L'objet est un tableau ou une structure (sinon, c'est un
									///< binaire)
		kObjFrame		= 0x02,		///< L'objet est une structure. Requiert kObjSlotted.
		kSizeShift		= 8,		///< Nombre de décalage à gauche on doit faire sur la taille.
		kObjFlagsMask	= 0xFF,
		kMaxSize		= 0xFFFFFF,	///< Taille maximale des objets (entête compris).
		kSymbolClass	= 0x55552,	///< Classe spéciale des symboles.
		kMapSorted		= 1,		///< Si les clés de la carte sont triées.
		kMapProto		= 4,		///< Si la carte ou une super carte comprend _proto
		kTagMask		= 0x03,		///< 0b11
		kPointerTag		= 0x01,		///< 0b01
		
		// Premier objet.
		k16BytesArray	= 0x00001041,
		k4BytesPadding	= 0x00000001,	///< L'aligment est sur 4 octets.
		k8BytesPadding	= 0x00000000,	///< L'alignement est sur 8 octets.
		kNILClass		= 0x00000002	///< NIL.
	};
};

#endif
		// _KDCLPKG_H

// =================================================== //
// There's got to be more to life than compile-and-go. //
// =================================================== //
