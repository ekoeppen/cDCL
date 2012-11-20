// ==============================
// Fichier:			TDCLNSBinary.h
// Projet:			Desktop Connection Library
//
// Créé le:			07/08/2002
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
// The Original Code is TDCLNSBinary.h.
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
// $Id: TDCLNSBinary.h,v 1.5 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __TDCLNSBINARY__
#define __TDCLNSBINARY__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>

class TDCLStream;
class TDCLNSEncoder;
class TDCLNSOFDecoder;
class TDCLPkgDecoder;

///
/// Classe pour les binaires NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLNSBinary
	:
		public TDCLNSObject
{
public:
	///
	/// Constructeur à partir d'une classe et d'une taille.
	/// Remarque: inClass est normalement un symbole. Mais comme sur le
	/// Newton, on peut prendre n'importe quel objet.
	///
	TDCLNSBinary(
			const TDCLNSRef& inClass = KDCLSYM::kSYMbinary,
			KUInt32 inSize = 0 );

	///
	/// Constructeur par copie.
	/// Les données sont dupliquées.
	///
	/// \param inCopy	objet à copier.
	///
	explicit TDCLNSBinary( const TDCLNSBinary& inCopy );

	///
	/// Constructeur à partir d'un pointeur et d'une taille.
	///
	/// \param inData		données pour le binaire (copiées).
	/// \param inLength		taille, en octets, du binaire.
	/// \param inClass		classe du binaire.
	///
	TDCLNSBinary(
			const void* inData,
			KUInt32 inLength,
			const TDCLNSRef& inClass = KDCLSYM::kSYMbinary );

	///
	/// Constructeur à partir d'un flux et d'une taille.
	///
	/// \param inStream		flux où lire les données.
	/// \param inLength		nombre d'octets à lire
	/// \param inClass		classe du binaire.
	///
	TDCLNSBinary(
			TDCLStream* inStream,
			KUInt32 inLength,
			const TDCLNSRef& inClass = KDCLSYM::kSYMbinary );

	///
	/// Destructeur.
	/// Libère la mémoire tampon.
	///
	virtual ~TDCLNSBinary( void );

	///
	/// Opérateur d'assignation.
	///
	/// \param inCopy		objet à copier
	///
	TDCLNSBinary& operator = ( const TDCLNSBinary& inCopy );

	///
	/// Détermine si cet objet est un binaire.
	///
	/// \return \c true
	///
	virtual Boolean IsBinary( void ) const;

	///
	/// Détermine si cet objet est une chaîne.
	///
	/// \return \c true si la classe correspond.
	///
	virtual Boolean IsString( void ) const;

	///
	/// Accesseur sur la taille du binaire.
	///
	/// \return la taille du binaire
	///
	inline KUInt32	GetLength( void ) const
		{
			return mBufferSize;
		}
	
	///
	/// Redimensionne le binaire.
	///
	/// \param inLength		nouvelle taille du binaire
	///
	void	SetLength( KUInt32 inLength );
	
	///
	/// Accesseur sur les données du binaire.
	///
	/// \return le pointeur sur le binaire (valide tant que le binaire
	///		n'est pas redimensionné)
	///
	inline void*	GetPointer( void )
		{
			return mBuffer;
		}

	///
	/// Accesseur sur les données du binaire, en lecture seule.
	///
	/// \return le pointeur sur le binaire (valide tant que le binaire
	///		n'est pas redimensionné)
	///
	inline const void*	GetPointer( void ) const
		{
			return mBuffer;
		}
	
	///
	/// Crée une copie de surface de la chaîne.
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
	static	TDCLNSBinary*	FromNSOF( TDCLNSOFDecoder* inDecoder );

	///
	/// Crée un objet à partir d'un flux NSOF (en tant que binaire large)
	///
	/// \param inDecoder	décodeur NSOF associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSBinary*	FromNSOFAsLargeBinary( TDCLNSOFDecoder* inDecoder );

	///
	/// Crée un objet à partir d'un paquet.
	///
	/// \param inDecoder	décodeur Pkg associé à un flux donné.
	/// \return un nouvel objet à partir de ce flux
	///
	static	TDCLNSBinary*	FromPkg( TDCLPkgDecoder* inDecoder );

	///
	/// Détermine la classe de ce binaire.
	/// Ce n'est pas forcément un symbole.
	///
	/// \return une référence vers la classe de ce binaire.
	///
	virtual TDCLNSRef ClassOf( void ) const;

	///
	/// Détermine la classe primaire de ce binaire.
	/// Il s'agit forcément d'un symbole.
	///
	/// \return une référence vers la classe primaire de ce binaire.
	///
	virtual TDCLNSRef PrimClassOf( void ) const;

	///
	/// Change la classe de ce binaire.
	/// Ce n'est pas forcément un symbole.
	///
	/// \param	inClass un objet qui sert de nouvelle classe à ce binaire.
	/// \throws TDCLNSException si une erreur est survenue.
	///
	virtual void SetClass( const TDCLNSRef& inClass );

protected:
	///
	/// Crée une représentation NSOF du binaire.
	/// Comprend le type et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToNSOF( TDCLNSOFEncoder* inEncoder ) const;

	///
	/// Constructeur à partir d'une zone de mémoire déjà allouée.
	///
	/// \param inData		données pour le binaire (non copiées).
	/// \param inLength		taille, en octets, du binaire.
	/// \param inClass		classe du binaire.
	///
	TDCLNSBinary(
			void* inData,
			KUInt32 inLength,
			const TDCLNSRef& inClass = KDCLSYM::kSYMbinary );
	
private:
	///
	/// Constructeur par défaut volontairement indisponible.
	///
	TDCLNSBinary( void );

	/// \name Interface IStreamable
	// @{
	///
	/// Crée une représentation XML du binaire.
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
	/// Crée une représentation sous forme de texte du binaire.
	/// Comprend les chevrons et les données.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder		encodeur avec la liste des objets déjà
	///							encodés pour les IDs.
	///
	virtual void ToText( TDCLTextEncoder* inEncoder ) const;

	///
	/// Crée une représentation du binaire pour mettre dans un
	/// paquet.
	/// Appelle récursivement les méthodes de l'encodeur.
	///
	/// \param inEncoder	encodeur pour le paquet.
	/// \param ioOffset		en entrée et en sortie, décalage par rapport
	///						au paquet.
	///
	virtual void ToPkg( TDCLPkgEncoder* inEncoder, KUInt32* ioOffset ) const;
	// @}
	
	/// \name Variables Privées
	TDCLNSRef		mClass;			///< Classe du binaire
	void*			mBuffer;		///< Mémoire tampon
	KUInt32			mBufferSize;	///< Taille de la mémoire tampon
};

#endif
		// __TDCLNSBINARY__

// =========================================== //
// I've looked at the listing, and it's right! //
//                 -- Joel Halpern             //
// =========================================== //
