// ==============================
// Fichier:			TDCLTimeoutException.h
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
// The Original Code is TDCLTimeoutException.h.
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
// $Id: TDCLTimeoutException.h,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef _TDCLTIMEOUTEXCEPTION_H
#define _TDCLTIMEOUTEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLTimeout							\
		TDCLTimeoutException( __FILE__, __LINE__ )
	#define DCLPlatformTimeout( inError )		\
		TDCLTimeoutException( __FILE__, __LINE__, inError )
#else
	#define DCLTimeout							\
		TDCLTimeoutException()
	#define DCLPlatformTimeout( inError )		\
		TDCLTimeoutException( inError )
#endif
//@}

///
/// Classe pour une exception signifiant que le temps pour effectuer une
/// opération a été dépassé.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLTimeoutException
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
	TDCLTimeoutException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLTimeoutException( KSInt32 inError = 0 );
#endif
};

#endif
		// _TDCLTIMEOUTEXCEPTION_H

// =========================================================================== //
// Suppose for a moment that the automobile industry had developed at the same //
// rate as computers and over the same period:  how much cheaper and more      //
// efficient would the current models be?  If you have not already heard the   //
// analogy, the answer is shattering.  Today you would be able to buy a        //
// Rolls-Royce for $2.75, it would do three million miles to the gallon, and   //
// it would deliver enough power to drive the Queen Elizabeth II.  And if you  //
// were interested in miniaturization, you could place half a dozen of them on //
// a pinhead.                                                                  //
//                 -- Christopher Evans                                        //
// =========================================================================== //
