// ==============================
// Fichier:			IDCLFiles.h
// Projet:			Desktop Connection Library
//
// Créé le:			26/10/2002
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
// The Original Code is IDCLFiles.h.
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
// $Id: IDCLFiles.h,v 1.7 2006/02/12 01:23:56 pguyot Exp $
// ===========

#ifndef __IDCLFILES__
#define __IDCLFILES__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Streams/TDCLStream.h>

///
/// Interface pour gérer les fichiers.
/// Cette classe doit être dérivée pour chaque plateforme ou
/// application.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class IDCLFiles
{
public:
	///
	/// Destructor.
	///
	virtual ~IDCLFiles( void ) {};

	///
	/// Retourne le code du type de la plateforme tel qu'il sera indiqué au Newton
	/// (Mac ou Windows). Ceci influe sur le comportement à avoir pour les méthodes
	/// suivantes.
	///
	/// \return TDCLDockCommand::kMac_DType ou TDCLDockCommand::kWin_DType
	///
	virtual KUInt32	GetKind( void ) = 0;

	///
	/// Détermine le dossier bureau ou le dossier racine pour un disque donné.
	///
	/// \param inRootName	nom du disque (peut-être nil).
	/// \return une instance de TDCLFolder qui appartient à IDCLFiles.
	///
	virtual TDCLFSItemRef	GetRootFolder(
									const KUInt16* inRootName = nil ) = 0;

	///
	/// Détermine le nombre de dossiers racine.
	///
	/// \return le nombre de dossiers racine.
	///
	virtual KUInt32			CountRootFolders( void ) const = 0;

	///
	/// Récupère le nom du n-ième dossier racine.
	/// Le nom appartient à l'objet IDCLFiles.
	/// Ce nom doit obligatoirement se terminer par :
	///
	/// \param inFolderIndex	un indice de dossier racine, entre 0 et
	///							CountRootFolders() - 1.
	/// \return le nom du dossier racine correspondant.
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const = 0;
};

#endif
		// __IDCLFILES__

// ============================================================================== //
// APL is a mistake, carried through to perfection.  It is the language of the    //
// future for the programming techniques of the past: it creates a new generation //
// of coding bums.                                                                //
//                 -- Edsger W. Dijkstra, SIGPLAN Notices, Volume 17, Number 5    //
// ============================================================================== //
