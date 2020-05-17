// ==============================
// Fichier:			TDCLDockCmdPackageList.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/5/2003
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
// The Original Code is TDCLDockCmdPackageList.h.
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
// $Id: TDCLDockCmdPackageList.h,v 1.4 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef _TDCLDOCKCMDPACKAGELIST_H
#define _TDCLDOCKCMDPACKAGELIST_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

///
/// Classe pour la commande Newton comprenant la liste des paquets 1.x à
/// sauvegarder.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLDockCmdPackageList
	:
		public TDCLDockCommand
{
public:
	///
	/// Structure pour les informations sur un paquet.
	///
	struct SPackageInfo
	{
		KUInt32	fPackageSize;		///< Taille du paquet
		KUInt32	fPackageID;			///< Identifiant du paquet.
		KUInt32	fPackageVersion;	///< Version du paquet.
		KUInt32	fPackageFormat;		///< Format (?)
		KUInt32	fDeviceKind;		///< (?)
		KUInt32	fDeviceNumber;		///< (?)
		KUInt32	fDeviceID;			///< (?)
		KUInt32	fModificationDate;	///< Date de modification du paquet
		KUInt32	fIsCopyProtected;	///< Si le bit copy protected est à un
		KUInt32	fNameLength;		///< Longueur (en octets) de la chaîne.
		KUInt16	fName[0];			///< Nom du paquet (en unicode)
	};
	
	///
	/// Constructeur unique à partir de données.
	///
	/// \param inLength	taille des données
	/// \param inData	données (copiées)
	///
	TDCLDockCmdPackageList( KUInt32 inLength, const void* inData );

	///
	/// Destructeur
	/// (libère la mémoire utilisée)
	///
	virtual ~TDCLDockCmdPackageList( void );

	///
	/// Accesseur sur le nombre de paquets.
	///
	/// \return le nombre d'enregistrements pour les paquets.
	///
	inline KUInt32	GetPackageCount( void ) const
		{
			return mPackageCount;
		}
	
	///
	/// Accesseur sur les informations sur un paquet.
	///
	/// \param inIndex	indice du paquet dont on veut les informations.
	/// \return les informations sur un paquet.
	///
	inline const SPackageInfo*	GetPackageInfo( KUInt32 inIndex )
		{
			return mInfos[inIndex];
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdPackageList( const TDCLDockCmdPackageList& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLDockCmdPackageList& operator = ( const TDCLDockCmdPackageList& inCopy );

	/// \name Variables

	KUInt32			mPackageCount;		///< Nombre de paquets.
	SPackageInfo**	mInfos;				///< Pointeurs sur les informations.
};

#endif
		// _TDCLDOCKCMDPACKAGELIST_H

// =============================================== //
// Is your job running?  You'd better go catch it! //
// =============================================== //
