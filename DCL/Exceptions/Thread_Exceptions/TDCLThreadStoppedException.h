// ==============================
// Fichier:			TDCLThreadStoppedException.h
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
// The Original Code is TDCLThreadStoppedException.h.
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
// $Id: TDCLThreadStoppedException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLTHREADSTOPPEDEXCEPTION_H
#define _TDCLTHREADSTOPPEDEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLThreadStopped							\
		TDCLThreadStoppedException( __FILE__, __LINE__ )
	#define DCLPlatformThreadStopped( inError )			\
		TDCLThreadStoppedException( __FILE__, __LINE__, inError )
#else
	#define DCLThreadStopped							\
		TDCLThreadStoppedException()
	#define DCLPlatformThreadStopped( inError )			\
		TDCLThreadStoppedException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant que le processus léger est arrêté.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLThreadStoppedException
	:
		public TDCLThreadException
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
	TDCLThreadStoppedException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLThreadStoppedException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLTHREADSTOPPEDEXCEPTION_H

// ============================================================================ //
// About the use of language: it is impossible to sharpen a pencil with a blunt //
// ax.  It is equally vain to try to do it with ten blunt axes instead.         //
//                 -- Edsger Dijkstra                                           //
// ============================================================================ //
