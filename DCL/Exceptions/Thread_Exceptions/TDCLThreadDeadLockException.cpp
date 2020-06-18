// ==============================
// Fichier:			TDCLThreadDeadLockException.cp
// Projet:			Desktop Connection Library
// 
// Créé le:			29/4/2004
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
// The Original Code is TDCLThreadDeadLockException.cp.
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
// $Id: TDCLThreadDeadLockException.cp,v 1.3 2004/11/24 14:08:58 paul Exp $
// ===========

#include <K/Defines/KDefinitions.h>
#include "TDCLThreadDeadLockException.h"

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/Thread_Exceptions/TDCLThreadDeadLockException.h>

#if kDCLDebugFILEandLINE

// ------------------------------------------------------------------------- //
//  * TDCLThreadDeadLockException( const char*, KUInt32, KSInt32 )
// ------------------------------------------------------------------------- //
TDCLThreadDeadLockException::TDCLThreadDeadLockException(
					const char* inFileName,
					KUInt32 inLine,
					KSInt32 inError )
	:
		TDCLThreadException(
				TDCLException::exThreadDeadLock,
				inError,
				inFileName,
				inLine )
{
}

#else

// ------------------------------------------------------------------------- //
//  * TDCLThreadDeadLockException( KSInt32 )
// ------------------------------------------------------------------------- //
TDCLThreadDeadLockException::TDCLThreadDeadLockException( KSInt32 inError )
	:
		TDCLThreadException( TDCLException::exThreadDeadLock, inError )
{
}

#endif

// ==================================================== //
// The only thing worse than X Windows: (X Windows) - X //
// ==================================================== //
