// ==============================
// Fichier:			DCLFinalMacOSPPCPrefix.h
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			9/4/2003
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
// The Original Code is DCLFinalMacOSPPCPrefix.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLFinalMacOSPPCPrefix.h,v 1.4 2004/11/24 14:08:59 paul Exp $
// ===========

// Ce fichier est uniquement pour CodeWarrior avec le pch.
// Il ne fait pas de mal avec d'autres compilateurs (sans pch).

#define KDebugOn 0

#ifdef __MWERKS__
	#pragma	warn_resultnotused	on
	#include "DCLFinalMacOSPPCPrefix++"
#endif

#include "MacPrefix.h"

// ================================================ //
// "What is the Nature of God?"                     //
//                                                  //
//     CLICK...CLICK...WHIRRR...CLICK...=BEEP!=     //
//     1 QT. SOUR CREAM                             //
//     1 TSP. SAUERKRAUT                            //
//     1/2 CUT CHIVES.                              //
//     STIR AND SPRINKLE WITH BACON BITS.           //
//                                                  //
// "I've just GOT to start labeling my software..." //
//                 -- Bloom County                  //
// ================================================ //