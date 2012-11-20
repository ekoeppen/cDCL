// ==============================
// Fichier:			DCLDefinitions.h
// Projet:			Desktop Connection Library
//
// Créé le:			29/06/2000
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
// The Original Code is DCLDefinitions.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLDefinitions.h,v 1.5 2004/11/24 14:08:58 paul Exp $
// ===========

#ifndef __DCLDEFINITIONS__
#define __DCLDEFINITIONS__

// Inclusion des K Définitions:
#include <K/Defines/KDefinitions.h>

// K (définition de KDebugOn)
#include <K/Tests/KDebug.h>

/// Si cette constante est à 1, les exceptions comprennent le nom du fichier
/// et la ligne. Requiert les macros __FILE__ et __LINE__
#ifndef kDCLDebugFILEandLINE
	#define kDCLDebugFILEandLINE KDebugOn
#endif

#endif
		// __DCLDEFINITIONS__

// ================================ //
// COBOL is for morons.             //
//                 -- E.W. Dijkstra //
// ================================ //
