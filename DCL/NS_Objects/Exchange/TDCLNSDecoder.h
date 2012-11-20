// ==============================
// Fichier:			TDCLNSDecoder.h
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
// The Original Code is TDCLNSDecoder.h.
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
// $Id: TDCLNSDecoder.h,v 1.5 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __TDCLNSDECODER__
#define __TDCLNSDECODER__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSCodec.h>

// DCL
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLStream;
class TDCLNSObject;

///
/// Classe pour le décodage d'un flux d'objets NS.
/// Gère les références des objets déjà décodés.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLNSDecoder
	:
		private TDCLNSCodec
{
public:
	///
	/// Destructeur.
	///
	virtual ~TDCLNSDecoder( void );

	///
	/// Accesseur sur le flux d'entrée.
	///
	/// \return le flux d'entrée d'où on lit les données.
	///
	inline TDCLStream* GetInputStream( void ) const
		{
			return GetStream();
		}

	///
	/// Crée l'objet suivant.
	/// Cette méthode doit être dérivée dans chacun des décodeurs
	/// qui doivent appeler AddObjectToList pour les pointeurs et
	/// qui peuvent appeler GetObject pour obtenir le pointeur vers
	/// l'objet d'un ID donné.
	///
	/// \return une référence vers l'objet suivant.
	///
	virtual TDCLNSRef	GetNextObject( void ) = 0;

	///
	/// Ajoute un objet dans la liste des objets.
	///
	/// \param inObject		objet à ajouter
	/// \param inCookie		biscuit à stocker avec l'objet.
	///
	inline void	AddObjectToList(
					const TDCLNSRef& inObject,
					KUIntPtr inCookie = 0 )
		{
			(void) TDCLNSCodec::AddObjectToList( inObject, inCookie );
		}

protected:
	///
	/// Constructeur à partir d'un flux.
	///
	TDCLNSDecoder( TDCLStream* inStream );

	///
	/// Récupère un objet de la liste des objets.
	///
	/// \param inID			ID de l'objet à récupérer.
	/// \return un pointeur vers l'objet en question
	/// \throws TDCLNSException si l'ID n'est pas valide.
	///
	TDCLNSRef	GetObject( KUInt32 inID );

	///
	/// Récupère un objet de la liste des objets à partir de son biscuit, à
	/// partir d'un indice donné.
	///
	/// \param inCookie		Biscuit de l'objet à récupérer.
	/// \param ioID			en entrée, ID de l'objet par lequel commencer,
	///						en sortie, ID de l'objet trouvé.
	/// \return un pointeur vers l'objet en question ou NILREF si aucun
	///			objet ne correspond.
	/// \throws TDCLNSException si l'ID n'est pas valide.
	///
	TDCLNSRef	GetNextObjectWithCookie(
							KUIntPtr inCookie,
							KUInt32* ioID = nil );

private:
	///
	/// Constructeur par défaut non disponible.
	///
	TDCLNSDecoder( void );
};

#endif
		// __TDCLNSDECODER__

// ============================================= //
// Overload -- core meltdown sequence initiated. //
// ============================================= //
