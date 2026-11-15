// ==============================
// Fichier:			TDCLNSSymbol.h
// Projet:			Desktop Connection Library
//
// Créé le:			06/08/2002
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
// The Original Code is TDCLNSSymbol.h.
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
// $Id: TDCLNSSymbol.h,v 1.7 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSSYMBOL__
#define __TDCLNSSYMBOL__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>

class TDCLNSOFDecoder;
class TDCLPkgDecoder;

///
/// Classe pour les symboles NewtonScript.
///
/// Le point sur les symboles 8 bits.
/// ---------------------------------
///
/// Les symboles ne contiennent normalement que des caractères entre 32 et 127.
/// Cependant, NTK et WinNTK peuvent, dans certaines circonstances, créer des
/// symboles 8 bits. Chacun calcule la valeur de hachage en mettant la chaîne
/// en majuscule, mais chacun le fait dans l'encodage du système
/// (Windows, MacRoman). Le Newton en revanche supprime froidement les accents
/// lorsqu'on tente de créer des symboles 8 bits.
///
/// Je suppose qu'un symbole 8 bits ne pourra être déclaré comme égal à un autre
/// symbole 8 bits que si ce symbole est construit à la main ou a été créé par
/// le même programme (WinNTK ou NTK) afin que la chaîne soit la même et que la
/// valeur de hachage soit aussi la même.
///
/// Pour la DCL, les choses se passent ainsi:
/// - comme sur Newton et NTK/WinNTK, les tests d'ordre et d'égalité commencent
///   par la valeur de hachage.
/// - les constructeurs normaux traduisent les chaînes avec des séquences \\u.
/// - il y a un constructeur avec une chaîne 8 bits et une valeur de hachage.
/// - les symboles dans les paquets sont lus avec la valeur de hachage du
///   paquet.
/// - la chaîne est considérée comme théoriquement sur 7 bits mais pour
///   l'export XML, elle est traitée comme une chaîne Latin1.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLNSSymbol
	:
		public TDCLNSObject
{
public:
	///
	/// Constructeur à partir d'une chaîne de caractère ISO-8859-1
	///
	/// La chaîne est convertie en ASCII 32/127 (avec des slash-u-code-slash-u).
	///
	/// \param inString		chaîne de caractère pour ce symbole.
	///
	TDCLNSSymbol( const char* inString );

	///
	/// Constructeur à partir d'une chaîne ASCII et d'une valeur de hachage.
	///
	/// \param inString		chaîne de caractère pour ce symbole.
	/// \param inHashCode	valeur de hachage pour la chaîne.
	///
	TDCLNSSymbol( const char* inString, KUInt32 inHashCode );

	///
	/// Constructeur à partir d'une chaîne de caractère UCS-2.
	///
	/// La chaîne est convertie en ASCII 32/127 (avec des slash-u-code-slash-u).
	///
	/// \param inString 	chaîne de caractère pour ce symbole.
	///
	TDCLNSSymbol( const KUInt16* inString );

	///
	/// Constructeur par copie.
	/// La chaîne est copiée.
	///
	/// \param inCopy 		objet à copier
	///
	TDCLNSSymbol( const TDCLNSSymbol& inCopy );

	///
	/// Opérateur d'assignation.
	/// La chaîne est copiée.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSSymbol& operator = ( const TDCLNSSymbol& inCopy );

	///
	/// Destructeur.
	///
	virtual ~TDCLNSSymbol( void );

	///
	/// Détermine si cet objet est un symbole.
	///
	/// \return \c false
	///
	virtual Boolean IsSymbol( void ) const;

	///
	/// Accesseur sur la valeur de hachage.
	///
	/// \return 	la valeur de hachage.
	///
	inline KUInt32 GetHashCode( void ) const
		{
			return mHashCode;
		}

	///
	/// Accesseur sur la chaîne
	///
	/// \return 	la chaîne (encodée en ASCII 32/127), terminée par un nul.
	///
	inline const char* GetString( void ) const
		{
			return mString;
		}
	
	///
	/// Opérateur pour faire de même.
	///
	/// \return		la chaîne (encodée en ASCII 32/127), terminée par un nul.
	///
	inline operator const char*() const
		{
			return mString;
		}

	///
	/// Compare \c this à un autre symbole.
	///
	/// La comparaison fonctionne ainsi:
	/// - d'abord les valeurs de hachage sont comparées.
	/// - ensuite, si elles sont égales, les chaînes sont comparées selon
	///   l'ordre lexicographique.
	///
	/// \param	inAlter		symbole avec lequel se comparer
	/// \return	un entier:
	///			- 0 si les deux symboles sont égaux.
	///			- un entier négatif si \c this est plus petit que \p inAlter
	///			- un entier positif si \c this est plus grand que \p inAlter
	int Compare( const TDCLNSSymbol& inAlter ) const;

	///
	/// Crée une copie de surface du symbole.
	/// Appelle le constructeur par copie.
	///
	/// \return une copie de surface de this.
	///
	virtual TDCLNSObject*	Clone( void ) const;

	///
	/// Crée un objet à partir d'un flux NSOF
	///
	/// \param inDecoder	décodeur NSOF associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSSymbol*	FromNSOF( TDCLNSOFDecoder* inDecoder );

	///
	/// Crée un objet à partir d'un paquet.
	///
	/// \param inDecoder	décodeur Pkg associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSSymbol*	FromPkg( TDCLPkgDecoder* inDecoder );

	///
	/// Détermine la classe de ce symbole.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de ce symbole.
	///
	virtual TDCLNSRef ClassOf( void ) const;

	///
	/// Détermine la classe primaire de ce symbole.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de ce symbole.
	///
	virtual TDCLNSRef PrimClassOf( void ) const;

	///
	/// Détermine si le symbole this est une sous-classe d'un symbole donné.
	/// I.e. s'il est de la forme super.foo.
	/// Remarque: si le paramètre est égal à this, on retourne aussi \c true.
	///
	/// \return une référence vers la classe primaire de ce symbole.
	///
	Boolean IsSubClass( const TDCLNSSymbol& inSuper ) const;

	///
	/// Change la classe de ce symbole.
	/// Ceci n'est pas possible avec la DCL (mais ça l'est avec NewtonOS).
	/// Si on change la classe d'un symbole sous NewtonOS, l'objet est
	/// transformé en un binaire.
	/// Les symboles dans la DCL sont en lecture seule.
	///
	/// \param	inClass un objet qui sert de nouvelle classe à ce binaire.
	/// \throws TDCLNSException pour indiquer que l'objet est en lecture
	///			seule.
	///
	virtual void SetClass( const TDCLNSRef& inClass );

	///
	/// Calcule la valeur de hachage d'une chaîne.
	/// La chaîne doit être composée de caractères entre 32 et 127.
	///
	/// \param	inString	chaîne à hacher.
	/// \return	valeur de hachage comme spécifiée dans Newton Formats.
	///
	static KUInt32	HashFunction( const char* inString );

private:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kHashMagic = 0x9E3779B9		///< Et oui, le hash, c'est magique.
	};

	///
	/// Constructeur à partir d'un flux et d'un nombre d'octets.
	/// La chaîne est lue telle quelle.
	///
	/// \param inStream		flux
	/// \param inNbChars	nombre d'octets à lire (sans le zéro final)
	///
	TDCLNSSymbol( TDCLStream* inStream, KUInt32 inNbChars );

	///
	/// Constructeur à partir d'un flux et d'un nombre d'octets et d'une valeur
	/// de hachage.
	/// La chaîne est lue telle quelle.
	///
	/// \param inStream		flux
	/// \param inNbChars	nombre d'octets à lire (sans le zéro final)
	/// \param inHashCode	valeur de hachage.
	///
	TDCLNSSymbol(
			TDCLStream* inStream,
			KUInt32 inNbChars,
			KUInt32 inHashCode );

	///
	/// Convertit une chaîne ISO-8859-1 en ASCII 32/127
	///
	/// \param	inString	chaîne à convertir.
	/// \return la nouvelle chaîne, allouée avec malloc.
	///
	static char*	ToASCII32_127( const char* inString );

	///
	/// Convertit une chaîne UCS-2 en ASCII 32/127
	///
	/// \param	inString	chaîne à convertir.
	/// \return la nouvelle chaîne, allouée avec malloc.
	///
	static char*	ToASCII32_127( const KUInt16* inString );

	/// \name Interface IStreamable
	
	///
	/// Crée une représentation XML du symbole.
	/// Comprend la balise de début, les données et la balise de fin.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	/// \param inObjectID		ID de l'objet dans ce flux.
	///
	virtual void ToXML(
				TDCLXMLEncoder* inEncoder,
				KUInt32			inObjectID ) const;

	///
	/// Crée une représentation NSOF du symbole.
	/// Comprend le type et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToNSOF(
				TDCLNSOFEncoder* inEncoder ) const;

	///
	/// Crée une représentation sous forme de texte du symbole.
	/// Comprend les crochets et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToText(
				TDCLTextEncoder* inEncoder ) const;

	///
	/// Crée une représentation du symbole à mettre dans un
	/// paquet.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder	encodeur pour le paquet.
	/// \param ioOffset		en entrée et en sortie, décalage par rapport
	///						au paquet.
	///
	virtual void ToPkg( TDCLPkgEncoder* inEncoder, KUInt32* ioOffset ) const;

	/// \name Variables Privées
		
	const char*				mString;	///< Chaîne de caractère pour ce symbole.
	const KUInt32			mHashCode;	///< Valeur de hachage (doit être
										///< après!)
};

#endif
		// __TDCLNSSYMBOL__

// ============================================================================== //
//         The wise programmer is told about the Tao and follows it.  The average //
// programmer is told about the Tao and searches for it.  The foolish programmer  //
// is told about the Tao and laughs at it.  If it were not for laughter, there    //
// would be no Tao.                                                               //
//         The highest sounds are the hardest to hear.  Going forward is a way to //
// retreat.  Greater talent shows itself late in life.  Even a perfect program    //
// still has bugs.                                                                //
//                 -- Geoffrey James, "The Tao of Programming"                    //
// ============================================================================== //
