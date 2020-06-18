// ==============================
// Fichier:			UTestXML.h
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
// $Id: UTestXML.h,v 1.6 2003/09/17 10:43:50 paul Exp $
// ===========

#ifndef __UTESTXML__
#define __UTESTXML__

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les tests de l'encodeur XML.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \bug	aucun bogue connu.
/// \test	aucun test défini.
///
class UTestXML
{
public:
	///
	/// Test "Walter Smith"
	/// Encode l'exemple de Walter Smith.
	///
	static void WalterSmith( void );

	///
	/// Test sur les classes.
	///
	static void Classes( void );
};

#endif
		// __UTESTXML__

// ============================================================================= //
//         This is where the bloodthirsty license agreement is supposed to go,   //
// explaining that Interactive Easyflow is a copyrighted package licensed for    //
// use by a single person, and sternly warning you not to pirate copies of it    //
// and explaining, in detail, the gory consequences if you do.                   //
//         We know that you are an honest person, and are not going to go around //
// pirating copies of Interactive Easyflow; this is just as well with us since   //
// we worked hard to perfect it and selling copies of it is our only method of   //
// making anything out of all the hard work.                                     //
//         If, on the other hand, you are one of those few people who do go      //
// around pirating copies of software you probably aren't going to pay much      //
// attention to a license agreement, bloodthirsty or not.  Just keep your doors  //
// locked and look out for the HavenTree attack shark.                           //
//                 -- License Agreement for Interactive Easyflow                 //
// ============================================================================= //
