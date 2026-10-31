// ==============================
// Fichier:			KDefinitions.h
// Projet:			K
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

#ifndef __KDEFINITIONS__
#define __KDEFINITIONS__

/// \file KDefinitions.h
/// Modern C++17 definitions and cross-platform detection for cDCL / K.
/// Supported targets: macOS, Linux, Windows (MinGW-w64).

#include <cstdint>
#include <cstddef>

// -------------------------------------------------------------------------- //
// Target Platform Identification
// -------------------------------------------------------------------------- //

#if defined(_WIN32)
    #ifndef TARGET_OS_WIN32
        #define TARGET_OS_WIN32 1
    #endif
    #define TARGET_OS_POSIX 0
    #define TARGET_OS_COMPAT_POSIX 0
    #define TARGET_OS_MAC 0
    #define TARGET_OS_OPENSTEP 0
    #define TARGET_OS_LINUX 0
#elif defined(__APPLE__)
    #define TARGET_OS_WIN32 0
    #define TARGET_OS_POSIX 1
    #define TARGET_OS_COMPAT_POSIX 1
    #define TARGET_OS_MAC 1
    #define TARGET_OS_OPENSTEP 1
    #define TARGET_OS_LINUX 0
    #if (defined(__arm64__) || defined(__aarch64__))
        #define TARGET_OS_OPENSTEP_ARM64 1
        #define TARGET_OS_OPENSTEP_AMD64 0
    #elif (defined(__x86_64__) || defined(__amd64__))
        #define TARGET_OS_OPENSTEP_ARM64 0
        #define TARGET_OS_OPENSTEP_AMD64 1
    #else
        #define TARGET_OS_OPENSTEP_ARM64 0
        #define TARGET_OS_OPENSTEP_AMD64 0
    #endif
#elif defined(__linux__)
    #define TARGET_OS_WIN32 0
    #define TARGET_OS_POSIX 1
    #define TARGET_OS_COMPAT_POSIX 1
    #define TARGET_OS_MAC 0
    #define TARGET_OS_OPENSTEP 0
    #define TARGET_OS_LINUX 1
#elif defined(TARGET_OS_POSIX) && TARGET_OS_POSIX
    #define TARGET_OS_WIN32 0
    #define TARGET_OS_COMPAT_POSIX 1
    #define TARGET_OS_MAC 0
    #define TARGET_OS_OPENSTEP 0
    #define TARGET_OS_LINUX 0
#else
    #error "Unsupported target platform! Only macOS, Linux, and Windows are supported."
#endif

// Legacy platforms explicitly disabled
#define TARGET_OS_BEOS 0
#define TARGET_OS_BSD 0
#define TARGET_OS_CYGWIN 0
#define TARGET_OS_MACOS 0
#define TARGET_OS_NEWTON 0

// -------------------------------------------------------------------------- //
// Endianness Detection
// -------------------------------------------------------------------------- //

#if defined(__BYTE_ORDER__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN 0
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define TARGET_RT_LITTLE_ENDIAN 0
        #define TARGET_RT_BIG_ENDIAN 1
    #else
        #error "Unsupported endianness!"
    #endif
#elif defined(__BIG_ENDIAN__)
    #define TARGET_RT_LITTLE_ENDIAN 0
    #define TARGET_RT_BIG_ENDIAN 1
#elif defined(__LITTLE_ENDIAN__)
    #define TARGET_RT_LITTLE_ENDIAN 1
    #define TARGET_RT_BIG_ENDIAN 0
#else
    // Default to little-endian for modern x86/ARM host architectures
    #define TARGET_RT_LITTLE_ENDIAN 1
    #define TARGET_RT_BIG_ENDIAN 0
#endif

// -------------------------------------------------------------------------- //
// Scalar Types (Standard Fixed Width)
// -------------------------------------------------------------------------- //

using KUInt64  = std::uint64_t;
using KSInt64  = std::int64_t;
using KUInt32  = std::uint32_t;
using KSInt32  = std::int32_t;
using KUInt16  = std::uint16_t;
using KSInt16  = std::int16_t;
using KUInt8   = std::uint8_t;
using KSInt8   = std::int8_t;
using KUIntPtr = std::uintptr_t;
using Boolean  = bool;

#define HAS_C99_LONGLONG 1
#define HAS_C99_SNPRINTF 1
#define HAS_NAMESPACES 1
#define NAMESPACE(__name) namespace __name
#define FOUR_CHAR_CODE(x) ((long)(x))

#ifndef NULL
    #define NULL 0L
#endif
#ifndef nil
    #define nil nullptr
#endif

#if TARGET_OS_WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #undef Yield
#endif

#endif // __KDEFINITIONS__
