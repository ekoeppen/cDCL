// ==============================
// Fichier:			TDCLSocket.h
// Projet:			Desktop Connection Library
//
// Créé le:			29/3/2003
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
// The Original Code is TDCLSocket.h.
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
// $Id: TDCLSocket.h,v 1.7 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef _TDCLSOCKET_H
#define _TDCLSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>

#if !TARGET_OS_COMPAT_POSIX
	#error TDCLSocket requiert la compatibilité POSIX
#endif

// POSIX
#include <sys/types.h>

///
/// Classe de base pour une couche de communication utilisant des "sockets".
/// Pour l'instant, on a deux sous-classes: MacOS X ADSP et BSD Sockets.
/// Ces couches de communication doivent utiliser select(3).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	UTestCommLayers
///
class TDCLSocket
	:
		public TDCLSyncCommLayer
{
public:
	///
	/// Constantes pour TDCLSocket
	///
	enum {
		kMaxSocketBackLog		= 128,	///< taille maximale de la file
										///< d'attente
		kSocketDefaultTimeout	= 30	///< (en secondes)
	};
	
	///
	/// Unique constructeur, à partir d'une interface pour les processus légers
	/// et du nombre maximum de connexions.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inMaxPipeCount	nombre maximal de connexion simultanées
	///							autorisées.
	/// \param inTimeout		temporisation maximale à attendre lors
	///							d'opération d'E/S (format \c TDCLCommLayer)
	///
	TDCLSocket(
		IDCLThreads*	inThreadsIntf,
		KUInt32			inMaxPipeCount = kDefaultPipeCount,
		long			inTimeout = kDefaultTimeout
		);

	///
	/// Appelle StopListening() et ferme les interfaces.
	/// Cette méthode doit être appelée par les destructeurs des sous-classes.
	///
	void				Destroy( void );

	/// \name interface serveur

	///
	/// Change la temporisation pour les nouvelles connexions.
	/// La temporisation est exprimée au format \c TDCLCommLayer.
	///
	/// \param inTimeout	nouvelle temporisation au format \c TDCLCommLayer
	///
	virtual	void		SetTimeout( long inTimeout );

	///
	/// Récupère la temporisation utilisée pour les nouvelles connexions.
	/// La temporisation est exprimée au format \c TDCLCommLayer.
	///
	/// \return la temporisation au format \c TDCLCommLayer
	///
	virtual	long		GetTimeout( void );

	///
	/// Méthode appelée par TPipe::Disconnect() pour indiquer qu'une connexion
	/// a été fermée. Décrémente le compteur des connexions ouvertes.
	///
	/// \param inPipe	connexion qui a été fermée
	///
	virtual	void		Disconnected( TDCLPipe* inPipe );

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
	/// Attend qu'une requête se présente.
	///
	/// \return \c true si une requête est présente, \c false si la méthode a
	///			été interrompue.
	///
	virtual	Boolean		WaitForIncomingRequest( void );

	///
	/// Retourne un objet connexion représentant le canal de communication
	/// avec le client ou \c nil s'il n'y a pas de requête de connexion
	/// présente.
	/// Ce doit être un objet créé avec new. L'appelant devra le supprimer.
	///
	/// \return	un nouvel objet TPipe créé avec new représentant la connexion
	///			ou \c nil.
	///
	virtual	TDCLPipe*	DoAccept( void );

	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	///
	virtual	void		DoRefuse( void );

	/// \name Interface TDCLSocket
	
	///
	/// Crée l'interface de communication pour écouter, la lie et écoute.
	///
	/// \param	inBackLogSize	taille de la liste des requêtes (peut être
	///			inférieure à cette valeur si la couche ne gère pas de liste
	///			aussi grande).
	/// \return une référence vers l'interface de communication ou un nombre
	///			négatif si une erreur est survenue. Le code de l'erreur est
	///			dans errno.
	///
	virtual int			SocketBindAndListen( KUInt32 inBackLogSize ) = 0;
	
	///
	/// Ferme une interface de communication.
	///
	/// \param inCookie	biscuit du client, \c nil pour le serveur.
	/// \param inSocket interface de communication à fermer.
	/// \return \c 0 ou un nombre négatif si une erreur est survenue. Le code
	///			de l'erreur est dans errno.
	///
	virtual int			DoClose( void* inCookie, int inSocket ) = 0;
	
	///
	/// Attend qu'une requête arrive et accepte cette requête.
	/// Si l'interface de communication est ensuite fermée, la requête sera
	/// refusée.
	///
	/// \param outCookie		en sortie, biscuit du client.
	/// \param inServerSocket	interface de communication qui attend la requête.
	/// \return la référence vers la nouvelle interface de communication pour
	///			le client, 0 si on a été interrompu ou un nombre négatif si une
	///			erreur est survenue. Le code de l'erreur est dans \c errno.
	///
	virtual int			DoAccept( void** outCookie, int inServerSocket ) = 0;
	
	///
	/// Configure une interface de communication (client).
	/// Par défaut ne fait rien.
	///
	/// \param inCookie	biscuit du client (fourni par DoAccept).
	/// \param inSocket interface de communication à configurer.
	/// \return un nombre négatif si une erreur est survenue, 0 si tout va
	///			bien.
	///
	virtual int			SetupClientSocket( void* inCookie, int inSocket );

	///
	/// Lit des octets depuis une interface de communication. L'interface
	/// doit attendre que le délai de temporisation soit écoulé avant de
	/// retourner une erreur si les données ne sont pas présentes.
	///
	/// \param inCookie			biscuit du client (fourni par DoAccept).
	/// \param inSocket			interface de communication où lire.
	/// \param outBuffer		mémoire tampon où mettre les octets lus.
	/// \param inCount			nombre d'octets à lire.
	/// \return le nombre d'octet effectivement lu ou un nombre négatif si une
	///			erreur est survenue. Le code de l'erreur est dans \c errno.
	///
	virtual int			Recv(
							void* inCookie,
							int inSocket,
							void* outBuffer,
							size_t inCount ) = 0;
	
	///
	/// Envoie des octets vers une interface de communication. L'interface
	/// doit attendre que le délai de temporisation soit écoulé avant de
	/// retourner une erreur si les données ne sont pas envoyées.
	///
	/// \param inCookie			biscuit du client (fourni par DoAccept).
	/// \param inClientSocket	interface de communication où écrire.
	/// \param inBuffer			mémoire tampon où se trouvent les octets à
	///							envoyer.
	/// \param inCount			nombre d'octets à lire.
	/// \return le nombre d'octet effectivement écrits ou un nombre négatif si
	///			une erreur est survenue. Le code de l'erreur est dans \c errno.
	///
	virtual int			Send(
							void* inCookie,
							int inClientSocket,
							const void* inBuffer,
							size_t inCount ) = 0;
	
	///
	/// Change la temporisation pour une interface de communication.
	/// La temporisation est exprimée en secondes, 0 signifie pas de
	/// temporisation.
	/// Par défaut ne fait rien, la temporisation est gérée dans les boucles
	/// de TSocketPipe::Read et de TSocketPipe::Write.
	///
	/// \param inCookie		biscuit du client (fourni par DoAccept).
	/// \param inSocket		interface de communication à régler.
	/// \param inTimeout	temporisation (en secondes)
	/// \return \c 0 si tout va bien ou un nombre négatif si une erreur est
	///			survenue. Le code de l'erreur est dans \c errno.
	///
	virtual	int			SetClientSocketTimeout(
							void* inCookie,
							int inSocket,
							long inTimeout );

	///
	/// Détermine la taille de la mémoire tampon de TDCLBufferedPipe à utiliser.
	/// Par défaut, utilise la taille par défaut.
	///
	/// \return la taille de la mémoire tampon pour TDCLBufferedPipe
	///
	virtual KUInt32		GetBufferedPipeBufferSize( void ) const;

	///
	/// Accesseur sur l'interface de communication qui écoute.
	///
	/// \return	la référence sur l'interface de communication du serveur
	///
	inline int			GetServerSocket( void )
		{
			return mServerSocket;
		}

private:
	///
	/// Droits pour la classe fille.
	///
	class TSocketPipe;
	friend class TSocketPipe;
	
	///
	/// Class pour les connexions.
	///
	class TSocketPipe
		:
			public TSyncPipe
	{
	public:
		/// La classe mère est une amie.
		friend class TDCLSocket;

		///
		/// Destructeur. Ferme la connexion si elle n'était pas déjà fermée.
		///		
		virtual				~TSocketPipe( void );

		///
		/// Détermine si des octets sont disponibles dans la mémoire tampon
		/// d'entrée.
		///
		/// \return \c true si des octets sont disponibles, \c false sinon.
		///
		virtual	Boolean		BytesAvailable( void );

		///
		/// Récupère la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des
		/// constantes de TDCLCommLayer (dit "format \c TDCLCommLayer").
		///
		/// \return la temporisation au format \c TDCLCommLayer.
		///
		virtual	long		GetTimeout( void );

		///
		/// Change la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des
		/// constantes de \c TDCLCommLayer.
		///
		/// \param inTimeout	temporisation au format \c TDCLCommLayer.
		///
		virtual	void		SetTimeout( long inTimeout );

		///
		/// Attend que des données se présentent. Cette méthode est synchrone
		/// (c'est l'intérêt de cette classe). Elle retourne si des données
		/// sont présentes ou si elle a été interrompue par Disconnect.
		///
		/// \return \c true si une requête est présente, \c false si la méthode
		///			a été interrompue.
		///
		virtual Boolean 	WaitForIncomingData( void );

		///
		/// Déconnecte le canal de communication avec le client.
		///
		/// \remark	une fois que cette méthode est appelée, l'objet connexion
		///			est inutilisable. Il peut être supprimé par l'appelant à
		///			tout instant.
		///
		virtual	void		DoDisconnect( void );
		
		///
		/// Lit des octets.
		///
		/// \param outBuffer	mémoire tampon pour les octets lus.
		/// \param ioCount		nombre d'octets à lire en entrée, lus en
		///						sortie. Cette valeur est mise à jour avant que
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
		
	protected:
		///
		/// Constructeur.
		/// Utilisé par la classe TDCLSocket.
		///
		/// \param inCookie			biscuit du client.
		/// \param inCommLayer		couche de communication à laquelle la
		///							connexion appartient.
		/// \param inClientSocket	descripteur de fichier de l'interface de
		///							connexion.
		/// \param inTimeout		temporisation pour cette connexion.
		///
		TSocketPipe(
				void*			inCookie,
				TDCLSocket*		inCommLayer,
				int				inClientSocket,
				long			inTimeout
			);

		///
		/// Accesseur sur le descripteur de fichier de l'interface de
		/// connexion.
		///
		/// \return descripteur de fichier de l'interface de connexion.
		///
		inline	int	GetClientSocket( void )
			{
				return mClientSocket;
			}
	
	private:
		/// \name Variables

		void*	 		mClientCookie;			///< Biscuit du client.
		int				mClientSocket;			///< Référence sur l'interface
												///< de communication.
		long			mTimeout;				///< Temporisation pour cette
												///< interface.
		int				mPublicPairMember;		///< Élément public de la
												///< "socket pair"
		int				mPrivatePairMember;		///< Élément privé (= thread)
												///< de la "socket pair"
		Boolean			mClientSocketIsClosed;	///< Si l'interface de
												///< communication est fermée.
	};
	
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLSocket( const TDCLSocket& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLSocket& operator = ( const TDCLSocket& inCopy );

	/// \name Variables privées.
	
	int			mServerSocket;			///< Descripteur de fichier de
										///< l'interface de communication du
										///< serveur.
	KUInt32		mCurrentPipeCount;		///< Nombre de connexions ouvertes.
	KUInt32		mBackLogSize;			///< Taille de la file d'attente de
										///< listen.
	long		mTimeout;				///< Temporisation au format
										///< \c TDCLCommLayer (nombre de
										///< secondes + constantes)
	int			mPublicPairMember;		///< Élément public de la "socket pair"
	int			mPrivatePairMember;		///< Élément privé (= thread) de la
										///< "socket pair"
	Boolean		mSocketsAreClosed;		///< Si les sockets sont fermées.
};

#endif
		// _TDCLSOCKET_H

// ========================================================================= //
//         The FIELD GUIDE to NORTH AMERICAN MALES                           //
//                                                                           //
// SPECIES:        Cranial Males                                             //
// SUBSPECIES:     The Hacker (homo computatis)                              //
// Description:                                                              //
//         Gangly and frail, the hacker has a high forehead and thinning     //
//         hair.  Head disproportionately large and crooked forward,         //
//         complexion wan and sightly gray from CRT illumination.  He has    //
//         heavy black-rimmed glasses and a look of intense concentration,   //
//         which may be due to a software problem or to a pork-and-bean      //
//         breakfast.                                                        //
// Feathering:                                                               //
//         HOMO COMPUTATIS saw a Brylcreem ad fifteen years ago and believed //
//         it.  Consequently, crest is greased down, except for the cowlick. //
// Song:                                                                     //
//         A rather plaintive "Is it up?"                                    //
// ========================================================================= //
