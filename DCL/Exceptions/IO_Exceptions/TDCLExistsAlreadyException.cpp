// ==============================
// Fichier:			TDCLExistsAlreadyException.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			31/7/2003
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
// The Original Code is TDCLExistsAlreadyException.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLExistsAlreadyException.cp,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLExistsAlreadyException.h>

#if kDCLDebugFILEandLINE

// ------------------------------------------------------------------------- //
//  * TDCLExistsAlreadyException( const char*, KUInt32, KSInt32 )
// ------------------------------------------------------------------------- //
TDCLExistsAlreadyException::TDCLExistsAlreadyException(
					const char* inFileName,
					KUInt32 inLine,
					KSInt32 inError )
	:
		TDCLIOException(
				TDCLException::exExistsAlready,
				inError,
				inFileName,
				inLine )
{
}

#else

// ------------------------------------------------------------------------- //
//  * TDCLExistsAlreadyException( KSInt32 )
// ------------------------------------------------------------------------- //
TDCLExistsAlreadyException::TDCLExistsAlreadyException( KSInt32 inError )
	:
		TDCLIOException( TDCLException::exExistsAlready, inError )
{
}

#endif

// ============================================================================= //
//         Price Wang's programmer was coding software.  His fingers danced upon //
// the keyboard.  The program compiled without an error message, and the program //
// ran like a gentle wind.                                                       //
//         Excellent!" the Price exclaimed, "Your technique is faultless!"       //
//         "Technique?" said the programmer, turning from his terminal, "What I  //
// follow is the Tao -- beyond all technique.  When I first began to program I   //
// would see before me the whole program in one mass.  After three years I no    //
// longer saw this mass.  Instead, I used subroutines.  But now I see nothing.   //
// My whole being exists in a formless void.  My senses are idle.  My spirit,    //
// free to work without a plan, follows its own instinct.  In short, my program  //
// writes itself.  True, sometimes there are difficult problems.  I see them     //
// coming, I slow down, I watch silently.  Then I change a single line of code   //
// and the difficulties vanish like puffs of idle smoke.  I then compile the     //
// program.  I sit still and let the joy of the work fill my being.  I close my  //
// eyes for a moment and then log off."                                          //
//         Price Wang said, "Would that all of my programmers were as wise!"     //
//                 -- Geoffrey James, "The Tao of Programming"                   //
// ============================================================================= //
