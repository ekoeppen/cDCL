// =========== Header ===========
// File:				PDemo.intf.s
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

	.arch armv3
    .section .interp, "ax"
    . = 0
    .balign 4
    .global _ZN5PDemo9ClassInfoEv
    .global _ZN5PDemo3NewEv
    .global _ZN5PDemo6DeleteEv
    .global _ZN5PDemo14GetConstStringERK6RefVar
    .global _ZN5PDemo15GetArrayElementERK6RefVarS2_
    .global _ZN5PDemo14BreakInMonitorERK6RefVar

    .global main

main:
local___ZN5PDemo9ClassInfoEv:
@ (reserved for now)
    .word   0
    .word   __implname__ - .
    .word   __protname__ - .
    .word   __cap__ - .
    .word   __btbl__ - .
    .word   __monent__ - .
@ branch to sizeof glue
    B		_ZN5PDemo6SizeofEv
@ zero, or branch to Alloc method
    .word	0
@ zero, or branch to Free method
    .word   0
@ return, or branch to New(void)
    B		_ZN5PDemo3NewEv
@ return, or branch to Delete
	B		_ZN5PDemo6DeleteEv
@ version number
    .word	0x00010000
@ flags
    .word   0x00000000
@ (reserved)
    .word   0x00000000
@ (reserved)
	B		return__nil__5PDemo

_ZN5PDemo9ClassInfoEv:
    adr		r0,local___ZN5PDemo9ClassInfoEv
    mov		pc,r14

return__nil__5PDemo:
    mov		r0,#0
    mov		pc,r14

__implname__:
    .string "PDemo"
__protname__:
    .string "PDemoInterface"
    .balign 4
__cap__:
    .word   0
__btbl__:
    .word   0
    B   _ZN5PDemo9ClassInfoEv
    B   _ZN5PDemo3NewEv
    B   _ZN5PDemo6DeleteEv
    B   _ZN5PDemo14GetConstStringERK6RefVar
    B   _ZN5PDemo15GetArrayElementERK6RefVarS2_
    B   _ZN5PDemo14BreakInMonitorERK6RefVar

__monent__:
    .end

// ========================================================================= //
// Real computer scientists only write specs for languages that might run on //
// future hardware.  Nobody trusts them to write specs for anything homo     //
// sapiens will ever be able to fit on a single planet.                      //
// ========================================================================= //
