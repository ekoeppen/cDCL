// ==============================
// Fichier:			UDCLMacMachineName.h
// Projet:			Desktop Connection Library
//
// Créé le:			26/01/2002
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
// The Original Code is UDCLMacMachineName.h.
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
// $Id: UDCLMacMachineName.h,v 1.4 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef __UDCLMACMACHINENAME__
#define __UDCLMACMACHINENAME__

#include <DCL/Headers/DCLDefinitions.h>

#if !TARGET_OS_MAC
	#error "UDCLMacMachineName is only for MacOS."
#endif

#if TARGET_OS_OPENSTEP
	#include <CoreFoundation/CFString.h>
#endif

// DCL
#include <DCL/Exceptions/TDCLNotAvailableException.h>

///
/// Classe pour récupérer le nom de la machine sur Mac.
///
/// Code provenant de la Q&A 11078 (ici modifié)
/// http://developer.apple.com/qa/qa2001/qa1078.html
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class UDCLMacMachineName
{
public:
	///
	/// Récupère le nom de cette machine.
	/// Suivant la version du système, on utilise une des trois méthodes
	/// suivantes:
	/// - Ressource (< X)
	/// - Fichier (10.0.x)
	/// - CSCopyMachineName (>= 10.1)
	///
	/// \return le nom de cette machine alloué avec \c malloc.
	/// \throws TDCLNotAvailableException si le nom n'a pu être récupéré.
	///
	static const char*	GetMachineName( void );

private:
#if TARGET_OS_OPENSTEP
	///
	/// Versions de MacOS X
	///
	enum {
	    kMacOSX10point1		= 0x01010	///< Avant: fichier.
	    								///< Sinon, fonction Carbon
	};

	///
	/// Récupère le nom de la machine avec la méthode du fichier.
	///
	/// \return le nom de cette machine alloué avec \c malloc.
	/// \throws TDCLNotAvailableException si le fichier n'est pas présent ou ne
	///			peut être lu
	///
	static const char*	ReadMachineNameFromFile( void );

#else // TARGET_OS_MACOS
	///
	/// Constantes pour cette classe.
	///
	enum {
		kMachineNameSTRID	= -16413	///< ID de la ressource
	};

	///
	/// Récupère le nom de la machine avec la méthode de la ressource.
	///
	/// \return le nom de cette machine alloué avec \c malloc.
	/// \throws TDCLNotAvailableException si le nom n'a pu être récupéré.
	///
	static const char*	ReadMachineNameFromResource( void );
#endif
};

#endif
		// __UDCLMACMACHINENAME__

// =========================================================================== //
// The so-called "desktop metaphor" of today's workstations is instead an      //
// "airplane-seat" metaphor.  Anyone who has shuffled a lap full of papers     //
// while seated between two portly passengers will recognize the difference -- //
// one can see only a very few things at once.                                 //
//                 -- Fred Brooks                                              //
// =========================================================================== //