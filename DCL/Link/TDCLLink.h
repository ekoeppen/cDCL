// ==============================
// Fichier:			TDCLLink.h
// Projet:			Desktop Connection Library
//
// Créé le:			28/06/2000
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
// The Original Code is TDCLLink.h.
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
// $Id: TDCLLink.h,v 1.6 2004/11/24 14:09:03 paul Exp $
// ===========

#ifndef _TDCLLINK_H
#define _TDCLLINK_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Interfaces/TDCLThread.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

class TDCLApplication;
class TDCLAppCommand;
class TDCLServer;
class TDCLPipe;

///
/// Classe abstraite pour les liens.
///
/// Un lien est une connexion avec un Newton donné et fournit des services.
///
/// Le lien est représenté par un processus léger. Il itère dans une boucle en
/// attendant:
/// - des données du Newton
/// - l'expiration d'une temporisation (permet d'appeler Idle).
/// - un appel du serveur ou de l'application.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
class TDCLLink
	:
		private TDCLThread
{
public:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kDefaultIdlePeriod	= 15000	///< Temps d'attente en millisecondes entre
									///< entre les appels à Idle.
	};
	
	///
	/// Etat du lien.
	///
	enum EState {
		kDisconnected,	///< Déconnecté (le processus léger ne tourne pas)
		kConnecting,	///< En train de se connecter (la connexion avec le Newton
						///< est établie)
		kConnected,		///< Connecté au Newton (la phase de connexion est finie)
		kRunning,		///< En train d'échanger des données avec le Newton.
		kDisconnecting,	///< On est en train de se déconnecter.
		kUnknown		///< Etat inconnu.
	};

	///
	/// Accesseur sur l'application.
	///
	/// \return un pointeur sur l'application.
	///	
	inline TDCLApplication*	GetApplication( void )
		{
			return mApplication;
		}
	
	// --- connection parameters ---
	
	// General note regarding connection parameters:
	// These are to be set before the connection is initiated (it's useless otherwise).
	// They are initialized by the constructor with the values you passed to it.
	
	// Connection type: parameters used in the kDInitiateDocking command.
	// The 8 session types are defined in TDCLDockCommand.
	// There are in fact two constants that you want to use:
	//	TDCLDockCommand::kNone_SType (the default). Perfect for general connection.
	//	TDCLDockCommand::kLoadPackage_SType: the proper constant for package loading.
	
	///
	/// Accesseur sur le type de la connexion.
	/// Le type est utilisé avec ;a commande kDInitiateDocking.
	/// Les constantes utilisables sont définies dans TDCLDockCommand.
	/// Il y a deux constantes habituelles:
	/// - TDCLDockCommand::kNone_SType (valeur par défaut): pour une connexion
	///		générale.
	/// - TDCLDockCommand::kLoadPackage_SType: pour l'installation de paquets.
	///
	/// \return le type de la connexion.
	///
	inline KUInt32		GetType( void )
		{
			return mType;
		}
	
	///
	/// Sélecteur sur le type de la connexion.
	///
	/// \param inType	nouveau type de la connexion
	/// \see GetType
	///
	inline void			SetType( KUInt32 inType )
		{
			mType = inType;
		}

	/// \name interface avec le serveur/l'application.

	///
	/// Méthode asynchrone pour connecter le Newton.
	/// Démarre le processus léger.
	///
	/// \param inPipe	Interface de communication associée au lien.
	/// \param inServer	Serveur qui gère l'interface de communication.
	/// \return \c true si le lien était déconnecté, \c false sinon.
	///
	Boolean				Connect( TDCLPipe* inPipe, TDCLServer* inServer );
		
	///
	/// Méthode asynchrone pour déconnecter le Newton.
	/// Envoie un événement au processus léger pour qu'il se déconnecte.
	/// Ne fait rien si le processus léger n'était pas démarré.
	/// Ceci est la méthode souple pour déconnecter.
	///
	/// \return \c true si le lien était connecté, \c false sinon.
	///
	Boolean				Disconnect( void );
		
	///
	/// Méthode invoquée lorsque l'interface de connexion a été fermée.
	///
	/// \return \c true si le lien était connecté, \c false sinon.
	///
	Boolean				Close( void );

	///
	/// Détermine si le lien est connecté.
	///
	/// \return \c true si le lien est connecté, \c false sinon.
	///
	Boolean				IsConnected( void );

	///
	/// Détermine si le lien est occupé à exécuter une commande.
	///
	/// \return \c true si le lien est occupé, \c false sinon.
	///
	inline Boolean		IsBusy( void )
		{
			return mIsBusy;
		}

	///
	/// Accesseur sur l'interface de communication.
	///
	/// \return un pointeur sur l'interface de communication.
	///
	inline TDCLPipe* 	GetPipe( void )
		{
			return mPipe;
		}

	///
	/// Méthode appelée par l'interface de communication pour dire que
	/// des données sont présentes. Cette méthode est appelée une seule fois
	/// et sera rappellée lorsque le lien aura appelé ClearDataRequest.
	/// Cette méthode envoie un événement au processus léger du serveur.
	///
	void				DataPresent( void );

	///
	/// Accesseur sur le nom du Newton.
	/// Ceci est en fait le nom du possesseur du Newton.
	///
	/// \return le pointeur sur le nom du Newton.
	///
	inline const KUInt16*	GetNewtonName( void )
		{
			return mNewtonName;
		}

	///
	/// Accesseur sur un nombre identifiant le Newton de façon plus ou
	/// moins unique.
	/// Ceci ne peut pas être le numéro de série matériel du Newton vu que
	/// (a) les Newton 2.0 n'en ont pas, (b) c'est une valeur sur 32 bits.
	/// Je pense que c'est dérivé de l'ID du magasin ou un truc dans le genre.
	///
	/// \return l'ID du Newton
	///
	inline KUInt32			GetNewtonID( void )
		{
			return mNewtonID;
		}
	
	///
	/// Accesseur sur le champ TGestaltSystemInfo.fManufacturer.
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fManufacturer
	///
	inline KUInt32			GetManufacturer( void )
		{
			return mManufacturer;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fMachineType
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fMachineType
	///
	inline KUInt32			GetMachineType( void )
		{
			return mMachineType;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fROMVersion
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fROMVersion
	///
	inline KUInt32			GetROMVersion( void )
		{
			return mROMVersion;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fROMStage
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fROMStage
	///
	inline KUInt32			GetROMStage( void )
		{
			return mROMStage;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fRAMSize
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fRAMSize
	///
	inline KUInt32			GetRAMSize( void )
		{
			return mRAMSize;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fScreenHeight
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fScreenHeight
	///
	inline KUInt32			GetScreenHeight( void )
		{
			return mScreenHeight;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fScreenWidth
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fScreenWidth
	///
	inline KUInt32			GetScreenWidth( void )
		{
			return mScreenWidth;
		}
	inline KUInt32			GetSystemUpdate( void )
		{
			return mSystemUpdate;
		}
	inline KUInt32			GetNOSVersion( void )
		{
			return mNOSVersion;
		}
	inline KUInt32			GetInternalStoreSig( void )
		{
			return mInternalStoreSig;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fScreenResolution.v
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fScreenResolution.v
	///
	inline KUInt32			GetVertScreenRes( void )
		{
			return mVertScreenRes;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fScreenResolution.h
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fScreenResolution.h
	///
	inline KUInt32			GetHoriScreenRes( void )
		{
			return mHoriScreenRes;
		}

	///
	/// Accesseur sur le champ TGestaltSystemInfo.fScreenDepth
	/// Cf <NewtonGestalt.h>
	///
	/// \return la valeur de TGestaltSystemInfo.fScreenDepth
	///
	inline KUInt32			GetScreenDepth( void )
		{
			return mScreenDepth;
		}
	
protected:
	///
	/// Constantes protégées.
	///
	enum {
		kDefaultTimeout	=	30	///< Valeur par défaut de la temporisation
								///< passée au Newton avec kDSetTimeout.
	};

	///
	/// Constructeur unique.
	///
	/// \param inApplication	interface avec l'application.
	/// \param inType			type de connexion (expliqué plus haut)
	/// \param inIdlePeriod		fréquence des appels à Idle.
	/// \see GetType()
	///
	TDCLLink(
		TDCLApplication* inApplication,
		KUInt32 inType = TDCLDockCommand::kNone_SType,
		KUInt32 inIdlePeriod = kDefaultIdlePeriod );
	
	///
	/// Destructeur.
	/// Indique que le lien a été déconnecté si nécessaire.
	///
	virtual		~TDCLLink( void );

	///
	/// Poste une commande à effectuer.
	/// Trois cas se présentent:
	/// - le lien n'est pas établi, la méthode retourne \c false.
	/// - le lien était établi, la méthode retourne \c true et
	///		envoie un événement.
	///		- l'événement est récupéré, la commande est passée à
	///			ProcessAppCommand et puis à TDCLApplication::AppCommandProcessed,
	///			enfin l'événement sera supprimé avec delete par le lien.
	///		- l'événement n'est pas récupéré avant que lien ne soit
	///			interrompu, la commande sera supprimée lorsque le processus
	///			léger sera terminé avec delete.
	///		- un problème est survenu dans AppCommandProcessed, la commande
	///			sera supprimée avec delete.
	///
	/// \return \c true si le lien était établi, \c false sinon.
	///
	Boolean				PostAppCommand( TDCLAppCommand* inCommand );

	///
	/// Méthode synchrone pour connecter le Newton.
	/// Peut être surchargée.
	/// Cette méthode est appelée dans le processus léger du lien et effectue
	/// le dialogue minimal pour la connexion:
	///
	/// Desktop			Newton
	///				<- kDRequestToDock
	/// kDInitiateDocking ->
	///				<- kDNewtonName
	///
	/// \throws TDCLException si un problème est survenu.
	///
	virtual void		DoConnect( void );

	///
	/// Méthode synchrone pour déconnecter le Newton.
	/// Peut être surchargée.
	/// Cette méthode est appelée dans le processus léger du lien et envoie la
	/// commande kDDisconnect au Newton
	///
	/// Desktop			Newton
	/// kDDisconnect ->
	///
	virtual void		DoDisconnect( void );


	///
	/// Méthode appelée lorsque le Newton a envoyé une commande.
	///
	/// Cette méthode est appelée pour toutes les commandes interceptées
	/// dans la boucle principale.
	/// Les protocoles peuvent échanger des commandes avec le Newton sans
	/// qu'elles soient traitées dans la boucle principale.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée. Si la valeur est
	///						\c false, une exception est levée dans la boucle.
	/// \return le prochain état du lien (l'état actuel étant kRunning),
	///			par exemple kRunning ou kDisconnecting (DoDisconnect est appelé
	///			dans la boucle)
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	EState	ProcessDockCommand(
								TDCLDockCommand* inCommand,
								Boolean* outProcessed ) = 0;

	///
	/// Méthode appelée lorsqu'on a reçu une commande de l'application.
	/// La commande est supprimée une fois que cette méthode retourne.
	/// Cette classe ne fait rien -- retourne kRunning et met false dans
	/// \c outProcessed.
	///
	/// \param inCommand	commande reçue.
	/// \param outProcessed	si la commande a été traitée. Si la valeur est
	///						\c false, une exception est levée dans la boucle.
	/// \return le prochain état du lien (l'état actuel étant kRunning)
	/// \throws TDCLException si une erreur est survenue.
	///
	virtual	EState	ProcessAppCommand(
								TDCLAppCommand* inCommand,
								Boolean* outProcessed );

	///
	/// Méthode appelée de temps en temps lorsqu'on n'a pas de nouvelles
	/// ni de l'application du Newton.
	///
	/// Par défaut, appelle Yield.
	/// Il faudrait en fait dire bonjour.
	///
	/// \return le prochain état du lien (l'état actuel étant kRunning)
	///
	virtual	EState			Idle( void );

	///
	/// Boucle principale du lien.
	/// Attend:
	/// - un dépassement de la temporisation pour appeler Idle
	/// - des données du Newton (lit alors toute la commande)
	/// - un appel de l'application.
	///
	virtual void	Run( void );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée.
	/// Lorsque cette méthode retourne, le processus est terminé.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void HandleException( TDCLException* inException );

	///
	/// Méthode invoquée lorsque le processus de connexion est
	/// terminé.
	///
	/// Cette méthode appelle l'interface des messages pour lui dire et lui
	/// transmet le nom du Newton.
	///
	virtual void		Connected( void );

	///
	/// Méthode invoquée lorsque le processus de déconnexion est
	/// terminé.
	///
	/// Elle appelle l'interface des messages pour lui dire.
	/// Elle libère aussi le nom du Newton dont on n'a plus besoin.
	///
	virtual void		Disconnected( void );

	///
	/// Détermine les commandes de l'application que le lien
	/// peut traiter. Cette méthode est appelée à chaque fois que le
	/// lien va attendre un événement. Par défaut, le lien peut traiter
	/// toutes les commandes d'application.
	///
	/// \return le masque pour les commandes de l'application
	///
	virtual KUInt32		GetAppCommandMask( void );

	///
	/// Receive a link specfic command. This function blocks
	/// until completion.
	///
	/// \return the received command.
	///
	virtual TDCLDockCommand* ReceiveCommand( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLink( const TDCLLink& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLLink& operator = ( const TDCLLink& inCopy );

	///
	/// Méthode appelée par la boucle du processus léger pour faire le boulot.
	///
	/// \return le nouvel état.
	///
	EState		TransitionFromRunning( void );

protected:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kClosingTimeout	= 5		// Number of seconds to wait with pipes that do not support
								// flush after having sent the disconnect command.
								// 5 seconds is enough on my mac with ADSP CTB tool.
	};

	/// \name Variables privées.
	TDCLPipe*					mPipe;				///< Interface de communication.
													///< \c nil si on n'est pas connecté.
	TDCLServer*					mServer;			///< Serveur qui gère l'interface de
													///< communication.
	TDCLApplication* 			mApplication;		///< Référence sur l'application.
	IDCLThreads::ISemaphore*	mStateMutex;		///< Sémaphore sur l'état.
	EState						mState;				///< Etat.
	KUInt32						mType;				///< Type de connexion Dock.
	KUInt32						mIdlePeriod;		///< Temps en millisecondes entre les
													///< appels à Idle
	Boolean						mIsBusy;			///< Si le lien est occupé.
	
	/// \name Informations sur le Newton.
	const KUInt16*				mNewtonName;		///< Nom du (possesseur du) Newton
	KUInt32						mNewtonID;			///< Identifiant unique (?)
	KUInt32						mManufacturer;		///< Tel que fourni par Gestalt
	KUInt32						mMachineType;		///< Idem
	KUInt32						mROMVersion;		///< Idem
	KUInt32						mROMStage;			///< Idem
	KUInt32						mRAMSize;			///< Idem
	KUInt32						mScreenHeight;		///< Idem, en pixels.
	KUInt32						mScreenWidth;		///< Idem, en pixels.
	KUInt32						mSystemUpdate;		///< Non nul s'il y a une rustine.
	KUInt32						mNOSVersion;		///< Version du format NSOF, je suppose.
													///< (Newton Object System)
	KUInt32						mInternalStoreSig;	///< Comme le nom l'indique.
	KUInt32						mVertScreenRes;		///< Nombre de pixels par pouce sur la
													///< hauteur.
	KUInt32						mHoriScreenRes;		///< Nombre de pixels par pouce sur la
													///< longueur.
	KUInt32						mScreenDepth;		///< Profondeur en bits (e.g. 1 ou 4)
};

#endif
		// _TDCLLINK_H

// ========================================================================== //
// Why do we want intelligent terminals  when there are so many stupid users? //
// ========================================================================== //
