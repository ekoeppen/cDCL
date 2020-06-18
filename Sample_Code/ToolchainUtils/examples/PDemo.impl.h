// =========== Header ===========
// File:				PDemo.impl.h
// Project:				DCL / ToolchainUtils
// Written by:			Paul Guyot (pguyot@kallisys.net)
//
// Created on:			25/05/2020
// Internal version:	1
//
// Copyright:			© 2020 by Paul Guyot.
// 						All rights reserved worldwide.
// ===========

// =========== Change History ===========
// 25/05/2020	v1	[PG]	Creation of the file
// ===========

#ifndef __PDEMO__
#define __PDEMO__

#include "PDemoInterface.h"
#include <Newton.h>
#include <NSandDDKIncludes.h>

///
/// Demo protocol
///
PROTOCOL PDemo : public PDemoInterface
    PROTOCOLVERSION(1.0)
{
public:
    PROTOCOL_IMPL_HEADER_MACRO( PDemo );
    
	///
	/// Protocol constructor
	///
	PDemoInterface* New(void);

	///
	/// Protocol desctructor.
	///
	void			Delete (void);

	Ref GetConstString( RefArg );
	Ref GetArrayElement( RefArg, RefArg );
	Ref BreakInMonitor( RefArg );

};

#endif
		// __PDEMO__

// =============================================================== //
// A political man can have as his aim the realization of freedom, //
// but he has no means to realize it other than through violence.  //
//       -- Jean-Paul Sartre                                       //
// =============================================================== //
