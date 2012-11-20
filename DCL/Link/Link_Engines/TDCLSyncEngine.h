// ==============================
// Fichier:			TDCLSyncEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			7/4/2003
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
// The Original Code is TDCLSyncEngine.h.
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
// $Id: TDCLSyncEngine.h,v 1.7 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef _TDCLSYNCENGINE_H
#define _TDCLSYNCENGINE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLLinkEngine.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSString.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

// Pré-déclarations.
class TDCLSyncSourceEngine;

///
/// Classe pour le moteur de synchronisation.
///
/// Ce moteur fonctionne avec des sources.
/// Les synchronisations sont totales ou partielles. Elles peuvent être
/// partielles (optimisées) lorsqu'elles ont été faites dans le passé,
/// grâce aux commandes qui demandent au Newton la liste des entrées et
/// la liste des entrées modifiées (on en déduit ainsi la liste des
/// entrées supprimées).
///
/// Une synchronisation non optimisée procède ainsi:
/// - on commence (généralement) par dresser une liste des entrées sur le
/// serveur
/// - on télécharge ensuite les entrées du Newton et on filtre celles qui
/// sont concernées par la synchronisation.
/// - on vérifie ensuite que les entrées concernées ont toutes une clé unique.
/// Dans le cas contraire, on ne traite qu'une entrée par clé.
/// - on synchronise ensuite les entrées du Newton une par une. La source peut
/// modifier, supprimer ou laisser une entrée inchangée. Lors d'une
/// synchronisation non optimisée, la source n'a aucune donnée sur le serveur
/// et donc elle ne fait que modifier (lors d'une unification) ou laisser
/// inchangées les entrées.
/// - on ajoute enfin les nouvelles entrées sur le Newton (les entrées du
/// serveur qui n'avaient pas d'équivalent).
///
/// Une synchronisation optimisée procède ainsi:
/// - on dresse la liste des entrées sur le serveur ainsi que celles qui
/// ont été modifiées, qui ont été créées et celles qui ont été supprimées.
/// - on récupère du Newton la liste des entrées et des entrées modifiées et on
/// en déduit la liste des entrées supprimées et des nouvelles entrées.
/// - on télécharge les nouvelles entrées et les entrées modifiées. On filtre
/// les nouvelles entrées. On vérifie que les entrées modifiées ne sont pas
/// filtrées, sinon on considère qu'il s'agit d'une suppression. On vérifie
/// qu'elles ont la même clé qu'avant, sinon on considère qu'il s'agit d'une
/// suppression et d'une création.
/// - on synchronise toutes les entrées du Newton, mais on fournit \c nil à la
/// méthode SyncNewtonEntry pour l'entrée si l'entrée du Newton a été supprimée
/// ou n'a pas été modifiée (auquel cas elle n'a pas été téléchargée).
/// - on ajoute enfin les nouvelles entrées sur le Newton (les nouvelles
/// entrées du serveur qui n'avaient pas d'équivalent).
/// 
/// Chaque source garde trace de ce qui a été synchronisé. Ces données sont
/// gérées par la présente classe, avec le format suivant:
///
/// {
/// 	formatMajor: 1,
///		formatMinor: 0,
///		version: _entier_,						// version de la source.
///		newtons: [
///			{
///				newtonID: _newton ID_,			// (binaire de 32 bits)
///				internalStoreSig: _signature_,	// entier Newton
///				stores: [
///					{
///						signature: _signature_, // entier Newton
///						lastSyncDate: _date_,	// (binaire de 32 bits)
///												// (date fournie par le Newton)
///						soups: [
///							{
///								name: _nom_,	// chaîne NS
///								entries: [
///								{
///									index: x,	// indice (entier)
///									key: cle,	// clé
///								}, ...
///								]
///							},
///							// [...]
///						],
///					},
///					// [...]
///				]
///			},
///			// [...]
///	}
///	
/// Diagramme d'états (par rapport au Newton):
///					
/// \image html TDCLSyncEngine-State-Diagram.png
/// \image latex TDCLSyncEngine-State-Diagram.eps "TDCLSyncEngine State Diagram" width=20cm
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLSyncEngine
	:
		public TDCLLinkEngine
{
public:
	///
	/// Constructeur à partir d'un lien.
	///
	/// \param inLink	référence sur le lien.
	///
	TDCLSyncEngine( TDCLFullDockLink* inLink );
	
	///
	/// Destructeur. Libère la mémoire.
	///
	~TDCLSyncEngine( void );
	
	///
	/// Méthode appelée par le lien pour commencer la synchronisation.
	/// Par défaut demande les options de synchronisation au Newton et
	/// initialise le tableau avec les résultats.
	///
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	void	StartSynchronizing( void );
	
	///
	/// Interrompt la synchronisation.
	/// L'interruption est asynchrone.
	///
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	void	StopSynchronizing( void );
	
	///
	/// Méthode appelée pour gérer des commandes venant du Newton.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return \c true si le moteur continue, \c false sinon
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	Boolean	ProcessDockCommand(
									TDCLDockCommand* inCommand,
									Boolean* outProcessed );

	///
	/// Progression de la synchronisation.
	///
	enum EProgressState {
		kSynchronizing1xPackages,	///< Synchronisation des paquets 1.x.
									///< - source = nil
									///< - entry index/count: indice sur les
									///<   paquets.
		kSynchronizingPatches,		///< Synchronisation des rustines.
									///< - source = nil
									///< - entry index/count: indice sur les
									///<   rustines.
		kFetchingEntries,			///< Récupération des entrées.
		kModifyingEntries,			///< Modification des entrées.
		kAddingEntries				///< Ajout de nouvelles entrées.
									///< - store index/count sans signification
	};

protected:
	///
	/// Etat du moteur de synchronisation.
	///
	enum EState {
		kUninitialized,				///< StartSynchronizing n'a pas été appelé.
		kWFOptions,					///< On attend les options du Newton
		kWFStoreNames,				///< On attend les noms des magasins.
		kWFPreliminaryTime,			///< On attend la première commande
									///< kDCurrentTime.
		kWFSetStoreResult,			///< On vient de changer de magasin et on
									///< attend le résultat (rustines & paquets).
		kWFPackageIDs,				///< On attend les identifiants des paquets.
		kWFSetStoreResult2,			///< On vient de changer de magasin et on
									///< attend le résultat (sources).
		kWFStoreSyncTime,			///< On attend la commande kDCurrentTime
									///< pour le magasin (sources).
		kWFSoupInfo,				///< On attend les informations sur une
									///< soupe (avant de la synchroniser).
		kWFSoupIDs,					///< On attend la liste de toutes les
									///< entrées.
		kWFChangedIDs,				///< On attend la liste des entrées
									///< modifiées.
		kWFEntry,					///< On attend la prochaine entrée.
		kWFSetStoreToDefaultResult,	///< On attend le résultat après avoir
									///< changé le magasin courant.
		kWFModifiedResult,			///< On attend le résultat de la
									///< modification ou de la suppression.
		kWFAddedID,					///< On attend confirmation pour l'ajout.
		kEnd,						///< Fin
		kInterrupted				///< Interruption
	};
	
	///
	/// Si on synchronise les informations système (les rustines).
	/// Par défaut, on ne les synchronise pas.
	///
	/// \return \c true si on synchronise les informations système, \c false
	/// 		sinon.
	///
	virtual Boolean				DoSynchronizePatches( void );

	///
	/// Si on synchronise les paquets.
	/// Uniquement appelée si mDoPackages vaut \c true.
	/// Par défaut, on ne les synchronise pas.
	///
	/// \return \c true si on synchronise les paquets, \c false sinon.
	///
	virtual Boolean				DoSynchronizePackages( void );

	///
	/// (Re)met le curseur sur les sources au début.
	/// Par défaut, ne fait rien.
	///
	virtual void				ResetSourceCursor( void );

	///
	/// Récupère un pointeur sur la source suivante ou \c nil s'il n'y a plus
	/// de source.
	/// Par défaut, retourne \c nil.
	///
	virtual TDCLSyncSourceEngine*	GetNextSourceEngine( void );

	///
	/// Etat de la progression.
	/// Par défaut ne fait rien.
	///
	/// \param inCurrentState		état de la progression.
	/// \param inCurrentSource		\c nil ou la source courante.
	/// \param inStoreIndex			\c 0 ou indice du magasin.
	/// \param inStoreCount			\c 0 ou nombre de magasins.
	/// \param inEntryIndex			\c 0 ou indice de l'entrée.
	/// \param inEntryCount			\c 0 ou nombre d'entrées.
	///
	virtual void				SyncProgress(
									EProgressState inCurrentState,
									TDCLSyncSourceEngine* inCurrentSource,
									KUInt32 inStoreIndex,
									KUInt32 inStoreCount,
									KUInt32 inEntryIndex,
									KUInt32 inEntryCount );

	///
	/// Passe à la prochaine source.
	///
	/// \return le nouvel état, \c kEnd si on a fini.
	///
	EState						SyncNextSource( void );

	///
	/// Détermine la prochaine soupe (éventuellement, change de magasin ou
	/// termine la synchronisation) et envoie ce qu'il faut au Newton pour
	/// synchroniser cette soupe.
	///
	/// \return le nouvel état, \c kEnd si on a fini.
	///
	EState						SyncNextSoup( void );

	///
	/// Récupère l'entrée suivante de la soupe.
	///
	/// \return le nouvel état, \c kEnd si on a fini.
	///
	EState						FetchNextEntry( void );

	///
	/// Synchronise les entrées.
	/// Commence par gérer les entrées avec les mêmes clés.
	///
	/// \return le nouvel état, \c kEnd si on a fini.
	///
	EState						SyncEntries( void );

	///
	/// Synchronise l'entrée suivante.
	///
	/// \return le nouvel état, \c kEnd si on a fini.
	///
	EState						SyncNextEntry( void );

	///
	/// Accesseur sur les options.
	/// C'est un objet envoyé par le Newton.
	///
	/// \return une référence sur les options.
	///
	inline TDCLNSRef&			GetSyncOptions( void )
		{
			return mSyncOptions;
		}

	///
	/// Accesseur sur l'état.
	///
	/// \return l'état du moteur de synchronisation
	///
	inline EState				GetState( void ) const
		{
			return mState;
		}

	///
	/// Sélecteur sur l'état.
	///
	/// \param inNewState nouvel état du moteur de synchronisation
	///
	inline void					SetState( EState inNewState )
		{
			mState = inNewState;
		}

	///
	/// Accesseur sur l'heure du Newton.
	///
	/// \return la date au début de la synchronisation
	///
	inline KSInt32				GetNewtonTime( void ) const
		{
			return mNewtonTime;
		}

	///
	/// Normalise une adresse électronique Newton.
	/// Pour le moment, cette méthode n'appelle aucune fonction sur le Newton,
	/// mais à terme, il faudra sans doute appeler NormalizeAddress sur le
	/// Newton. Comme \c NormalizeAddress sur le Newton, si l'objet passé en
	/// paramètre n'est pas une chaîne, il est retourné tel quel.
	///
	/// \param inOriginalEmail	adresse électronique sur le Newton (sa classe
	///							est utilisée) pour détermine le système de
	///							courrier électronique associé.
	/// \return la chaîne originale ou une nouvelle chaîne normalisée.
	///
	static TDCLNSRef			NormalizeAddress(
									const TDCLNSRef& inOriginalEmail );

	///
	/// Récupère une représentation sous forme de chaîne du système de courrier
	/// électronique d'une adresse électronique donnée.
	/// Pour le moment, cette méthode n'appelle aucune fonction sur le Newton,
	/// mais à terme, il faudra sans doute regarder du côté de la variable globale
	/// EmailSystemsRegistrar (elle doit s'appeler comme ça).
	///
	/// \param inEmailAddress	adresse électronique sur le Newton (c'est sa
	///							classe qui compte)
	/// \return une chaîne représentant le système de courrier d'une adresse
	///			donnée ou NILREF si l'objet passé en entrée n'a pas l'air
	///			d'être une adresse électronique.
	///
	static TDCLNSRef			GetEmailSystemName(
									const TDCLNSRef& inEmailAddress );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLSyncEngine( const TDCLSyncEngine& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLSyncEngine& operator = ( const TDCLSyncEngine& inCopy );

	///
	/// Fonction de transition
	/// kUnitialized -> kWFOptions
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromUninitialized(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFOptions -> kWFStoreNames, kWFPreliminaryTime
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFOptions(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFStoreNames -> kWFPreliminaryTime
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFStoreNames(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFPreliminaryTime -> kWFSetStoreResult, kWFSetStoreResult2
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFPreliminaryTime(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFSetStoreResult -> kWFPackageIDs
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFSetStoreResult(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFPackageIDs -> exception.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFPackageIDs(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFSetStoreResult2 -> kWFStoreSyncTime, kWFSoupInfo
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFSetStoreResult2(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFStoreSyncTime -> kWFSoupInfo
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFStoreSyncTime(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFSoupInfo -> kWFSoupIDs
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFSoupInfo(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFSoupIDs -> kWFChangedIDs, kWFEntry
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFSoupIDs(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFChangedIDs -> kWFEntry
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFChangedIDs(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFEntry -> kWFEntry, kWFModifiedResult
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFEntry(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFModifiedResult -> kWFModifiedResult, kWFSetStoreToDefaultResult
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFModifiedResult(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFSetStoreToDefaultResult -> kWFAddedID
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFSetStoreToDefaultResult(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kWFAddedID -> kWFAddedID, kWFSoupInfo, kWFSetStoreResult2, kEnd
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromWFAddedID(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Fonction de transition
	/// kInterrupted -> kEnd
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée.
	/// \return le nouvel état
	/// \throws TDCLException si une erreur est survenue.
	///
	EState						TransitionFromInterrupted(
										TDCLDockCommand* inCommand,
										Boolean* outProcessed );

	///
	/// Crée une structure NewtonScript avec des informations sur le Newton
	/// connecté.
	/// Modifie aussi la variable mSyncDataStoresArray en conséquence.
	///
	/// \return une telle structure.
	///
	TDCLNSRef					CreateCurrentNewtonFrame( void );

	///
	/// Crée la structure NewtonScript pour les données de synchronisation de
	/// la source courante. Crée une structure pour le Newton courant.
	/// Stocke cette structure dans \c mSyncData.
	/// Modifie aussi la variable \c mSyncDataStoresArray en conséquence.
	///
	/// \param inSourceVersion	version de la source
	///
	void						CreateSyncData(
										const TDCLNSRef& inSourceVersion );

	///
	/// Recherche le Newton courant dans les données de synchronisation.
	/// S'il n'est pas présent, crée une structure et l'ajoute dans les
	/// données de synchronisation (\c mSyncData).
	/// Modifie la variable \c mSyncDataStoresArray.
	///
	void						LookForCurrentNewtonInSyncData( void );

	///
	/// Récupère la structure sur le magasin courant des données de
	/// synchronisation ou en crée une s'il n'y en a pas. Met à jour
	/// \c mSyncDataSoupsArray.
	///
	/// Retourne la structure sur le magasin courant dans les données de
	/// synchronisation.
	///
	/// \param outLastSyncDate	en sortie, date de la dernière synchronisation.
	///
	TDCLNSFrame&				GetCurrentStoreFrame(
										KUInt32* outLastSyncDate = nil );
	
	///
	/// Récupère la clé d'une entrée à partir de la liste des entrées des
	/// données de synchronisation. On en profite pour supprimer l'entrée de la
	/// liste dans les données de synchronisation (ceci afin de repérer les
	/// entrées qui ont été supprimées). Si l'entrée n'est pas dans la liste
	/// des entrées connues, retourne \c NILREF.
	///
	TDCLNSRef					GetEntryKeyFromSyncData(
										const TDCLNSRef& inID );
	
	/// \name Variables

	TDCLNSRef			mSyncOptions;	///< Options pour la synchronisation
										///< (envoyées par le Newton).
	Boolean				mDoPackages;	///< Si on fait les paquets.
	Boolean				mSyncAll;		///< Si on synchronise tout.
	TDCLNSRef			mStoresRef;		///< Liste des magasins (référence)
	TDCLNSArray*		mStores;		///< Liste des magasins
	KUInt32				mStoresCount;	///< Nombre de magasins à synchroniser.
	EState				mState;			///< Etat du moteur de synchronisation.
	KSInt32				mNewtonTime;	///< Heure sur le Newton.
	KUInt32				mStoreIndex;	///< Indice du magasin synchronisé.
	TDCLNSRef			mSoups;			///< Liste des soupes
	TDCLNSArray*		mSoupsArray;	///< Liste des soupes (idem, mais
										///< déréférencé)
	KSInt32				mSoupsCount;	///< Nombre de soupes à synchroniser.
	KSInt32				mSoupIndex;		///< Indice de la soupe synchronisée.
	TDCLNSRef			mSyncData;		///< Données de synchronisation pour la
										///< source.
	TDCLNSArray*		mSyncDataStoresArray;
										///< Tableau sur les magasins dans les
										///< données de synchronisation pour la
										///< source.
	TDCLNSArray*		mSyncDataSoupsArray;
										///< Tableau sur les soupes du magasin
										///< courant dans les données de
										///< synchronisation pour la source.
	TDCLNSArray*		mSyncDataEntriesArray;
										///< Tableau sur les entrées de la
										///< soupe courante dans les données de
										///< synchronisation pour la source.
	TDCLNSRef			mSoupInfo;		///< Information sur la soupe courante.
	TDCLNSString*		mSoupName;		///< Nom de la soupe.
	TDCLNSRef			mEntries;		///< Entrées de la soupe courante
	TDCLNSArray*		mEntriesArray;	///< Entrées de la soupe courante
										///< (déref.)
	KUInt32				mEntriesCount;	///< Nombre d'entrées de la soupe
										///< courante synchro.	
	KSInt32				mEntryIndex;	///< Indice sur les entrées.
	TDCLNSFrame*		mCurrentEntry;	///< Entrée courante (dans le tableau).
	TDCLSyncSourceEngine*
						mCurrentSourceEngine;
										///< Moteur pour la source en cours.
	Boolean				mAddingEntries;	///< Si on ajoute les entrées.
	Boolean				mCompleteSync;	///< Si on fait une synchronisation
										///< complète ou pas.
	Boolean				mInterrupted;	///< Si on a été interrompu.
};

#endif
		// _TDCLSYNCENGINE_H

// ============================================ //
// A nasty looking dwarf throws a knife at you. //
// ============================================ //
