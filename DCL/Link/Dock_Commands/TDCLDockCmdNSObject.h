// ==============================
// Fichier:			TDCLDockCmdNSObject.h
// Projet:			Desktop Connection Library
//
// Créé le:			17/01/2002
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
// The Original Code is TDCLDockCmdNSObject.h.
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
// $Id: TDCLDockCmdNSObject.h,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCMDNSOBJECT__
#define __TDCLDOCKCMDNSOBJECT__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour une commande du Dock dont les données sont un objet NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdNSObject
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLDockCmdNSObject( void );

	///
	/// Constructeur pour l'envoi d'un objet NewtonScript à partir
	/// d'une référence.
	///
	/// \param inCommand	code définissant la commande
	/// \param inObject		référence sur l'objet
	///
	TDCLDockCmdNSObject(
				KUInt32 inCommand,
				const TDCLNSRef& inObject );

	///
	/// Constructeur pour la réception d'un objet NewtonScript.
	///
	/// \param inCommand	code définissant la commande
	/// \param inLength		longueur des données
	/// \param inData		données
	///
	TDCLDockCmdNSObject(
				KUInt32 inCommand,
				KUInt32 inLength,
				const void* inData );
	
	///
	/// Destructeur.
	///
	/// Libère la mémoire tampon.
	///
	virtual		~TDCLDockCmdNSObject( void );

	///
	/// Retourne un nouvel objet créé à partir de la mémoire tampon.
	/// Retourne NILREF si la mémoire tampon n'a pas été allouée.
	///
	/// \return une référence vers un nouvel objet ou NILREF.
	///
	TDCLNSRef				GetObject( void ) const;

	///
	/// Remplace l'objet dans la mémoire tampon.
	///
	/// \param inObject		référence sur l'objet
	///
	void					SetObject( const TDCLNSRef& inObject );

protected:
	///
	/// Récupère le pointeur sur les données.
	///
	/// \return le pointeur sur la mémoire tampon.
	///
	virtual const void*		GetData( void );
	
	///
	/// Récupère la taille des données.
	///
	/// \return la taille de la mémoire tampon
	///
	virtual	KUInt32			GetLength( void ) const;

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdNSObject( const TDCLDockCmdNSObject& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdNSObject& operator = ( const TDCLDockCmdNSObject& inCopy );

	/// \name Variables
	void*		mBuffer;		///< Mémoire tampon
	KUInt32		mBufferSize;	///< Taille allouée de la mémoire tampon
};

#endif
		// __TDCLDOCKCMDNSOBJECT__

// ================================================= //
// [It is] best to confuse only one issue at a time. //
//                 -- K&R                            //
// ================================================= //
