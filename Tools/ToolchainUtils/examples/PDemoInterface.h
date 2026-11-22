// =========== Header ===========
// File:                PDemoInterface.h
// Project:             DCL / ToolchainUtils
// Written by:          Paul Guyot (pguyot@kallisys.net)
//
// Created on:          25/05/2020
// Internal version:    1
//
// Copyright:           © 2020 by Paul Guyot.
//                      All rights reserved worldwide.
// ===========

// =========== Change History ===========
// 25/05/2020   v1  [PG]    Creation of the file
// ===========

#ifndef __PDEMOINTERFACE__
#define __PDEMOINTERFACE__

#include <Protocols.h>

// Newton OS
#include <Newton.h>
#include <NSandDDKIncludes.h>

///
/// Glue protocol
///
PROTOCOL PDemoInterface
    :
        public TProtocol
{
public:
    ///
    /// Protocol constructor.
    ///
    static  PDemoInterface* New( char* inImplementationName );

    ///
    /// Protocol destructor.
    ///
    void            Delete();

    // Interface to demo protocol functions.

    Ref GetConstString( RefArg );
    Ref GetArrayElement( RefArg, RefArg );
    Ref BreakInMonitor( RefArg );
};

#endif
        // __PDEMOINTERFACE__


// ======================================================== //
// The moving cursor writes, and having written, blinks on. //
// ======================================================== //
