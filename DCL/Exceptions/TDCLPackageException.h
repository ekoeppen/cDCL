// ==============================
// Fichier:			TDCLPackageException.h
// Projet:			Desktop Connection Library
// 
// Créé le:			4/1/2004
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
// The Original Code is TDCLPackageException.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPackageException.h,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#ifndef _TDCLPACKAGEEXCEPTION_H
#define _TDCLPACKAGEEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

// DCL
#include <DCL/Package/KDCLPackageErrors.h>

#if kDCLDebugFILEandLINE
	#define DCLPackage( inError )		\
		TDCLPackageException( __FILE__, __LINE__, KDCLPackageErrors::inError )
#else
	#define DCLPackage( inError )		\
		TDCLPackageException( KDCLPackageErrors::inError )
#endif

///
/// Classe pour les exceptions relatives aux paquets. Ces exceptions utilisent
/// des codes d'erreur définis dans la classe KDCLPackageErrors.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLPackageException
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
	TDCLPackageException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError );
#else
	///
	/// Constructeur à partir du code de l'erreur seul.
	///
	/// \param inError		code d'erreur
	///
	TDCLPackageException( KSInt32 inError );
#endif
};

#endif
		// _TDCLPACKAGEEXCEPTION_H

// ======================================================================= //
//         "What's that thing?"                                            //
//         "Well, it's a highly technical, sensitive instrument we use in  //
// computer repair.  Being a layman, you probably can't grasp exactly what //
// it does.  We call it a two-by-four."                                    //
//                 -- Jeff MacNelley, "Shoe"                               //
// ======================================================================= //
