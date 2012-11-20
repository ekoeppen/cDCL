// ==============================
// Fichier:			TDCLDockCmdGeneric.h
// Projet:			Desktop Connection Library
//
// Créé le:			02/02/2001
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
// The Original Code is TDCLDockCmdGeneric.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLDockCmdGeneric.h,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCMDGENERIC__
#define __TDCLDOCKCMDGENERIC__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour une commande du dock avec des données quelconques.
/// (s'il n'y a pas de données, autant utiliser TDCLDockCmdNoData, s'il
/// n'y a qu'un long, c'est TDCLDockCmdSingleLong qui convient le mieux).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdGeneric
	:
		public TDCLDockCommand
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLDockCmdGeneric( void );
	
	///
	/// Constructeur à partir d'un identifiant de commande et des données.
	///
	/// \param inCommand	identifiant pour la commande.
	/// \param inLength		taille des données.
	/// \param inData		pointeur sur les données (copiées)
	///
	TDCLDockCmdGeneric(
					KUInt32 inCommand,
					KUInt32 inLength = 0,
					const void* inData = nil );

	///
	/// Destructeur.
	/// Libère la copie des données.
	///
	virtual		~TDCLDockCmdGeneric( void );

	///
	/// Sélecteur sur l'identifiant de la commande.
	///
	/// \param inCommand	nouvel identifiant pour la commande.
	///
	inline	void	SetCommand( KUInt32 inCommand )
		{
			TDCLDockCommand::SetCommand( inCommand );
		};
	
	///
	/// Accesseur sur la taille des données.
	///
	/// \return la taille des données.
	///
	virtual	KUInt32			GetLength( void ) const;
	
	///
	/// Sélecteur sur la taille des données.
	/// Redimensionne la mémoire tampon.
	///
	/// \param inLength		nouvelle taille des données.
	///
	void					SetLength( KUInt32 inLength );

	///
	/// Accesseur sur les données (pour pouvoir les changer).
	///
	/// \return le pointeur sur les données.
	///
	inline	void*			GetDataPtr( void )
		{
			return mData;
		};
	
	///
	/// Sélecteur sur les données.
	/// Copie les données fournies en paramètre dans la mémoire
	/// tampon de la commande.
	///
	/// \param inData		données à copier.
	///
	void					CopyData( const void* inData );

protected:
	///
	/// Accesseur sur les données (en lecture seule).
	/// Utilisé pour envoyer la commande.
	///
	/// \return le pointeur sur les données (non modifiables)
	///
	virtual const void*		GetData( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdGeneric( const TDCLDockCmdGeneric& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdGeneric& operator = ( const TDCLDockCmdGeneric& inCopy );

	/// \name Variables

	KUInt32			mLength;	///< Taille de la mémoire tampon.
	void*			mData;		///< Mémoire tampon avec les données.
};

#endif
		// __TDCLDOCKCMDGENERIC__

// =============================================================== //
// System going down at 5 this afternoon to install scheduler bug. //
// =============================================================== //
