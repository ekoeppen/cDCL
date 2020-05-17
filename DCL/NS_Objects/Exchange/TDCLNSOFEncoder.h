// ==============================
// Fichier:			TDCLNSOFEncoder.h
// Projet:			Desktop Connection Library
//
// Créé le:			15/10/2002
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
// The Original Code is TDCLNSOFEncoder.h.
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
// $Id: TDCLNSOFEncoder.h,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __TDCLNSOFENCODER__
#define __TDCLNSOFENCODER__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/KDCLNSOF.h>
#include <DCL/NS_Objects/Exchange/TDCLNSEncoder.h>

///
/// Classe pour l'encodage au format NSOF.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	UTestNSOF
///
class TDCLNSOFEncoder
	:
		public TDCLNSEncoder,
		public KDCLNSOF
{
public:
	///
	/// Constructeur à partir d'un flux.
	///
	/// \param inStream		flux utilisé par cet encodeur.
	///
	TDCLNSOFEncoder( TDCLStream* inStream );

	///
	/// Destructeur.
	///
	virtual ~TDCLNSOFEncoder( void );

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
};

#endif
		// __TDCLNSOFENCODER__

// ============================================================================ //
// Build a system that even a fool can use and only a fool will want to use it. //
// ============================================================================ //
