// ==============================
// Fichier:			TDCLCommLayer.h
// Projet:			Desktop Connection Library
//
// Créé le:			23/08/2000
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
// The Original Code is TDCLCommLayer.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLCommLayer.h,v 1.4 2004/11/24 14:08:53 paul Exp $
// ===========

#ifndef __TDCLCOMMLAYER__
#define __TDCLCOMMLAYER__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>

// Pré-déclarations
class TDCLPipe;
class TDCLServer;

///
/// Classe abstraite pour les couches de communication.
/// Ceci est la classe de base pour les couches asynchrones. Les couches
/// synchrones doivent dériver de TDCLSyncCommLayer.
///
/// Les couches de communications comprennent:
/// - des utilitaires concernant la couche (surtout une fonction pour
///		détermine si la couche est disponible)
/// - un serveur qui gère une ou plusieurs connexions.
///
/// Par asynchrone, j'entends qu'il y a un mécanisme de rappel de la part
/// du système. Exemple typique: OpenTransport avec la fonction 'Notify'.
/// Par synchrone, j'entends qu'il n'y a pas de tel mécanisme. Exemple typique:
/// bsd sockets. Dans ce cas, le mieux est d'utiliser les appels bloquants.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \todo	revoir les temporisations.
/// \test	UTestCommLayers
///
class TDCLCommLayer
{
public:
	enum {
		kDefaultPipeCount	=	0	///< nombre maximal de connexions par
									///< défaut. 0 veut dire pas de limite.
	};
	
	/// Constantes pour le délai de temporisation.
	enum {
		kNoTimeout			=	-1, ///< pas de temporisation
		kDefaultTimeout		=	0	///< temporisation par défaut (dépend de la
									///< couche de communication)
	};

	///
	/// Constructeur.
	///
	/// Enregistre le nombre maximum de connexion dans \c mMaxPipeCount.
	/// Enregistre aussi \p inFlushAvailable et \p inTimeout dans les champs
	/// privés correspondants.
	///
	/// \remark ce constructeur est public, mais la classe est abstraite.
	///
	/// \param inFlushAvailable	si la couche de communication sait vider la
	///							mémoire tampon de sortie.
	///							Si la couche de communication ne sait pas le
	///							faire, l'appelant devra attendre avant de
	///							fermer la connexion.
	/// \param inMaxPipeCount	nombre maximum de connexion simultanées.
	///							0 signifie pas de limite.
	///
						TDCLCommLayer(
								Boolean	inFlushAvailable = false,
								KUInt32	inMaxPipeCount = kDefaultPipeCount
							);
	
	///
	/// Destructeur.
	/// Celui-ci ne fait rien. Le destructeur des sous-classes ne doit pas
	/// supprimer les connexions qui restent, c'est à l'appelant de le faire.
	/// Cependant, l'appelant doit pouvoir fermer toutes les connexions,
	/// supprimer la couche de communication et ne supprimer les objets
	/// connexions qu'après.
	///
	virtual				~TDCLCommLayer( void ) {}
	
	/// \name interface serveur

	///
	/// Commence à écouter.
	/// Cette méthode appelle DoStartListening (qui est la méthode à
	/// surcharger).
	///
	/// \param inServer	serveur à prévenir lorsqu'une requête est arrivée.
	/// \throws TDCLException si le serveur est déjà en train d'écouter.
	///
	virtual void		StartListening( TDCLServer* inServer );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	/// Cette méthode appelle DoStopListening (qui est la méthode à
	/// surcharger).
	/// Ne fait rien si on n'est pas en train d'écouter.
	///
	virtual void		StopListening( void );

	///
	/// Retourne un objet connexion représentant le canal de communication avec
	/// le client ou \c nil s'il n'y a pas de requête de connexion présente.
	/// C'est un objet créé avec \c new. L'appelant devra le supprimer.
	///
	/// \return un nouvel objet TDCLPipe créé avec \c new représentant la
	///			connexion ou \c nil.
	///
	virtual	TDCLPipe*	Accept( void ) = 0;
	
	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	///
	virtual	void		Refuse( void ) = 0;

	///
	/// Détermine si la couche de communication sait vider la mémoire tampon de
	/// sortie.
	///
	/// Certaines couches de communication ne savent pas le faire.
	///
	/// \return \c true si la couche de communication sait le faire, \c false
	/// sinon.
	///
	inline	Boolean		FlushAvailable( void )
		{
			return mFlushAvailable;
		}
	
	///
	/// Méthode appelée pour perdre du temps.
	///
	/// Elle est appelée (par l'implémentation par défaut de TDCLPipe::Idle par
	/// exemple) à intervalles réguliers.
	///
	/// L'implémentation par défaut ne fait rien.
	///
	virtual	void		Idle( void );

	///
	/// Change la temporisation pour les nouvelles connexions.
	/// La temporisation est exprimée au format TDCLCommLayer.
	///
	/// \param inTimeout	nouvelle temporisation au format \c TDCLCommLayer
	///
	virtual	void		SetTimeout( long inTimeout ) = 0;

	///
	/// Récupère la temporisation utilisée pour les nouvelles connexions.
	/// La temporisation est exprimée au format TDCLCommLayer.
	///
	/// \return la temporisation au format \c TDCLCommLayer
	///
	virtual	long		GetTimeout( void ) = 0;

	///
	/// Méthode appelée par TDCLPipe::Disconnect() pour indiquer qu'une
	/// connexion a été fermée. Décrémente le compteur des connexions ouvertes.
	///
	/// \param inPipe	connexion qui a été fermée
	///
	virtual	void		Disconnected( TDCLPipe* inPipe ) = 0;

protected:
	///
	/// Commence effectivement à écouter.
	///
	virtual	void		DoStartListening( void ) = 0;

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	///
	virtual	void		DoStopListening( void ) = 0;


	///
	/// Sélecteur sur mFlushAvailable.
	///
	/// \param inNewFlushAvailable	\c true pour dire que cette couche de
	/// 							communication sait vider la mémoire tampon
	///								de sortie, \c false sinon.
	///
	inline	void		SetFlushAvailable( Boolean inNewFlushAvailable )
		{
			mFlushAvailable = inNewFlushAvailable;
		}

	///
	/// Accesseur sur le nombre maximum de connexions ouvertes.
	///
	/// \return le nombre maximum de connexions que le serveur peut gérer à la
	///			fois.
	///
	inline	KUInt32		GetMaxPipeCount( void ) const
		{
			return mMaxPipeCount;
		}
		
	///
	/// Sélecteur sur le nombre maximum de connexions ouvertes.
	///
	/// \param inMaxPipeCount	nombre maximum de connexions que le serveur
	///							peut gérer à la fois.
	///
	inline	void		SetMaxPipeCount( KUInt32 inMaxPipeCount )
		{
			mMaxPipeCount = inMaxPipeCount;
		}

	///
	/// Accesseur sur le serveur associé (\c nil lorsqu'on n'écoute pas).
	///
	/// \return le serveur associé à cette couche de communication.
	///
	inline	TDCLServer*	GetServer( void ) const
		{
			return mServer;
		}
		
	///
	/// Sélecteur sur le serveur associé.
	/// (utilisé uniquement si StartListening est surchargée).
	///
	/// \param inServer	nouveau serveur
	///
	inline	void		SetServer( TDCLServer* inServer )
		{
			mServer = inServer;
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLCommLayer( const TDCLCommLayer& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLCommLayer& operator = ( const TDCLCommLayer& inCopy );

	Boolean				mFlushAvailable;	///< Si cette couche de
											///< communication sait vider la
											///< mémoire tampon.
	KUInt32				mMaxPipeCount;		///< Nombre maximum de connexion
											///< que ce serveur peut gérer à la
											///< fois.
	TDCLServer*			mServer;			///< Serveur associé à l'interface
											///< de communication.
};

#endif
		// __TDCLCOMMLAYER__

// ====================================================================== //
// Il y en a qui ne savent pas déballer leur ordinateur de la boîte       //
// d'emballage. Faudrait aussi prévoir une doc là-dessus (parce que celle //
// fournie avec la boîte, il y a plein de mots et pas beaucoup d'images)  //
// -+- Jaco in Guide du Linuxien pervers - "[OUI] à fcol.deballage" -+-   //
// ====================================================================== //
