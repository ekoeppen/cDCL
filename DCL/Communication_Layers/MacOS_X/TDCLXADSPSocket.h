// ==============================
// Fichier:			TDCLXADSPSocket.h
// Projet:			Desktop Connection Library
// 
// Créé le:			25/4/2004
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
// The Original Code is TDCLXADSPSocket.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLXADSPSocket.h,v 1.3 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef _TDCLXADSPSOCKET_H
#define _TDCLXADSPSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLThread.h>

// DCL
#include <DCL/Interfaces/IDCLThreads.h>

///
/// Classe pour l'émulation socket BSD des sockets ADSP.
/// Les sockets ADSP sous MacOS X:
/// - n'utilisent pas send/recv
/// - gèrent très mal select
/// Cette classe gère l'émulation des sockets ADSP connectées (uniquement).
/// Elle fonctionne avec un processus léger qui lit tout ce qui se présente
/// et l'envoie sur une socket pair.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLXADSPSocket
	:
		protected TDCLThread
{
public:
	///
	/// Constructeur à partir d'une socket ADSP
	///
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inADSPSocket		interface de communication ADSP.
	///
	TDCLXADSPSocket(
			IDCLThreads* inThreadsIntf,
			int inADSPSocket );

	///
	/// Destructeur.
	///
	virtual ~TDCLXADSPSocket( void );

	///
	/// Accesseur sur la socket publique.
	///
	/// \return l'interface de communication BSD publique.
	///
	inline int	GetBSDSocket( void )
		{
			return mPublicPairMember;
		}

	///
	/// Ferme la paire et l'interface ADSP.
	/// Ceci a aussi pour conséquence de sortir de la boucle du processus
	/// léger.
	///
	void			Close( void );

	///
	/// Détermine si une exception est survenue.
	///
	/// \return \c true si une exception est survenue.
	///
	inline Boolean	HadException( void ) const
		{
			return mHadException;
		}

	///
	/// Détermine l'exception qui est survenue.
	///
	/// \return l'exception survenue ou \c nil si ce n'est pas une
	///			TDCLException.
	///
	inline TDCLException*	GetException( void ) const
		{
			return mException;
		}

private:
	///
	/// Méthode exécutée dans le processus.
	/// Boucle tant que WaitForIncomingRequest retourne \c true. À chaque fois que
	/// cette dernière méthode retourne \c true, envoie un événement au serveur.
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

	///
	/// Méthode appelée lorsqu'une erreur est survenue.
	/// Lance une exception si nécessaire.
	///
	/// \return \c true si l'erreur est EBADF (socket fermée)
	///
	Boolean 	HandleError( void ) const;

	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLXADSPSocket( const TDCLXADSPSocket& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLXADSPSocket& operator = ( const TDCLXADSPSocket& inCopy );

	/// \name Variables
	int				mADSPSocket;			///< Interface ADSP.
	int				mPublicPairMember;		///< Élément public de la
											///< "socket pair"
	int				mPrivatePairMember;		///< Élément privé (= thread) de la
											///< "socket pair"
	Boolean			mSocketsAreClosed;		///< Si les sockets sont fermées.
	TDCLException*	mException;				///< Exception
	Boolean			mHadException;			///< Si une exception est survenue.
};

#endif
		// _TDCLXADSPSOCKET_H

// ================ //
// /usr/news/gotcha //
// ================ //
