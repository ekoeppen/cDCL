// ==============================
// Fichier:			TDCLDockCmdTwoNSObjects.h
// Projet:			Desktop Connection Library
// 
// Créé le:			1/11/2003
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
// The Original Code is TDCLDockCmdTwoNSObjects.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdTwoNSObjects.h,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef _TDCLDOCKCMDTWONSOBJECTS_H
#define _TDCLDOCKCMDTWONSOBJECTS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour une commande Dock avec deux objets NS.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdTwoNSObjects
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLDockCmdTwoNSObjects( void );

	///
	/// Constructeur pour l'envoi de deux objets NewtonScript à partir de
	/// références.
	///
	/// \param inCommand		code définissant la commande
	/// \param inFirstObject	référence sur le premier objet
	/// \param inSecondObject	référence sur le second objet
	///
	TDCLDockCmdTwoNSObjects(
				KUInt32 inCommand,
				const TDCLNSRef& inFirstObject,
				const TDCLNSRef& inSecondObject );

	///
	/// Constructeur pour la réception d'un objet NewtonScript.
	///
	/// \param inCommand	code définissant la commande
	/// \param inLength		longueur des données
	/// \param inData		données
	///
	TDCLDockCmdTwoNSObjects(
				KUInt32 inCommand,
				KUInt32 inLength,
				const void* inData );
	
	///
	/// Destructeur.
	///
	/// Libère la mémoire tampon.
	///
	virtual ~TDCLDockCmdTwoNSObjects( void );

	///
	/// Retourne un tableau avec les deux objets créés à partir de la mémoire
	/// tampon. Retourne \c NILREF si la mémoire tampon n'a pas été allouée.
	///
	/// \return une référence vers un tableau contenant deux nouveaux objets
	///			ou \c NILREF.
	///
	TDCLNSRef				GetObjects( void ) const;

	///
	/// Remplace les objets dans la mémoire tampon.
	///
	/// \param inFirstObject	référence sur le premier objet
	/// \param inSecondObject	référence sur le second objet
	///
	void					SetObjects(
								const TDCLNSRef& inFirstObject,
								const TDCLNSRef& inSecondObject );

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
	TDCLDockCmdTwoNSObjects( const TDCLDockCmdTwoNSObjects& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdTwoNSObjects& operator =
							( const TDCLDockCmdTwoNSObjects& inCopy );

	/// \name Variables
	void*		mBuffer;		///< Mémoire tampon
	KUInt32		mBufferSize;	///< Taille allouée de la mémoire tampon
};

#endif
		// _TDCLDOCKCMDTWONSOBJECTS_H

// ========================================================================= //
// Real programmers disdain structured programming.  Structured programming  //
// is for compulsive neurotics who were prematurely toilet-trained.  They    //
// wear neckties and carefully line up pencils on otherwise clear desks.     //
// ========================================================================= //
