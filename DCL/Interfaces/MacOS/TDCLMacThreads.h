// ==============================
// Fichier:			TDCLMacThreads.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/10/2002
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
// The Original Code is TDCLMacThreads.h.
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
// $Id: TDCLMacThreads.h,v 1.7 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _TDCLMACTHREADS_H
#define _TDCLMACTHREADS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/IDCLThreads.h>

// MacOS
#if TARGET_OS_MACOS
#include <OSUtils.h>
#include <Timer.h>
#include <Threads.h>
#else
#include <CoreServices/CoreServices.h>
#endif

///
/// Mise en place de l'interface IDCLThreads pour MacOS.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class TDCLMacThreads
	:
		public IDCLThreads
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLMacThreads( void );	

	///
	/// Destructeur.
	///
	virtual ~TDCLMacThreads( void );	

	///
	/// Donne du temps aux autres processus.
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
	///
	/// Sémaphore pour MacOS.
	///
	class TSemaphore
		:
			public ISemaphore
	{
	public:
		///
		/// Constructeur unique.
		/// À partir de inThreads pour pouvoir faire des Yield.
		///
		TSemaphore( TDCLMacThreads* inThreadsIntf );
		
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
		/// Libère le sémaphore.
		///
		virtual	void		Release ( void );
	
	private:
		/// \name variables privées.
		QHdr				mQueue;			///< Queue pour le sémaphore.
		TDCLMacThreads*		mThreads;		///< Objet IDCLThreads pour faire Yield.
		Boolean				mSemaphore;		///< Valeur du sémaphore.
	};

	///
	/// Processus léger pour MacOS (coopératif)
	///
	class TThread
		:
			public IThread
	{
	public:
		///
		/// Constructeur à partir d'un client.
		///
		/// \param inThreadsIntf	interface des processus légers.
		/// \param inThreadedObject	objet associé au processus léger.
		///
		TThread( TDCLMacThreads* inThreadsIntf, TDCLThread* inThreadedObject );

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
		/// Suivant les classes concrètes, désactive les interruptions
		/// ou utilise les variables de condition.
		///
		/// \param inMillisecs		nombre de millisecondes à dormir
		/// \return	\c true si l'on a dormi tout le temps demandé, \c false si on a été
		///			interrompu.
		///
		virtual	Boolean	Sleep( KUInt32 inMillisecs = kForever );

		///
		/// Réveille un processus qui dort.
		/// Incrémente le compteur des réveils.
		/// Suivant les classes concrètes, désactive les interruptions
		/// ou utilise les variables de condition.
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
		/// La fonction vraiment appelée par le système.
		/// Elle appelle la méthode Run du client.
		///
		/// \param inThis	pointeur sur l'objet TThread.
		/// \return \c nil
		///
		static pascal void* Run( TThread* inThis );

		/// \name variables privées.
		ThreadID		mThreadID;		///< L'ID du processus léger
		EState			mState;			///< L'état du processus.
		TSemaphore		mMutex;			///< Sémaphore utilisé pour synchroniser
										///< l'état.
		ThreadEntryTPP	mRunTPP;		///< TPP pour Run.
		TDCLMacThreads*	mThreads;		///< Objet IDCLThreads pour la UPP qui nous réveille.
		KUInt32			mWakeCount;		///< Compteur de réveils.
	};

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacThreads( const TDCLMacThreads& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMacThreads& operator = ( const TDCLMacThreads& inCopy );

	///
	/// Fonction appelée par la tâche du TimeManager qui va réveiller
	/// le processus léger.
	///
	/// \param inTMTaskPtr	pointeur sur la structure TMTask.
	///
	static void TimerProc( TMTaskPtr inTMTaskPtr );
	
	/// \name variables privées.
	TimerUPP	mTimerProcUPP;	///< ProcPtr sur la fonction précédente.
	
	/// TThread a besoin d'accéder la variable précédente.
	friend class TThread;
};

#endif
		// _TDCLMACTHREADS_H

// =========================================================================== //
// Our documentation manager was showing her 2 year old son around the office. //
// He was introduced to me, at which time he pointed out that we were both     //
// holding bags of popcorn.  We were both holding bottles of juice.  But only  //
// *__he* had a lollipop.                                                    //
//         He asked his mother, "Why doesn't HE have a lollipop?"              //
//         Her reply: "He can have a lollipop any time he wants to.  That's    //
// what it means to be a programmer."                                          //
// =========================================================================== //
