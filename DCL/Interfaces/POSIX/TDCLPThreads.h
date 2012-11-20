// ==============================
// Fichier:			TDCLPThreads.h
// Projet:			Desktop Connection Library
//
// Créé le:			26/1/2003
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
// The Original Code is TDCLPThreads.h.
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
// $Id: TDCLPThreads.h,v 1.7 2004/11/24 14:09:02 paul Exp $
// ===========

#ifndef _TDCLPTHREADS_H
#define _TDCLPTHREADS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLThreads.h>

// POSIX
#include <pthread.h>

///
/// Mise en place de l'interface IDCLThreads avec les pthreads.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLPThreads
	:
		public IDCLThreads
{
public:
	///
	/// Donne du temps aux autres processus.
	/// Ne fait rien puisque les pthreads sont préemptifs.
	///
	virtual	void	Yield( void );

	///
	/// Crée un objet IThread. À surcharger pour chaque plateforme.
	///
	/// \param inThreadedObject		objet associé à passer comme paramètre
	///								pour IThread
	/// \return un nouvel objet IThread
	///
	virtual IThread*	CreateThread( TDCLThread* inThreadedObject );

	///
	/// Crée un objet sémaphore. À surcharger pour chaque plateforme.
	///
	/// \return un nouvel objet sémaphore
	///
	virtual ISemaphore*	CreateSemaphore( void );

private:
	class TThread;
	
	///
	/// Classe pour un sémaphore.
	/// À l'inverse d'un mutex POSIX, le sémaphore dans la DCL peut:
	/// - bloqué un processus léger avec deux acquisitions
	/// - être débloqué par un autre processus léger
	///
	class TSemaphore
		:
			public ISemaphore
	{
	public:
		/// Le thread est un ami.
		/// (permet de récupérer un pointeur sur l'objet pthread_mutex)
		friend class TThread;

		///
		/// Constructeur unique.
		///
		TSemaphore( void );
		
		///
		/// Destructeur.
		/// Le sémaphore ne doit pas être utilisé.
		///
		virtual				~TSemaphore( void );

		///
		/// Attend jusqu'à ce que le sémaphore soit libéré si nécessaire
		/// puis acquiert le sémaphore.
		///
		virtual	void		Acquire ( void );
		
		///
		/// Libère un processus léger du sémaphore.
		///
		virtual	void		Release ( void );
	
	private:
		/// \name variables privées.		
		pthread_mutex_t		mMutex;		///< Mutex pour protéger le nombre
										///< de processus dans le sémaphore.
		KUInt32				mNbThreads;	///< Nombre de processus dans le
										///< sémaphore.
		pthread_cond_t		mWaitCond;	///< Variable de condition pour
										///< l'attente.
	};

	///
	/// Classe pour un processus léger.
	///
	class TThread
		:
			public IThread
	{
	public:
		///
		/// Constructeur à partir d'un client.
		///
		/// \param inThreadedObject	objet associé au processus léger.
		///
		TThread( TDCLThread* inThreadedObject );
		
		///
		/// Destructeur.
		///
		/// \throws TDCLException si le processus léger n'est pas arrêté.
		///
		virtual ~TThread( void );

		///
		/// Démarre le processus.
		///
		/// Appelle la méthode Run de TDCLThread dans le nouveau
		/// processus léger.
		/// Lance une exception si le processus léger n'est pas arrêté.
		///
		/// \throws TDCLException si le processus léger n'est pas arrêté.
		///
		virtual void	Start( void );
		
		///
		/// Arrête le processus (de manière violente).
		///
		/// Lance une exception si le processus léger est arrêté.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual void	Stop( void );

		///
		/// Suspend le processus léger.
		/// Cette méthode peut être appelée à partir d'un autre processus
		/// léger.
		/// Lance une exception si le processus léger est arrêté, mais
		/// est silencieux si le processus est suspendu.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual	void	Suspend( void );

		///
		/// Reprend le processus léger.
		/// Cette méthode est appelée à partir d'un autre processus léger.
		/// Lance une exception si le processus léger n'est pas arrêté,
		/// mais est silencieux si le processus n'est pas suspendu.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual	void	Resume( void );
	
		///
		/// Dort pendant n millisecondes ou jusqu'à ce que la méthode
		/// WakeUp soit appelée.
		/// Note: le processus peut aussi dormir plus longtemps.
		/// Cette méthode doit être appelée par le processus courant.
		/// Décrémente le compteur des réveils ou dort s'il est à zéro.
		/// Utilise les variables de condition.
		///
		/// \param inMillisecs		nombre de millisecondes à dormir
		/// \return	\c true si l'on a dormi tout le temps demandé, \c false si on a été
		///			interrompu.
		///
		virtual	Boolean	Sleep( KUInt32 inMillisecs );

		///
		/// Réveille un processus qui dort.
		/// Incrémente le compteur des réveils.
		/// Utilise les variables de condition.
		///
		virtual	void	WakeUp( void );

		///
		/// Détermine l'état du processus.
		///
		/// \return l'état du processus (arrêté, actif, suspendu)
		///
		virtual	EState	GetState( void );

	private:
		///
		/// Constructeur par défaut volontairement indisponible.
		///
		TThread( void );
		
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TThread( const TThread& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TThread& operator = ( const TThread& inCopy );

		///
		/// La fonction vraiment appelée par le système.
		/// Elle appelle la méthode Run du client.
		///
		/// \param inThis	pointeur sur l'objet TThread.
		/// \return \c nil
		///
		static void* Run( TThread* inThis );

		///
		/// Fonction appelée lorsque le signal SIGUSR1 est envoyé.
		/// Ne fait rien.
		///
		/// \param inSignal	signal envoyé (toujours SIGUSR1).
		///
		static void SignalUSR1( int inSignal );

		///
		/// Fonction appelée lorsque le signal SIGUSR2 est envoyé.
		/// Appelle sigsuspend (on attend SIGUSR1 (seulement)).
		///
		/// \param inSignal	signal envoyé (toujours SIGUSR2).
		///
		static void SignalUSR2( int inSignal );

		///
		/// Acquisition du mutex mMutex.
		///
		void		AcquireMutex( void );

		///
		/// Libération du mutex mMutex.
		///
		void		ReleaseMutex( void );
		
		/// \name variables privées.
		
		pthread_t		mThread;		///< Ben le processus léger.
		EState			mState;			///< L'état du processus.
		TSemaphore		mSyncSemaphore;	///< Sémaphore utilisé pour
										///< synchroniser.
		pthread_mutex_t	mMutex;			///< Mutex pour protéger l'état.
		pthread_cond_t	mSleepCond;		///< Variable de condition pour
										///< Sleep/WakeUp.
		KUInt32			mWakeCount;		///< Compteur de réveils.
	};
};

#endif
		// _TDCLPTHREADS_H

// =================================================================== //
// Man is the best computer we can put aboard a spacecraft ... and the //
// only one that can be mass produced with unskilled labor.            //
//                 -- Wernher von Braun                                //
// =================================================================== //
