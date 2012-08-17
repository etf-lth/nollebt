/*
 * Ericsson Baseband Controller
 * Backpack Entry Point
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

.section .head
.thumb
    b       init_hook
    b       uart2_rx_int
.int        backpackProc
.int        procmem+2048

.text
.global InitHW
.thumb_func
InitHW:
    ldr     r0, =0x1fff
    ldr     r1, =0x0080090c
    strh    r0, [r1]

    mov     r1, #0
    ldr     r0, =0x00800d04
    strh    r1, [r0]
    ldr     r0, =0x00800c0c
    strh    r1, [r0]
    ldr     r0, =0x00800920
    strh    r1, [r0]

    mov     r1, #0x43
    ldr     r2, =0x00800924
    strb    r1, [r2]

    mov     r1, #0x26
    strb    r1, [r0]

    mov     pc, lr
