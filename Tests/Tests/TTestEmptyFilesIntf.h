//==============================
// Fichier:			TTestEmptyFilesIntf.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			17/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: TTestEmptyFilesIntf.h,v 1.6 2003/12/28 07:04:50 paul Exp $
// ===========

#ifndef _TTESTEMPTYFILESINTF_H
#define _TTESTEMPTYFILESINTF_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLFiles.h>

///
/// Classe pour indiquer le type de système de fichier.
/// La méthode \c GetRootFolder lance une exception.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
class TTestEmptyFilesIntf
	:
		public IDCLFiles
{
public:
	///
	/// Retourne le code du type de la plateforme tel qu'il sera indiqué au Newton
	/// (Mac ou Windows).
	/// Cette classe de test renvoie \c TDCLDockCommand::kMac_DType
	///
	/// \return \c TDCLDockCommand::kMac_DType
	///
	virtual KUInt32	GetKind( void );

	///
	/// Détermine le dossier bureau ou le dossier racine pour un disque donné.
	///
	/// \param inRootName	nom du disque (pour "Windows", si le Newton croit
	///						que nous sommes un Mac, ce paramètre sera \c nil)
	/// \return rien du tout
	/// \throws DCLNotImplementedError dans tous les cas.
	///
	virtual TDCLFSItemRef	GetRootFolder( const KUInt16* inRootName = nil );

	///
	/// Détermine le nombre de dossiers racine.
	///
	/// \return rien du tout
	/// \throws DCLNotImplementedError dans tous les cas.
	///
	virtual KUInt32			CountRootFolders( void ) const;

	///
	/// Récupère le nom du n-ième dossier racine.
	/// Le nom appartient à l'objet IDCLFiles.
	///
	/// \param inFolderIndex	un indice de dossier racine, entre 0 et
	///							CountRootFolders() - 1.
	/// \return rien du tout
	/// \throws DCLNotImplementedError dans tous les cas.
	///
	virtual const KUInt16*	GetRootFolderName( KUInt32 inFolderIndex ) const;
};

#endif
		// _TTESTEMPTYFILESINTF_H

// ============================================================ //
// Think of it!  With VLSI we can pack 100 ENIACs in 1 sq. cm.! //
// ============================================================ //
