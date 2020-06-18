// ==============================
// Fichier:			TDCLOTTCP.h
// Projet:			Desktop Connection Library
//
// Créé le:			25/01/2002
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
// The Original Code is TDCLOTTCP.h.
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
// $Id: TDCLOTTCP.h,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef __TDCLOTTCP__
#define __TDCLOTTCP__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLOpenTransport.h>

///
/// Couche de communication pour TCP/IP avec OpenTransport.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLOTTCP
	:
		public TDCLOpenTransport
{
public:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kIANAAssignedPort =	3679	///< Numéro de port assigné par IANA.
	};

	///
	/// Constructeur public.
	///
	/// \param inPort			port TCP/IP sur lequel écouter.
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inMaxPipeCount	nombre maximal de connexions simultanées
	/// \param inTimeout		délai de temporisation
	///
	TDCLOTTCP(
		long			inPort,
		IDCLThreads*	inThreadsIntf,
		KUInt32			inMaxPipeCount = kDefaultPipeCount,
		long			inTimeout = kDefaultTimeout );

	///
	/// Détermine si ce service est disponible.
	///
	/// \return \c true si le service est disponible, \c false sinon.
	///
	static	Boolean		IsAvailable( void );
};

#endif
		// __TDCLOTTCP__

// ============================================================================= //
// Dear Emily:                                                                   //
//         I collected replies to an article I wrote, and now it's time to       //
// summarize.  What should I do?                                                 //
//                 -- Editor                                                     //
//                                                                               //
// Dear Editor:                                                                  //
//         Simply concatenate all the articles together into a big file and post //
// that.  On USENET, this is known as a summary.  It lets people read all the    //
// replies without annoying newsreaders getting in the way.  Do the same when    //
// summarizing a vote.                                                           //
//                 -- Emily Postnews Answers Your Questions on Netiquette        //
// ============================================================================= //
