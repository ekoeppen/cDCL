// =========== Header ===========
// File:                PDemo.cpp
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

#include "PDemo.impl.h"

// NewtonOS
#include <NewtonScript.h>
#include <Objects.h>

static const char* constStringA = "Hello, world";
static const char* const constArray[] = { "Area 51", "Groom Lake" };

PROTOCOL_IMPL_SOURCE_MACRO(PDemo)           // Magic stuff, do not touch

// ------------------------------------------------------------------------- //
//  * New()
// ------------------------------------------------------------------------- //
PDemoInterface*
PDemo::New()
{
    // Init anything we should init here...
    return this;
}

// ------------------------------------------------------------------------- //
//  * Delete()
// ------------------------------------------------------------------------- //
void
PDemo::Delete()
{
    // Cleanup
}

// ------------------------------------------------------------------------- //
//  * GetConstString( RefArg )
// ------------------------------------------------------------------------- //

Ref
PDemo::GetConstString( RefArg inRcvr )
{
    return MakeString(constStringA);
}

// ------------------------------------------------------------------------- //
//  * GetArrayElement( RefArg, RefArg )
// ------------------------------------------------------------------------- //
Ref
PDemo::GetArrayElement( RefArg inRcvr, RefArg inIndex )
{
    long indexValue = RINT( inIndex );
    return MakeString(constArray[indexValue % 2]);
}

// ------------------------------------------------------------------------- //
//  * BreakInMonitor( RefArg )
// ------------------------------------------------------------------------- //
Ref
PDemo::BreakInMonitor( RefArg inRcvr )
{
    asm("stmdb   sp!, {lr}\n"
        "ldr     lr, id\n"
        "mcr     p10, 0, lr, c0, c0\n"
        "ldmia   sp!, {pc}\n"
        "id:\n"
        ".word      0x116\n");
    return 0;
}

// ================================================== //
// MAIDEN AUNT:                                       //
//     A girl who never had the sense to say "uncle." //
// ================================================== //
