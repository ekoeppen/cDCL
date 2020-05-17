// ==============================
// Fichier:			UTestCommLayers.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			17/4/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestCommLayers.h,v 1.12 2003/09/17 10:43:49 paul Exp $
// ===========

#ifndef _UTESTCOMMLAYERS_H
#define _UTESTCOMMLAYERS_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL - Tests
#include "TTestApplication.h"
#include "TTestServer.h"

///
/// Classe pour tester une couche de communication.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.12 $
///
class UTestCommLayers
{
public:
	///
	/// Test "Connect & Disconnect" pour les sockets BSD
	/// Attend une connexion du Newton et lorsqu'il est connecté, raconte sa vie
	/// et déconnecte.
	///
	static void ConnectAndDisconnectBSDSocket( void );

	///
	/// Le même, pour FDSerial.
	///
	/// \param inDevPath	chemin pour le port série
	///
	static void ConnectAndDisconnectFDSerialPort( const char* inDevPath );

	///
	/// Le même, pour X ADSP.
	///
	static void ConnectAndDisconnectXADSP( void );

	///
	/// Test "Start & Stop Twice" pour les sockets BSD
	/// Lance le serveur puis l'arrête deux fois.
	///
	static void StartAndStopTwiceBSDSocket( void );

	///
	/// Le même, pour FDSerial.
	///
	/// \param inDevPath	chemin pour le port série
	///
	static void StartAndStopTwiceFDSerialPort( const char* inDevPath );

	///
	/// Le même, pour X ADSP.
	///
	static void StartAndStopTwiceXADSP( void );

private:

	///
	/// Serveur sans lien et avec une couche de communication.
	///
	class TServer
		:
			public TTestServer
	{
	public:
		///
		/// Constructeur à partir d'une référence sur l'application et
		/// d'une couche de communication.
		///
		/// \param inApplication	interface avec le système hôte.
		/// \param inCommLayer		couche de communication
		///
		TServer( TDCLLogApplication* inApplication, TDCLCommLayer* inCommLayer );

		///
		/// Destructeur.
		///
		virtual				~TServer( void );

		///
		/// Commence à écouter.
		/// Cette méthode est appelée lorsque le serveur est démarré (depuis
		/// le processus léger du serveur).
		///
		virtual	void		DoStartListening( void );

		///
		/// Arrête le serveur.
		///
		virtual	void		DoStop( void );
		
		///
		/// Ferme toutes les connexions brutalement.
		///
		virtual	void		DoKill( void );

		///
		/// Méthode appelée par le serveur lorsque rien ne se passe.
		/// Elle permet d'appeler Idle sur les interfaces de communication qui en
		/// ont besoin.
		///
		virtual	void		DoIdle( void );
	
	private:
		TDCLCommLayer*		mCommLayer;
	};

	///
	/// Application pour le test Connect & Disconnect
	///
	class TCADApplication
		:
			public TTestApplication
	{
	public:
		///
		/// Constructeur à partir d'une interface pour les processus légers.
		/// Les informations sont envoyées sur stdout.
		///
		/// \param inThreadsIntf	interface pour les processus légers.
		///
		TCADApplication( IDCLThreads* inThreadsIntf );

		///
		/// Destructeur.
		///
		virtual ~TCADApplication( void );

		///
		/// Méthode appelée par le serveur une fois que le nom est connu.
		/// Ici, on déconnecte.
		///
		/// \param inLink	le lien courant
		/// \param inName	nom de l'utilisateur (officiellement, nom du Newton). Ce pointeur
		///					est la propriété du lien.
		///	
		virtual	void	ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName );

		///
		/// Attend que le test soit terminé.
		/// Cette méthode (avec un sémaphore) attend que la notification de déconnexion
		/// soit arrivée (ou une notification d'exception).
		///
		void			WaitForTestEnd( void );

		///
		/// Méthode appelée lorsque le lien a été déconnecté (juste avant
		/// que le nom ne soit libéré).
		///
		/// \param inLink	le lien courant
		///
		virtual void	Disconnected( TDCLLink* inLink );

	protected:
		///
		/// Méthode appelée lorsqu'une exception (quelconque) est arrivée.
		///
		virtual void 	ExceptionOccurred( void );

	private:
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TCADApplication( const TCADApplication& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TCADApplication& operator = ( const TCADApplication& inCopy );

	private:
		IDCLThreads::ISemaphore*	mWaitMutex;		///< Sémaphore pour attendre la fin des tests.
	};

	///
	/// Application pour le test Start & Stop Twice
	///
	class TSASTApplication
		:
			public TTestApplication
	{
	public:
		///
		/// Constructeur à partir d'une interface pour les processus légers.
		/// Les informations sont envoyées sur stdout.
		///
		/// \param inThreadsIntf	interface pour les processus légers.
		///
		TSASTApplication( IDCLThreads* inThreadsIntf );

		///
		/// Destructeur.
		///
		virtual ~TSASTApplication( void );

		///
		/// Méthode appelée lorsque le serveur commence à écouter sur une interface donnée.
		///
		/// \param inServer	serveur qui commence à écouter
		/// \param inLayer	couche de communication qui attend la connexion.
		///
		virtual	void	WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer );

		///
		/// Méthode appelée lorsque le serveur quitte.
		/// Cette méthode est appelée par le processus léger du serveur juste avant
		/// qu'il ne se termine (remarque: si une exception est arrivée, cette
		/// méthode ne sera pas appelée).
		///
		/// \param inServer	serveur qui quitte
		///
		virtual	void	ServerIsDown( TDCLServer* inServer );

		///
		/// Attend que le serveur soit fermé.
		/// Cette méthode (avec un sémaphore) attend que la notification d'arrêt du serveur
		/// soit arrivée (ou une notification d'exception).
		///
		void			WaitUntilServerIsDown( void );

		///
		/// Recommence le test (i.e. ré-acquiert le sémaphore pour recommencer).
		///
		void			ResetTest( void );

	protected:
		///
		/// Méthode appelée lorsqu'une exception (quelconque) est arrivée.
		///
		virtual void 	ExceptionOccurred( void );

	private:
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TSASTApplication( const TSASTApplication& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TSASTApplication& operator = ( const TSASTApplication& inCopy );

	private:
		IDCLThreads::ISemaphore*	mWaitMutex;		///< Sémaphore pour attendre la fin des tests.
	};

	///
	/// Test "Connect & Disconnect"
	/// Attend une connexion du Newton et lorsqu'il est connecté, raconte sa vie
	/// et déconnecte.
	///
	/// \param inCommLayer		couche de communication.
	/// \param inApplication	application pour le test.
	///
	static void ConnectAndDisconnect(
					TDCLCommLayer*		inCommLayer,
					TCADApplication*	inApplication  );

	///
	/// Test "Start & Stop Twice"
	/// Lance et arrête le serveur (deux fois).
	///
	/// \param inCommLayer		couche de communication.
	/// \param inApplication	application pour le test.
	///
	static void StartAndStopTwice(
					TDCLCommLayer*		inCommLayer,
					TSASTApplication*	inApplication  );
};

#endif
		// _UTESTCOMMLAYERS_H

// ==================== //
// You might have mail. //
// ==================== //
