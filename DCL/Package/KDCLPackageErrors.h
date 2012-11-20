// ==============================
// Fichier:			KDCLPackageErrors.h
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
// The Original Code is KDCLPackageErrors.h.
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
// $Id: KDCLPackageErrors.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _KDCLPACKAGEERRORS_H
#define _KDCLPACKAGEERRORS_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les codes d'erreurs relatifs aux paquets.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class KDCLPackageErrors
{
public:
	///
	/// Enumération des codes d'erreurs utilisés dans la DCL pour les paquets.
	///
	enum {
		noErr						= 0,
		kPkgErrUnknownSignature		= -10450,	///< Signature qui n'est pas
												///< package{0,1}
		kPkgStringsDontSeemUniCStr	= -10451,	///< Les chaînes (copyright &
												///< nom) ne semblent pas être
												///< correctes.
		kPkgErrWeirdDirectorySize	= -10452,	///< Taille du catalogue
												///< incorrecte par rapport aux
												///< données du catalogue.
		kPkgErrWeirdPartSize		= -10453,	///< Taille d'une partie
												///< incorrecte par rapport à
												///< la taille du paquet.
		kPkgErrNoName				= -10454	///< Taille du nom nulle.
	};
};

#endif
		// _KDCLPACKAGEERRORS_H

// ========================================================================== //
// If I have seen farther than others, it is because I was standing on the    //
// shoulders of giants.                                                       //
//                 -- Isaac Newton                                            //
//                                                                            //
// In the sciences, we are now uniquely priviledged to sit side by side with  //
// the giants on whose shoulders we stand.                                    //
//                 -- Gerald Holton                                           //
//                                                                            //
// If I have not seen as far as others, it is because giants were standing on //
// my shoulders.                                                              //
//                 -- Hal Abelson                                             //
//                                                                            //
// Mathematicians stand on each other's shoulders.                            //
//                 -- Gauss                                                   //
//                                                                            //
// Mathemeticians stand on each other's shoulders while computer scientists   //
// stand on each other's toes.                                                //
//                 -- Richard Hamming                                         //
//                                                                            //
// It has been said that physicists stand on one another's shoulders.  If     //
// this is the case, then programmers stand on one another's toes, and        //
// software engineers dig each other's graves.                                //
//                 -- Unknown                                                 //
// ========================================================================== //
