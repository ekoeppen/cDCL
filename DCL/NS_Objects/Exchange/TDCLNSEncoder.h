// ==============================
// Fichier:			TDCLNSEncoder.h
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
// The Original Code is TDCLNSEncoder.h.
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
// $Id: TDCLNSEncoder.h,v 1.5 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __TDCLNSENCODER__
#define __TDCLNSENCODER__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSCodec.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

class TDCLNSObject;
class TDCLStream;

///
/// Classe pour l'encodage d'objets NS. Elle gère la liste des objets avec les IDs.
/// Un objet peut demander l'ID courant ou chercher si un objet est déjà dans la liste.
/// (les structures NewtonScript ne sont pas des arbres mais des DAG).
/// Cette classe est abstraite.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	UTestNSOF
///
class TDCLNSEncoder
	:
		private TDCLNSCodec
{
public:
	///
	/// Destructeur.
	///
	virtual ~TDCLNSEncoder( void );

	///
	/// Ajoute un objet dans un flux donné.
	/// Cette méthode commence par regarder si l'objet est un immédiat
	/// ou pas. Si c'est un immédiat, elle appelle PutRef, sinon elle
	/// appelle PutObject.
	/// Cette méthode ajoute l'objet ou une référence si l'objet est
	/// déjà dans la liste.
	/// Remarque: la comparaison n'est faite que sur les pointeurs.
	///
	/// \param inObject		référence de l'objet à ajouter
	///
	virtual void	AddObject( const TDCLNSRef& inObject );

	///
	/// Accesseur sur le flux de sortie.
	///
	/// \return le flux de sortie sur lequel écrire les données.
	///
	inline TDCLStream* GetOutputStream( void ) const
		{
			return GetStream();
		}

	///
	/// Détermine si un objet est déjà dans la liste.
	///
	/// \param inObject			objet à chercher dans la liste.
	/// \param outObjectID		en sortie, identifiant de l'objet, non
	///							renseigné si \c nil.
	/// \param outObjectCookie	en sortie, biscuit associé à l'objet (idem).
	/// \return \c true si l'objet a déjà été encodé.
	///
	Boolean WasObjectEncoded(
				TDCLNSObject* inObject,
				KUInt32* outObjectID,
				KUIntPtr* outObjectCookie );

protected:
	///
	/// Constructeur à partir d'un flux.
	///
	/// \param inStream		flux où écrire les données encodées.
	///
	TDCLNSEncoder( TDCLStream* inStream );

	///
	/// Met dans le flux la référence d'un objet déjà encodé.
	///
	/// \param inRefID		référence de l'objet ajouté.
	/// \param inCookie		en entrée, biscuit laissé par PutObject
	///						lorsque l'objet a été ajouté la première fois.
	///
	virtual void	PutPrecedent( KUInt32 inRefID, KUIntPtr inCookie ) = 0;

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
						KUIntPtr* outCookie ) = 0;

	///
	/// Met dans le flux un immédiat donné.
	///
	/// \param inRef		référence à ajouter.
	///
	virtual void	PutRef( const TDCLNSRef& inRef ) = 0;

	///
	/// Détermine si l'objet donné peut être référencé par un "precedent".
	///
	/// Sert pour les symboles. NSOF dit oui, Text et XML disent non.
	/// Par défaut, retourne \c true.
	///
	/// \param inObject		objet considéré.
	/// \return \c true si l'objet peut être référencé, \c false sinon.
	///
	virtual Boolean	CanHavePrecedentID( TDCLNSObject* inObject );
	
private:
	///
	/// Constructeur par défaut non disponible.
	///
	TDCLNSEncoder( void );
};

#endif
		// __TDCLNSENCODER__

// ========================================================================= //
// A LISP programmer knows the value of everything, but the cost of nothing. //
//                 -- Alan Perlis                                            //
// ========================================================================= //
