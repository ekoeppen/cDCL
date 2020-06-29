// ==============================
// Fichier:			TDCLPkgEncoder.h
// Projet:			Desktop Connection Library
// 
// Créé le:			21/2/2004
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
// The Original Code is TDCLPkgEncoder.h.
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
// $Id: TDCLPkgEncoder.h,v 1.8 2004/11/24 14:09:06 paul Exp $
// ===========

#ifndef _TDCLPKGENCODER_H
#define _TDCLPKGENCODER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>
#include <DCL/NS_Objects/Exchange/KDCLPkg.h>

#include <list>
#include <utility>

class TDCLRandomAccessStream;

///
/// Classe pour l'encodage au format paquet.
/// Le format est décrit dans Newton Formats.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.8 $
///
/// \test	UTestPkg
///
class TDCLPkgEncoder
	:
		public TDCLNSEncoder,
		public KDCLPkg
{
public:
	///
	/// Constructeur à partir d'un flux et d'un décalage dans le flux.
	/// Ce décalage correspond au nombre d'octets déjà écrits dans le
	/// paquet et le même décalage devra être utilisé pour le décodage.
	///
	/// \param inStream		flux utilisé par cet encodeur.
	/// \param inOffset		décalage pour l'encodage des références.
	/// \param inFourBytesPadding	si on aligne sur 4 octets (NewtonOS 2.x
	///						uniquement). Par défaut, l'alignement est sur
	///						8 octets.
	///
	TDCLPkgEncoder(
			TDCLRandomAccessStream* inStream,
			KUInt32 inOffset,
			Boolean inFourBytesPadding = false );

	///
	/// Constructeur à partir d'un flux. Le décalage est la position
	/// actuelle du flux.
	///
	/// \param inStream		flux utilisé par cet encodeur.
	/// \param inFourBytesPadding	si on aligne sur 4 octets (NewtonOS 2.x
	///						uniquement). Par défaut, l'alignement est sur
	///						8 octets.
	///
	TDCLPkgEncoder(
			TDCLRandomAccessStream* inStream,
			Boolean inFourBytesPadding = false );

	///
	/// Destructeur.
	///
	virtual ~TDCLPkgEncoder( void );

	///
	/// Accesseur sur le flux de sortie.
	///
	/// \return le flux de sortie sur lequel écrire les données.
	///
	inline TDCLRandomAccessStream* GetOutputStream( void ) const
		{
			return (TDCLRandomAccessStream*)
				TDCLNSEncoder::GetOutputStream();
		}

	///
	/// Ajoute un objet dans le flux.
	/// Méthode appelée récursivement par les méthodes ToPkg des objets.
	///
	/// \param inObject		référence de l'objet à ajouter
	///
	virtual void	AddObject( const TDCLNSRef& inObject );

	///
	/// Détermine le décalage du prochain objet.
	///
	/// \return le décalage vers le prochain objet dans le flux.
	///
	inline KUInt32	GetNextObjectOffset( void ) const
		{
			return mOffset;
		}

	///
	/// Aligne une positiond dans le flux, sans modifier le flux, par
	/// rapport à un décalage donné en suivant \c mFourBytesPadding.
	///
	/// \param inOffset	en entrée, décalage courant, en sortie, décalage
	///					après l'alignement.
	///
	void			AlignOffset( KUInt32* inOffset ) const;

	///
	/// Aligne le flux si nécessaire par rapport à un décalage donné en suivant
	/// \c mFourBytesPadding.
	///
	/// \param inOffset	en entrée, décalage courant, en sortie, décalage
	///					après l'alignement.
	///
	void			AlignStream( KUInt32* inOffset ) const;

	///
	/// Détermine si le flux est aligné sur 8 octets ou 4.
	///
	/// \return \c true si l'alignement se fait sur 4 octets.
	///
	inline Boolean	IsFourBytesPadded( void ) const
		{
			return mFourBytesPadding;
		}

	///
	/// Get the best supermap during encoding for a given frame.
	///
	Boolean GetBestSupermap(const TDCLNSFrame*, KUInt32* outSharedKeysCount, KUInt32* outSupermapRef);

	///
	/// Add a map to be used as supermap.
	///
	void AddMap(const TDCLNSFrame*, KUInt32 mapRef);

protected:
	///
	/// Met dans le flux la référence d'un objet déjà encodé.
	///
	/// \param inRefID		référence de l'objet ajouté.
	/// \param inCookie		en entrée, biscuit laissé par PutObject
	///						lorsque l'objet a été ajouté la première fois.
	///
	virtual void	PutPrecedent( KUInt32 inRefID, KUIntPtr inCookie );

	///
	/// Met dans le flux un objet donné.
	///
	/// \param inObject		objet à ajouter.
	/// \param inObjectID	ID de l'objet à ajouter, -1 si CanHavePrecedentID
	///						a retourné \c false.
	/// \param outCookie	en sortie, biscuit passé par la suite à
	///						PutPrecedent, \c nil si CanHavePrecedentID a
	///						retourné \c false.
	///
	virtual void	PutObject(
						const TDCLNSObject* inObject,
						KUInt32 inObjectID,
						KUIntPtr* outCookie );

	///
	/// Met dans le flux une référence ou un immédiat donnés.
	///
	/// \param inRef		référence à ajouter.
	///
	virtual void	PutRef( const TDCLNSRef& inRef );

private:
	///
	/// Crée la pile et écrit l'entête (le premier tableau).
	///
	void			Init( void );

	///
	/// Constantes pour cette classe.
	///
	enum {
		kStackChunkSize		= 10,	///< Incrément de la pile lorsqu'elle est pleine.
		kStackDecrThreshold	= 50	///< Décrément de la pile lorsqu'elle se vide.
	};

	///
	/// Structure pour les objets dans la pile.
	///
	struct SStackItem {
		TDCLNSObject*		fObjectPtr;	///< Pointeur sur l'objet.
		KSInt64				fPtrOffset;	///< Décalage vers le pointeur dans le flux.
	};

	///
	/// Pousse un objet dans la pile.
	///
	/// \param inObject		référence sur l'objet à stocker dans la pile.
	///
	void			PushToStack( const TDCLNSRef& inObject );

	KSInt64			mOriginOffset;			///< Décalage du début du paquet
											///< dans le flux.
	KUInt32			mOffset;				///< Décalage courant dans le flux.
	KUInt32			mPartOffset;			///< Décalage de la partie par
											///< rapport au début du paquet.
	Boolean			mFourBytesPadding;		///< Alignement sur 4 octets (au lieu
											///< de 8).
	SStackItem*		mStack;					///< Pile des objets à stocker.
	KUInt32			mStackAllocatedSize;	///< Nombre d'éléments que la pile
											///< peut contenir.
	KUInt32			mStackSize;				///< Nombre d'éléments que la pile
											///< contient.
	Boolean			mInSlottedObject;		///< Si le flux est dans un objet.
	std::list<std::pair<const TDCLNSFrame*, KUInt32>> mEncodedMaps;
};

#endif
		// _TDCLPKGENCODER_H

// ======================================= //
// I wish you humans would leave me alone. //
// ======================================= //
