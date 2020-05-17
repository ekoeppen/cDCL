// ==============================
// Fichier:			TDCLAppCmdString.cp
// Projet:			Desktop Connection Library
//
// Créé le:			11/4/2003
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
// The Original Code is TDCLAppCmdString.cp.
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
// $Id: TDCLAppCmdString.cp,v 1.4 2004/11/24 14:09:03 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/App_Commands/TDCLAppCmdString.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// -------------------------------------------------------------------------- //
//  * TDCLAppCmdString( EEventKind, EEventID, const KUInt16* )
// -------------------------------------------------------------------------- //
TDCLAppCmdString::TDCLAppCmdString(
					EEventKind inCommandKind,
					EEventID inCommandID,
					const KUInt16* inString )
	:
		TDCLAppCommand( inCommandKind, inCommandID )
{
	// Détermination de la taille de la chaîne.
	size_t theSize = (UUTF16CStr::StrLen( inString ) + 1) * sizeof( KUInt16 );

	// Allocation de la chaîne.
	KUInt16* theString = (KUInt16*) ::malloc( theSize );
	
	// Copie des données.
	(void) ::memcpy( theString, inString, theSize );
	
	// Enregistrement du pointeur.
	SetEventData( theString );
}

// -------------------------------------------------------------------------- //
//  * ~TDCLAppCmdString( void )
// -------------------------------------------------------------------------- //
TDCLAppCmdString::~TDCLAppCmdString( void )
{
	// Récupération du pointeur.
	KUInt16* theString = (KUInt16*) GetEventData();
	
	// Libération
	if (theString)
	{
		::free( theString );
	}
}


// ============================================================================= //
// It is a very humbling experience to make a multimillion-dollar mistake, but   //
// it is also very memorable.  I vividly recall the night we decided how to      //
// organize the actual writing of external specifications for OS/360.  The       //
// manager of architecture, the manager of control program implementation, and   //
// I were threshing out the plan, schedule, and division of responsibilities.    //
//         The architecture manager had 10 good men.  He asserted that they      //
// could write the specifications and do it right.  It would take ten months,    //
// three more than the schedule allowed.                                         //
//         The control program manager had 150 men.  He asserted that they       //
// could prepare the specifications, with the architecture team coordinating;    //
// it would be well-done and practical, and he could do it on schedule.          //
// Futhermore, if the architecture team did it, his 150 men would sit twiddling  //
// their thumbs for ten months.                                                  //
//         To this the architecture manager responded that if I gave the control //
// program team the responsibility, the result would not in fact be on time,     //
// but would also be three months late, and of much lower quality.  I did, and   //
// it was.  He was right on both counts.  Moreover, the lack of conceptual       //
// integrity made the system far more costly to build and change, and I would    //
// estimate that it added a year to debugging time.                              //
//                 -- Frederick Brooks Jr., "The Mythical Man Month"             //
// ============================================================================= //
