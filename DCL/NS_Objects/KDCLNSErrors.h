// ==============================
// Fichier:			KDCLNSErrors.h
// Projet:			Desktop Connection Library
//
// Créé le:			19/10/2002
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
// The Original Code is KDCLNSErrors.h.
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
// $Id: KDCLNSErrors.h,v 1.6 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __KDCLNSERRORS__
#define __KDCLNSERRORS__

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les codes d'erreur NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
class KDCLNSErrors
{
public:
	///
	/// Enumération des codes d'erreurs utilisés dans la DCL
	/// Ces codes sont les mêmes que sur NewtonOS.
	///
	/// Ajoutez vos codes selons vos besoin.
	///
	enum {
		noErr						= 0,
		kNSErrObjectCorrupted		= -48005,
		kNSErrUnknownStreamFormat	= -48006,
		kNSErrObjectPointerOfNonPtr	= -48200,
		kNSErrOutOfBounds			= -48205,
		kNSErrObjectReadOnly		= -48214,
		kNSErrOutOfRange			= -48219,
		kNSErrNotAFrame				= -48400,
		kNSErrNotAnArray			= -48401,
		kNSErrNotAString			= -48402,
		kNSErrNotAnInteger			= -48406,
		kNSErrNotACharacter			= -48407,
		kNSErrNotABinaryObject		= -48408,
		kNSErrNotASymbol			= -48410,
		kNSErrNotAMagicPtr			= -48450,	// AJOUT DCL
		kError_Bad_Package			= -10401
	};
};

#endif
		// __KDCLNSERRORS__

// ======================================================== //
// Blinding speed can compensate for a lot of deficiencies. //
//                 -- David Nichols                         //
// ======================================================== //
