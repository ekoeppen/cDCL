// ==============================
// Fichier:			TDCLCTBIrDA.cp
// Projet:			Desktop Connection Library
//
// Créé le:			22/01/2002
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
// The Original Code is TDCLCTBIrDA.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLCTBIrDA.cp,v 1.4 2004/11/24 14:08:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLCTBIrDA.h>

#define kIrDAToolStr		"IrDA Tool"
#define kIrDAConfigToolStr	\
	"myClass: Docker myAttr: IrDA:IrLMP:LsapSel peerClass: NewtConnection "\
	"peerAttr: IrDA:IrLMP:LsapSel"

// ------------------------------------------------------------------------- //
//  * TDCLCTBIrDA( IDCLThreads*, long )
// ------------------------------------------------------------------------- //
TDCLCTBIrDA::TDCLCTBIrDA(
		IDCLThreads* inThreadsIntf,
		long	inTimeout /* = kDefaultTimeout */
	)
		:
			TDCLCommToolBox(
				kIrDAToolStr,
				kIrDAConfigToolStr,
				inThreadsIntf,
				1,			// Only one connection with IrDA
				inTimeout )
{
}

// ------------------------------------------------------------------------- //
//  * IsAvailable( void )
// ------------------------------------------------------------------------- //
Boolean
TDCLCTBIrDA::IsAvailable( void )
{
	return TDCLCommToolBox::IsAvailable( kIrDAToolStr );
}

// ====================================================================== //
// Documentation is the castor oil of programming.                        //
// Managers know it must be good because the programmers hate it so much. //
// ====================================================================== //
