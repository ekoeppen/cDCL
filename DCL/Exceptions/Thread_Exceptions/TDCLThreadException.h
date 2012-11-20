// ==============================
// Fichier:			TDCLThreadException.h
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
// The Original Code is TDCLThreadException.h.
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
// $Id: TDCLThreadException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLTHREADEXCEPTION_H
#define _TDCLTHREADEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

///
/// Classe pour les exceptions relatives aux processus légers.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLThreadException
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
	TDCLThreadException(
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
	TDCLThreadException( ExCode inExCode, KSInt32 inError );
#endif
};

#endif
		// _TDCLTHREADEXCEPTION_H

// ============================================================================= //
// As the system comes up, the component builders will from time to time appear, //
// bearing hot new versions of their pieces -- faster, smaller, more complete,   //
// or putatively less buggy.  The replacement of a working component by a new    //
// version requires the same systematic testing procedure that adding a new      //
// component does, although it should require less time, for more complete and   //
// efficient test cases will usually be available.                               //
//                 -- Frederick Brooks Jr., "The Mythical Man Month"             //
// ============================================================================= //
