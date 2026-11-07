// ==============================
// Fichier:			UTestMNPPipe.h
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========

#ifndef _UTESTMNPPIPE_H
#define _UTESTMNPPIPE_H

///
/// Integration tests for the TDCLMNPPipe facade over an in-memory pipe:
/// the pipe API (Connected, Read, Write, FlushOutput, BytesAvailable,
/// Disconnect, PeerDisconnected) against the scenarios of
/// .topics/mnp-pipe-bdd.md. Protocol details are covered by UTestMNPProtocol.
///
class UTestMNPPipe
{
public:
	static void Run( void );
};

#endif
// _UTESTMNPPIPE_H
