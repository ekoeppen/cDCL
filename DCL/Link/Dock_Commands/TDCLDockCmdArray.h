// ==============================
// Fichier:			TDCLDockCmdArray.h
// Projet:			Desktop Connection Library
//
// Créé le:			14/4/2003
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
// The Original Code is TDCLDockCmdArray.h.
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
// $Id: TDCLDockCmdArray.h,v 1.3 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLDOCKCMDARRAY_H
#define _TDCLDOCKCMDARRAY_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour une commande du dock contenant un tableau.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdArray
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur à partir d'une taille et d'un tableau.
	/// Attention à ne pas le confondre avec le constructeur à
	/// partir d'une mémoire tampon.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inArray		tableau (copié)
	/// \param inSize		nombre d'éléments dans le tableau
	///
	TDCLDockCmdArray( KUInt32 inCommand, const KUInt32* inArray, KUInt32 inSize );

	///
	/// Constructeur à partir d'une taille. Les éléments ne sont pas initialisés.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inSize		nombre d'éléments dans le tableau
	///
	TDCLDockCmdArray( KUInt32 inCommand, KUInt32 inSize );

	///
	/// Constructeur à partir d'une mémoire tampon et d'une taille.
	///
	/// \param inCommand	identifiant de la commande.
	/// \param inBufferSize	taille de la mémoire tampon
	/// \param inBuffer		mémoire tampon (copiée)
	///
	TDCLDockCmdArray( KUInt32 inCommand, KUInt32 inBufferSize, void* inBuffer );

	///
	/// Destructeur.
	///
	virtual ~TDCLDockCmdArray( void );

	///
	/// Accesseur sur un élément.
	///
	/// \param inIndex	indice de l'élément à récupérer
	/// \return le n-ième élément.
	///
	inline KUInt32		Get( KUInt32 inIndex ) const
		{
			return mBufferData[inIndex+1];
		}

	///
	/// Sélecteur sur un élément.
	///
	/// \param inIndex		indice de l'élément à modifier
	/// \param inNewValue	nouvelle valeur
	/// \return le n-ième élément.
	///
	inline void			Set( KUInt32 inIndex, KUInt32 inNewValue ) const
		{
			mBufferData[inIndex+1] = inNewValue;
		}

	///
	/// Accesseur sur le nombre d'éléments
	///
	/// \return le nombre d'éléments composant cette commande.
	///
	inline KUInt32		GetCount( void ) const
		{
			return mBufferData[0];
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdArray( const TDCLDockCmdArray& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdArray& operator = ( const TDCLDockCmdArray& inCopy );

	///
	/// Méthode pour récupérer un pointeur sur les données afin de les
	/// envoyer au Newton.
	///
	/// \return un pointeur sur les données
	///
	virtual const void*		GetData( void );
	
	///
	/// Méthode pour connaître la taille des données afin de les envoyer
	/// au Newton.
	///
	/// \return la taille de la mémoire tampon.
	///
	virtual	KUInt32			GetLength( void ) const;

	/// \name Variables

	KUInt32*			mBufferData;	/// Données (taille puis éléments)
};

#endif
		// _TDCLDOCKCMDARRAY_H

// ======================================================== //
// The moving cursor writes, and having written, blinks on. //
// ======================================================== //
