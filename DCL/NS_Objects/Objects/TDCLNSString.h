// ==============================
// Fichier:			TDCLNSString.h
// Projet:			Desktop Connection Library
//
// Créé le:			21/10/2002
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
// The Original Code is TDCLNSString.h.
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
// $Id: TDCLNSString.h,v 1.5 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSSTRING__
#define __TDCLNSSTRING__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSBinary.h>

// K
#include <K/Unicode/UUTF16CStr.h>

class TDCLStream;

///
/// Classe pour les chaînes NewtonScript.
/// Ces chaînes sont stockées au format UCS-2.
///
/// Cette classe est un peu particulière. Elle n'a aucun destructeur
/// virtuel ou membre parce qu'en fait, les objets de cette classe doivent
/// correspondre exactement à ceux de \c TDCLNSBinary qui sont des chaînes, vu
/// que seule la classe différencient les chaînes parmi les binaires.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	UTestNS::CatenateStrings
///
class TDCLNSString
	:
		public TDCLNSBinary	
{
public:
	///
	/// Constructeur à partir d'une chaîne ISO 8859-1.
	///
	/// \param inString		chaîne de caractères considérée.
	///
	TDCLNSString( const char* inString );

	///
	/// Constructeur à partir d'une chaîne UCS-2.
	///
	/// \param inString		chaîne de caractères considérée.
	///
	TDCLNSString( const KUInt16* inString );

	///
	/// Créateur pour une chaîne à partir d'une suite de caractères UCS-2.
	/// \param inString		suite de caractères considérée.
	/// \param inSize		nombre de caractères.
	///
	TDCLNSString( const KUInt16* inString, KUInt32 inSize );

	///
	/// Constructeur à partir d'un flux et d'une taille.
	///
	/// \param inStream		flux où lire les données.
	/// \param inLength		nombre d'octets à lire
	///						(2 * taille de la chaîne avec terminateur)
	///
	TDCLNSString(
			TDCLStream* inStream,
			KUInt32 inLength );

	///
	/// Crée un objet à partir d'un flux NSOF
	///
	/// \param inDecoder	décodeur NSOF associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSString*	FromNSOF( TDCLNSOFDecoder* inDecoder );

	///
	/// Compare cette chaîne avec une autre chaîne ISO 8859-1.
	///
	/// \return \c true si les deux chaînes sont égales à la casse près.
	///
	Boolean StrEqual( const char* inAlter ) const;

	///
	/// Compare cette chaîne avec une autre chaîne unicode.
	///
	/// \return \c true si les deux chaînes sont égales à la casse près.
	///
	Boolean StrEqual( const KUInt16* inAlter ) const;

	///
	/// Compare cette chaîne avec une autre chaîne.
	///
	/// \return \c true si les deux chaînes sont égales à la casse près.
	///
	Boolean StrEqual( const TDCLNSString& inAlter ) const;

	///
	/// Accesseur sur la chaîne.
	/// Valide tant que la chaîne n'est pas redimensionnée.
	///
	/// \return la chaîne en unicode.
	///
	inline KUInt16* GetString( void )
		{
			return (KUInt16*) GetPointer();
		}

	///
	/// Accesseur sur la chaîne.
	/// Comme le précédent mais en lecture seule.
	///
	/// \return la chaîne en unicode.
	///
	inline const KUInt16* GetString( void ) const
		{
			return (const KUInt16*) GetPointer();
		}

	///
	/// Accesseur sur la taille de la chaîne.
	/// Comme StrLen sur Newton.
	///
	/// \return la taille de la chaîne.
	///
	inline KUInt32 GetStrLen( void ) const
		{
			return UUTF16CStr::StrLen( GetString() );
		}

	///
	/// Concaténation.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter.
	///
	inline TDCLNSString* Catenate( const TDCLNSString& inAlter ) const
		{
			return Catenate( inAlter.GetString() );
		}

	///
	/// Concaténation.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter.
	///
	TDCLNSString* Catenate( const char* inAlter ) const;

	///
	/// Concaténation.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter.
	///
	TDCLNSString* Catenate( const KUInt16* inAlter ) const;

	///
	/// Concaténation avec espace.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter, les deux chaînes étant séparées par une espace.
	///
	inline TDCLNSString* CatenateWithSpace( const TDCLNSString& inAlter ) const
		{
			return CatenateWithSpace( inAlter.GetString() );
		}

	///
	/// Concaténation avec espace.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter, les deux chaînes étant séparées par une espace.
	///
	TDCLNSString* CatenateWithSpace( const char* inAlter ) const;

	///
	/// Concaténation avec espace.
	///
	/// \param inAlter	chaîne avec laquelle concaténer cette chaîne.
	/// \return	une nouvelle chaîne composée de cette chaîne et de la chaîne
	///			inAlter, les deux chaînes étant séparées par une espace.
	///
	TDCLNSString* CatenateWithSpace( const KUInt16* inAlter ) const;

private:
	///
	/// Constructeur à partir d'une chaîne UCS-2 et d'une taille.
	/// Ce constructeur ne sert que pour la concaténation.
	///
	TDCLNSString( KUInt16* inBuffer, KUInt32 inData );

	///
	/// Constructeur par défaut volontairement indisponible.
	///
	TDCLNSString( void );
	
	///
	/// Espace en UTF-16.
	///
	static const KUInt16 kSpaceChar;
};

#endif
		// __TDCLNSSTRING__

// ========================================================================= //
// Think of your family tonight.  Try to crawl home after the computer       //
// crashes.                                                                  //
// ========================================================================= //
