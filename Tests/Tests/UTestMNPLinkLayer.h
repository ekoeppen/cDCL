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
/// Tests TDCLLink over TMNPProtocol and TDCLSessionBus: handshake, stalls,
/// keepalive, segmentation, credit throttling, clean disconnect, peer LD,
/// cancellation, and protocol/transport error propagation.
///
class UTestMNPLinkLayer
{
public:
	static void Run( void );
};

#endif
// _UTESTMNPLINKLAYER_H
