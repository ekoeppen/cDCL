// ==============================
// Fichier:			TDCLKeyboardEngine.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			10/4/2003
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
// The Original Code is TDCLKeyboardEngine.cp.
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
// $Id: TDCLKeyboardEngine.cp,v 1.4 2004/11/24 14:09:05 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Link_Engines/TDCLKeyboardEngine.h>

// K
#include <K/Unicode/UUTF16CStr.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Link/TDCLFullDockLink.h>
#include <DCL/Link/App_Commands/TDCLAppCommand.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>

// -------------------------------------------------------------------------- //
//  * TDCLKeyboardEngine( TDCLFullDockLink* )
// -------------------------------------------------------------------------- //
TDCLKeyboardEngine::TDCLKeyboardEngine( TDCLFullDockLink* inLink )
	:
		TDCLLinkEngine( inLink )
{
}

// -------------------------------------------------------------------------- //
//  * ProcessAppCommand( TDCLAppCommand*, Boolean* )
// -------------------------------------------------------------------------- //
Boolean
TDCLKeyboardEngine::ProcessAppCommand(
					TDCLAppCommand* inCommand,
					Boolean* outProcessed )
{
	Boolean theResult = true;
	*outProcessed = false;

	switch (inCommand->GetCommandID())
	{
		case TDCLAppCommand::kEndUsingKeyboard:
			{
				// Envoi de la commande pour arrter
				TDCLDockCmdNoData theCancelCmd(
									TDCLDockCommand::kDOperationCanceled );
				theCancelCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}
			break;

		case TDCLAppCommand::kSendCharacter:
			{
				// Les donnŽes pointent sur deux mots de 16 bits.
				// Le premier est le caractre, le second des drapeaux.
				KUInt16* theChar = (KUInt16*) inCommand->GetEventData();

				// Envoi de la commande au Newton
				TDCLDockCmdSingleLong theStringCmd(
							TDCLDockCommand::kDKeyboardChar,
							(KUInt32) (theChar[0] << 16) | theChar[1] );
				theStringCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}
			break;

		case TDCLAppCommand::kSendString:
			{
				// RŽcupŽration de la cha”ne.
				KUInt16* theString = (KUInt16*) inCommand->GetEventData();
				size_t theSize =
						(UUTF16CStr::StrLen( theString ) + 1) * sizeof( KUInt16 );

				// Envoi de la commande au Newton
				TDCLDockCmdGeneric theStringCmd(
							TDCLDockCommand::kDKeyboardString,
							theSize, theString );
				theStringCmd.SendCommand( GetLink()->GetPipe() );
				*outProcessed = true;
			}
			break;
	}

	return theResult;
}

// --------------------------------------------------------------------------------	//
//  *ÊGetAppCommandMask( void )
// --------------------------------------------------------------------------------	//
KUInt32
TDCLKeyboardEngine::GetAppCommandMask( void )
{
	return (KUInt32) TDCLEvent::kKbdAppCommand;
}

// ====================================================================== //
// "Section 2.4.3.5   AWNS   (Acceptor Wait for New Cycle State).         //
//         In AWNS the AH function indicates that it has received a       //
// multiline message byte.                                                //
//         In AWNS the RFD message must be sent false and the DAC message //
// must be sent passive true.                                             //
//         The AH function must exit the AWNS and enter:                  //
//         (1)  The ANRS if DAV is false                                  //
//         (2)  The AIDS if the ATN message is false and neither:         //
//                 (a)  The LADS is active                                //
//                 (b)  Nor LACS is active"                               //
//                                                                        //
//                 -- from the IEEE Standard Digital Interface for        //
//                    Programmable Instrumentation                        //
// ====================================================================== //
