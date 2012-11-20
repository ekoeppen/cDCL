// ==============================
// Fichier:			TDCLNotImplementedError.h
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
// The Original Code is TDCLNotImplementedError.h.
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
// $Id: TDCLNotImplementedError.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLNOTIMPLEMENTEDERROR_H
#define _TDCLNOTIMPLEMENTEDERROR_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLNotImplementedError							\
		TDCLNotImplementedError( __FILE__, __LINE__ )
	#define DCLPlatformNotImplementedError( inError )		\
		TDCLNotImplementedError( __FILE__, __LINE__, inError )
#else
	#define DCLNotImplementedError							\
		TDCLNotImplementedError()
	#define DCLPlatformNotImplementedError( inError )		\
		TDCLNotImplementedError( inError )
#endif
//@}

///
/// Classe pour une exception signifiant que le code n'a pas encore été écrit
/// ou alors que les méthodes virtuelles kivonbien n'ont pas été dérivées.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLNotImplementedError
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
	TDCLNotImplementedError(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLNotImplementedError( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLNOTIMPLEMENTEDERROR_H

// =============================================== //
// This system will self-destruct in five minutes. //
// =============================================== //
