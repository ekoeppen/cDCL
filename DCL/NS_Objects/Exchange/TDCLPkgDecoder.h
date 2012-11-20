// ==============================
// Fichier:			TDCLPkgDecoder.h
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
// The Original Code is TDCLPkgDecoder.h.
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
// $Id: TDCLPkgDecoder.h,v 1.9 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLPKGDECODER_H
#define _TDCLPKGDECODER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSDecoder.h>
#include <DCL/NS_Objects/Exchange/KDCLPkg.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

///
/// Classe pour le décodeur du format paquet.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.9 $
///
/// \test	UTestPkg
///
class TDCLPkgDecoder
	:
		public TDCLNSDecoder,
		public KDCLPkg
{
public:
	///
	/// Constructeur à partir d'un flux et d'un décalage dans le flux.
	/// C'est le décalage depuis le début du paquet. Le même décalage
	/// a dû être utilisé lors de l'encodage.
	///
	/// \param inStream		flux où lire les données.
	/// \param inOffset		décalage pour l'encodage des références.
	///
	TDCLPkgDecoder(
			TDCLRandomAccessStream* inStream,
			KUInt32 inOffset );

	///
	/// Constructeur à partir d'un flux. Le décalage est la position
	/// actuelle du flux.
	///
	/// \param inStream		flux utilisé par cet encodeur.
	///
	TDCLPkgDecoder( TDCLRandomAccessStream* inStream );

	///
	/// Destructeur.
	///
	virtual ~TDCLPkgDecoder( void );

	///
	/// Crée l'objet suivant. Lit le tableau initial.
	/// Positionne ensuite le flux sur les octets après le dernier objet,
	/// éventuellement après des octets d'alignement.
	///
	/// \return une référence vers l'objet suivant.
	///
	virtual TDCLNSRef	GetNextObject( void );

	///
	/// Accesseur sur le flux de lecture.
	///
	/// \return le flux où lire les données.
	///
	inline TDCLRandomAccessStream* GetInputStream( void ) const
		{
			return (TDCLRandomAccessStream*)
				TDCLNSDecoder::GetInputStream();
		}

	///
	/// Accesseur sur le décalage dans le flux de lecture.
	/// Cette méthode est utilisée pour déterminer le décalage d'un objet lorsqu'il
	/// est décodé.
	///
	/// \return le décalage du flux par rapport au début du paquet.
	///
	inline KUInt32 GetCurrentStreamOffset( void ) const
		{
			return (KUInt32) (GetInputStream()->GetCursor() - mOriginOffset);
		}

	///
	/// Ajoute un objet dans la liste des objets.
	///
	/// \param inObject		objet à ajouter
	/// \param inOffset		décalage de l'objet dans le flux par rapport au
	///						début du paquet.
	///
	inline void	AddObjectToList(
					const TDCLNSRef& inObject,
					KUInt32 inOffset )
		{
			TDCLNSDecoder::AddObjectToList( inObject, (KUIntPtr) inOffset );
		}

	///
	/// Accesseur sur l'alignement.
	///
	/// \return \c true si l'alignement se fait sur 8 octets.
	///
	inline Boolean		GetFourBytesPadding( void )
		{
			return mFourBytesPadding;
		}

private:
	///
	/// Lecture du premier tableau.
	/// Positionne le flux sur le pointeur vers le premier objet.
	///
	/// \param inStream		flux à lire.
	///
	void		ReadFirstArray( TDCLRandomAccessStream* inStream );
	
	KSInt64		mOriginOffset;		///< Décalage dans le flux du début du paquet.
	KSInt64		mMaxOffset;			///< Décalage maximal.
	Boolean		mTopLevel;			///< Si on est à la racine de l'arbre de récursion.
	Boolean		mFourBytesPadding;	///< Si l'alignement se fait sur 8 octets.
};

#endif
		// _TDCLPKGDECODER_H

// =========================================================================== //
// Thus spake the master programmer:                                           //
//         "A well-written program is its own heaven; a poorly-written program //
// is its own hell."                                                           //
//                 -- Geoffrey James, "The Tao of Programming"                 //
// =========================================================================== //
