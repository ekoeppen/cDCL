// ==============================
// Fichier:			TDCLOpenTransport.h
// Projet:			Desktop Connection Library
//
// Créé le:			23/01/2002
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
// The Original Code is TDCLOpenTransport.h.
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
// $Id: TDCLOpenTransport.h,v 1.5 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef __TDCLOPENTRANSPORT__
#define __TDCLOPENTRANSPORT__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLCommLayer.h>

#if !TARGET_OS_MAC
	#error "OpenTransport est seulement disponible sur MacOS/MacOS X"
#endif

// Open Transport headers.
#if !CALL_NOT_IN_CARBON
	#ifndef OTCARBONAPPLICATION
		#define OTCARBONAPPLICATION 1
	#endif
#endif

#if TARGET_OS_MACOS
	// Bug dans les derniers UH: il faut Files.h
	#include <Files.h>
	#include <OpenTransport.h>
	#include <OpenTransportProviders.h>
#else
	#define OTOpenEndpoint(config, oflag, info, err) \
		OTOpenEndpointInContext(config, oflag, info, err, NULL)
	#define InitOpenTransport() \
		InitOpenTransportInContext(kInitOTForApplicationMask, NULL)
	#define CloseOpenTransport() \
		CloseOpenTransportInContext(NULL)
	#include <CoreServices/CoreServices.h>
#endif

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>

// Pré-déclarations
class IDCLThreads;

///
/// Classe pour la couche de communication OpenTransport.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \todo	revoir les temporisations.
/// \test	aucun test défini.
///
class TDCLOpenTransport
	:
		public TDCLCommLayer
{
public:
	///
	/// Constructeur publique pour cette classe.
	/// Le nombre de connexion et la temporisation par défaut sont définies
	/// comme des constantes. Les sous-classes utiliseront probablement plutôt
	/// l'autre constructeur.
	///
	/// \param inConfiguration	configuration pour cette couche de
	///							communication. Cette configuration est clonée.
	/// \param inLocalBindInfo	adresse du serveur. Copiée.
	/// \param inThreadsIntf	interface sur les processus légers
	/// \param inMaxPipeCount	nombre maximum de connexion ouvertes en même
	///							temps.
	/// \param inTimeout		temporisation.
	///
	TDCLOpenTransport(
			OTConfigurationRef	inConfiguration,
			TBind*				inLocalBindInfo,
			IDCLThreads*		inThreadsIntf,
			KUInt32				inMaxPipeCount = kDefaultPipeCount,
			long				inTimeout = kDefaultTimeout
		);

	///
	/// Destructeur.
	/// Arrête le serveur (i.e. d'écouter). Décrémente aussi le nombre de
	/// clients OpenTransport.
	///
	virtual				~TDCLOpenTransport( void );
	
	/// \name interface serveur

	///
	/// Retourne un objet connexion représentant le canal de communication
	/// avec le client ou \c nil s'il n'y a pas de requête de connexion
	/// présente.
	/// Ce doit être un objet créé avec new. L'appelant devra le supprimer.
	///
	/// \return	un nouvel objet TDCLPipe créé avec new représentant la connexion
	///			ou \c nil.
	///
	virtual	TDCLPipe*	Accept( void );

	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	///
	virtual	void		Refuse( void );

	///
	/// Cette méthode est appelée par TDCLPipe::Idle.
	/// Appelle OTIdle et mInterface->Yield.
	///
	virtual	void		Idle( void );

	///
	/// Change la temporisation pour les nouvelles connexions.
	/// La temporisation est exprimée au format TDCLCommLayer.
	///
	/// \param inTimeout	nouvelle temporisation au format \c TDCLCommLayer
	///
	virtual	void		SetTimeout( long inTimeout );

	///
	/// Récupère la temporisation utilisée pour les nouvelles connexions.
	/// La temporisation est exprimée au format TDCLCommLayer.
	///
	/// \return la temporisation au format \c TDCLCommLayer
	///
	virtual	long		GetTimeout( void );

	///
	/// Méthode appelée par TDCLPipe::Disconnect() pour indiquer qu'une
	/// connexion a été fermée. Décrémente le compteur des connexions ouvertes.
	///
	/// \param inPipe	connexion qui a été fermée
	///
	virtual	void		Disconnected( TDCLPipe* inPipe );

	/// \name utilitaires

	///
	/// Détermine si OpenTransport est disponible.
	///
	/// \return	\c true si OpenTransport est disponible, \c false sinon.
	///
	static	Boolean		IsAvailable( void );
	
	///
	/// Ferme OpenTransport s'il n'est plus utilisé, i.e. s'il n'y a plus
	/// d'instance de cette classe.
	/// Cette méthode est appelée par le destructeur. Cependant, si vous avez
	/// appelé \c IsAvailable pour une sous-classe qui peut avoir besoin de OT
	/// et que finalement, vous décidez de ne pas utiliser OT, c'est une bonne
	/// idée d'appeler cette méthode pour fermer OT. Ça ne peut pas faire de
	/// mal.
	///
	static	void		CloseOT( void );

protected:
	///
	/// Commence à écouter.
	///
	virtual	void		DoStartListening( void );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	///
	virtual	void		DoStopListening( void );

	///
	/// Initialise OpenTransport si requis et joue avec gOTState.
	///
	/// \throws TDCLNotAvailableException si OT n'est pas disponible.
	///
	static	void		InitOT( void );

	///
	/// Constructeur pour les sous-classes.
	/// Ce constructeur n'appelle pas Init. C'est pour les classes qui veulent
	/// gérer leur configuration OT d'abord et appeler Init ensuite.
	///
	/// \param inThreadsIntf	interface sur les processus légers.
	/// \param inMaxPipeCount	nombre maximal de connexion simultanées.
	/// \param inTimeout		délai de temporisation
	///
	TDCLOpenTransport(
					IDCLThreads* inThreadsIntf,
					KUInt32	inMaxPipeCount = kDefaultPipeCount,
					long	inTimeout = kDefaultTimeout
				);

	///
	/// Initialise la couche et incrémente le compteur de couches OT ouvertes.
	///
	/// \param inConfiguration	configuration pour OT
	/// \param inLocalBindInfo	adresse locale du serveur.
	///
	void		Init(
					OTConfigurationRef inConfiguration,
					TBind* inLocalBindInfo
				);

private:

	///
	/// Classe pour une connexion avec OpenTransport.
	///
	class TOTPipe
		:
			public TDCLPipe
	{
	public:
		///
		/// La classe principale peut accéder aux méthodes protégées et privées.
		///
		friend class TDCLOpenTransport;
		
		///
		/// Destructeur. Ferme la connexion si elle n'était pas déjà fermée.
		///
		virtual				~TOTPipe( void );

		///
		/// Lit des octets.
		///
		/// \param outBuffer	mémoire tampon pour les octets lus.
		/// \param ioCount		nombre d'octets à lire en entrée, lus en sortie.
		///						Cette valeur est mise à jour avant que
		///						l'exception ne soit lancée si un problème est
		///						survenu.
		/// \throws TDCLIOException	si un problème est survenu (autre que EOF)
		///
		virtual	void		Read( void* outBuffer, KUInt32* ioCount );

		///
		/// Ecrit des octets.
		///
		/// \param inBuffer		mémoire tampon pour les octets à écrire.
		/// \param ioCount		nombre d'octets à écrire en entrée, écris en
		///						sortie. Cette valeur est mise à jour avant que
		///						l'exception ne soit lancée si un problème est
		///						survenu.
		/// \throws TDCLIOException	si un problème est survenu.
		///
		virtual	void		Write( const void* inBuffer, KUInt32* ioCount );
		
		///
		/// Détermine quel est le nombre d'octets disponibles dans la mémoire
		/// tampon d'entrée.
		///
		/// \return le nombre d'octets disponibles.
		///
		virtual	Boolean		BytesAvailable( void );

		///
		/// Vide la mémoire tampon de sortie. Tout simplement.
		///
		virtual void		FlushOutput( void );
		
		///
		/// Déconnecte le canal de communication avec le client.
		///
		/// \remark	une fois que cette méthode est appelée, l'objet connexion
		///			est inutilisable. Il peut être supprimé par l'appelant à
		///			tout instant.
		///
		virtual	void		Disconnect( void );
		
		///
		/// Récupère la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des
		/// constantes de TDCLCommLayer (dit "format TDCLCommLayer").
		///
		/// \return la temporisation au format TDCLCommLayer.
		///
		virtual	long		GetTimeout( void );

		///
		/// Change la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des
		/// constantes de TDCLCommLayer.
		///
		/// \param inTimeout	temporisation au format TDCLCommLayer.
		///
		virtual	void		SetTimeout( long inTimeout );

		///
		/// Méthode appelée par le lien pour dire qu'il souhaite
		/// être prévenu dès que des données sont présentes.
		/// Au départ, l'interface de communication ne prévient
		/// pas le lien de la présence de données.
		///
		virtual	void		ClearDataPresent( void );

	protected:
		///
		/// Constructeur unique.
		/// Il est protégé parce que seule les couches de communication doivent
		/// créer des connexions.
		///
		/// \param inCommLayer	couche de communication à laquelle est
		///						rattachée la connexion.
		/// \param inEndpoint	interface de communication OpenTransport
		///						associée à la connexion.
		/// \param inTimeout	délai de temporisation au format TDCLCommLayer.
		///
		TOTPipe(
				TDCLOpenTransport*	inCommLayer,
				TEndpoint*			inEndpoint,
				long				inTimeout
				);

		///
		/// Récupère l'interface de communication OpenTransport associée à
		/// cette connexion.
		///
		/// \return l'interface de communication OpenTransport.
		///
		inline	TEndpoint*	GetEndpoint( void )
			{
				return mEndpoint;
			}
	
	private:
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TOTPipe( const TOTPipe& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TOTPipe& operator = ( const TOTPipe& inCopy );

		///
		/// Fonction de notification OpenTransport pour gérer la déconnexion.
		///
		/// \param inContext	contexte, ici objet connexion.
		/// \param inEvent		événement qui est l'objet de la notification.
		/// \param inResult		résultat de cet événement.
		/// \param inCookie		biscuit (inutilisé).
		static pascal void	Notify(
									void* inContext,
									OTEventCode inEvent,
									OTResult inResult,
									void* inCookie
								);

		/// \name Variables

		long			mTimeout;		///< temporisation au format
										///< TDCLCommLayer
		TEndpoint*		mEndpoint;		///< interface de communication
										///< OpenTransport pour cette
										///< connexion.
		Boolean			mDisconnected;	///< drapeau qui vaut true si la
										///< connexion a été fermée (par le
										///< client)
		OTNotifyUPP		mNotifierUPP;	///< UPP pour la fonction de
										///< notification.
		Boolean			mDataPresent;	///< Si on sait que des données sont
										///< présentes.
		
	};

	///
	/// La classe TOTPipe doit pouvoir accéder aux méthodes de cette classe.
	///
	friend class TOTPipe;

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLOpenTransport( const TDCLOpenTransport& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLOpenTransport& operator = ( const TDCLOpenTransport& inCopy );

	///
	/// Crée un nouveau bout de connexion à partir de la configuration et
	/// de l'addresse locale.
	///
	/// \param forListening	si le nouveau bout de connexion doit être pour
	///						écouter
	/// \return un nouveau bout de connexion OT
	///
	TEndpoint*			GetNewEndpoint( Boolean forListening );

	///
	/// Gestionnaire d'événements appelé par OT lors d'interruptions.
	///
	static pascal void	Notify(
								void* inContext,
								OTEventCode inEvent,
								OTResult inResult,
								void* inCookie );
	
	///
	/// Constantes pour cette classe.
	///
	enum {
		kOTMaxBackLogSize	=	128,	///< Nombre de requêtes simultanées
										///< gérées par Listen.
		kOTDefaultTimeout	=	30		///< Délai de temporisation par
										///< défaut (en secondes)
	};

	///
	/// Etat d'OpenTransport.
	///	
	enum EOTState {
		kUninitialized,		///< Etat initial
		kNotAvailable,		///< OT n'est pas disponible
		kInitialized,		///< OT est disponible et a été initialisé
		kAvailable			///< OT est disponiblel mais a été fermé (ou n'a
							///< pas été initialisé)
	};

	/// \name Variables

	OTConfigurationRef	mConfiguration;		///< Configuration pour les bouts
											///< de connexion.
	TBind				mLocalBindInfo;		///< Adresse locale.
	KUInt32				mCurrentPipeCount;	///< Nombre de connexions ouvertes.
	TEndpoint*			mServerEndpoint;	///< Bout de connexion du serveur.
	
	long				mTimeout;			///< Temporisation au format
											///< TDCLCommLayer (nombre de
											///< secondes + constantes)
	IDCLThreads*		mThreadsIntf;		///< Interface pour les processus
											///< légers (pour Yield)
	static KUInt32		gInstanceCount;		///< Compteur sur lesinstances de
											///< TDCLOpenTransport pour fermer
											///< OpenTransport lorsqu'il n'est
											///< plus utilisié.
											///< Note: some sub-classes might
											///< need to open OT to know if
											///< their service is available
											///< (typically Serial). In that
											///< case, OT will only be closed
											///< when last instance dies (or
											///< if you call
											///< CloseOpenTransport).
	static EOTState		gOTState;			///< Whether OpenTransport is
											///< available/initialized/closed.
	Boolean				mListening;			///< Whether we are listening or not
	OTNotifyUPP			mNotifierUPP;		///< Référence sur le gestionnaire
											///< d'événements.
};

#endif
		// __TDCLOPENTRANSPORT__

// ========================================================================== //
// The use of anthropomorphic terminology when dealing with computing systems //
// is a symptom of professional immaturity.                                   //
//                 -- Edsger Dijkstra                                         //
// ========================================================================== //
