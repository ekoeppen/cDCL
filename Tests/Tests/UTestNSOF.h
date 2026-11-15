// ==============================
// Fichier:			UTestNSOF.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			21/10/2002
// Tabulation:		4 espaces
//
// Copyright:		© 2002-2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestNSOF.h,v 1.6 2003/09/17 10:43:49 paul Exp $
// ===========

#ifndef _UTESTNSOF_H
#define _UTESTNSOF_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les tests de l'encodeur et du décodeur NSOF.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
class UTestNSOF
{
public:
	///
	/// Test "Walter Smith"
	/// Encode l'exemple de Walter Smith.
	///
	static void WalterSmith( void );
};

#endif
		// _UTESTNSOF_H

// ====================================================================== //
// Nobody's gonna believe that computers are intelligent until they start //
// coming in late and lying about it.                                     //
// ====================================================================== //
