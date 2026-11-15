// ==============================
// Fichier:			TDCLFile.h
// Projet:			Desktop Connection Library
//
// Créé le:			04/02/2003
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
// The Original Code is TDCLFile.h.
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
// $Id: TDCLFile.h,v 1.9 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __TDCLFILE__
#define __TDCLFILE__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>
#include <DCL/Interfaces/TDCLFSItem.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

///
/// Classe pour les fichiers.
///
/// Cette classe est purement virtuelle. Elle doit être dérivée pour chaque
/// plateforme afin de stocker les informations concernant les fichiers.
/// C'est une bonne idée d'appeler la méthode \c Close dans le destructeur.
///
/// Les fichiers peuvent être de vrais fichiers ou des ressources, en fait
/// n'importe quoi qui ressemble un peu à un fichier.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.9 $
///
/// \test	aucun test défini.
///
class TDCLFile
	:
		public TDCLFSItem,
		public TDCLRandomAccessStream
{		
public:
	///
	/// Ouvre le fichier.
	///
	/// \param inReadOnly	si le fichier doit être ouvert en lecture seule
	/// \throws TDCLIOException si l'ouverture a échoué.
	///
	virtual void		Open( Boolean inReadOnly ) = 0;	

	///
	/// Crée le fichier.
	///
	/// \throws TDCLIOException si la création a échoué.
	///
	virtual void		Create( void ) = 0;	

	///
	/// Supprime le fichier.
	///
	/// \throws TDCLIOException si la suppression a échoué.
	///
	virtual void		Delete( void ) = 0;	

	///
	/// Determine si le fichier est ouvert.
	///
	/// \return \c true si le fichier est ouvert, \c false sinon.
	///
	virtual Boolean		IsOpen( void ) const = 0;

	///
	/// Determine si le fichier est ouvert en lecture seule.
	///
	/// \return \c true si le fichier est ouvert en lecture seule,
	///			\c false sinon. Le résultat est indéterminé si le fichier
	///			n'est pas ouvert.
	///
	virtual Boolean		IsReadOnly( void ) const = 0;
	
	///
	/// Retourne la longueur (logique) du fichier.
	///
	/// \return la taille du fichier, en octets.
	///
	virtual KUInt64		GetLength( void ) = 0;
	
	///
	/// Récupère l'octet suivant sans avancer le curseur.
	/// Par défaut, lit le fichier pour un octet et retourne en arrière.
	///
	/// \return l'octet lu.
	/// \throws TDCLIOException	si un problème est survenu (y compris si la
	///							fin du fichier est rencontrée)
	///
	virtual	KUInt8		PeekByte( void );

	///
	/// Ferme le fichier.
	/// Vous souhaitez probablement appeler la méthode close dans votre
	/// destructeur.
	/// Ne fait rien si le fichier n'était pas ouvert.
	///
	virtual void		Close( void ) = 0;

	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Retourne une structure avec type: 'file.
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef	ToFrame( void );
	
	///
	/// Récupère le type du fichier sous forme de chaîne NewtonScript.
	/// C'est ce qui est retourné au Newton pour affichage dans
	/// le butineur.
	///
	/// \return une chaîne NewtonScript avec le type du fichier.
	///
	virtual TDCLNSRef	GetKind( void ) const = 0;
	
	///
	/// Récupère la date de création au format des dates Newton
	/// (minutes depuis 1904).
	///
	/// \return la date de création du fichier.
	///
	virtual KUInt32		GetCreationDate( void ) const = 0;

	///
	/// Récupère la date de modification au format des dates Newton
	/// (minutes depuis 1904).
	///
	/// \return la date de modification du fichier.
	///
	virtual KUInt32		GetModificationDate( void ) const = 0;

	///
	/// Récupère le chemin du fichier dans une forme lisible par
	/// l'utilisateur. Ce chemin n'a pas d'autre fonction que
	/// d'être affiché sur le Newton.
	///
	/// \return le chemin du fichier dans une chaîne NewtonScript.
	///
	virtual TDCLNSRef	GetStringPath( void ) const = 0;

	///
	/// Récupère l'icône du fichier sous forme de binaire noir et blanc
	/// 32x32. Si le résultat est \c nil, l'icône n'est pas envoyée au
	/// Newton.
	/// Par défaut renvoie \c nil.
	///
	/// \return l'icône du fichier ou \c nil.
	///
	virtual TDCLNSRef	GetIcon( void ) const;

	///
	/// Détermine si le fichier est un paquet.
	/// Appelle TDCLPackage::IsPackage, mais retourne \c false en cas
	/// d'exception.
	///
	/// \return \c true si le fichier est un paquet, \c false sinon.
	///
	Boolean				IsPackage( void );

protected:
	///
	/// Constructeur protégé à partir d'un dossier.
	/// Ce constructeur doit (généralement?) être appelé par une
	/// extension de la classe avec une extension de la classe
	/// TDCLFolder compatible.
	///
	/// \param inFilesIntf		interface pour les fichiers.
	/// \param inParentFolder	dossier où se trouve le fichier.
	///					c'est aussi le dossier qui a créé l'objet.
	///					(\c nil signifie que MakeParentFolder sera
	///					appelé plus tard).
	///
	TDCLFile( IDCLFiles* inFilesIntf, TDCLFSItemRef inParentFolder );

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est
	/// normalement jamais appelée puisque l'objet parent a été fourni
	/// comme paramètre au constructeur ci-dessus.
	///
	/// \throws TDCLNotImplementedException
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;
};

#endif
		// __TDCLFILE__

// ======================================================================== //
// Congratulations!  You are the one-millionth user to log into our system. //
// If there's anything special we can do for you, anything at all, don't    //
// hesitate to ask!                                                         //
// ======================================================================== //
