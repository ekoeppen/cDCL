// ==============================
// Fichier:			TDCLClassCastException.h
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
// The Original Code is TDCLClassCastException.h.
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
// $Id: TDCLClassCastException.h,v 1.3 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLCLASSCASTEXCEPTION_H
#define _TDCLCLASSCASTEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLClassCast							\
		TDCLClassCastException( __FILE__, __LINE__ )
	#define DCLPlatformClassCast( inError )			\
		TDCLClassCastException( __FILE__, __LINE__, inError )
#else
	#define DCLClassCast							\
		TDCLClassCastException()
	#define DCLPlatformClassCast( inError )			\
		TDCLClassCastException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant que l'objet n'est pas de la bonne
/// classe. En particulier, une telle exception est levée lorsqu'on essaye de
/// dé-référencer une référence (telle TDCLFSItemRef) en un objet de la
/// mauvaise classe (telle TDCLFile).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLClassCastException
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
	TDCLClassCastException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLClassCastException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLCLASSCASTEXCEPTION_H

// ========================================================================== //
//         In the beginning there was data.  The data was without form and    //
// null, and darkness was upon the face of the console; and the Spirit of     //
// IBM was moving over the face of the market.  And DEC said, "Let there      //
// be registers"; and there were registers.  And DEC saw that they            //
// carried; and DEC separated the data from the instructions.  DEC called     //
// the data Stack, and the instructions they called Code.  And there was      //
// evening and there was morning, one interrupt.                              //
//                 -- Rico Tudor, "The Story of Creation or, The Myth of Urk" //
// ========================================================================== //
