// ==============================
// Fichier:			UByteSex.h
// Projet:			K
//
// Créé le:			13/01/2002
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
// The Original Code is UByteSex.h.
//
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2002-2004
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef _UBYTESEX_H
#define _UBYTESEX_H

#include <bit>
#include <K/Defines/KDefinitions.h>

///
/// Classe pour gérer le sexe des octets (endianness).
///
/// Convertit des mots de grand indien vers petit indien et inversement.
/// Fournit des helpers pour faire la conversion selon la plateforme.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
class UByteSex
{
public:
	///
	/// Echange les deux octets d'un mot de 16 bits.
	/// XXYY -> YYXX
	///
	/// \param inWord	mot de 16 bits en entrée.
	/// \return le mot de 16 bits inversé.
	///
	static constexpr KUInt16	Swap( const KUInt16 inWord )
		{
#if defined(__GNUC__) || defined(__clang__)
			return __builtin_bswap16(inWord);
#else
			return (KUInt16) ((inWord << 8) | (inWord >> 8));
#endif
		}

	///
	/// Echange les octets d'un mot de 32 bits.
	/// XXYYZZTT -> TTZZYYXX
	///
	/// \param inWord	mot de 32 bits en entrée.
	/// \return le mot de 32 bits inversé.
	///
	static constexpr KUInt32	Swap( const KUInt32 inWord )
		{
#if defined(__GNUC__) || defined(__clang__)
			return __builtin_bswap32(inWord);
#else
			return (
					((inWord >> 24)	& 0x000000FF) |
					((inWord >> 8)	& 0x0000FF00) |
					((inWord << 8)	& 0x00FF0000) |
					((inWord << 24)	& 0xFF000000));
#endif
		}

	///
	/// Echange les octets d'un mot de 64 bits.
	///
	/// \param inWord	mot de 64 bits en entrée.
	/// \return le mot de 64 bits inversé.
	///
	static constexpr KUInt64	Swap( const KUInt64 inWord )
		{
#if defined(__GNUC__) || defined(__clang__)
			return __builtin_bswap64(inWord);
#else
			return (
					(((KUInt64) Swap((KUInt32) (inWord & 0xFFFFFFFFULL))) << 32) |
					((KUInt64) Swap((KUInt32) (inWord >> 32))));
#endif
		}

	///
	/// Generic template to handle any integer type based on size.
	/// Resolves ambiguity when typedefs differ from base types.
	///
	/// \param inWord	integer value to swap.
	/// \return the byte-swapped value.
	///
	template<typename T>
	static constexpr T Swap(T inWord)
		{
			static_assert(
				sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
				"UByteSex::Swap<T>: unsupported type width (must be 2, 4, or 8 bytes)");
			if constexpr (sizeof(T) == 2)
				return (T) Swap((KUInt16) inWord);
			else if constexpr (sizeof(T) == 4)
				return (T) Swap((KUInt32) inWord);
			else
				return (T) Swap((KUInt64) inWord);
		}

	template<typename T>
	static constexpr T FromBigEndian(T inWord)
		{
			if constexpr (std::endian::native == std::endian::little)
				return Swap(inWord);
			else
				return inWord;
		}

	template<typename T>
	static constexpr T FromLittleEndian(T inWord)
		{
			if constexpr (std::endian::native == std::endian::little)
				return inWord;
			else
				return Swap(inWord);
		}

	template<typename T>
	static constexpr T ToBigEndian(T inWord)
		{
			return FromBigEndian(inWord);
		}

	template<typename T>
	static constexpr T ToLittleEndian(T inWord)
		{
			return FromLittleEndian(inWord);
		}

	#define	UByteSex_FromBigEndian( inWord )	( UByteSex::FromBigEndian( inWord ) )
	#define	UByteSex_FromLittleEndian( inWord )	( UByteSex::FromLittleEndian( inWord ) )
	#define	UByteSex_ToBigEndian( inWord )		( UByteSex::ToBigEndian( inWord ) )
	#define	UByteSex_ToLittleEndian( inWord )	( UByteSex::ToLittleEndian( inWord ) )
};

#endif
		// _UBYTESEX_H

// ====================== //
// Disks travel in packs. //
// ====================== //
