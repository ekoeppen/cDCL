// ==============================
// Fichier:			TDCLIOException.cp
// Projet:			Desktop Connection Library
//
// Créé le:			11/5/2003
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
// The Original Code is TDCLIOException.cp.
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
// $Id: TDCLIOException.cp,v 1.3 2004/11/24 14:08:57 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

#if kDCLDebugFILEandLINE

// ------------------------------------------------------------------------- //
//  * TDCLIOException( ExCode, KSInt32, const char*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLIOException::TDCLIOException(
						ExCode inExCode,
						KSInt32 inError,
						const char* inFileName,
						KUInt32 inLine )
	:
		TDCLException( inExCode, inError, inFileName, inLine )
{
}

#else

// ------------------------------------------------------------------------- //
//  * TDCLIOException( ExCode, KSInt32 )
// ------------------------------------------------------------------------- //
TDCLIOException::TDCLIOException( ExCode inExCode, KSInt32 inError )
	:
		TDCLException( inExCode, inError )
{
}

#endif

// ============================================================================ //
// OS/2 Skyways:                                                                //
// The terminal is almost empty, with only a few prospective passengers milling //
// about. The announcer says that their flight has just departed, wishes them a //
// good flight, though there are no planes on the runway. Airline personnel     //
// walk around, apologising profusely to customers in hushed voices, pointing   //
// from time to time to the sleek, powerful jets outside the terminal on the    //
// field. They tell each passenger how good the real flight will be on these    //
// new jets and how much safer it will be than Windows Airlines, but that they  //
// will have to wait a little longer for the technicians to finish the flight   //
// systems. Maybe until mid-1995. Maybe longer.                                 //
// ============================================================================ //
