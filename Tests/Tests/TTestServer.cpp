//==============================
// Fichier:			TTestServer.cp
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
// $Id: TTestServer.cp,v 1.4 2003/09/17 10:43:48 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include "TTestServer.h"

// ANSI C
#include <stdio.h>

// DCL
#include <DCL/Interfaces/Common/TDCLLogApplication.h>

// -------------------------------------------------------------------------- //
//  * TTestServer( TDCLApplication* )
// -------------------------------------------------------------------------- //
TTestServer::TTestServer( TDCLLogApplication* inApplication )
	:
		TDCLServer( inApplication ),
		mLogFile( inApplication->GetLogFile() )
{
}

// -------------------------------------------------------------------------- //
//  * DoStartListening( void )
// -------------------------------------------------------------------------- //
void
TTestServer::DoStartListening( void )
{
	(void) ::fprintf( mLogFile, "TTestServer::DoStartListening\n" );
}

// -------------------------------------------------------------------------- //
//  * DoStop( void )
// -------------------------------------------------------------------------- //
void
TTestServer::DoStop( void )
{
	(void) ::fprintf( mLogFile, "TTestServer::DoStop\n" );
}

// -------------------------------------------------------------------------- //
//  * DoKill( void )
// -------------------------------------------------------------------------- //
void
TTestServer::DoKill( void )
{
	(void) ::fprintf( mLogFile, "TTestServer::DoKill\n" );
}

// -------------------------------------------------------------------------- //
//  * HandleIncomingConnection( TDCLCommLayer* )
// -------------------------------------------------------------------------- //
TTestServer::EState
TTestServer::HandleIncomingConnection( TDCLCommLayer* /* inCommLayer */ )
{
	(void) ::fprintf( mLogFile, "TTestServer::HandleIncomingConnection\n" );
	
	return kRunning;
}

// -------------------------------------------------------------------------- //
//  * HandleLinkDisconnection( TDCLLink* )
// -------------------------------------------------------------------------- //
TTestServer::EState
TTestServer::HandleLinkDisconnection( TDCLLink* /* inLink */ )
{
	(void) ::fprintf( mLogFile, "TTestServer::HandleLinkDisconnection\n" );
	
	return kRunning;
}

// -------------------------------------------------------------------------- //
//  * DoIdle( void )
// -------------------------------------------------------------------------- //
void
TTestServer::DoIdle( void )
{
	(void) ::fprintf( mLogFile, "TTestServer::DoIdle\n" );
}

// ======================================================================= //
// I cannot conceive that anybody will require multiplications at the rate //
// of 40,000 or even 4,000 per hour ...                                    //
//                 -- F. H. Wales (1936)                                   //
// ======================================================================= //
