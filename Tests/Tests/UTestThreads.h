//==============================
// Fichier:			UTestThreads.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			14/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestThreads.h,v 1.5 2004/04/30 16:20:45 paul Exp $
// ===========

#ifndef _UTESTTHREADS_H
#define _UTESTTHREADS_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Interfaces/TDCLThread.h>

///
/// Classe pour les tests des mises en place de IDCLThreads.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class UTestThreads
{
public:
	///
	/// Test pour les threads POSIX.
	///
	static void Posix( void );

private:
	///
	/// Classe pour le test.
	/// Ce processus léger acquiert deux fois le sémaphore fourni comme
	/// paramètre.
	///
	class TBlocker
		:
			public TDCLThread
	{
	public:
		///
		/// La classe UTestThreads peut déterminer notre état interne.
		///
		friend class UTestThreads;

		///
		/// Constructeur à partir d'un mutex.
		///
		/// \param inThreadsIntf	interface pour les processus légers.
		/// \param inSemaphore		sémaphore utilisé pour se synchroniser.
		/// \param inSemaphore		sémaphore utilisé comme mutex
		///
		TBlocker(
				IDCLThreads* inThreadsIntf,
				IDCLThreads::ISemaphore* inSemaphore,
				IDCLThreads::ISemaphore* inMutex );
		
		///
		/// Méthode exécutée dans le processus.
		///
		virtual void Run( void );
	
		///
		/// Méthode appelée lorsqu'une exception n'est pas interceptée.
		/// Lorsque cette méthode retourne, le processus est terminé.
		///
		/// \param inException	l'exception si c'est une TDCLException,
		///			\c nil sinon.
		///
		virtual void HandleException( TDCLException* inException );

	private:
		///
		/// Constructeur par copie volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TBlocker( const TBlocker& inCopy );

		///
		/// Opérateur d'assignation volontairement indisponible.
		///
		/// \param inCopy		objet à copier
		///
		TBlocker& operator = ( const TBlocker& inCopy );

		/// \name Variables

		IDCLThreads::ISemaphore*	mSemaphore;	///< Référence sur le sémaphore.
		IDCLThreads::ISemaphore*	mMutex;		///< Référence sur le mutex.
	};

	///
	/// Test pour une mise en place de IDCLThreads donnée.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	///
	static void TestThreads( IDCLThreads* inThreadsIntf );
};

#endif
		// _UTESTTHREADS_H

// ======================================================================= //
// If God had intended Man to program, we'd be born with serial I/O ports. //
// ======================================================================= //
