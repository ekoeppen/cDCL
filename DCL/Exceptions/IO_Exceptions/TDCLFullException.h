// ==============================
// Fichier:			TDCLFullException.h
// Projet:			Desktop Connection Library
// 
// Créé le:			31/7/2003
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
// The Original Code is TDCLFullException.h.
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
// $Id: TDCLFullException.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLFULLEXCEPTION_H
#define _TDCLFULLEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLFull						\
		TDCLFullException( __FILE__, __LINE__ )
	#define DCLPlatformFull( inError )		\
		TDCLFullException( __FILE__, __LINE__, inError )
#else
	#define DCLFull						\
		TDCLFullException()
	#define DCLPlatformFull( inError )		\
		TDCLFullException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant qu'un disque (ou un dossier) est plein.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFullException
	:
		public TDCLIOException
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
	TDCLFullException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLFullException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLFULLEXCEPTION_H

// =========================================================== //
// A well-used door needs no oil on its hinges.                //
// A swift-flowing steam does not grow stagnant.               //
// Neither sound nor thoughts can travel through a vacuum.     //
// Software rots if not used.                                  //
//                                                             //
// These are great mysteries.                                  //
//                 -- Geoffrey James, "The Tao of Programming" //
// =========================================================== //
