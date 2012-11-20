// ==============================
// Fichier:			TDCLNSRef.h
// Projet:			Desktop Connection Library
//
// Créé le:			30/06/2002
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
// The Original Code is TDCLNSRef.h.
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
// $Id: TDCLNSRef.h,v 1.11 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSREF__
#define __TDCLNSREF__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>

class TDCLNSArray;
class TDCLNSBinary;
class TDCLNSEncoder;
class TDCLNSFrame;
class TDCLNSString;
class TDCLNSSymbol;
class TDCLStream;

///
/// Classe pour une référence sur un objet NewtonScript.
/// Une référence est un immédiat ou un pointeur.
/// Cette classe ne contient volontairement qu'un KUIntPtr.
/// Si le type ne correspond pas à une opération donnée,
/// on lance une exception.
///
/// Remarques générales sur l'utilisation des objets et des références.
///
/// NewtonScript possède un ramasse-miettes. Cette classe vise à remplacer le
/// ramasse-miettes tout en étant le plus facile d'utilisation possible. Il
/// faut respecter certaines règles pour à la fois éviter les fuites de mémoire
/// et les accès à des objets déjà supprimés et pour éviter que le code ne soit
/// trop lourd.
///
/// Aucun objet (pointeur) n'est copié implicitement. Pour copier un objet,
/// il faut utiliser les méthodes Clone, DeepClone et TotalClone qui ont la
/// même sémantique que sur le Newton.
///
/// - 1. Valeur de retour des fonctions
///
/// Une référence est normalement ou le plus souvent une référence sur un
/// objet qui n'est pas sur la pile. Les fonctions MakeArray, MakeFrame, etc.
/// créent de tels pointeurs, et le constructeur TDCLNSRef( TDCLNSObject* )
/// prend un pointeur.
///
/// C'est ce type de référence qu'il faut retourner. Une fonction ne doit
/// jamais retourner un objet directement (comme TDCLFrame, TDCLString, etc.).
/// À la rigueur, une fonction peut retourner un pointeur sur un objet,
/// l'appelant devant savoir qu'en faire.
///
/// - 2. Objets sur la pile
///
/// Pour des raisons d'efficacité, on peut vouloir créer des objets
/// NewtonScript comme des tableaux ou des structures sur la pile. Certaines
/// fonctions prennent directement des références vers des objets et donc ceci
/// ne pose aucun problème pour ces fonctions.
///
/// D'autres fonctions en revanche, notamment celles qui insèrent des
/// références dans des objets, prennent des références comme paramètre. Il y a
/// une bonne raison pour cela, c'est qu'il ne faut (généralement) pas leur
/// donner une référence sur un objet créé sur la pile.
///
/// C'est quand même possible en créant pour les appeler une référence sur ces
/// objets qui sont dans la pile. Ceci est possible grâce au constructeur
/// TDCLNSRef( TDCLNSObject& ). Ce constructeur doit être appelé partout où
/// s'assure que l'objet référence dure plus longtemps que son utilisation.
///
/// Néanmoins, comme l'objet référencé est dans la pile, la référence ne doit
/// pas lui survivre. (remarque: si DCLDebugOn est défini, une exception sera
/// lancée si cette référence survit à l'objet).
///
/// Voici un exemple de code foireux:
/// <code>
/// TDCLNSRef TBar::Foo( void )
/// {
/// 	TDCLFrame myFrame;
///		myFrame.Set("bar", TDCLNSRef::MakeInt(3));
///		return TDCLNSRef(myFrame);
/// } // <-- plantage
/// </code>
/// (c'est pourquoi le mot clé explicit est utilisé, afin d'empêcher
/// <code>return myFrame;</code> qui appellerait implicitement le constructeur)
///
/// Voici un autre exemple, plus subtil:
/// <code>
/// TDCLNSRef TBar::Foo( TDCLNSRef& inFrame )
/// {
/// 	TDCLArray myArray;
///		inFrame.ToFrame().Set("bar", TDCLNSRef(myArray));
/// } // <-- plantage
/// </code>
/// (remarque: <code>inFrame.ToFrame().Set("bar", myArray);</code> ne compile
/// pas non plus mais appellerait aussi implicitement le constructeur)
///
/// Dans ces deux exemples, il faut utiliser des références sur des objets
/// créés avec <code>new</code>.
/// Par exemple:
/// <code>
/// TDCLNSRef TBar::Foo( void )
/// {
///		TDCLNSRef myFrameRef = TDCLNSRef::MakeFrame();
/// 	TDCLFrame& myFrame = myFrameRef.ToFrame();
///		myFrame.Set("bar", TDCLNSRef::MakeInt(3));
///		return myFrameRef;
/// }
/// </code>
///
/// On peut aussi cloner l'objet si le renvoi de l'objet n'est pas sûr.
/// Par exemple:
/// <code>
/// TDCLNSRef TBar::Foo( TDCLNSFrame& inFrame )
/// {
/// 	TDCLArray myArray;
///		if (Unsure())
///		{
///			inFrame.Set("bar", TDCLNSRef(myArray.Clone()));
///		}
/// }
/// </code>
///
/// Les objets sur la pile doivent être créés dans le bon ordre.
/// <code>
/// {
/// 	TDCLArray myArray;
/// 	TDCLFrame myFrame;
///		myArray.Add( TDCLNSRef( myFrame ) );
/// } // <-- plantage
/// </code>
///
/// - 3. Paramètres des fonctions
///
/// Si le paramètre d'une fonction est de type TDCLNSRef, le constructeur par
/// copie de TDCLNSRef est appelé. Ceci introduit quelques opérations souvent,
/// si ce n'est toujours, inutiles.
/// Pour éviter ceci, il faut considérer deux autres types possibles pour les
/// paramètres:
/// 	- <code>const TDCLNSRef&</code> si la référence sera copiée au besoin
///		  dans le corps d'une fonction (normalement tout le temps le cas)
///		- <code>TDCLNSObject&</code> (e.g. <code>TDCLNSFrame&</code>) si
///		  l'objet ne sera pas référencé en dehors du corps de la fonction et
///		  que son type est connu -- dans ce cas, la fonction devra parfois
///		  construire une référence avec TDCLNSRef( TDCLNSObject& ).
///
/// - 4. Vérification du type & conversions
///
/// Si vous avez besoin d'effectuer plusieurs opération sur cet objet, il
/// est plus sage de stocker la référence dans une variable locale. En effet,
/// l'opérateur vérifie le type, et donc ne pas stocker va consister à vérifier
/// le type à chaque conversion.
/// Exemple:
/// <code>
///		TDCLNSFrame& theFrame = (TDCLNSFrame&) inFrameRef;
///		TDCLNSRef theResult = TDCLNSRef::kNILREF;
///		if (theFrame.HasSlot( (TDCLNSSymbol&) inSymbolRef ))
///		{
///			theFrame.Get( (TDCLNSSymbol&) inSymbolRef2 ))
///		}
///		return theResult;
/// </code>
///
/// peut être remplacé par:
///
/// <code>
///		TDCLNSFrame& theFrame = (TDCLNSFrame&) inFrameRef;
///		TDCLNSSymbol& theSymbol = (TDCLNSSymbol&) inSymbolRef;
///		TDCLNSRef theResult = TDCLNSRef::kNILREF;
///		if (theFrame.HasSlot( theSymbol ))
///		{
///			theFrame.Get( theSymbol ))
///		}
///		return theResult;
/// </code>
///
/// Remarques:
///		- <code>(TDCLNSFrame&) inFrameRef</code> est équivalent à l'appel
///		  à ToFrame()
///		- <code>(TDCLNSSymbol&) inSymbolRef2</code> est équivalent à juste
///		  inSymbolRef2 dans ce cas.
///
/// Enfin, le mot clé C++ explicit est utilisé pour vous empêcher de faire
/// l'erreur suivante:
/// <code>
///		TDCLNSFrame theFrame = inFrameRef.ToFrame();
/// </code>
///
/// Cette ligne *copie* la structure (enfin, elle ne compile pas, mais si elle
/// compilait, cela conduirait à une copie).
///
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.11 $
///
/// \test	aucun test défini.
///
class TDCLNSRef
{
	friend class TDCLNSEncoder;
	friend class TDCLNSOFEncoder;
	friend class TDCLTextEncoder;
	friend class TDCLXMLEncoder;
	friend class TDCLPkgEncoder;
	friend class TDCLNSDecoder;
	friend class TDCLNSOFDecoder;
	friend class TDCLTextDecoder;
	friend class TDCLXMLDecoder;

public:
	/// \name Constructeurs et destructeurs publics

	///
	/// Constructeur par défaut.
	/// Initialise la référence à kNILRefValue
	///
	TDCLNSRef( void );

	///
	/// Destructeur.
	/// Décrémente la référence s'il s'agit d'un pointeur.
	///
	~TDCLNSRef( void );
	
	///
	/// Constructeur par copie.
	/// Incrémente la référence s'il s'agit d'un pointeur.
	///
	/// \param inCopy		référence à copier.
	///
	TDCLNSRef( const TDCLNSRef& inCopy );

	///
	/// Constructeur à partir d'un objet sur la pile.
	/// Non seulement l'objet NS n'est pas copié, mais en plus une exception
	/// sera lancée si la référence continue d'exister après l'objet et si
	/// <code>DCLDebugOn</code> est définie.
	/// Attention, ceci peut ne pas être ce que vous souhaitez.
	///
	/// \param inObject		objet à référencer.
	///
	explicit TDCLNSRef( TDCLNSObject& inObject );

	///
	/// Constructeur à partir d'un objet.
	/// L'objet NS n'est pas copié.
	/// Attention, ceci peut ne pas être ce que vous souhaitez.
	///
	/// \param inObject		objet à référencer.
	///
	TDCLNSRef( TDCLNSObject* inObject );

	///
	/// Opérateur d'assignation.
	/// Décrémente la copie de l'ancienne référence s'il s'agit
	/// d'un pointeur et incrémente la nouvelle référence si
	/// nécessaire.
	///
	/// \param inCopy		référence à copier.
	///
	TDCLNSRef& operator = ( const TDCLNSRef& inCopy );

	///
	/// Opérateur d'assignation.
	/// Décrémente la copie de l'ancienne référence s'il s'agit
	/// d'un pointeur et incrémente la nouvelle référence.
	///
	/// \param inObject		objet à référencer.
	///
	TDCLNSRef& operator = ( TDCLNSObject& inObject );

	///
	/// Opérateur d'égalité.
	/// Ceci est l'égalité de surface. Deux TDCLRef sont
	/// égaux si leur valeurs (mRef) sont égales.
	/// Cependant, si ce sont tous deux des symboles, ils
	/// sont comparés. Ceci donne un opérateur sémantiquement
	/// équivalent à = en NewtonScript.
	///
	/// \param inAlter		référence à comparer.
	///
	Boolean operator == ( const TDCLNSRef& inAlter ) const;

	///
	/// Opérateur d'inégalité.
	/// La négation du précédent.
	///
	/// \param inAlter		référence à comparer.
	///
	inline Boolean operator != ( const TDCLNSRef& inAlter ) const
		{
			return !(*this == inAlter);
		}

	/// \name Fonctions sur la référence

	///
	/// Créateur à partir d'une référence.
	///
	/// \param inRef	référence.
	/// \return un nouvel objet TDCLNSRef avec cette référence.
	///
	static TDCLNSRef Ref( KUIntPtr inRef );

	///
	/// Récupère la valeur de la référence.
	///
	/// \return la valeur de la référence.
	///
	inline KUIntPtr RefOf( void ) const
		{
			return mRef;
		}

	/// \name Constantes

	///
	/// Constante pour NIL.
	///
	static const TDCLNSRef kNILREF;

	///
	/// Constante pour TRUE.
	///
	static const TDCLNSRef kTRUEREF;

	///
	/// Constante pour Ref(50), la classe des fonctions.
	///
	static const TDCLNSRef kNewFuncClass;

	/// \name Créateurs

	///
	/// Créateur à partir d'un entier.
	///
	/// \param inInt	entier (signé).
	/// \return un nouvel objet TDCLNSRef représentant cet entier.
	///	\throws	TDCLNSException si les deux bits de poids fort (30 & 29)
	///			ne sont pas nuls.
	///
	static TDCLNSRef MakeInt( KSInt32 inInt );

	///
	/// Créateur à partir d'un caractère.
	///
	/// \param inChar	caractère (Unicode).
	/// \return un nouvel objet TDCLNSRef représentant ce caractère.
	///
	static TDCLNSRef MakeChar( KUInt16 inChar );

	///
	/// Créateur à partir d'un caractère.
	///
	/// \param inChar	caractère (ISO-8859-1).
	/// \return un nouvel objet TDCLNSRef représentant ce caractère.
	///
	inline static TDCLNSRef MakeChar( unsigned char inChar )
		{
			return MakeChar( (KUInt16) inChar );
		}

	///
	/// Créateur à partir d'un booléen.
	///
	/// \param inBoolean	booléen.
	/// \return un nouvel objet TDCLNSRef représentant ce booléen.
	///
	static TDCLNSRef MakeBoolean( Boolean inBoolean );

	///
	/// Créateur pour un pointeur magique.
	///
	/// \param inValue	valeur du pointeur magique (indice et table)
	/// \return un nouvel objet TDCLNSRef représentant ce pointeur magique.
	///	\throws	TDCLNSException si l'indice ou le numéro de la table sont trop
	///			grands.
	///
	static TDCLNSRef MakeMagicPtr( KUInt32 inValue );

	///
	/// Créateur pour un pointeur magique.
	///
	/// \param inTable	table pour le pointeur magique.
	/// \param inIndex	indice du pointeur magique.
	/// \return un nouvel objet TDCLNSRef représentant ce pointeur magique.
	///	\throws	TDCLNSException si l'indice ou le numéro de la table sont trop
	///			grands.
	///
	static TDCLNSRef MakeMagicPtr( KUInt32 inTable, KUInt32 inIndex );

	///
	/// Créateur pour une structure vide.
	///
	/// \return un nouvel objet TDCLNSRef représentant une nouvelle structure.
	///
	static TDCLNSRef MakeFrame( void );

	///
	/// Créateur pour une fonction vide.
	///
	/// \return un nouvel objet TDCLNSRef représentant une nouvelle fonction.
	///
	static TDCLNSRef MakeFunction( void );

	///
	/// Créateur pour un tableau.
	///
	/// \param inSize	taille du tableau
	/// \param inRef	valeur initiale pour les éléments du tableau
	/// \return un nouvel objet TDCLNSRef représentant un nouveau tableau.
	///
	static TDCLNSRef MakeArray(
						KUInt32 inSize = 0,
						const TDCLNSRef& inRef = TDCLNSRef::kNILREF );

	///
	/// Créateur pour un symbole à partir d'une chaîne ISO-8859-1.
	///
	/// \param inString		chaîne pour ce symbole.
	/// \return un nouvel objet TDCLNSRef représentant un nouveau symbole.
	///
	static TDCLNSRef MakeSymbol( const char* inString );

	///
	/// Créateur pour un symbole à partir d'une chaîne UCS-2.
	///
	/// \param inString		chaîne pour ce symbole.
	/// \return un nouvel objet TDCLNSRef représentant un nouveau symbole.
	///
	static TDCLNSRef MakeSymbol( const KUInt16* inString );

	///
	/// Créateur pour une chaîne à partir d'une chaîne ISO-8859-1.
	///
	/// \param inString		chaîne considérée.
	/// \return un nouvel objet TDCLNSRef représentant une nouvelle chaîne.
	///
	static TDCLNSRef MakeString( const char* inString );

	///
	/// Créateur pour une chaîne à partir d'une chaîne UCS-2.
	///
	/// \param inString		chaîne considérée (terminée par un nul).
	/// \return un nouvel objet TDCLNSRef représentant une nouvelle chaîne.
	///
	static TDCLNSRef MakeString( const KUInt16* inString );

	///
	/// Créateur pour une chaîne à partir d'une suite de caractères UCS-2.
	///
	/// \param inString		suite de caractères considérée.
	/// \param inSize		nombre de caractères.
	/// \return un nouvel objet TDCLNSRef représentant une nouvelle chaîne.
	///
	static TDCLNSRef MakeString( const KUInt16* inString, KUInt32 inSize );

	///
	/// Créateur pour un binaire à partir d'un pointeur et d'une taille.
	/// La classe est la classe par défaut.
	///
	/// \param inData		données pour le binaire.
	/// \param inSize		taille des données pour le binaire.
	/// \return un nouvel objet TDCLNSRef représentant un nouveau binaire.
	///
	static TDCLNSRef MakeBinary(
							const void* inData,
							KUInt32 inSize );

	///
	/// Créateur pour un binaire à partir d'un pointeur, d'une taille
	/// et d'une classe.
	///
	/// \param inData		données pour le binaire.
	/// \param inSize		taille des données pour le binaire.
	/// \param inClass		classe du binaire.
	/// \return un nouvel objet TDCLNSRef représentant un nouveau binaire.
	///
	static TDCLNSRef MakeBinary(
							const void* inData,
							KUInt32 inSize,
							const TDCLNSRef& inClass );

	/// \name Tests sur le type

	///
	/// Détermine si la référence est un entier.
	///
	/// \return \c true si c'est un entier, \c false sinon.
	///
	inline Boolean IsInt( void ) const
		{
			return (mRef & kTagMask) == kIntegerTag;
		}

	///
	/// Détermine si la référence est un caractère.
	///
	/// \return \c true si c'est un caractère, \c false sinon.
	///
	inline Boolean IsChar( void ) const
		{
			return (mRef & (kTagMask | kImmedBitsMask)) == (kImmedTag | kImmedChar);
		}

	///
	/// Détermine si la référence est un booléen (un des deux booléens).
	///
	/// \return \c true si c'est un booléen, \c false sinon.
	///
	inline Boolean IsBoolean( void ) const
		{
			return (mRef == kNILRefValue) || (mRef == kTRUERefValue);
		}

	///
	/// Détermine si la référence est NIL.
	///
	/// \return \c true si c'est NIL, \c false sinon.
	///
	inline Boolean IsNIL( void ) const
		{
			return (mRef == kNILRefValue);
		}

	///
	/// Détermine si la référence est TRUE.
	///
	/// \return \c true si c'est TRUE, \c false sinon.
	///
	inline Boolean IsTRUE( void ) const
		{
			return (mRef == kTRUERefValue);
		}

	///
	/// Détermine si la référence est un pointeur (magique ou non).
	///
	/// \return \c true si c'est un pointeur, \c false sinon.
	///
	inline Boolean IsPtr( void ) const
		{
			return IsRealPtr() || IsMagicPtr();
		}

	///
	/// Détermine si la référence est un pointeur magique.
	///
	/// \return \c true si c'est un pointeur magique, \c false sinon.
	///
	inline Boolean IsMagicPtr( void ) const
		{
			return (mRef & kTagMask) == kMagicPtrTag;
		}

	///
	/// Détermine si la référence est un pointeur normal.
	///
	/// \return \c true si c'est un pointeur normal, \c false sinon.
	///
	inline Boolean IsRealPtr( void ) const
		{
			return (mRef & kTagMask) == kPointerTag;
		}

	///
	/// Détermine si la référence est un symbole.
	///
	/// \return \c true si c'est un symbole, \c false sinon.
	///
	inline Boolean IsSymbol( void ) const
		{
			return IsRealPtr() && GetPointer()->IsSymbol();
		}

	///
	/// Détermine si la référence est une structure.
	///
	/// \return \c true si c'est une structure, \c false sinon.
	///
	inline Boolean IsFrame( void ) const
		{
			return IsRealPtr() && GetPointer()->IsFrame();
		}

	///
	/// Détermine si la référence est un tableau.
	///
	/// \return \c true si c'est un tableau, \c false sinon.
	///
	inline Boolean IsArray( void ) const
		{
			return IsRealPtr() && GetPointer()->IsArray();
		}

	///
	/// Détermine si la référence est un binaire.
	///
	/// \return \c true si c'est un binaire, \c false sinon.
	///
	inline Boolean IsBinary( void ) const
		{
			return IsRealPtr() && GetPointer()->IsBinary();
		}

	///
	/// Détermine si la référence est une chaîne.
	///
	/// \return \c true si c'est une chaîne, \c false sinon.
	///
	inline Boolean IsString( void ) const
		{
			return IsRealPtr() && GetPointer()->IsString();
		}

	/// \name Conversion des immédiats

	///
	/// Récupère l'entier représenté par cette référence.
	///
	/// \return l'entier représenté par cette référence.
	///	\throws	TDCLException si cette référence n'est pas un entier.
	///
	KSInt32 ToInt( void ) const;

	///
	/// Récupère le caractère représenté par cette référence.
	///
	/// \return le caractère représenté par cette référence (en unicode).
	///	\throws	TDCLException si cette référence n'est pas un caractère.
	///
	KUInt16 ToChar( void ) const;

	///
	/// Récupère le pointeur magique représenté par cette référence.
	///
	/// \return le pointeur magique représenté par cette référence.
	///	\throws	TDCLException si cette référence n'est pas un pointeur magique.
	///
	KUInt32 ToMagicPtr( void ) const;

	///
	/// Récupère l'indice du pointeur magique représenté par cette référence.
	///
	/// \return l'indice du pointeur magique.
	///	\throws	TDCLException si cette référence n'est pas un pointeur magique.
	///
	KUInt32 GetMagicPtrIndex( void ) const;

	///
	/// Récupère le numéro de la table du pointeur magique représenté par cette
	/// référence.
	///
	/// \return le numéro de la table du pointeur magique.
	///	\throws	TDCLException si cette référence n'est pas un pointeur magique.
	///
	KUInt32 GetMagicPtrTable( void ) const;

	/// \name Fonctions générales

	///
	/// Copie l'objet référencé (s'il s'agit d'un pointeur).
	///
	/// \return une référence pointant vers une copie de l'objet.
	///
	TDCLNSRef Clone( void ) const;

	///
	/// Copie l'objet référencé en profondeur (s'il s'agit
	/// d'un pointeur).
	///
	/// \return une référence pointant vers une copie de l'objet.
	///
	TDCLNSRef DeepClone( void ) const;

	///
	/// Copie l'objet référencé en profondeur (s'il s'agit
	/// d'un pointeur).
	///
	/// \return une référence pointant vers une copie de l'objet.
	///
	TDCLNSRef TotalClone( void ) const;

	/// \name Fonctions sur la classe

	///
	/// Détermine la classe cet objet.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de cet objet.
	///
	TDCLNSRef ClassOf( void ) const;

	///
	/// Détermine la classe primaire de cet objet.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de cet objet.
	///
	TDCLNSRef PrimClassOf( void ) const;

	///
	/// Change la classe de cet objet.
	/// Ceci n'est pas possible pour les immédiats (lance
	/// kNSErrObjectPointerOfNonPtr) ou pour les symboles (cf TDCLNSSymbol.h)
	///
	/// \param	inClass un objet qui sert de nouvelle classe à cet objet.
	/// \return *this
	/// \throws TDCLNSException si une erreur est survenue.
	///
	TDCLNSRef SetClass( const TDCLNSRef& inClass );

	///
	/// Change la classe de cet objet.
	/// Ceci n'est pas possible pour les immédiats (lance
	/// kNSErrObjectPointerOfNonPtr) ou pour les symboles (cf TDCLNSSymbol.h)
	///
	/// \param	inClass la nouvelle classe de l'objet
	/// \return *this
	/// \throws TDCLNSException si une erreur est survenue.
	///
	inline TDCLNSRef SetClass( const char* inClass )
		{
			return SetClass( MakeSymbol( inClass ) );
		}

	/// \name Fonctions sur les structures & les éléments

	/// \name Conversion vers TDCLNSObject&

	///
	/// Conversion vers TDCLNSSymbol&
	///
	/// \return une référence vers le TDCLNSSymbol pointé par cet objet.
	/// \throw TDCLException si l'objet n'est pas un symbole.
	/// 
	inline operator TDCLNSSymbol& ( void ) const
		{
			return ToSymbol();
		}
	
	///
	/// Conversion vers TDCLNSSymbol&
	///
	/// \return une référence vers le TDCLNSSymbol pointé par cet objet.
	/// \throw TDCLException si l'objet n'est pas un symbole.
	/// 
	TDCLNSSymbol& ToSymbol( void ) const;
	
	///
	/// Conversion vers TDCLNSFrame&
	///
	/// \return une référence vers la TDCLNSFrame pointée par cet objet.
	/// \throw TDCLException si l'objet n'est pas une structure.
	/// 
	inline operator TDCLNSFrame& ( void ) const
		{
			return ToFrame();
		}
	
	///
	/// Conversion vers TDCLNSFrame&
	///
	/// \return une référence vers la TDCLNSFrame pointée par cet objet.
	/// \throw TDCLException si l'objet n'est pas une structure.
	/// 
	TDCLNSFrame& ToFrame( void ) const;
	
	///
	/// Conversion vers TDCLNSArray&
	///
	/// \return une référence vers le TDCLNSArray pointé par cet objet.
	/// \throw TDCLException si l'objet n'est pas un tableau.
	/// 
	inline operator TDCLNSArray& ( void ) const
		{
			return ToArray();
		}
	
	///
	/// Conversion vers TDCLNSArray&
	///
	/// \return une référence vers le TDCLNSArray pointé par cet objet.
	/// \throw TDCLException si l'objet n'est pas un tableau.
	/// 
	TDCLNSArray& ToArray( void ) const;
	
	///
	/// Conversion vers TDCLNSBinary&
	///
	/// \return une référence vers la TDCLNSBinary pointée par cet objet.
	/// \throw TDCLException si l'objet n'est pas une chaîne.
	/// 
	inline operator TDCLNSBinary& ( void ) const
		{
			return ToBinary();
		}
	
	///
	/// Conversion vers TDCLNSBinary&
	///
	/// \return une référence vers la TDCLNSBinary pointée par cet objet.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	/// 
	TDCLNSBinary& ToBinary( void ) const;
	
	///
	/// Conversion vers TDCLNSString&
	///
	/// \return une référence vers la TDCLNSString pointée par cet objet.
	/// \throw TDCLException si l'objet n'est pas une chaîne.
	/// 
	inline operator TDCLNSString& ( void ) const
		{
			return ToString();
		}
	
	///
	/// Conversion vers TDCLNSString&
	///
	/// \return une référence vers la TDCLNSString pointée par cet objet.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	/// 
	TDCLNSString& ToString( void ) const;
	
	///
	/// Opérateur de concaténation.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator & ( const TDCLNSRef& inAlter ) const;
	
	///
	/// Opérateur de concaténation.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator & ( const char* inAlter ) const;
	
	///
	/// Opérateur de concaténation.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator & ( const KUInt16* inAlter ) const;
	
	///
	/// Opérateur de concaténation avec une espace.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator && ( const TDCLNSRef& inAlter ) const;

	///
	/// Opérateur de concaténation avec une espace.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator && ( const char* inAlter ) const;

	///
	/// Opérateur de concaténation avec une espace.
	///
	/// \param inAlter		chaîne à concaténer avec cette chaîne.
	/// \throw TDCLNSException si l'objet n'est pas une chaîne.
	///
	TDCLNSRef operator && ( const KUInt16* inAlter ) const;

private:
	/// \name Constructeur privé

	///
	/// Constructeur à partir d'une référence.
	/// Incrémente la référence s'il s'agit d'un pointeur.
	///
	TDCLNSRef( KUIntPtr inRef );

	/// \name Gestion du pointeur

	///
	/// Récupère le pointeur sur l'objet.
	///
	inline TDCLNSObject* GetPointer( void ) const
		{
			return (TDCLNSObject*) (mRef - kPointerTag);
		}

	/// \name Interface Entrée/Sortie

	///
	/// Crée une représentation XML de la référence.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	void ToXML( TDCLXMLEncoder* inEncoder ) const;

	///
	/// Crée une représentation NSOF de la référence.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	void ToNSOF( TDCLNSOFEncoder* inEncoder ) const;

	///
	/// Crée une représentation sous forme de texte de la référence.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	void ToText( TDCLTextEncoder* inEncoder ) const;

	///
	/// Crée une représentation pour un paquet de la référence.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	void ToPkg( TDCLPkgEncoder* inEncoder ) const;

	/// \name Référence
	KUIntPtr	mRef;
	
	///
	/// Constantes pour la référence.
	///
	enum {
		kTagShift		= 0x2,
		kTagMask		= 0x3,		// 0b11
		kIntegerTag		= 0x0,		// 0b00
		kPointerTag		= 0x1,		// 0b01
		kImmedTag		= 0x2,		// 0b10
		kMagicPtrTag	= 0x3,		// 0b11

		kImmedBitsShift	= 0x4,
		kImmedBitsMask	= 0xC,		// 0b1100
		kImmedSpecial	= 0x0,		// 0b0000
		kImmedChar		= 0x4,		// 0b0100
		kImmedBoolean	= 0x8,		// 0b1000
		kImmedReserved	= 0xC,		// 0b1100
		
		kNILRefValue	= 0x2,		// Immed, Special, 0
		kTRUERefValue	= 0x1A,		// Immed, Boolean, 1

		kMagicPtrTShift	= 12,		// En plus des 2 bits pour le type.
		kMagicPtrIMask	= 0xFFF		// 0b1111 1111 1111
	};
};

#endif
		// __TDCLNSREF__

// ============================================================================= //
// I *____knew* I had some reason for not logging you off... If I could just //
// remember what it was.                                                         //
// ============================================================================= //
