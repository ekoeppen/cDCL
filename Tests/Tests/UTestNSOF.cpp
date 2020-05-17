// ==============================
// Fichier:			UTestNSOF.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
//
// Créé le:			21/10/2002
// Tabulation:		4 espaces
//
// Copyright:		© 2002-2003 by Paul Guyot, Michael Vacík & Nicolas Zinovieff.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: UTestNSOF.cp,v 1.12 2004/03/13 08:51:22 paul Exp $
// ===========

#include "UTestNSOF.h"

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFEncoder.h>
#include <DCL/NS_Objects/Exchange/TDCLNSOFDecoder.h>
#include <DCL/Streams/TDCLMemStream.h>
#include <DCL/NS_Objects/Objects/TDCLNSSymbol.h>

// Tests
#include "UTestNS.h"

// ------------------------------------------------------------------------- //
//  * WalterSmith( void )
// ------------------------------------------------------------------------- //
void
UTestNSOF::WalterSmith( void )
{
	// Récupération de la structure Walter Smith
	TDCLNSRef theWalterSmithFrameRef = UTestNS::GetWalterSmith();
	TDCLNSFrame& theWalterSmithFrame = theWalterSmithFrameRef.ToFrame();

	// Ensuite, encodage sur stdout.
	TDCLMemStream theMemStream;
	TDCLNSOFEncoder theEncoder( &theMemStream );
	
	theEncoder.AddObject( theWalterSmithFrameRef );
	
	// Affichage des données en hexa.
	const KUInt8* theData = (const KUInt8*) theMemStream.GetBuffer();
	KUInt32 theDataCount = theMemStream.GetBufferSize();
	
	KUInt32 indexData;
	for ( indexData = 0; indexData < theDataCount; indexData++ )
	{
		(void) ::printf( "%.2X", theData[indexData] );
		if (indexData % 16 == 15)
		{
			(void) ::printf( "\n" );
		}
	}
	(void) ::printf( "\n" );
	
	// Vérification que le décodage se passe bien.
	theMemStream.SetCursor( 0, TDCLMemStream::kFromStart );

	TDCLNSOFDecoder theDecoder( &theMemStream );
	TDCLNSRef theFrameDecodedRef = theDecoder.GetNextObject();
	TDCLNSFrame& theFrameDecoded = theFrameDecodedRef.ToFrame();
	
	// Les deux structures doivent être identiques.
	// Je vais juste vérifier que les clés et les classes/valeurs immédiates
	// sont les mêmes
	KUInt32 nbPairs = theFrameDecoded.GetLength();
	if (nbPairs != theWalterSmithFrame.GetLength())
	{
		(void) ::printf(
				"Erreur: les tailles divergent (%u != %u)\n",
				(unsigned int) nbPairs,
				(unsigned int) theWalterSmithFrame.GetLength());
	} else {
		KUInt32 indexPairs;
		for (indexPairs = 0; indexPairs < nbPairs; indexPairs++)
		{
			TDCLNSSymbol& theKey =
				theWalterSmithFrame.GetKey(indexPairs).ToSymbol();
			if (!theFrameDecoded.HasSlot(theKey))
			{
				(void) ::printf(
						"Erreur: la structure décodée n'a pas la clé %s\n",
						(const char*) theKey );
			} else {
				// Est-ce un immédiat?
				TDCLNSRef theWSValue = theWalterSmithFrame.Get(theKey);
				TDCLNSRef theFDValue = theFrameDecoded.Get(theKey);
				
				if (theWSValue != theFDValue)
				{
					if (theWSValue.ClassOf() != theFDValue.ClassOf())
					{
						(void) ::printf(
							"Erreur: Clé %s: les classes des valeurs"
							" divergent\n",
							theKey.GetString());
					} else if (!theWSValue.IsPtr()) {
						(void) ::printf(
							"Erreur: Clé %s: les valeurs divergent\n",
							theKey.GetString());
					}
				}
			}
		}
	}

	// Encodage à nouveau sur stdout.
	TDCLMemStream anotherMemStream;
	TDCLNSOFEncoder anotherEncoder( &anotherMemStream );
	
	anotherEncoder.AddObject( theFrameDecodedRef );
	
	// Affichage des données en hexa.
	theData = (const KUInt8*) anotherMemStream.GetBuffer();
	theDataCount = anotherMemStream.GetBufferSize();
	
	for ( indexData = 0; indexData < theDataCount; indexData++ )
	{
		(void) ::printf( "%.2X", theData[indexData] );
		if (indexData % 16 == 15)
		{
			(void) ::printf( "\n" );
		}
	}
	(void) ::printf( "\n" );
}

// ===================================== //
// Multics is security spelled sideways. //
// ===================================== //
