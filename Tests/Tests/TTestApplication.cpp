//==============================
// Fichier:			TTestApplication.cp
// Projet:			(Desktop Connection Library)
// Ecrit par:		Paul Guyot (pguyot@kallisys.net)
// 
// Créé le:			17/6/2003
// Tabulation:		4 espaces
// 
// Copyright:		© 2003 by Paul Guyot.
// 					Tous droits réservés pour tous pays.
// Licence:			Licence Réflexive Kallisys
// 					http://www.kallisys.org/reflexive/
// ===========
// $Id: TTestApplication.cp,v 1.4 2004/05/01 21:40:54 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "TTestApplication.h"

// ANSI
#include <stdio.h>

// DCL - Tests
#include "TTestEmptyFilesIntf.h"

// ------------------------------------------------------------------------- //
//  * TTestApplication( IDCLThreads* )
// ------------------------------------------------------------------------- //
TTestApplication::TTestApplication( IDCLThreads* inThreadsIntf )
	:
		TDCLLogApplication( stdout ),
		mThreadsIntf( inThreadsIntf )
{
}

// ------------------------------------------------------------------------- //
//  * ~TTestApplication( void )
// ------------------------------------------------------------------------- //
TTestApplication::~TTestApplication( void )
{
}

// ------------------------------------------------------------------------- //
//  * HandleServerException( TDCLServer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TTestApplication::HandleServerException(
							TDCLServer* inServer,
							TDCLException* inException )
{
	TDCLLogApplication::HandleServerException( inServer, inException );
	ExceptionOccurred();
}

// ------------------------------------------------------------------------- //
//  * HandleCommLayerException( TDCLCommLayer*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TTestApplication::HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException )
{
	TDCLLogApplication::HandleCommLayerException( inCommLayer, inException );
	ExceptionOccurred();
}

// ------------------------------------------------------------------------- //
//  * HandleLinkException( TDCLLink*, TDCLException* )
// ------------------------------------------------------------------------- //
void
TTestApplication::HandleLinkException(
							TDCLLink* inLink,
							TDCLException* inException )
{
	TDCLLogApplication::HandleLinkException( inLink, inException );
	ExceptionOccurred();
}

// ------------------------------------------------------------------------- //
//  * HandleException( TDCLException* )
// ------------------------------------------------------------------------- //
void
TTestApplication::HandleException( TDCLException* inException )
{
	TDCLLogApplication::HandleException( inException );
	ExceptionOccurred();
}

// ------------------------------------------------------------------------- //
//  * ExceptionOccurred( void )
// ------------------------------------------------------------------------- //
void
TTestApplication::ExceptionOccurred( void )
{
}

// ------------------------------------------------------------------------- //
//  * CreateThreadsIntf( void )
// ------------------------------------------------------------------------- //
IDCLThreads*
TTestApplication::CreateThreadsIntf( void )
{
	return mThreadsIntf;
}

// ------------------------------------------------------------------------- //
//  * CreateFilesIntf( void )
// ------------------------------------------------------------------------- //
IDCLFiles*
TTestApplication::CreateFilesIntf( void )
{
	return new TTestEmptyFilesIntf();
}


// ================================== //
// Help!  I'm trapped in a PDP 11/70! //
// ================================== //
