// =========== Header ===========
// File:				ntkmodule.cp
// Project:				DCL / ToolchainUtils
// Written by:			Paul Guyot (pguyot@kallisys.net)
//
// Created on:			23/05/2020
// Internal version:	1
//
// Copyright:			© 2020 by Paul Guyot.
// 						All rights reserved worldwide.
// ===========

// =========== Change History ===========
// 23/05/2020	v1	[PG]	Creation of the file
// ===========

// NewtonOS
#include <NewtonScript.h>
#include <Objects.h>

#include "PDemoInterface.h"

// ========================================================================	//
//	Prototypes
// ========================================================================	//
// 
extern "C"
{
	Ref GetConstString( RefArg );
	Ref GetArrayElement( RefArg, RefArg );
	Ref BreakInMonitor( RefArg );
	Ref ProtocolGetConstString( RefArg );
	Ref ProtocolGetArrayElement( RefArg, RefArg );
	Ref ProtocolBreakInMonitor( RefArg );
}


static const char* constStringA = "Hello, world";
static const char* const constArray[] = { "Area 51", "Groom Lake" };

// ------------------------------------------------------------------------- //
//  * GetConstString( RefArg )
// ------------------------------------------------------------------------- //

extern "C"
Ref
GetConstString( RefArg inRcvr )
{
    return MakeString(constStringA);
}

// ------------------------------------------------------------------------- //
//  * GetArrayElement( RefArg, RefArg )
// ------------------------------------------------------------------------- //
extern "C"
Ref
GetArrayElement( RefArg inRcvr, RefArg inIndex )
{
    long indexValue = RINT( inIndex );
    return MakeString(constArray[indexValue % 2]);
}

// ------------------------------------------------------------------------- //
//  * BreakInMonitor( RefArg )
// ------------------------------------------------------------------------- //
extern "C"
Ref
BreakInMonitor( RefArg inRcvr )
{
    asm("stmdb   sp!, {lr}\n"
        "ldr     lr, id\n"
        "mcr     p10, 0, lr, c0, c0\n"
        "ldmia   sp!, {pc}\n"
        "id:\n"
        ".word      0x116\n");
    return 0;
}

// ------------------------------------------------------------------------- //
//  * ProtocolGetConstString( RefArg )
// ------------------------------------------------------------------------- //

extern "C"
Ref
ProtocolGetConstString( RefArg inRcvr )
{
    RefVar result;
	PDemoInterface* interface = (PDemoInterface*) ::NewByName("PDemoInterface", "PDemo");
	if (interface == nil)
	{
        OutOfMemory();
	} else {
	    result = interface->GetConstString( inRcvr );
    	interface->Delete();
	}

	return result;
}

// ------------------------------------------------------------------------- //
//  * ProtocolGetArrayElement( RefArg )
// ------------------------------------------------------------------------- //

extern "C"
Ref
ProtocolGetArrayElement( RefArg inRcvr, RefArg inArg )
{
    RefVar result;
	PDemoInterface* interface = PDemoInterface::New("PDemo");
	if (interface == nil)
	{
        OutOfMemory();
	} else {
	    result = interface->GetArrayElement( inRcvr, inArg );
    	interface->Delete();
	}
	
	return result;
}

// ------------------------------------------------------------------------- //
//  * ProtocolBreakInMonitor( RefArg )
// ------------------------------------------------------------------------- //

extern "C"
Ref
ProtocolBreakInMonitor( RefArg inRcvr )
{
    RefVar result;
	PDemoInterface* interface = (PDemoInterface*) ::NewByName("PDemoInterface", "PDemo");
	if (interface == nil)
	{
        OutOfMemory();
	} else {
	    result = interface->BreakInMonitor( inRcvr );
    	interface->Delete();
	}

	return result;
}
