// ==============================
// Fichier:			TDCLDockCmdCString.h
// Projet:			Desktop Connection Library
//
// Créé le:			13/4/2003
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
// The Original Code is TDCLDockCmdCString.h.
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
// $Id: TDCLDockCmdCString.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLDOCKCMDCSTRING_H
#define _TDCLDOCKCMDCSTRING_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSString.h>

///
/// Classe pour une commande du dock contenant une chaîne UTF-16.
/// terminée par un nul.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdCString
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur à partir d'une commande et d'une chaîne.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inString		la chaîne composant cette commande (copiée)
	///
	TDCLDockCmdCString( KUInt32 inCommand, const KUInt16* inString );

	///
	/// Constructeur à partir d'une commande et d'une chaîne NS.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inString		la chaîne composant cette commande (copiée)
	///
	TDCLDockCmdCString( KUInt32 inCommand, const TDCLNSString& inString );

	///
	/// Constructeur à partir d'une mémoire tampon et d'une taille.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inBufferSize	taille de la mémoire tampon
	/// \param inBuffer		mémoire tampon (copiée)
	///
	TDCLDockCmdCString( KUInt32 inCommand, KUInt32 inBufferSize, void* inBuffer );

	///
	/// Destructeur.
	///
	virtual ~TDCLDockCmdCString( void );

	///
	/// Accesseur sur la chaîne.
	///
	/// \return la chaîne composant cette commande.
	///
	inline const KUInt16*	GetString( void ) const
		{
			return mString;
		}
protected:
	///
	/// Méthode pour récupérer un pointeur sur les données afin de les
	/// envoyer au Newton.
	///
	/// \return un pointeur sur la chaîne
	///
	virtual const void*		GetData( void );
	
	///
	/// Méthode pour connaître la taille des données afin de les envoyer
	/// au Newton.
	///
	/// \return mLength
	///
	virtual	KUInt32			GetLength( void ) const;

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdCString( const TDCLDockCmdCString& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdCString& operator = ( const TDCLDockCmdCString& inCopy );

	/// \name Variables

	KUInt16*	mString;		///< Chaîne.
	KUInt32		mLength;		///< Taille de la chaîne (avec le terminateur, en octets).
};

#endif
		// _TDCLDOCKCMDCSTRING_H

// =========================================================================== //
//         One day a student came to Moon and said, "I understand how to make  //
// a better garbage collector.  We must keep a reference count of the pointers //
// to each cons."                                                              //
//         Moon patiently told the student the following story -- "One day a   //
// student came to Moon and said, "I understand how to make a better garbage   //
// collector..."                                                               //
// =========================================================================== //
