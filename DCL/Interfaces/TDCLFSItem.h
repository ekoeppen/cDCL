// ==============================
// Fichier:			TDCLFSItem.h
// Projet:			Desktop Connection Library
//
// Créé le:			09/02/2003
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
// The Original Code is TDCLFSItem.h.
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
// $Id: TDCLFSItem.h,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef __TDCLFSITEM__
#define __TDCLFSITEM__

#include <DCL/Headers/DCLDefinitions.h>
#include <K/Misc/TRefCounter.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>
#include <DCL/Interfaces/IDCLFiles.h>

// Pré-déclarations.
class TDCLFolder;
class IDCLFiles;

///
/// Classe pour un élément sur un système de fichiers.
///
/// Pour le moment, les seuls éléments sont des fichiers ou des dossiers. Mais
/// ceci est susceptible de changer.
///
/// Le butinage dans le système de fichier se fait comme suit:
/// - il existe un élément racine, le plus souvent virtuel, qui n'a pas de dossier parent.
/// - seuls les éléments que le Newton cherche à voir sont créés.
/// - les éléments disposent tous d'un compteur de référence pour un nombre minimum
///   d'occurrences en mémoire.
/// - les fichiers ne sont a priori référencés qu'une seule fois (on peut toutefois imaginer
///   que les dossiers gardent en cache une liste des fichiers).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFSItem
	:
		private TRefCounter
{
	/// La classe qui s'occupe des références est une amie.
	friend class TDCLFSItemRef;

public:
	///
	/// Différents types d'éléments.
	/// La classe de l'élément doit concorder avec le type.
	///
	enum EFSItemClass {
		kFile,			///< l'élément est un TDCLFile
		kFolder			///< l'élément est un TDCLFolder
	};
	
	///
	/// Destructeur.
	/// Décrémente le compteur de référence du dossier parent
	/// (et supprime ce dossier si nécessaire)
	///
	virtual ~TDCLFSItem( void );

	///
	/// Accesseur sur la classe.
	///
	/// \return la classe de l'élément.
	///
	inline	EFSItemClass	GetClass( void )
		{
			return mClass;
		}

	///
	/// Récupère le nom de l'élément.
	///
	/// \return le nom de l'élément, en UTF-16 (cette chaîne appartient à l'objet
	///			TDCLFSItem).
	///
	const KUInt16*		GetName( void );

	///
	/// Récupère le chemin de l'élément. Cette méthode est récursive (on appelle
	/// la méthode du dossier parent).
	/// Cette méthode appelle la méthode ToFrame.
	///
	/// \return le chemin de l'objet pour le butinage.
	///
	virtual TDCLNSRef	GetPath( void );

	///
	/// Récupère le dossier parent de l'élément.
	/// Ce dossier peut être \c nil si l'élément est l'élément racine.
	/// Ce dossier est créé si nécessaire et appartient à l'objet FSItem.
	/// Note: cette méthode ne doit être dérivée que dans la classe TDCLRootFolder.
	///
	/// \return un objet représentant le dossier parent ou \c nil.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef	GetParentFolder( void );

	///
	/// Récupère la description de l'élément sous forme de structure.
	/// Cette description est utilisée pour:
	/// - le chemin
	/// - la liste des éléments d'un dossier
	///
	/// \return la description de l'objet pour le butinage.
	///
	virtual TDCLNSRef	ToFrame( void ) = 0;

protected:
	///
	/// Change le nom de l'élément en interne.
	/// Cette méthode ne doit être appelée que si le nom (mName) vaut nil.
	/// Il ne s'agit pas de renommer le fichier.
	///
	/// \param inName	le nom de l'élément, en UTF-16 (cette chaîne appartient
	///					ensuite à l'objet TDCLFSItem et sera libérée avec free).
	/// \throw TDCLException si le nom existe déjà.
	///
	void				SetName( KUInt16* inName );

	///
	/// Constructeur à partir du type d'élément et d'un dossier parent (optionel)
	/// La référence du dossier parent est incrémentée.
	///
	/// \param inFilesIntf		interface sur les fichiers.
	/// \param inItemClass		classe de l'élément.
	/// \param inParentFolder	dossier parent (\c nil signifie que MakeParentFolder sera
	///							appelé plus tard).
	///
	TDCLFSItem(
			IDCLFiles* inFilesIntf,
			EFSItemClass inItemClass,
			TDCLFSItemRef inParentFolder );
	
	///
	/// Accesseur sur l'interface pour les fichiers.
	///
	/// \return l'interface pour les fichiers.
	///
	inline IDCLFiles* GetFilesIntf( void ) const
		{
			return mFilesIntf;
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFSItem( const TDCLFSItem& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFSItem& operator = ( const TDCLFSItem& inCopy );

	///
	/// Construit l'objet TDCLFolder parent. Cette méthode n'est appelée
	/// que si nécessaire.
	///
	/// \return un objet représentant le dossier parent ou \c nil.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual TDCLFSItemRef	MakeParentFolder( void ) const = 0;

	///
	/// Construit le nom de l'objet. C'est une chaîne unicode UTF-16 qui est allouée
	/// avec malloc. Cette méthode n'est appelée que si nécessaire (au plus une fois).
	///
	/// \return le nom alloué avec malloc.
	/// \throw TDCLException si un problème est survenu.
	///
	virtual KUInt16*		MakeName( void ) const = 0;

	/// \name Variables
	IDCLFiles*		mFilesIntf;		///< Interface pour les fichiers.
	EFSItemClass	mClass;			///< Classe de l'élément.
	TDCLFSItemRef	mParentFolder;	///< Parent de l'élément (\c nil tant qu'il n'est
									///< pas créé)
	KUInt16*		mName;			///< Nom de l'élément (\c nil tant qu'il n'est
									///< pas créé)
};

#endif
		// __TDCLFSITEM__

// ==================================================================== //
// If the code and the comments disagree, then both are probably wrong. //
//                 -- Norm Schryer                                      //
// ==================================================================== //
