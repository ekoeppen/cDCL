// ==============================
// Fichier:			TDCLDoesntExistException.h
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
// The Original Code is TDCLDoesntExistException.h.
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
// $Id: TDCLDoesntExistException.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLDOESNTEXISTEXCEPTION_H
#define _TDCLDOESNTEXISTEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLDoesntExist							\
		TDCLDoesntExistException( __FILE__, __LINE__ )
	#define DCLPlatformDoesntExist( inError )		\
		TDCLDoesntExistException( __FILE__, __LINE__, inError )
#else
	#define DCLDoesntExist							\
		TDCLDoesntExistException()
	#define DCLPlatformDoesntExist( inError )		\
		TDCLDoesntExistException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant qu'un fichier n'existe pas.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLDoesntExistException
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
	TDCLDoesntExistException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLDoesntExistException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLDOESNTEXISTEXCEPTION_H

// =============================================================================== //
// ... an anecdote from IBM's Yorktown Heights Research Center.  When a            //
// programmer used his new computer terminal, all was fine when he was sitting     //
// down, but he couldn't log in to the system when he was standing up.  That       //
// behavior was 100 percent repeatable: he could always log in when sitting and    //
// never when standing.                                                            //
//                                                                                 //
// Most of us just sit back and marvel at such a story; how could that terminal    //
// know whether the poor guy was sitting or standing?  Good debuggers, though,     //
// know that there has to be a reason.  Electrical theories are the easiest to     //
// hypothesize: was there a loose with under the carpet, or problems with static   //
// electricity?  But electrical problems are rarely consistently reproducible.     //
// An alert IBMer finally noticed that the problem was in the terminal's keyboard: //
// the tops of two keys were switched.  When the programmer was seated he was a    //
// touch typist and the problem went unnoticed, but when he stood he was led       //
// astray by hunting and pecking.                                                  //
//         -- "Programming Pearls" column, by Jon Bentley in CACM February 1985    //
// =============================================================================== //
