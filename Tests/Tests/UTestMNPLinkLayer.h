// ==============================
// File:			UTestMNPLinkLayer.h
// Project:			Desktop Connection Library
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
// The Original Code is UTestMNPLinkLayer.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
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
