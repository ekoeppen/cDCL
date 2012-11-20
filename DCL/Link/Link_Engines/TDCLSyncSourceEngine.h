// ==============================
// Fichier:			TDCLSyncSourceEngine.h
// Projet:			Desktop Connection Library
// 
// Créé le:			28/12/2003
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
// The Original Code is TDCLSyncSourceEngine.h.
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
// $Id: TDCLSyncSourceEngine.h,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLSYNCSOURCEENGINE_H
#define _TDCLSYNCSOURCEENGINE_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Link/Link_Engines/TDCLSyncEngine.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Interface pour gérer la synchronisation entre une source et une ou plusieurs
/// soupes.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLSyncSourceEngine
{
public:
	///
	/// Politique globale pour la synchronisation.
	///
	enum
	{
		kSameKey_ConsiderFirstOnly	= 0,	///< En cas d'enregistrements multiples
											///< avec une clé donnée, considère uniquement
											///< le premier
		kSameKey_ConsiderNewestOnly	= 1,	///< Idem, mais avec le plus récent
		kSameKey_DeleteOthers		= 2,	///< Supprime les autre clés
		kSameKey_AskUser			= 4,	///< Demande à l'utilisateur en cas
											///< d'enregistrements avec la même clé.
		kOnlyOnNewtonCopyToMac		= 0,	///< Copie les enregistrements seulement sur
											///< le Newton
		kOnlyOnNewtonSkip			= 8,	///< Ignore les enregistrements seulement sur
											///< le Newton
		kOnlyOnNewtonDelete			= 16,	///< Supprime les enregistrements seulement
											///< sur le Newton
		kOnlyOnMacCopyToNewton		= 0,	///< Copie les enregistrements seulement sur
											///< le Mac.
		kOnlyOnMacSkip				= 32,	///< Ignore les enregistrements seulement sur
											///< le Mac.
		kOnlyOnMacDelete			= 64	///< Supprime les enregistrements seulement
											///< sur le Mac.
	};

	///
	/// État d'un enregistrement provenant du Newton
	///
	enum ESyncModification
	{
		kCompleteSync		= 0,	///< La synchronisation est complète
		kNewEntry			= 1,	///< Nouvel enregistrement depuis la dernière
									///< synchronisation
		kModifiedEntry		= 2,	///< Enregistrement modifié depuis la dernière
									///< synchronisation
		kDeletedEntry		= 3		///< Enregistrement supprimé depuis la dernière
									///< synchronisation
	};

	///
	/// Résultat d'une synchronisation avec un élément.
	///
	enum ESyncResult
	{
		kUnchanged		= 0,	// L'entrée n'a pas été modifiée.
		kModified		= 1,	// L'entrée a été modifiée.
		kDeleted		= 2		// L'entrée doit être supprimée.
	};
	
	///
	/// Initialisation du moteur avant de commencer la synchronisation.
	/// Retourne le numéro de la version. Si la version ne correspond pas
	/// aux données enregistrées, on procèdera à une synchronisation complète.
	///
	/// \param inNewtonTime	heure courante sur le Newton.
	/// \return le numéro de la version.
	///
	virtual KUInt16		Init( KSInt32 inNewtonTime ) = 0;
	
	///
	/// Vérifie que deux enregistrements n'ont pas la même clé.
	/// Cette méthode est appelée tant qu'elle retourne vrai.
	///
	/// \param outSummaries		tableau des résumés des enregistrements, par ordre
	///							de date de modification (TDCLNSArray de TDCLNSString)
	/// \param outKey			clé partagée par ces deux enregistrements
	/// \return \c true si deux enregistrements ont la même clé, \c false sinon.
	///
	virtual bool		NextCommonKey(
								TDCLNSRef* outSummaries,
								TDCLNSRef* outKey ) = 0;
	
	///
	/// Méthode appelée pour régler un conflit avec les clés détecté avec la précédente
	/// méthode.
	///
	/// \param inKey			clé concernée.
	/// \param inRecordIndex	indice de l'enregistrement conservé.
	/// \param inDeleteOthers	si les autres enregistrements doivent être supprimés.
	///
	virtual void		HandleCommonKey(
								const TDCLNSRef& inKey,
								KUInt32 inRecordIndex,
								bool inDeleteOthers ) = 0;

	///
	/// Compare deux clés.
	///
	/// \param inFirstKey	première clé à comparer
	/// \param inSecondKey	deuxième clé à comparer
	/// \return \c 0 si les deux clés sont identiques, un entier négatif
	///			si \c inFirstKey est inférieure à \c inSecondKey,
	///			un entier positif dans le cas contraire.
	///
	virtual int			CompareKeys(
								const TDCLNSRef& inFirstKey,
								const TDCLNSRef& inSecondKey ) = 0;

	///
	/// Détermine la liste des soupes à synchroniser.
	///
	/// \return une liste de soupes à synchroniser (dans un tableau TDCLNSArray,
	///			chaque élément est un TDCLNSString).
	///
	virtual TDCLNSRef	GetSoupNames( void ) = 0;

	///
	/// Détermine si les données de la source ont une date de modification, auquel
	/// cas on peut optimiser la synchronisation.
	///
	/// \return \c true si on peut optimiser la synchronisation en utilisant la date
	///			de la dernière synchronisation.
	///
	virtual Boolean		HasModificationDate( void ) = 0;

	///
	/// Retourne les données de synchronisation telles que sauvegardées par
	/// SetSyncEngineData. Ces données permettent de savoir quel est l'état
	/// de la synchronisation la dernière fois qu'elle a eu lieu. Ces données sont
	/// gérées par TDCLSyncEngine et peuvent être sérialisées avec les encodeurs/
	/// décodeurs dérivés de TDCLNSCodec.
	/// Si aucune donnée n'a encore été enregistrée, il faut retourner
	/// \c NILREF.
	///
	/// \return les données de synchronisation telles que sauvegardées par
	/// la source.
	///
	virtual TDCLNSRef	GetSyncEngineData( void ) = 0;

	///
	/// Enregistre les données de synchronisation pour la prochaine
	/// synchronisation.
	///
	/// \param inSyncData	données de synchronisation à enregistrer.
	///
	virtual void		SetSyncEngineData( const TDCLNSRef& inSyncData ) = 0;

	///
	/// Détermine si un enregistrement sur le Newton doit être synchronisé.
	///
	/// \param inSoupName		nom de la soupe à laquelle appartient l'enregistrement.
	/// \param inNewtonEntry	enregistrement de la soupe.
	/// \return \c true si l'enregistrement doit être synchronisé.
	///
	virtual Boolean		FilterEntry(
								const TDCLNSString& inSoupName,
								const TDCLNSFrame& inNewtonEntry ) = 0;

	///
	/// Récupère une clé pour un enregistrement sur le Newton donné.
	///
	/// \param inSoupName		nom de la soupe à laquelle appartient l'enregistrement.
	/// \param inNewtonEntry	enregistrement de la soupe dont on veut une clé.
	///
	virtual TDCLNSRef	GetEntryKey(
								const TDCLNSString& inSoupName,
								const TDCLNSFrame& inNewtonEntry ) = 0;

	///
	/// Extraction d'un résumé de l'entrée dans une chaîne NS.
	///
	/// \param inSoupName		nom de la soupe à laquelle appartient l'enregistrement.
	/// \param inNewtonEntry	enregistrement de la soupe dont on veut un résumé.
	/// \return le résumé de l'entrée (chaîne NS)
	///
	virtual TDCLNSRef	GetEntrySummary(
								const TDCLNSString& inSoupName,
								const TDCLNSFrame& inNewtonEntry ) = 0;

	///
	/// Synchronise un enregistrement ajouté au Newton depuis la dernière synchronisation.
	/// Cette méthode a comme paramètre la clé donnée par \c GetEntryKey.
	///
	/// \param inModification		type de modification depuis la dernière
	///								synchronisation.
	/// \param inNewtonLastSyncTime	date de la dernière synchronisation sur le
	///								Newton.
	/// \param inSoupName			nom de la soupe.
	/// \param inEntryKey			clé pour l'enregistrement fournie par
	///								GetEntryKey.
	/// \param inNewtonEntry		enregistrement de la soupe à synchroniser,
	///								ou \c nil si l'entrée a été supprimée ou
	///								n'a pas été modifiée.
	/// \return un code qui détermine ce que le moteur doit faire avec cette
	///			entrée. Si inNewtonEntry était \c nil et que cette méthode
	///			retourne kModified, l'entrée sera téléchargée du Newton pour
	///			être modifiée.
	///
	virtual ESyncResult	SyncNewtonEntry(
								ESyncModification inModification,
								KSInt32 inNewtonLastSyncTime,
								const TDCLNSString& inSoupName,
								const TDCLNSRef& inEntryKey,
								const TDCLNSRef& inNewtonEntry ) = 0;

	///
	/// Récupère une nouvelle entrée à ajouter ou \c NILREF.
	/// Cette méthode n'est appelée que pour le magasin par défaut.
	/// Elle est appelée tant que le résultat n'est pas \c NILREF.
	///
	/// \return une nouvelle entrée à ajouter à cette soupe ou \c NILREF.
	///
	virtual TDCLNSRef	GetNextNewEntry( void ) = 0;
};

#endif
		// _TDCLSYNCSOURCEENGINE_H

// =============== //
// PURGE COMPLETE. //
// =============== //
