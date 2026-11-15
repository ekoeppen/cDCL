// ==============================
// Fichier:			TDCLNSException.h
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
// The Original Code is TDCLNSException.h.
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
// $Id: TDCLNSException.h,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef __TDCLNSEXCEPTION__
#define __TDCLNSEXCEPTION__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

// DCL
#include <DCL/NS_Objects/KDCLNSErrors.h>

#if kDCLDebugFILEandLINE
	#define DCLNS( inError )		\
		TDCLNSException( __FILE__, __LINE__, KDCLNSErrors::inError )
#else
	#define DCLNS( inError )		\
		TDCLNSException( KDCLNSErrors::inError )
#endif

///
/// Classe pour les exceptions dans le système NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLNSException
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
	TDCLNSException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLNSException( KSInt32 inError );
#endif
};

#endif
		// __TDCLNSEXCEPTION__

// ========================================================================= //
// A programmer is a person who passes as an exacting expert on the basis of //
// being able to turn out, after innumerable punching, an infinite series of //
// incomprehensible answers calculated with micrometric precisions from      //
// vague assumptions based on debatable figures taken from inconclusive      //
// documents and carried out on instruments of problematical accuracy by     //
// persons of dubious reliability and questionable mentality for the avowed  //
// purpose of annoying and confounding a hopelessly defenseless department   //
// that was unfortunate enough to ask for the information in the first       //
// place.                                                                    //
//                 -- IEEE Grid newsmagazine                                 //
// ========================================================================= //
