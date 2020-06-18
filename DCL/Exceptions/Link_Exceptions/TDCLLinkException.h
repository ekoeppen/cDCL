// ==============================
// Fichier:			TDCLLinkException.h
// Projet:			Desktop Connection Library
//
// Créé le:			12/5/2003
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
// The Original Code is TDCLLinkException.h.
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
// $Id: TDCLLinkException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLLINKEXCEPTION_H
#define _TDCLLINKEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

///
/// Classe pour les exceptions relatives au lien.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLLinkException
	:
		public TDCLException
{
protected:
#if kDCLDebugFILEandLINE
	///
	/// Constructeur à partir du code de l'exception, d'un code d'erreur, du
	/// nom de fichier et du numéro de ligne.
	///
	/// \param inExCode		code de l'exception.
	/// \param inError		code d'erreur.
	/// \param inFileName	nom du fichier. N'est pas copié.
	/// \param inLine		numéro de ligne.
	///
	TDCLLinkException(
			ExCode inExCode,
			KSInt32 inError,
			const char* inFileName,
			KUInt32 inLine );
#else
	///
	/// Constructeur à partir du code de l'exception et de l'erreur seuls.
	///
	/// \param inExCode		code de l'exception.
	/// \param inError		code d'erreur
	///
	TDCLLinkException( ExCode inExCode, KSInt32 inError );
#endif
};

#endif
		// _TDCLLINKEXCEPTION_H

// ============================================================================= //
// Overconfidence breeds error when we take for granted that the game will       //
// continue on its normal course; when we fail to provide for an unusually       //
// powerful resource -- a check, a sacrifice, a stalemate.  Afterwards the       //
// victim may wail, `But who could have dreamt of such an idiotic-looking move?' //
//                 -- Fred Reinfeld, "The Complete Chess Course"                 //
// ============================================================================= //
