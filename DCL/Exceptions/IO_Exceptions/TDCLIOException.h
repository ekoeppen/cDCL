// ==============================
// Fichier:			TDCLIOException.h
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
// The Original Code is TDCLIOException.h.
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
// $Id: TDCLIOException.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLIOEXCEPTION_H
#define _TDCLIOEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

///
/// Classe pour les exceptions relatives aux opérations d'entrée/sortie.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLIOException
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
	TDCLIOException(
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
	TDCLIOException( ExCode inExCode, KSInt32 inError );
#endif
};

#endif
		// _TDCLIOEXCEPTION_H

// ======================================================================= //
// Grand Master Turing once dreamed that he was a machine.  When he awoke  //
// he exclaimed:                                                           //
//         "I don't know whether I am Turing dreaming that I am a machine, //
//         or a machine dreaming that I am Turing!"                        //
//                 -- Geoffrey James, "The Tao of Programming"             //
// ======================================================================= //
