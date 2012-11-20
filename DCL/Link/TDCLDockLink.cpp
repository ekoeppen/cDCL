// ==============================
// Fichier:			TDCLDockLink.cp
// Projet:			Desktop Connection Library
//
// Créé le:			13/08/2001
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
// The Original Code is TDCLDockLink.cp.
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
// $Id: TDCLDockLink.cp,v 1.7 2006/07/07 08:27:56 pguyot Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLDockLink.h>

// ANSI C
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// K
#include <K/Misc/TSmartPtr.h>

// DCL
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/TDCLNewtonException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Exceptions/Link_Exceptions/TDCLUnexpDockCmdException.h>
#include <DCL/Interfaces/IDCLFiles.h>
#include <DCL/Interfaces/IDCLThreads.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdDesktopInfo.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPassword.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// ------------------------------------------------------------------------- //
//  * TDCLDockLink( TDCLApplication*, KUInt32 )
// ------------------------------------------------------------------------- //
TDCLDockLink::TDCLDockLink(
		TDCLApplication* inApplication,
		KUInt32 inIcons )
	:
		TDCLLink( inApplication ),
		mIcons( inIcons )

{
	// Gaz à tous les étages.
}

// ------------------------------------------------------------------------- //
//  * DoConnect( void )
// ------------------------------------------------------------------------- //
void
TDCLDockLink::DoConnect( void )
{
	// TDCLLink fait la première partie du boulot.
	TDCLLink::DoConnect();
	
	// Ouverture de la session.
	TSmartPtr<TDCLDockCommand> theNewtMessage;
	
	KUInt32 theChallenge[2];

	// Envoi de kDDesktopInfo
	{
		// Je dis que je suis NCU.
		// Structure NewtonScript pour ce gros mensonge.
		TDCLNSFrame theDesktopAppFrame;
		
		// Tableau NewtonScript avec les informations.
		TDCLNSArray theDesktopAppsArray;
		
		// Le nom.
		theDesktopAppFrame.Set(
					"name",
					TDCLNSRef::MakeString( "Newton Connection Utilities" ) );

		// L'ID (2, c'est NCU)
		theDesktopAppFrame.Set( "id", TDCLNSRef::MakeInt( 2 ) );

		// La version.
		theDesktopAppFrame.Set( "version", TDCLNSRef::MakeInt( 1 ) );
		
		// Si on permet l'auto-dock.
//		theDesktopAppFrame.Set( "doesAuto", TDCLNSRef::kTRUEREF );

		// Cette structure va dans le tableau.
		theDesktopAppsArray.Add( TDCLNSRef( theDesktopAppFrame ) );

		// Le défi est constitué de deux nombres aléatoires
		// Il faut donc initialiser le générateur aléatoire.
		time_t theTime;
		(void) ::time( &theTime );
		::srand( (unsigned int) theTime );

		// On crée le défi.
		TDCLDockCmdPassword::CreateChallenge( theChallenge );
		
		// Et on envoie l'information.
		// Création de la commande.
		TDCLDockCmdDesktopInfo theDesktopInfoMessage(
										theDesktopAppsArray,
										theChallenge[0],
										theChallenge[1],
										true,	// selective sync
										TDCLDockCommand::kSettingUp_SType,
										GetApplication()->GetFilesIntf()
																->GetKind() );
		
		// Envoi.
		theDesktopInfoMessage.SendCommand( GetPipe() );
	}
	
	KUInt32 theNewtonChallenge[2];
	
	// Réception de kDNewtonInfo
	{
		theNewtMessage = TDCLDockCommand::ReceiveCommand( GetPipe() );

		// Vérification que c'est bien une commande kDNewtonName.
		if (theNewtMessage->GetCommand() != TDCLDockCommand::kDNewtonInfo)
		{
			// Duh!
			
			// Je devrais regarder si ce n'est pas un message d'erreur venant
			// du Newton (on ne sait jamais)
			
			throw DCLUnexpDockCmd;
		}

		// C'est bien une structure TDCLDockCmdNewtonInfo
		TDCLDockCmdNewtonInfo* theNewtonInfo =
			(TDCLDockCmdNewtonInfo*) theNewtMessage.Get();
		
		// Je sauvegarde la clef pour le DES.
		theNewtonChallenge[0] = theNewtonInfo->GetChallenge()[0];
		theNewtonChallenge[1] = theNewtonInfo->GetChallenge()[1];
		
		// Libération du message.
		theNewtMessage.Delete();
	}

	// Envoi de kDWhichIcons.
	{
		// Création.
		TDCLDockCmdSingleLong theWhichIconMessage(
			TDCLDockCommand::kDWhichIcons, mIcons );

		// Envoi.
		theWhichIconMessage.SendCommand( GetPipe() );
	}
	
	// Réception de kDResult.
	{
		theNewtMessage = TDCLDockCommand::ReceiveCommand( GetPipe() );

		// Vérification que c'est bien une commande kDResult.
		if (theNewtMessage->GetCommand() != TDCLDockCommand::kDResult)
		{
			// Duh!
			throw DCLUnexpDockCmd;
		}

		// C'est bien une structure	TDCLDockCmdSingleLong
		TDCLDockCmdSingleLong* theResult =
			(TDCLDockCmdSingleLong*) theNewtMessage.Get();
		
		// On lève une exception si le code n'est pas 0.
		KSInt32 theResultInt = (KSInt32) theResult->GetLong();
		if (theResultInt)
		{
			throw DCLNewton( theResultInt );
		}
		
		// Libération.
		theNewtMessage.Delete();
	}

	// Envoi de kDSetTimeout.
	{
		// Récupération de la temporisation de l'interface de
		// communication.
		long theTimeout = GetPipe()->GetTimeout();
		if (theTimeout < 0)
		{
			// Valeur par défaut.
			theTimeout = kDefaultTimeout;
		}
		
		// Création.
		TDCLDockCmdSingleLong theSetTimeoutMessage(
			TDCLDockCommand::kDSetTimeout, (KUInt32) theTimeout );

		// Envoi.
		theSetTimeoutMessage.SendCommand( GetPipe() );
	}
	
	// Récupération de la commande kDPassword.
	// Le Newton a droit à 3 essais.
	{
		int indexAttempts;
		
		for (indexAttempts = 0; indexAttempts < 3; indexAttempts++)
		{
			theNewtMessage = TDCLDockCommand::ReceiveCommand( GetPipe() );

			// Vérification que c'est bien une commande kDPassword.
			if (theNewtMessage->GetCommand() != TDCLDockCommand::kDPassword)
			{
				throw DCLUnexpDockCmd;
			}

			// C'est bien une structure TDCLDockCmdPassword.
			TDCLDockCmdPassword* thePassword =
				(TDCLDockCmdPassword*) theNewtMessage.Get();
		
			// Je vérifie le mot de passe.
			Boolean correct =
				thePassword->VerifyPassword( theChallenge, GetPassword() );
			theNewtMessage.Delete();	// Libération.
			
			if (correct)
			{
				break;
			}

			// On dit au Newton qu'il a encore un essai ou pas.
			if (indexAttempts < 2)
			{
				// Envoi de kRetryPassword_Err
				TDCLDockCmdSingleLong theRetryPasswordCmd(
										TDCLDockCommand::kDResult,
										TDCLDockCommand::kRetryPassword_Err );

				theRetryPasswordCmd.SendCommand( GetPipe() );
			} else {
				// Envoi de kBadPassword_Err
				TDCLDockCmdSingleLong theRetryPasswordCmd(
										TDCLDockCommand::kDResult,
										TDCLDockCommand::kBadPassword_Err );

				theRetryPasswordCmd.SendCommand( GetPipe() );

#warning fix this?
				throw DCLBadDockCmd;
			}
		}
	}

	// Envoi de kDPassword.
	{
		TDCLDockCmdPassword thePasswordCmd(
										theNewtonChallenge, GetPassword() );

		thePasswordCmd.SendCommand( GetPipe() );
	}
}

// ------------------------------------------------------------------------- //
//  * ProcessDockCommand( TDCLDockCommand*, Boolean* )
// ------------------------------------------------------------------------- //
TDCLLink::EState
TDCLDockLink::ProcessDockCommand(
					TDCLDockCommand* inCommand,
					Boolean* outProcessed )
{
	TDCLLink::EState theResult = kRunning;	// Par défaut, on reste connecté.

	switch( inCommand->GetCommand() )
	{
		case TDCLDockCommand::kDDisconnect:
			// Le Newton a déconnecté.
			// Je retourne kDisconnected pour finir cette session.
			theResult = kDisconnected;
			*outProcessed = true;
			break;

		case TDCLDockCommand::kDHello:
			// Je crois qu'il ne faut pas répondre bonjour ici.
			*outProcessed = true;
			break;

		default:
			// Message inconnu.
			*outProcessed = false;
	}
	
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * GetPassword( void )
// ------------------------------------------------------------------------- //
const KUInt16*
TDCLDockLink::GetPassword( void )
{
	return nil;
}

// ============================================================================= //
// In a surprise raid last night, federal agents ransacked a house in search     //
// of a rebel computer hacker.  However, they were unable to complete the arrest //
// because the warrant was made out in the name of Don Provan, while the only    //
// person in the house was named don provan.  Proving, once again, that Unix is  //
// superior to Tops10.                                                           //
// ============================================================================= //
