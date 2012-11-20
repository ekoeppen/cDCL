// ==============================
// Fichier:			IDCLThreads.h
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
// The Original Code is IDCLThreads.h.
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
// $Id: IDCLThreads.h,v 1.6 2006/02/12 01:23:56 pguyot Exp $
// ===========

#ifndef __IDCLTHREADS__
#define __IDCLTHREADS__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>

class TDCLThread;

///
/// Interface pour gérer les processus légers.
/// Cette classe doit être dérivée pour chaque plateforme ou
/// application.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	UTestThreads
///
class IDCLThreads
{
public:
	///
	/// Etats possibles d'un processus léger.
	///
	enum EState {
		kStopped,		///< Arrêté.
		kRunning,		///< En train de tourner (n'a pas forcément le CPU)
		kSuspended,		///< Suspendu (attend un appel à la méthode Resume)
		kSleeping		///< Endormi (par la méthode Sleep) (attend WakeUp)
	};
	
	///
	/// Destructor.
	///
	virtual	~IDCLThreads( void ) {};

	///
	/// Donne du temps aux autres processus.
	/// Cette méthode est appelée à partir de la méthode
	/// Run de TDCLThread ou du processus léger principal.
	///
	virtual	void	Yield( void ) = 0;	

	///
	/// Abstraction pour les processus légers.
	///
	/// Cette interface est utilisée par TDCLThread (qui doit être dérivé pour
	/// les objets qui doivent être dans des processus légers).
	///
	/// Ces objets peuvent être en train de tourner (Running) ou pas, et s'ils
	/// sont en train de tourner, ils peuvent êtres suspendus ou pas.
	///
	class IThread
	{
	public:
		///
		/// Constantes spécifiques à IThread.	
		enum {
			kForever = 0	///< Dort jusqu'à ce que WakeUp soit appelé
		};

		///
		/// Constructeur à partir d'un objet TDCLThread.
		///
		/// \param inThreadedObject		objet à associé au processus léger.
		///
		inline IThread( TDCLThread* inThreadedObject )
			:
				mThreadedObject( inThreadedObject )
			{
			}

		///
		/// Destructor.
		///
		virtual ~IThread( void ) {};

		///
		/// Accesseur sur l'objet TDCLThread.
		///
		/// \return l'objet TDCLThread associé.
		///
		inline TDCLThread* GetThreadedObject( void )
			{
				return mThreadedObject;
			}

		///
		/// Démarre le processus.
		///
		/// Appelle la méthode Run de TDCLThread dans le nouveau
		/// processus léger.
		/// Lance une exception si le processus léger n'est pas arrêté.
		///
		/// \throws TDCLException si le processus léger n'est pas arrêté.
		///
		virtual void	Start( void ) = 0;

		///
		/// Arrête le processus (de manière violente).
		///
		/// Lance une exception si le processus léger est arrêté.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual void	Stop( void ) = 0;

		///
		/// Suspend le processus léger.
		/// Cette méthode peut être appelée à partir d'un autre processus
		/// léger.
		/// Lance une exception si le processus léger est arrêté, mais
		/// est silencieux si le processus est suspendu.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual	void	Suspend( void ) = 0;

		///
		/// Reprend le processus léger.
		/// Cette méthode est appelée à partir d'un autre processus léger.
		/// Lance une exception si le processus léger n'est pas arrêté.
		/// mais est silencieux si le processus n'est pas suspendu.
		///
		/// \throws TDCLException si le processus léger est arrêté.
		///
		virtual	void	Resume( void ) = 0;
	
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
		/// \return	\c true si l'on a dormi tout le temps demandé, \c false si
		///			on a été interrompu.
		///
		virtual	Boolean	Sleep( KUInt32 inMillisecs = kForever ) = 0;

		///
		/// Réveille un processus qui dort.
		/// Incrémente le compteur des réveils.
		/// Suivant les classes concrètes, désactive les interruptions
		/// ou utilise les variables de condition.
		///
		virtual	void	WakeUp( void ) = 0;

		///
		/// Détermine l'état du processus.
		///
		/// \return l'état du processus (arrêté, actif, suspendu)
		///
		virtual	EState	GetState( void ) = 0;

	private:
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		IThread( const IThread& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		IThread& operator = ( const IThread& inCopy );

		/// \name Variables privées.
		TDCLThread*		mThreadedObject;	///< Processus léger associé.
	};

	///
	/// Abstraction pour les sémaphores.
	///
	/// Ce sémaphore sert dans deux cas d'usage:
	///
	/// 1/ protection d'une ressource.
	/// Acquisition
	/// 				Acquisition
	/// Libération
	///					Libération
	///
	/// 2/ synchronisation
	/// Acquisition
	///	Acquisition
	///					Libération
	///	Libération
	/// 
	///
	class ISemaphore
	{
	public:
		///
		/// Destructor.
		///
		virtual	~ISemaphore( void ) {};

		///
		/// Verrouille le sémaphore.
		///
		virtual	void		Acquire( void ) = 0;
		
		///
		/// Libère un processus léger (au hasard) du sémaphore.
		///
		virtual	void		Release( void ) = 0;
	};

	///
	/// Crée un objet sémaphore. À surcharger pour chaque plateforme.
	///
	/// \return un nouvel objet sémaphore
	///
	virtual ISemaphore*	CreateSemaphore( void ) = 0;

private:
	///
	/// Seule la classe TDCLThread peut construire des threads.
	friend class TDCLThread;

	///
	/// Crée un objet IThread. À surcharger pour chaque plateforme.
	///
	/// \param inThreadedObject		objet associé à passer comme paramètre
	///								pour IThread
	/// \return un nouvel objet IThread
	///
	virtual IThread*	CreateThread( TDCLThread* inThreadedObject ) = 0;
};

#endif
		// __IDCLTHREADS__

// =========================================================== //
// I haven't lost my mind -- it's backed up on tape somewhere. //
// =========================================================== //
