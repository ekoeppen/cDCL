// ==============================
// Fichier:			TDCLCTBADSP.h
// Projet:			Desktop Connection Library
//
// Créé le:			21/01/2002
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
// The Original Code is TDCLCTBADSP.h.
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
// $Id: TDCLCTBADSP.h,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef __TDCLCTBADSP__
#define __TDCLCTBADSP__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLCommToolBox.h>

///
/// Classe pour l'outil ADSP de la CTB.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLCTBADSP
	:
		public TDCLCommToolBox
{
public:
	///
	/// Constructeur.
	///
	/// Si le nom est \c nil, on récupère le nom de la machine.
	///
	/// \param inThreadsIntf	interface pour les processus légers
	/// \param inMachineName	nom de la machine ou \c nil
	/// \param inMaxPipeCount	nombre maximal de connexions simultanées.
	/// \param inTimeout		délai de temporisation.
	/// \throws exNotAvailable si le service n'est pas disponible.
	///
	TDCLCTBADSP(
			IDCLThreads* inThreadsIntf,
			const char* inMachineName = nil,
			KUInt32	inMaxPipeCount = kDefaultPipeCount,
			long	inTimeout = kDefaultTimeout );


	///
	/// Destructeur.
	///
	virtual				~TDCLCTBADSP( void ) {}
	
	///
	/// Détermine si le service est disponible.
	///
	/// \return \c true si le service est disponible, \c false sinon.
	///
	static Boolean		IsAvailable ( void );
};

#endif
		// __TDCLCTBADSP__

// ======================================================== //
// Your computer account is overdrawn.  Please reauthorize. //
// ======================================================== //