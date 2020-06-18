//==============================
// Fichier:			TTestApplication.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			17/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: TTestApplication.h,v 1.3 2003/09/17 10:43:48 paul Exp $
// ===========

#ifndef _TTESTAPPLICATION_H
#define _TTESTAPPLICATION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/Common/TDCLLogApplication.h>

///
/// Classe pour une application qui raconte sa vie sur stdout.
/// Dérive de TDCLLogApplication.
/// Remarque: stdout est ouvert avec un ::fopen.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TTestApplication
	:
		public TDCLLogApplication
{
public:
	///
	/// Constructeur à partir d'une interface pour les processus légers.
	/// Les informations sont envoyées sur stdout.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	///
	TTestApplication( IDCLThreads* inThreadsIntf );

	///
	/// Destructeur.
	///
	virtual ~TTestApplication( void );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'un/du serveur.
	///
	/// \param inServer		serveur qui s'est vautré.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleServerException(
							TDCLServer* inServer,
							TDCLException* inException );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'une/de la couche de communication.
	///
	/// \param inCommLayer	couche qui s'est vautrée.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException );

	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger d'un/du lien.
	///
	/// \param inLink		lien qui s'est vautré.
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleLinkException(
							TDCLLink* inLink,
							TDCLException* inException );


protected:
	///
	/// Méthode appelée lorsqu'une exception n'est pas interceptée dans
	/// le processus léger. Cette méthode est appelée dans ce processus léger
	/// qui se termine lorsque la méthode retourne.
	///
	/// \param inException	l'exception si c'est une TDCLException,
	///			\c nil sinon.
	///
	virtual void 	HandleException( TDCLException* inException );

	///
	/// Méthode appelée lorsqu'une exception (quelconque) est arrivée.
	/// Par défaut, ne fait rien.
	///
	virtual void 	ExceptionOccurred( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TTestApplication( const TTestApplication& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TTestApplication& operator = ( const TTestApplication& inCopy );

	///
	/// Retourne l'abstraction pour les fichiers.
	/// Cette méthode est appelée la première fois que
	/// GetFilesIntf est appelée.
	///
	/// \return une abstraction pour utiliser les fichiers.
	///
	virtual IDCLFiles*		CreateFilesIntf( void );
	
	///
	/// Retourne l'abstraction pour les processus légers.
	/// Cette méthode est appelée la première fois que
	/// GetThreadsIntf est appelée.
	///
	/// \return une abstraction pour les processus légers.
	///
	virtual IDCLThreads*	CreateThreadsIntf( void );

	/// \name Variables

	IDCLThreads*				mThreadsIntf;	///< Interface pour les processus légers.
};

#endif
		// _TTESTAPPLICATION_H

// ======================================================================== //
// Real programmers don't bring brown-bag lunches.  If the vending machine  //
// doesn't sell it, they don't eat it.  Vending machines don't sell quiche. //
// ======================================================================== //
