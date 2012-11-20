// ==============================
// Fichier:			TDCLXADSP.h
// Projet:			Desktop Connection Library
//
// Créé le:			29/07/2002
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
// The Original Code is TDCLXADSP.h.
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
// $Id: TDCLXADSP.h,v 1.9 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef __TDCLXADSP__
#define __TDCLXADSP__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

#include <sys/types.h>
#include <sys/socket.h>

// AppleTalk dans Kernel.Framework
#include <netat/appletalk.h>
#include <netat/atp.h>

// AppleTalk dans AppleTalk.Framework
// (n'est pas dans le système mais dans Darwin)
extern "C" {
#include "at_proto.h"
}

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

#if !TARGET_OS_OPENSTEP
	#error "X ADSP n'est disponible que sur OS X"
#endif

///
/// Cette classe gère le lien ADSP sur MacOS X.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \author Nicolas Zinovieff <krugazor@poulet.org>
/// \version $Revision: 1.9 $
///
class TDCLXADSP
	:
		public TDCLSyncCommLayer
{
public:
	///
	/// Constantes pour la classe TDCLXADSP
	///
	enum {
		kMaxADSPPipeCount		= 1024,	///< nombre maximal de connexions par défaut.
										///< En ADSP/X, c'est 1024.
		kXADSPDefaultTimeout	= 30	///< Temps par défaut (en secondes)
	};

	///
	/// Unique constructeur, à partir d'une interface, d'un nom de machine,
	/// du nombre maximum de connexions et du timeout.
	///
	/// Le nombre de connexions et le timeout pour cette couche de
	/// communication sont définies comme des constantes.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inMachineName	nom de la machine, utilisée dans l'adresse
	///							NBP. nil signifie que l'on va chercher le nom dans
	///							les préférences systèmes.
	/// \param inMaxPipeCount	nombre maximal de connexion simultanées autorisées.
	/// \param inTimeout		temporisation
	///
	TDCLXADSP(
			IDCLThreads*	inThreadsIntf,
			const char*		inMachineName = nil,
			KUInt32			inMaxPipeCount = kMaxADSPPipeCount,
			long			inTimeout = kDefaultTimeout );

	///
	/// Destructeur.
	/// Appelle StopListening().
	///
	virtual				~TDCLXADSP( void );
	
	/// \name utilitaires

	///
	/// Détermine si ADSP est disponible.
	///
	/// \return true si ADSP est disponible.
	///
	static	Boolean		IsAvailable( void );
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
	/// Méthode appelée dans la méthode Run juste avant la boucle qui appelle
	/// WaitForIncomingRequest.
	///
	/// Enregistre le service AppleTalk (NBP).
	///
	virtual	void		DoStartListening( void );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	/// Cette méthode doit d'abord interrompre l'attente dans la méthode Run
	/// et ceci ne doit pas avoir de conséquence si on n'est pas en train d'attendre dans
	/// cette méthode.
	/// Attention aux possible problème d'attente/de réveil.
	///
	/// Désenregistre le service AppleTalk (NBP).
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

	///
	/// Crée l'interface de communication pour écouter, la lie et écoute.
	///
	/// \return une référence vers l'interface de communication ou un nombre
	///			négatif si une erreur est survenue. Le code de l'erreur est
	///			dans errno.
	///
	int					SocketBindAndListen( void );

private:
	///
	/// Droits pour la classe fille.
	///
	class TXADSPPipe;
	friend class TXADSPPipe;
	
	///
	/// Class pour les connexions.
	///
	class TXADSPPipe
		:
			public TSyncPipe
	{
	public:
		/// La classe mère est une amie.
		friend class TDCLXADSP;

		///
		/// Destructeur. Ferme la connexion si elle n'était pas déjà fermée.
		///		
		virtual				~TXADSPPipe( void );

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
		/// \param inCommLayer		couche de communication à laquelle la
		///							connexion appartient.
		/// \param inClientSocket	descripteur de fichier de l'interface de
		///							connexion.
		/// \param inTimeout		temporisation pour cette connexion.
		///
		TXADSPPipe(
				TDCLXADSP*	inCommLayer,
				int				inClientSocket,
				long			inTimeout
			);
	
	private:
		/// \name Variables

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
		KUInt8			mNextByte;				///< Octet suivant.
		Boolean			mHasNextByte;			///< Si on a l'octet suivant.
	};
	
	/// \name Variables privées.
	int				mServerSocket;			///< Descripteur de fichier de
											///< l'interface de communication
											///< du serveur.
	KUInt32			mCurrentPipeCount;		///< Nombre de connexions ouvertes.
	KUInt32			mBackLogSize;			///< Taille de la file d'attente de
											///< listen.
	long			mTimeout;				///< Temporisation au format
											///< \c TDCLCommLayer (nombre de
											///< secondes + constantes)
	int				mPublicPairMember;		///< Élément public de la "socket
											///< pair"
	int				mPrivatePairMember;		///< Élément privé (= thread) de la
											///< "socket pair"
	Boolean			mSocketsAreClosed;		///< Si les sockets sont fermées.
	at_entity_t		mMachineEntity;			///< Référence sur l'entité ADSP.
};

#endif
		// __TDCLXADSP__

// =============================================================================== //
// "This is lemma 1.1.  We start a new chapter so the numbers all go back to one." //
//                 -- Prof. Seager, C&O 351                                        //
// =============================================================================== //