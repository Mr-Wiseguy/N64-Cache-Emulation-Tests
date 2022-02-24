# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches
.set gp=64

.global __umoddi3
.global __udivdi3
.global __moddi3
.global __divdi3

.section .text, "ax"

/* -------------------------------------------------------------------------------------- */
/* need to asm these functions because lib32gcc-7-dev-mips-cross does not exist so we     */
/* cannot naturally link a libgcc variant for this target given this architecture and     */
/* compiler. Until we have a good workaround with a gcc target that doesn't involve       */
/* assuming a 32-bit to 64-bit change, we have to encode these functions as raw assembly  */
/* for it to compile.                                                                     */
/* -------------------------------------------------------------------------------------- */

/* TODO: Is there a non-insane way to fix this hack that doesn't involve the user compiling */
/* a library themselves?                                                                    */
__umoddi3:
    sw    $a0, ($sp)
    sw    $a1, 4($sp)
    sw    $a2, 8($sp)
    sw    $a3, 0xc($sp)
    ld    $t7, 8($sp)
    ld    $t6, ($sp)
    ddivu $zero, $t6, $t7
    bnez  $t7, .do_umoddi3
     nop
    break 7
.do_umoddi3:
    mfhi  $v0
    dsll32 $v1, $v0, 0
    dsra32 $v1, $v1, 0
    jr    $ra
     dsra32 $v0, $v0, 0

__udivdi3:
    sw    $a0, ($sp)
    sw    $a1, 4($sp)
    sw    $a2, 8($sp)
    sw    $a3, 0xc($sp)
    ld    $t7, 8($sp)
    ld    $t6, ($sp)
    ddivu $zero, $t6, $t7
    bnez  $t7, .do_udivdi3
     nop
    break 7
.do_udivdi3:
    mflo  $v0
    dsll32 $v1, $v0, 0
    dsra32 $v1, $v1, 0
    jr    $ra
     dsra32 $v0, $v0, 0

__moddi3:
    sw    $a0, ($sp)
    sw    $a1, 4($sp)
    sw    $a2, 8($sp)
    sw    $a3, 0xc($sp)
    ld    $t7, 8($sp)
    ld    $t6, ($sp)
    ddivu $zero, $t6, $t7
    bnez  $t7, .do_moddi3
     nop
    break 7
.do_moddi3:
    mfhi  $v0
    dsll32 $v1, $v0, 0
    dsra32 $v1, $v1, 0
    jr    $ra
     dsra32 $v0, $v0, 0

__divdi3:
    sw    $a0, ($sp)
    sw    $a1, 4($sp)
    sw    $a2, 8($sp)
    sw    $a3, 0xc($sp)
    ld    $t7, 8($sp)
    ld    $t6, ($sp)
    ddiv  $zero, $t6, $t7
    nop
    bnez  $t7, .do_divdi3_1
     nop
    break 7
.do_divdi3_1:
    daddiu $at, $zero, -1
    bne   $t7, $at, .do_divdi3_2
     daddiu $at, $zero, 1
    dsll32 $at, $at, 0x1f
    bne   $t6, $at, .do_divdi3_2
     nop
    break 6
.do_divdi3_2:
    mflo  $v0
    dsll32 $v1, $v0, 0
    dsra32 $v1, $v1, 0
    jr    $ra
     dsra32 $v0, $v0, 0
