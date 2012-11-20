// ==============================
// Fichier:			TDCLCommToolBox.cp
// Projet:			Desktop Connection Library
//
// Créé le:			20/01/2000
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
// The Original Code is TDCLCommToolBox.cp.
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
// $Id: TDCLCommToolBox.h,v 1.6 2004/11/24 18:11:02 paul Exp $
// ===========

#ifndef __TDCLCOMMTOOLBOX__
#define __TDCLCOMMTOOLBOX__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLCommLayer.h>

// Vérification de la cible.
#if !TARGET_OS_MACOS
	#error "The Communication Toolbox is only available on MacOS < X"
#endif

// MacOS
#ifdef __MWERKS__
	#pragma	warn_resultnotused	off
#endif

#include <Connections.h>

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
#endif

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>

#warning TDCLCommToolBox compiles but is no longer functional

class IDCLThreads;

///
/// Classe pour une couche de communication utilisant la Communication ToolBox.
/// Cette classe est initialisée à partir d'un nom d'outil CTB et d'une chaîne
/// de configuration. Le constructeur a aussi besoin de l'interface parce qu'il
/// faut appeler Yield de temps en temps pour les outils CTB un peu bogués.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
class TDCLCommToolBox
	:
		public TDCLCommLayer
{
public:
	///
	/// Constructeur public.
	/// Le nombre de connexions et la temporisation pour cette couche de
	/// communication sont définies comme des constantes.
	/// Les sous-classes devront probablement utiliser plutôt l'autre
	/// constructeur.
	///
	/// \param inToolName		nom de l'outil de communication à utiliser
	/// \param inConfigString	chaîne de configuration pour l'outil de
	///							communication.
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inMaxPipeCount	nombre de connexions simultanées maximum.
	/// \param inTimeout		délai de temporisation.
	///
	TDCLCommToolBox(
				const char* inToolName,
				const char* inConfigString,
				IDCLThreads* inThreadsIntf,
				KUInt32	inMaxPipeCount = kDefaultPipeCount,
				long inTimeout = kDefaultTimeout
			);

	///
	/// Destructeur.
	///
	virtual				~TDCLCommToolBox( void );
	
protected:
	///
	/// Commence à écouter.
	///
	virtual	void		DoStartListening( void );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	///
	virtual	void		DoStopListening( void );

	// Returns true if there is a present connection request (in which case
	// Accept will return a pipe connected to the client).
//	virtual	Boolean		ConnectionRequestPresent( void );

	///
	/// Retourne un objet connexion représentant le canal de communication avec
	/// le client ou \c nil s'il n'y a pas de requête de connexion présente.
	/// C'est un objet créé avec \c new. L'appelant devra le supprimer.
	///
	/// \return un nouvel objet TDCLPipe créé avec \c new représentant la
	///			connexion ou \c nil.
	///
	virtual	TDCLPipe*	Accept( void );
	
	///
	/// Refuse la première requête de connexion.
	/// Ne fait rien si aucune connexion n'est présente.
	///
	virtual	void		Refuse( void );

	///
	/// Méthode appelée pour perdre du temps.
	///
	/// Elle est appelée (par l'implémentation par défaut de TDCLPipe::Idle par
	/// exemple) à intervalles réguliers.
	///
	/// Appelle CMIdle avec le ConnHandle et mInterface->Yield.
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
	/// Récupère l'indice de la connexion et met la référence ConnHandle
	/// correspondante à \c nil.
	///
	/// \param inPipe	connexion qui a été fermée
	///
	virtual	void		Disconnected( TDCLPipe* inPipe );

	///
	/// Détermine si les outils de communications CTB sont disponibles.
	///
	/// \return \c true si la CTB est disponible, \c false sinon.
	///
	static	Boolean		IsAvailable( void );

	///
	/// Convert a timeout from seconds (or one of the constants) to CTB.
	///
	/// \param inSecsTimeout	délai de temporisation en secondes.
	/// \return un temps au format CTB.
	///
	static	long		ConvertTimeoutFromSeconds( long inSecsTimeout );

	///
	/// Convert a timeout from CTB to seconds.
	///
	/// \param inCTBTimeout	délai de temporisation au format CTB
	/// \return un nombre de secondes.
	///
	static	long		ConvertTimeoutToSeconds( long inCTBTimeout );

protected:
	///
	/// Constructor and initializer for sub-classes.
	/// This constructor does not call Init. It's for base classes that want to
	/// process their configuration strings and call Init later.
	///
	TDCLCommToolBox(
			IDCLThreads* inThreadsIntf,
			KUInt32	inMaxPipeCount = kDefaultPipeCount,
			long	inTimeout = kDefaultTimeout
		);

	///
	/// Initialize all the layer, including the tool and the table of 
	/// connections.
	///
	void		Init( const char* inToolName, const char* inConfigString );

#ifdef MPW_COMPILERS_WORKAROUND
	// Sounds like a bug in MrCpp 5.0.0d3c1 and SCpp 8.9.0d3e1
	// It complains in TDCLFoo::IsAvailable() that this method isn't
	// accessible. If I remove static keywords, it no longer complains.
public:
#endif

	///
	/// Détermine si un outil de communication est disponible.
	///
	/// \param inToolName	nom de l'outil à tester.
	/// \return \c true si l'outil de communication est disponible, \c false
	///			sinon.
	///
	static	Boolean		IsAvailable( const char* inToolName );
	
private:
	///
	/// Classe pour une connexion.
	///
	class TCTBPipe
		:
			public TDCLPipe
	{
	public:
		friend class TDCLCommToolBox;
		
		// Destructor closes the connection if it wasn't closed yet.
		virtual				~TCTBPipe( void );
	
		///
		/// Lit des octets.
		///
		/// \param outBuffer	mémoire tampon pour les octets lus.
		/// \param ioCount		nombre d'octets à lire en entrée, lus en
		///						sortie. Cette valeur est mise à jour avant que
		///						l'exception ne soit lancée si un problème est
		///						survenu.
		/// \throws TDCLException	si un problème est survenu (autre que EOF)
		///
		virtual	void		DoRead( void* outBuffer, KUInt32* ioCount );

		///
		/// Ecrit des octets.
		///
		/// \param inBuffer		mémoire tampon pour les octets à écrire.
		/// \param ioCount		nombre d'octets à écrire en entrée, écris en
		///						sortie. Cette valeur est mise à jour avant que
		///						l'exception ne soit lancée si un problème est
		///						survenu.
		/// \throws TDCLException	si un problème est survenu.
		///
		virtual	void		DoWrite( const void* inBuffer, KUInt32* ioCount );
		
		///
		/// Détermine si des octets sont disponibles dans la mémoire tampon
		/// d'entrée.
		///
		/// \return \c true s'il y a des octets disponibles, \c false sinon.
		///
		virtual	Boolean		BytesAvailable( void );

		// Flush does not work with CTB. So we use default implementation.
		
		///
		/// Déconnecte le canal de communication avec le client.
		///
		/// L'implémentation par défaut appelle la méthode
		/// Disconnected( TDCLPipe* ) de TDCLCommLayer.
		///
		/// \remark	une fois que cette méthode est appelée, l'objet connexion
		///			est inutilisable. Il peut être supprimé par l'appelant à
		///			tout instant.
		///
		virtual	void		Disconnect( void );
		
		///
		/// Méthode appelée pour indiquer que c'est le moment de perdre un peu
		/// de temps.
		///
		/// Certaines couches de communication ont besoin de ceci.
		/// L'implémentation par défaut appelle la méthode Idle( void ) de la
		/// couche de communication.
		///
		virtual	void		Idle( void );

		///
		/// Récupère la temporisation pour cette connexion.
		///
		/// \return la temporisation au format \c TDCLCommLayer.
		///
		virtual	long		GetTimeout( void );

		///
		/// Change la temporisation pour cette connexion.
		///
		/// \param inTimeout	temporisation au format \c TDCLCommLayer.
		///
		virtual	void		SetTimeout( long inTimeout );

	protected:
		///
		/// Constructeur protégé.
		///
		/// \param inCommLayer	référence sur la couche de communication.
		/// \param inConnHandle	référence sur la connexion.
		/// \param inTimeout	délai de temporisation (format CTB).
		// You're not supposed to create TCTBPipes yourself.
		TCTBPipe(
					TDCLCommToolBox*	inCommLayer,
					ConnHandle			inConnHandle,
					long				inTimeout
				);

		///
		/// Accesseur sur la référence CTB de la connexion.
		///
		/// \return la référence CTB de la connexion.
		///
		inline	ConnHandle	GetConnHandle( void )
			{
				return mConnection;
			}

	private:
		long				mTimeout;		///< timeout in ticks, suitable for
											///< CTB functions.
		ConnHandle			mConnection;	///< our connection handle.
	};
	
	///
	/// Create a new connection handle from the tool proc ID and the tool
	/// config string.
	///
	ConnHandle			GetNewConnHandle( void );

	///
	/// Initialize the CTB
	///
	static	void		InitCTB( void );

	///
	/// Start listening on a new ConnHandle (stored into mCurrentConnHandle).
	/// Also looks for a new nextIndex if current isn't fine.
	/// Do nothing if mCurrentConnHandle is not nil.
	///
	void				DoListen( void );

	enum {
		kCTBDefaultPipeCount	=	10,
		kCTBDefaultTimeout		=	1800,	///< 30 secondes.
		kCTBNoTimeout			=	-1
	};
	
	enum ECTBState {
		kUninitialized,		///< Startup state
		kNotAvailable,		///< CTB isn't available
		kInitialized		///< CTB is available and initialized.
	};

	short				mToolProcID;		///< ProcID associated with the
											///< tool name.
	char*				mToolConfig;		///< Tool configuration string
											///< (copied by the Init method).
	ConnHandle			mCurrentConnHandle;	///< Current connection handle
											///< (listening)
	KUInt32				mCurrentPipeCount;	///< Number of pipes currently
											///< instantiated.
	long				mTimeout;			///< Timeout in CTB format (ticks)
	IDCLThreads*		mThreadsIntf;		///< Required to Yield a little
											///< bit.
	static ECTBState	gCTBState;			///< Whether the CTB is
											///< initialized/available or not.
	Boolean				mListening;			///< Whether we are listening or
											///< not.
};

#endif
		// __TDCLCOMMTOOLBOX__

// ========================================== //
// The first version always gets thrown away. //
// ========================================== //