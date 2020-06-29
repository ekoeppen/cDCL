// ==============================
// Fichier:			UTestPkg.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			28/2/2004
// Tabulation:		4 espaces
// 
// Copyright:		© 2004 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestPkg.h,v 1.1 2004/02/28 12:46:36 paul Exp $
// ===========

#ifndef _UTESTPKG_H
#define _UTESTPKG_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les tests de l'encodeur et du décodeur Pkg.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.1 $
///
class UTestPkg
{
public:
	///
	/// Test "Walter Smith"
	/// Encode l'exemple de Walter Smith.
	///
	static void WalterSmith( void );

	///
	/// IdemPotent test.
	/// Ensure we can decode and recode a package identically.
	///
	static void IdemPotent( void );
};

#endif
		// _UTESTPKG_H

// ========================================================================== //
// I asked the engineer who designed the communication terminal's keyboards   //
// why these were not manufactured in a central facility, in view of the      //
// small number needed [1 per month] in his factory.  He explained that this  //
// would be contrary to the political concept of local self-sufficiency.      //
// Therefore, each factory needing keyboards, no matter how few, manufactures //
// them completely, even molding the keypads.                                 //
//                 -- Isaac Auerbach, IEEE "Computer", Nov. 1979              //
// ========================================================================== //
