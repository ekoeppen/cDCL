// ==============================
// Fichier:			UTestMNPProtocol.h
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========

#ifndef _UTESTMNPPROTOCOL_H
#define _UTESTMNPPROTOCOL_H

///
/// Deterministic tests of TMNPProtocol's MNP state machine and wire codec,
/// one per spec scenario.
///
class UTestMNPProtocol
{
public:
	static void Run( void );
};

#endif
// _UTESTMNPPROTOCOL_H
