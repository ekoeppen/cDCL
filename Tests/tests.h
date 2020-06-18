// ==============================
// Fichier:			tests.h
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			02/02/2003
// Tabulation:		4 espaces
//
// Copyright:		© 2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: tests.h,v 1.5 2003/08/26 14:46:42 paul Exp $
// ===========

#ifndef __TESTS__
#define __TESTS__

///
/// Effectue un test donné.
/// Si ce test n'existe pas, retourne une erreur.
/// Encapsule le tout dans un try/catch.
///
/// \param inTestName	nom du test à effectuer.
/// \param inArgument	argument supplémentaire (ou \c null)
///
void test( const char* inTestName, const char* inArgument );

#endif
		// __TESTS__

// ========================================================== //
// What this country needs is a good five cent microcomputer. //
// ========================================================== //
