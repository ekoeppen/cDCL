// ==============================
// Fichier:			TDCLUnexpDockCmdException.h
// Projet:			Desktop Connection Library
//
// Créé le:			11/5/2003
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
// The Original Code is TDCLUnexpDockCmdException.h.
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
// $Id: TDCLUnexpDockCmdException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLUNEXPDOCKCMDEXCEPTION_H
#define _TDCLUNEXPDOCKCMDEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLLinkException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLUnexpDockCmd							\
		TDCLUnexpDockCmdException( __FILE__, __LINE__ )
	#define DCLPlatformUnexpDockCmd( inError )		\
		TDCLUnexpDockCmdException( __FILE__, __LINE__, inError )
#else
	#define DCLUnexpDockCmd							\
		TDCLUnexpDockCmdException()
	#define DCLPlatformUnexpDockCmd( inError )		\
		TDCLUnexpDockCmdException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant qu'une commande inattendue a été reçue
/// du Newton.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLUnexpDockCmdException
	:
		public TDCLLinkException
{
public:
#if kDCLDebugFILEandLINE
	///
	/// Constructeur à partir du code de l'erreur, du nom de fichier
	/// et du numéro de ligne.
	///
	/// \param inFileName	nom du fichier. N'est pas copié.
	/// \param inLine		numéro de ligne.
	/// \param inError		code d'erreur
	///
	TDCLUnexpDockCmdException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLUnexpDockCmdException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLUNEXPDOCKCMDEXCEPTION_H

// ============================================================================== //
//         In the east there is a shark which is larger than all other fish.  It  //
// changes into a bird whose winds are like clouds filling the sky.  When this    //
// bird moves across the land, it brings a message from Corporate Headquarters.   //
// This message it drops into the midst of the program mers, like a seagull       //
// making its mark upon the beach.  Then the bird mounts on the wind and, with    //
// the blue sky at its back, returns home.                                        //
//         The novice programmer stares in wonder at the bird, for he understands //
// it not.  The average programmer dreads the coming of the bird, for he fears    //
// its message.  The master programmer continues to work at his terminal, for he  //
// does not know that the bird has come and gone.                                 //
//                 -- Geoffrey James, "The Tao of Programming"                    //
// ============================================================================== //
