    .arch armv3
    .global _Z9NewByNamePKcS0_
    .global _Z19AllocInstanceByNamePKcS0_
    .global _Z12FreeInstanceP9TProtocol

    .macro _Dispatch slotno
    ldr     r0, [r0, #4]                // r0 -> "real" this
    ldr     r12, [r0, #8]               // r12 -> btable
    add     pc, r12, #\slotno*4         // go there
    .endm

    .macro New symbol, className, slotno=2
    .section ".text.\symbol\()", "ax"
    .global \symbol
    .balign 4
asciz__\className\():
    mov     r0, pc                      // r0 -> asciz name
    mov     pc, r14                     // begone!
    .string "\className\()"
    .balign 4
\symbol\():
    stmfd   r13!, {r0-r3, r14}          // save New(...) args
    mov     r1, r0                      // r1 = implementation name
    bl      asciz__\className           // r0 = interface name
    bl      _Z19AllocInstanceByNamePKcS0_
    tst     r0, r0                      // success?
    add     r13, r13, #4                // pop (saved) string parm; r0 is 'this' now (or nil)
    ldmfd   r13!, {r1-r3, r14}          // pop New(...) args
    moveq   pc, r14                     // return nil if instance couldn't be made
    _Dispatch   \slotno
    .endm

    .macro Delete symbol, slotno=3
    .section ".text.\symbol\()", "ax"
    .global \symbol
    .balign 4
\symbol\():
    ldr     r0, [r0, #4]                // r0 -> real this
    stmfd   r13!, {r0, r14}             // save this, LK
    adr     r14, return                 // set return addr
    ldr     r12, [r0, #8]               // r12 -> btable
    add     pc, r12, #\slotno*4         // call it
return:
    ldmfd   r13!, {r0, r14}             // restore this, return addr
    b       _Z12FreeInstanceP9TProtocol // tail call to free mem
    .endm

    .macro Dispatch symbol, slotno
    .section ".text.\symbol\()", "ax"
    .global \symbol
    .balign 4
\symbol\():
    _Dispatch    \slotno
    .endm

    New _ZN14PDemoInterface3NewEPc, PDemoInterface
    Delete _ZN14PDemoInterface6DeleteEv

    Dispatch    _ZN14PDemoInterface14GetConstStringERK6RefVar, 4
    Dispatch    _ZN14PDemoInterface15GetArrayElementERK6RefVarS2_, 5
    Dispatch    _ZN14PDemoInterface14BreakInMonitorERK6RefVar, 6
