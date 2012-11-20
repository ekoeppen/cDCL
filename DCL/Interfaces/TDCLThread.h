// ==============================
// Fichier:			TDCLThread.h
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
// The Original Code is TDCLThread.h.
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
// $Id: TDCLThread.h,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef _TDCLTHREAD_H
#define _TDCLTHREAD_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Interfaces/TDCLEvent.h>

///
/// Classe pour un objet dans un processus léger.
/// Pour les détails, voir IDCLThreads.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLThread
{
public:
	///
	/// Constructeur à partir de l'interface IDCLThreads.
	///
	TDCLThread( IDCLThreads* inThreadFactory );

	///
	/// Destructeur.
	///
	virtual ~TDCLThread( void );

	///
	/// Démarre le processus léger.
	///
	inline void Start( void )
		{
			mThread->Start();
		}

	///
	/// Arrête le processus léger.
	///
	inline void Stop( void )
		{
			mThread->Stop();
		}

	///
	/// Suspend le processus léger.
	///
	inline void Suspend( void )
		{
			mThread->Suspend();
		}

	///
	/// Reprend le processus léger.
	///
	inline void Resume( void )
		{
			mThread->Resume();
		}

	///
	/// Poste un événement pour ce processus.
	/// Les événements sont gérés dans l'ordre d'arrivée (à améliorer?)
	/// L'événement sera supprimé par une méthode du processus léger
	/// qui appellera WaitNextEvent ou par le destructeur du processus léger.
	///
	/// \param inEvent	événement à poster
	/// \throws TDCLException si un problème est survenu
	///
	void PostEvent( TDCLEvent* inEvent );

	///
	/// Réveille le processus léger.
	///
	/// Remarque: l'utilisation de WakeUp/Sleep est exclusive de l'utilisation de
	/// WaitNextEvent/PostEvent.
	///
	inline void WakeUp( void )
		{
			mThread->WakeUp();
		}

	///
	/// Méthode exécutée dans le processus.
	///
	virtual void Run( void ) = 0;

	///
	/// Méthode appelée par le processus léger. C'est cette méthode
	/// qui se charge d'appeler la méthode Run ci-dessus.
	///
	void DoRun( void );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée.
	/// Lorsque cette méthode retourne, le processus est terminé.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void HandleException( TDCLException* inException ) = 0;

protected:
	///
	/// Récupère le prochain événement en attendant au plus inMilliseconds
	/// (en millisecondes). Si aucun événement n'est arrivé, retourne nil.
	/// Si le masque n'est pas kAnyEventMask, alors on attend inMilliseconds
	/// au plus entre deux événements. L'arrivée d'événements qui ne cadrent
	/// pas avec le masque peut rallonger l'attente.
	///
	/// Cette méthode appelle Sleep.
	///
	/// Remarque importante: l'appelant doit supprimer l'événement avec
	/// delete.
	///
	/// \param inEventMask		masque pour les événements à attendre.
	/// \param inMilliseconds	temps à attendre au maximum.
	/// \return un pointeur vers l'événement ou \c nil.
	/// \throws TDCLException si un problème est survenu
	///
	TDCLEvent* WaitNextEvent(
			KUInt32 inEventMask = TDCLEvent::kAnyEventMask,
			KUInt32 inMilliseconds = IDCLThreads::IThread::kForever );

	///
	/// Dort. Cette méthode doit être appelée par le processus.
	///
	/// Remarque: l'utilisation de WakeUp/Sleep est exclusive de l'utilisation de
	/// WaitNextEvent/PostEvent.
	///
	/// \param inMilliseconds	temps à dormir au maximum.
	/// \return \c true si on a dormi tout le temps, \c false sinon.
	///
	inline Boolean Sleep(
			KUInt32 inMilliseconds = IDCLThreads::IThread::kForever )
		{
			return mThread->Sleep( inMilliseconds );
		}

	///
	/// Détermine l'état du processus léger.
	///
	/// \return l'état du processus léger.
	///
	inline IDCLThreads::EState GetThreadState( void )
		{
			return mThread->GetState();
		}

	///
	/// Donne du temps aux autres processus.
	/// Cette méthode est appelée à partir de la méthode
	/// Run de TDCLThread ou du processus léger principal.
	///
	inline	void	Yield( void )
		{
			mThreadsIntf->Yield();
		}

	///
	/// Accesseur sur la fabrique de processus légers
	///
	/// \return une référence sur l'interface sur les processus légers.
	///
	inline	IDCLThreads*	GetThreadsIntf( void )
			{
				return mThreadsIntf;
			}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLThread( const TDCLThread& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLThread& operator = ( const TDCLThread& inCopy );

	/// \name Variables privées.
	IDCLThreads::IThread*		mThread;			///< Processus léger système
													///< associé.
	IDCLThreads::ISemaphore*	mQueueSemaphore;	///< Mutex sur la liste des événements.
	TDCLEvent*					mNextEvent;			///< Pointeur sur le prochain
													///< événement ou \c nil.
	TDCLEvent*					mLastEvent;			///< Pointeur sur le prochain
													///< événement ou \c nil.
	IDCLThreads*				mThreadsIntf;		///< Interface pour les processus légers.
};

#endif
		// _TDCLTHREAD_H

// ============================================================================== //
// I have sacrificed time, health, and fortune, in the desire to complete these   //
// Calculating Engines.  I have also declined several offers of great personal    //
// advantage to myself.  But, notwithstanding the sacrifice of these advantages   //
// for the purpose of maturing an engine of almost intellectual power, and        //
// after expending from my own private fortune a larger sum than the government   //
// of England has spent on that machine, the execution of which it only           //
// commenced, I have received neither an acknowledgement of my labors, not even   //
// the offer of those honors or rewards which are allowed to fall within the      //
// reach of men who devote themselves to purely scientific investigations...      //
//         If the work upon which I have bestowed so much time and thought were   //
// a mere triumph over mechanical difficulties, or simply curious, or if the      //
// execution of such engines were of doubtful practicability or utility, some     //
// justification might be found for the course which has been taken; but I        //
// venture to assert that no mathematician who has a reputation to lose will      //
// ever publicly express an opinion that such a machine would be useless if       //
// made, and that no man distinguished as a civil engineer will venture to        //
// declare the construction of such machinery impracticable...                    //
//         And at a period when the progress of physical science is obstructed    //
// by that exhausting intellectual and manual labor, indispensable for its        //
// advancement, which it is the object of the Analytical Engine to relieve, I     //
// think the application of machinery in aid of the most complicated and abtruse  //
// calculations can no longer be deemed unworthy of the attention of the country. //
// In fact, there is no reason why mental as well as bodily labor should not      //
// be economized by the aid of machinery.                                         //
//                 -- Charles Babbage, "The Life of a Philosopher"                //
// ============================================================================== //
