// ==============================
// Fichier:			UTestNS.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			31/01/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestNS.h,v 1.10 2004/03/21 16:09:32 paul Exp $
// ===========

#ifndef __UTESTNS__
#define __UTESTNS__

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour les tests relatifs aux objets NewtonScript.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.10 $
///
class UTestNS
{
public:
	///
	/// Retourne la structure Walter Smith.
	///
	static TDCLNSRef GetWalterSmith( void );

	///
	/// Test du tri des clés dans une structure NS: ajoute des
	/// clés au hasard puis essaye de récupérer les valeurs
	/// correctement.
	///
	static void FrameKeysSort( void );

	///
	/// Test de la concaténation de chaînes.
	///
	static void CatenateStrings( void );

	///
	/// Test sur les références sur les objets NS.
	///
	static void References( void );

	///
	/// Test d'opérations sur les tableaux et les structures.
	///
	static void ArraysAndFrames( void );
};

#endif
		// __UTESTNS__

// ====================== //
// Send some filthy mail. //
// ====================== //
