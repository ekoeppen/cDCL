// ==============================
// Fichier:			TDCLNotImplementedError.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			11/5/2003
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
// The Original Code is TDCLNotImplementedError.cp.
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
// $Id: TDCLNotImplementedError.cp,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>

#if kDCLDebugFILEandLINE

// ------------------------------------------------------------------------- //
//  * TDCLNotImplementedError( const char*, KUInt32, KSInt32 )
// ------------------------------------------------------------------------- //
TDCLNotImplementedError::TDCLNotImplementedError(
					const char* inFileName,
					KUInt32 inLine,
					KSInt32 inError )
	:
		TDCLException(
				TDCLException::exNotImplementedError,
				inError,
				inFileName,
				inLine )
{
}

#else

// ------------------------------------------------------------------------- //
//  * TDCLNotImplementedError( KSInt32 )
// ------------------------------------------------------------------------- //
TDCLNotImplementedError::TDCLNotImplementedError( KSInt32 inError )
	:
		TDCLException( TDCLException::exNotImplementedError, inError )
{
}

#endif

// ============================================================== //
// X windows:                                                     //
//         We will dump no core before its time.                  //
//         One good crash deserves another.                       //
//         A bad idea whose time has come.  And gone.             //
//         We make excuses.                                       //
//         It didn't even look good on paper.                     //
//         You laugh now, but you'll be laughing harder later!    //
//         A new concept in abuser interfaces.                    //
//         How can something get so bad, so quickly?              //
//         It could happen to you.                                //
//         The art of incompetence.                               //
//         You have nothing to lose but your lunch.               //
//         When uselessness just isn't enough.                    //
//         More than a mere hindrance.  It's a whole new barrier! //
//         When you can't afford to be right.                     //
//         And you thought we couldn't make it worse.             //
//                                                                //
// If it works, it isn't X windows.                               //
// ============================================================== //
