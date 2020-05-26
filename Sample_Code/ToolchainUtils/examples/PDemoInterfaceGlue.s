    .arch armv3
    .global _Z9NewByNamePKcS0_
    .global _Z19AllocInstanceByNamePKcS0_
    .global _Z12FreeInstanceP9TProtocol

    .macro Dispatch slotno
    ldr     r0, [r0, #4]                // r0 -> "real" this
    ldr     r12, [r0, #8]               // r12 -> btable
    add     pc, r12, #\slotno*4         // go there
    .endm

    .macro New classNameProc
    stmfd   r13!, {r0-r3, r14}          // save New(...) args
    mov     r1, r0                      // r1 = implementation name
    bl      \classNameProc              // r0 = interface name
    bl      _Z19AllocInstanceByNamePKcS0_ 
    tst     r0, r0                      // success?
    add     r13, r13, #4                // pop (saved) string parm; r0 is 'this' now (or nil)
    ldmfd   r13!, {r1-r3, r14}          // pop New(...) args
    moveq   pc, r14                     // return nil if instance couldn't be made
    .endm

    .macro Delete slotno
    ldr     r0, [r0, #4]                // r0 -> real this
    stmfd   r13!, {r0, r14}             // save this, LK
    adr     r14, return                 // set return addr
    ldr     r12, [r0, #8]               // r12 -> btable
    add     pc, r12, #\slotno*4         // call it
return:
    ldmfd   r13!, {r0, r14}             // restore this, return addr
    b       _Z12FreeInstanceP9TProtocol // tail call to free mem
    .endm

    .section ".text._ZN14PDemoInterface3NewEPc", "ax"
    .global _ZN14PDemoInterface3NewEPc
    .balign 4

asciz__PDemoInterface:
    mov     r0, pc                      // r0 -> asciz name
    mov     pc, r14                     // begone!
    .string "PDemoInterface"
    .balign 4

_ZN14PDemoInterface3NewEPc:
    New         asciz__PDemoInterface
    Dispatch    2


    .section ".text._ZN14PDemoInterface6DeleteEv", "ax"
    .global _ZN14PDemoInterface6DeleteEv
    .balign 4

_ZN14PDemoInterface6DeleteEv:
    Delete      3

    .section ".text._ZN14PDemoInterface14GetConstStringERK6RefVar", "ax"
    .global _ZN14PDemoInterface14GetConstStringERK6RefVar
    .balign 4

_ZN14PDemoInterface14GetConstStringERK6RefVar:
    Dispatch    4

    .section ".text._ZN14PDemoInterface15GetArrayElementERK6RefVarS2_", "ax"
    .global _ZN14PDemoInterface15GetArrayElementERK6RefVarS2_
    .balign 4

_ZN14PDemoInterface15GetArrayElementERK6RefVarS2_:
    Dispatch    5

    .section ".text._ZN14PDemoInterface14BreakInMonitorERK6RefVar", "ax"
    .global _ZN14PDemoInterface14BreakInMonitorERK6RefVar
    .balign 4

_ZN14PDemoInterface14BreakInMonitorERK6RefVar:
    Dispatch    6
