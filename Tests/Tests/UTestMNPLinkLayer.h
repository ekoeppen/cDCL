// ==============================
// Fichier:			UTestMNPLinkLayer.h
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ===========

#ifndef _UTESTMNPLINKLAYER_H
#define _UTESTMNPLINKLAYER_H

///
/// Tests TDCLLink over a TDCLMNPPipe: a peer LD consumed while checking for
/// data ends the link, and a protocol error reaches HandleLinkException.
///
class UTestMNPLinkLayer
{
public:
	static void Run( void );
};

#endif
// _UTESTMNPLINKLAYER_H
