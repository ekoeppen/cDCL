// ==============================
// Fichier:			TDCLThreadActiveException.h
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
// The Original Code is TDCLThreadActiveException.h.
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
// $Id: TDCLThreadActiveException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLTHREADACTIVEEXCEPTION_H
#define _TDCLTHREADACTIVEEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLThreadActive							\
		TDCLThreadActiveException( __FILE__, __LINE__ )
	#define DCLPlatformThreadActive( inError )		\
		TDCLThreadActiveException( __FILE__, __LINE__, inError )
#else
	#define DCLThreadActive							\
		TDCLThreadActiveException()
	#define DCLPlatformThreadActive( inError )		\
		TDCLThreadActiveException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant que le processus léger est actif.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLThreadActiveException
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
	TDCLThreadActiveException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLThreadActiveException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLTHREADACTIVEEXCEPTION_H

// ============================================================================== //
//                 The Guy on the Right Doesn't Stand a Chance                    //
// The guy on the right has the Osborne 1, a fully functional computer system     //
// in a portable package the size of a briefcase.  The guy on the left has an     //
// Uzi submachine gun concealed in his attache case.  Also in the case are four   //
// fully loaded, 32-round clips of 125-grain 9mm ammunition.  The owner of the    //
// Uzi is going to get more tactical firepower delivered -- and delivered on      //
// target -- in less time, and with less effort.  All for $795. It's inevitable.  //
// If you're going up against some guy with an Osborne 1 -- or any personal       //
// computer -- he's the one who's in trouble.  One round from an Uzi can zip      //
// through ten inches of solid pine wood, so you can imagine what it will do      //
// to structural foam acrylic and sheet aluminum.  In fact, detachable magazines  //
// for the Uzi are available in 25-, 32-, and 40-round capacities, so you can     //
// take out an entire office full of Apple II or IBM Personal Computers tied      //
// into Ethernet or other local-area networks.  What about the new 16-bit         //
// computers, like the Lisa and Fortune?  Even with the Winchester backup,        //
// they're no match for the Uzi.  One quick burst and they'll find out what       //
// Unix means.  Make your commanding officer proud.  Get an Uzi -- and come home  //
// a winner in the fight for office automatic weapons.                            //
//                 -- "InfoWorld", June, 1984                                     //
// ============================================================================== //
