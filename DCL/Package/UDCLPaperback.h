// ==============================
// Fichier:			UDCLPaperback.h
// Projet:			Desktop Connection Library
// 
// Créé le:			9/5/2004
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
// The Original Code is UDCLPaperback.h.
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
// $Id: UDCLPaperback.h,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _UDCLPAPERBACK_H
#define _UDCLPAPERBACK_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour la construction d'une structure book compatible avec PaperBack.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class UDCLPaperback
{
public:
	///
	/// Encodages pour des chaînes.
	///
	enum EEncoding {
		kUTF8,						///< UTF-8
		kUTF16,						///< UTF-16
		kISO88591,					///< ISO 8859-1, aka Latin1
		kISO88592,					///< ISO 8859-2, aka Latin2
		kMacRoman					///< MacRoman
	};
	
	///
	/// Construit le livre à partir d'un flux.
	///
	/// \param inTextStream		flux avec le texte (lu jusqu'à la fin)
	/// \param inEncoding		encodage des données.
	/// \param inBookTitle		titre du livre (en ISO-8869-1)
	/// \param inTOCPrefix		préfixe des ligne de la TOC (pas de TOC si \c nil)
	/// \param inDeleteTOCLines	si les lignes de la TOC doivent être supprimées
	/// \param inFontName		nom de la police par défaut (Geneva si \c nil)
	/// \param inFontSize		taille de la police par défaut
	///
	static TDCLNSRef BuildBook(
			TDCLStream* inTextStream,
			EEncoding inEncoding,
			const char* inBookTitle,
			const char* inTOCPrefix /* = nil */,
			Boolean inDeleteTOCLines /* = true */,
			const char* inFontName /* = nil */,
			int inFontSize /* = 9 */ );

private:
	///
	/// Extrait la chaîne.
	///
	/// \param inTextStream		flux du texte.
	/// \param inEncoding		encodage
	/// \param outUTF16Text		en sortie, texte alloué avec malloc.
	/// \param outSize			en sortie, nombre de caractères.
	/// \return une chaîne UTF-16 avec un saut de ligne à la fin.
	///
	static void ExtractString(
				TDCLStream* inTextStream,
				EEncoding inEncoding,
				KUInt16** outUTF16Text,
				KUInt32* outSize );

	///
	/// Conversion des fins de ligne vers les lignes Mac.
	///
	/// \param inUTF16Text	chaîne UTF-16 allouée avec malloc.
	/// \param ioSize		nombre de caractères (en entrée et en sortie)
	///
	static void ConvertEndLines( KUInt16* inUTF16Text, KUInt32* ioSize );

	///
	/// Construit la table des matières.
	///
	/// \param inText			le texte
	/// \param ioTextSize		nombre de caractères (en entrée et en sortie)
	/// \param inTOCLinePrefix	préfixe ou \c nil pour pas de TOC.
	/// \param inDeleteTOCLines si les lignes de la TOC doivent être supprimées.
	/// \param outTOCEntries	en sortie, pointeur sur les entrées alloué avec
	///							malloc. Les éléments sont aussi alloués avec
	///							malloc.
	/// \param outTOCIndexes	en sortie, pointeur sur les indices alloué avec
	///							malloc.
	/// \param outTOCSize		en sortie, taille de la table des matières.
	///
	static void BuildTOC(
				KUInt16* inText,
				KUInt32* ioTextSize,
				const char* inTOCLinePrefix,
				Boolean inDeleteTOCLines,
				KUInt16*** outTOCEntries,
				KUInt32** outTOCIndexes,
				KUInt32* outTOCSize );

	///
	/// Construit la police par défaut.
	///
	/// \param inFontName		nom de la police (ou \c nil pour Geneva)
	/// \param inFontSize		taille de la police.
	/// \return une spécification de police correspondant.
	///
	static TDCLNSRef BuildFont(
				const char* inFontName,
				int inFontSize );

	///
	/// Construit le livre.
	///
	/// \param inText			texte du livre
	/// \param inDefaultFont	police par défaut (de BuildFont)
	/// \param inBookTitle		titre du livre.
	/// \param inTOCEntries		pointeur sur les entrées (de BuildTOC)
	///							ou \c nil.
	/// \param inTOCIndexes		pointeur sur les indices (de BuildTOC)
	///							ou \c nil.
	/// \param inTOCSize		taille de la table des matières (en nombre
	///							d'entrées) (de BuildTOC).
	/// \return la structure du paquet.
	///
	static TDCLNSRef BuildBook(
				const TDCLNSRef& inText,
				const TDCLNSRef& inDefaultFont,
				const char* inBookTitle,
				const KUInt16* const * inTOCEntries,
				const KUInt32* inTOCIndexes,
				KUInt32 inTOCSize );
};

#endif
		// _UDCLPAPERBACK_H

// =============================================================== //
// Excessive login or logout messages are a sure sign of senility. //
// =============================================================== //
