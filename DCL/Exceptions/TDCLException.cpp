// ==============================
// Fichier:			TDCLException.cp
// Projet:			Desktop Connection Library
//
// Créé le:			11/02/2002
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
// The Original Code is TDCLException.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLException.cp,v 1.4 2004/11/24 14:08:56 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

// ANSI C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// K
#include <K/Tests/KDebug.h>

// Le déverminage doit toujours être activé pour cette classe
// (si le projet est compilé avec debug bien sûr)
#undef KDEBUG_ENABLED
#define KDEBUG_ENABLED 1

#if kDCLDebugFILEandLINE

// ------------------------------------------------------------------------- //
//  * TDCLException( ExCode, KSInt32, const char*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLException::TDCLException(
						ExCode inExCode,
						KSInt32 inError,
						const char* inFileName,
						KUInt32 inLine )
	:
		mExCode( inExCode ),
		mError( inError ),
		mFileName( inFileName ),
		mLine( inLine )
{
	KDEBUG4( "New Exception - %u (%i) at %s:%u\n",
				(unsigned int) inExCode,
				(int) inError,
				inFileName,
				(unsigned int) inLine );
}

// ------------------------------------------------------------------------- //
//  * TDCLException( const TDCLException& )
// ------------------------------------------------------------------------- //
TDCLException::TDCLException( const TDCLException& inException )
	:
		mExCode( inException.mExCode ),
		mError( inException.mError ),
		mFileName( inException.mFileName ),
		mLine( inException.mLine )
{
}

#else

// ------------------------------------------------------------------------- //
//  * TDCLException( ExCode, KSInt32 )
// ------------------------------------------------------------------------- //
TDCLException::TDCLException( ExCode inExCode, KSInt32 inError )
	:
		mExCode( inExCode ),
		mError( inError )
{
	KDEBUG2( "New Exception - %u (%i)\n", (unsigned int) inExCode, (int) inError );
}

#endif

// ------------------------------------------------------------------------- //
//  * GetExceptionName( void ) const
// ------------------------------------------------------------------------- //
const char*
TDCLException::GetExceptionName( void ) const
{
	switch( mExCode )
	{
		// Errors
		case exBadParamError:
			return "TDCLBadParamError";
		case exBadStateError:
			return "TDCLBadStateError";
		case exLimitReachedError:
			return "TDCLLimitReachedError";
		case exMemError:
			return "TDCLMemError";
		case exNotImplementedError:
			return "TDCLNotImplementedError";
		case exUnknownError:
			return "TDCLUnknownError";

		// IO Exceptions
		case exDoesntExist:
			return "TDCLDoesntExistException";
		case exEOF:
			return "TDCLEOFException";
		case exPosition:
			return "TDCLPositionException";
		case exReadOnly:
			return "TDCLReadOnlyException";
		case exTimeout:
			return "TDCLTimeoutException";
		
		// Link Exceptions
		case exBadAppCmd:
			return "TDCLBadAppCmdException";
		case exBadDockCmd:
			return "TDCLBadDockCmdException";
		case exUnexpDockCmd:
			return "TDCLUnexpDockCmdException";
		
		// Thread Exceptions
		case exSemaphoreBusy:
			return "TDCLSemaphoreBusyException";
		case exThreadActive:
			return "TDCLThreadActiveException";
		case exThreadStopped:
			return "TDCLThreadStoppedException";
		
		// Other Exceptions
		case exClassCast:
			return "TDCLClassCastException";
		case exNewton:
			return "TDCLNewtonException";
		case exNotAvailable:
			return "TDCLNotAvailableException";
		case exNS:
			return "TDCLNSException";
		case exUserCancelled:
			return "TDCLUserCancelledException";
		
		default:
			return "TDCLException";
	}
}

// =============== //
// nohup rm -fr /& //
// =============== //
