// ==============================
// Fichier:			TDCLUnknownError.h
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
// The Original Code is TDCLUnknownError.h.
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
// $Id: TDCLUnknownError.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLUNKNOWNERROR_H
#define _TDCLUNKNOWNERROR_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLUnknownError							\
		TDCLUnknownError( __FILE__, __LINE__ )
	#define DCLPlatformUnknownError( inError )		\
		TDCLUnknownError( __FILE__, __LINE__, inError )
#else
	#define DCLUnknownError							\
		TDCLUnknownError()
	#define DCLPlatformUnknownError( inError )		\
		TDCLUnknownError( inError )
#endif
//@}

///
/// Classe pour une exception signifiant une erreur inconnue.
/// Cette classe est typiquement utilisée pour les opérations qui doivent
/// normalement réussir mais qui retournent un code d'erreur.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLUnknownError
	:
		public TDCLException
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
	TDCLUnknownError(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLUnknownError( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLUNKNOWNERROR_H

// ======================================================================= //
// The following quote is from page 4-27 of the MSCP Basic Disk Functions  //
// Manual which is part of the UDA50 Programmers Doc Kit manuals:          //
//                                                                         //
// As stated above, the host area of a disk is structured as a vector of   //
// logical blocks.  From a performance viewpoint, however, it is more      //
// appropriate to view the host area as a four dimensional hyper-cube, the //
// four dimensions being cylinder, group, track, and sector.               //
// . . .                                                                   //
// Referring to our hyper-cube analogy, the set of potentially accessible  //
// blocks form a line parallel to the track axis.  This line moves         //
// parallel to the sector axis, wrapping around when it reaches the edge   //
// of the hyper-cube.                                                      //
// ======================================================================= //
