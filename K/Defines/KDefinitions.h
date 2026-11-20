// ==============================
// Fichier:			KDefinitions.h
// Projet:			K
// 
// Créé le:			13/01/2002
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
// The Original Code is KDefinitions.h.
// 
// The Initial Developer of the Original Code is Paul Guyot.
// Portions created by the Initial Developer are Copyright (C) 2002-2004
// the Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Eckhart Koeppen <eck@40hz.org>
// 
// ***** END LICENSE BLOCK *****
// ===========

#ifndef _KDEFINITIONS_H
#define _KDEFINITIONS_H

#include <cstddef>
#include <cstdint>

// Legacy fixed-width type aliases (transitional compatibility shims)
using KUInt64 = uint64_t;
using KSInt64 = int64_t;
using KUInt32 = uint32_t;
using KSInt32 = int32_t;
using KUInt16 = uint16_t;
using KSInt16 = int16_t;
using KUInt8  = uint8_t;
using KSInt8  = int8_t;
using KUIntPtr = uintptr_t;

#ifndef FOUR_CHAR_CODE
#define FOUR_CHAR_CODE(x) (x)
#endif

#ifndef NULL
#define NULL 0
#endif

#if !defined(__OBJC__) && !defined(nil)
#define nil nullptr
#endif

#endif
		// _KDEFINITIONS_H

// =========================================================== //
// There's no such thing as a free lunch.                      //
//                 -- Milton Friedman                          //
//                                                             //
// There's no such thing as a free lunch.                      //
//                 -- Robert Heinlein, "The Moon is a Harsh   //
//                    Mistress"                               //
// =========================================================== //
