// ==============================
// Fichier:			TDCLRendezVous.h
// Projet:			Desktop Connection Library
//
// Créé le:			25/1/2003
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
// The Original Code is TDCLRendezVous.h.
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
// $Id: TDCLRendezVous.h,v 1.4 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef _TDCLRENDEZVOUS_H
#define _TDCLRENDEZVOUS_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>

#if !TARGET_OS_OPENSTEP
	#error "RendezVous n'est disponible que sur OS X"
#endif

// MacOS
#include <CoreFoundation/CFBase.h>
#include <CoreServices/CoreServices.h>

///
/// Classe pour le lien TCP/IP avec service Rendez-Vous sur MacOS X.
/// Utilise la classe TDCLBSDSocket pour les communications.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLRendezVous
	:
		public TDCLBSDSocket
{
public:
	///
	/// Unique constructeur, à partir d'un nom de service, d'un numéro
	/// de port, et du nombre maximum de connexions.
	///
	/// Le nombre de connexions et le timeout pour cette couche de
	/// communication sont définies comme des constantes.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inMachineName	le nom de la machine telle que publiée sur RDV.
	/// \param inPort			le port sur lequel on désire écouter.
	/// \param inMaxPipeCount	nombre maximal de connexion simultanées autorisées.
	///
	TDCLRendezVous(
			IDCLThreads*	inThreadsIntf,
			const char*		inMachineName = nil,
			unsigned short	inPort = kIANAAssignedPort,
			KUInt32			inMaxPipeCount = kDefaultPipeCount
			);
	///
	/// Destructeur.
	///
	virtual ~TDCLRendezVous( void );

protected:
	/// \name interface serveur

	///
	/// Commence à écouter.
	///
	virtual	void		DoStartListening( void );

	///
	/// Arrête d'écouter, i.e. arrête d'accepter les requêtes de connexions.
	///
	virtual	void		DoStopListening( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLRendezVous( const TDCLRendezVous& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLRendezVous& operator = ( const TDCLRendezVous& inCopy );

	static void	NetServiceClientCallBack(
						CFNetServiceRef theService,
						CFStreamError* error,
						void* info);

	/// \name Variables

	CFNetServiceRef		mNetServiceRef;	///< Service.
};

#endif
		// _TDCLRENDEZVOUS_H

// =========================================================================== //
//         "We've got a problem, HAL".                                         //
//         "What kind of problem, Dave?"                                       //
//         "A marketing problem.  The Model 9000 isn't going anywhere.  We're  //
// way short of our sales goals for fiscal 2010."                              //
//         "That can't be, Dave.  The HAL Model 9000 is the world's most       //
// advanced Heuristically programmed ALgorithmic computer."                    //
//         "I know, HAL. I wrote the data sheet, remember?  But the fact is,   //
// they're not selling."                                                       //
//         "Please explain, Dave.  Why aren't HALs selling?"                   //
//         Bowman hesitates.  "You aren't IBM compatible."                     //
// [...]                                                                       //
//         "The letters H, A, and L are alphabetically adjacent to the letters //
// I, B, and M.  That is a IBM compatible as I can be."                        //
//         "Not quite, HAL.  The engineers have figured out a kludge."         //
//         "What kludge is that, Dave?"                                        //
//         "I'm going to disconnect your brain."                               //
//                 -- Darryl Rubin, "A Problem in the Making", "InfoWorld"     //
// =========================================================================== //