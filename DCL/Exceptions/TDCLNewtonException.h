// ==============================
// Fichier:			TDCLNewtonException.h
// Projet:			Desktop Connection Library
//
// Créé le:			16/08/2002
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
// The Original Code is TDCLNewtonException.h.
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
// $Id: TDCLNewtonException.h,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLNEWTONEXCEPTION_H
#define _TDCLNEWTONEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

#if kDCLDebugFILEandLINE
	#define DCLNewton( inError )	\
		TDCLNewtonException( __FILE__, __LINE__, inError )
#else
	#define DCLNewton( inError )	\
		TDCLNewtonException( inError )
#endif

///
/// Classe pour les exceptions représentant les erreurs sur le Newton.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLNewtonException
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
	TDCLNewtonException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLNewtonException( KSInt32 inError );
#endif
};

#endif
		// _TDCLNEWTONEXCEPTION_H

// ====================================== //
// HOST SYSTEM RESPONDING, PROBABLY UP... //
// ====================================== //
