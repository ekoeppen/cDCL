// ==============================
// Fichier:			TDCLFDSerialPort.h
// Projet:			Desktop Connection Library
//
// Créé le:			4/4/2003
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
// The Original Code is TDCLFDSerialPort.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Simon Stapleton <simon@tufty.co.uk>
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLFDSerialPort.h,v 1.4 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef _TDCLFDSERIALPORT_H
#define _TDCLFDSERIALPORT_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>

// POSIX
#include <termios.h>

///
/// Classe pour une couche de communication à base d'un port série accédé
/// via un descripteur de fichier. C'est un port série Unix (dans /dev/) qui
/// répond aux appels classiques fctl & co.
/// La communication sur ce port série se fait par défaut avec la compression
/// MNP.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	UTestCommLayers
///
class TDCLFDSerialPort
	:
		public TDCLSyncCommLayer
{
public:
    /// Modes of the associated pipe
    enum {
        kModePlain,
        kModeMNP,
        kModeHammer
    };

	///
	/// Unique constructeur à partir du chemin vers le port série.
	/// Si ce chemin est \c nil, il devra être précisé dans la méthode
	/// Init.
	///
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inDevPath		chemin vers le port série (copié)
	/// \param inBaud			vitesse de la connexion (38400: vitesse par défaut
	///							du Newton).
	/// \param inMode			pipe mode
	///
	TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud,
				KUInt32 inMode );

	///
	/// Unique constructeur à partir du chemin vers le port série.
	/// Si ce chemin est \c nil, il devra être précisé dans la méthode
	/// Init.
	///
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inDevPath		chemin vers le port série (copié)
	/// \param inBaud			vitesse de la connexion (38400: vitesse par défaut
	///							du Newton).
	/// \param inUseMNP			si on fait de la compression MNP.
	///
	TDCLFDSerialPort(
				IDCLThreads* inThreadsIntf,
				const char* inDevPath,
				long inBaud = 38400,
				Boolean inUseMNP = true );

	///
	/// Destructeur.
	/// Arrête le serveur.
	///
	virtual ~TDCLFDSerialPort( void );
	
	/// \name interface serveur

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
	/// \return \c true si une requête est présente, \c false si la méthode a été
	/// interrompue.
	///
	virtual	Boolean		WaitForIncomingRequest( void );

	///
	/// Accesseur sur l'interface de communication.
	///
	/// \return	la référence sur l'interface de communication.
	///
	inline int			GetSerialFD( void )
		{
			return mSerialFD;
		}

	///
	/// Initialise l'interface de communication pour un chemin BSD donné.
	/// Cette méthode permet de ne pas fournir le chemin dans le constructeur.
	/// Elle ne doit pas être appelée plus d'une fois ou si un chemin a été
	/// spécifié dans le constructeur.
	///
	void				Init( const char* inDevPath );

	///
	/// Attend des données (méthode bloquante).
	///
	/// \return \c true si des données sont présentes, \c false sinon.
	///
	Boolean				Select( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFDSerialPort( const TDCLFDSerialPort& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFDSerialPort& operator = ( const TDCLFDSerialPort& inCopy );

	///
	/// Ouvre et configure le port série.
	///
	/// \throws TDCLIOException si un problème est survenu.
	///
	void				OpenAndConfigure( void );

	///
	/// Ferme et rouvre le port série.
	///
	/// \throws TDCLIOException si un problème est survenu.
	///
	void				Reset( void );

	///
	/// On donne tous les droits a la classe fille.
	///
	class TFDSerialPortPipe;
	friend class TFDSerialPortPipe;

	///
	/// Class pour les connexions.
	///
	class TFDSerialPortPipe
		:
			public TSyncPipe
	{
	public:
		/// La classe mère est une amie.
		friend class TDCLFDSerialPort;

		///
		/// Destructeur. Ferme la connexion si elle n'était pas déjà fermée.
		///		
		virtual				~TFDSerialPortPipe( void );

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
		/// Détermine quel est le nombre d'octets disponibles dans la mémoire tampon d'entrée.
		///
		/// \return le nombre d'octets disponibles.
		///
		virtual	Boolean		BytesAvailable( void );

		///
		/// Attend que des données se présentent. Cette méthode est synchrone (c'est l'intérêt
		/// de cette classe). Elle retourne si des données sont présentes ou si elle a été
		/// interrompue par Disconnect.
		///
		/// \return \c true si une requête est présente, \c false si la méthode a été
		/// interrompue.
		///
		virtual Boolean 	WaitForIncomingData( void );

		///
		/// Déconnecte le canal de communication avec le client.
		///
		/// \remark	une fois que cette méthode est appelée, l'objet connexion est inutilisable.
		/// 		Il peut être supprimé par l'appelant à tout instant.
		///
		virtual	void		DoDisconnect( void );
		
		///
		/// Récupère la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des constantes
		/// de TDCLCommLayer (dit "format TDCLCommLayer").
		///
		/// \return la temporisation au format TDCLCommLayer.
		///
		virtual	long		GetTimeout( void );

		///
		/// Change la temporisation pour cette connexion.
		/// La temporisation est exprimée en secondes ou c'est une des constantes
		/// de TDCLCommLayer.
		///
		/// \param inTimeout	temporisation au format TDCLCommLayer.
		///
		virtual	void		SetTimeout( long inTimeout );

	protected:
		///
		/// Constructeur.
		/// Vous n'êtes pas supposés créer des TSocketPipe vous-mêmes.
		///
		/// \param inCommLayer		couche de communication à laquelle la connexion
		///							appartient.
		///
		TFDSerialPortPipe(
				TDCLFDSerialPort*	inCommLayer
			);

	private:
		int						mSerialFD;			///< Descripteur de fichier de l'interface
													///< de connexion.
		long					mTimeout;			///< Temporisation pour cette
													///< interface.
		Boolean					mDisconnected;		///< Si la connexion a été rompue par le client.
		FILE*					mLog;
	};
	
	/// \name Variables

	Boolean						mIsConnected;		///< Si une connexion est ouverte.
	int							mSerialFD;			///< Descripteur de fichier de l'interface
													///< de communication.
	char*						mDevPath;			///< Chemin vers le fichier BSD.
	long						mBaud;				///< Vitesse du lien en baud.
	KUInt32						mPipeMode;			///< On utilise MNP?
	struct termios				mOriginalOptions;	///< Options originales.
	Boolean						mReset;				///< Si on est en train de réinitialiser
													///< le fichier.
	Boolean						mDisconnecting;		///< Si on est en train de se déconnecter.
	Boolean						mRequestPresent;	///< Si une requête est présente.
	IDCLThreads::ISemaphore*	mResetMutex;		///< Sémaphore pour bloquer
													///< Select lors de la réinitialisation.
	IDCLThreads::ISemaphore*	mWaitRequestMutex;	///< Sémaphore pour bloquer
													///< Select tant qu'on n'est pas connecté.
	long						mTimeout;			///< Temporisation pour cette
													///< interface.
};

#endif
		// _TDCLFDSERIALPORT_H

// ================================================== //
// The less time planning, the more time programming. //
// ================================================== //
