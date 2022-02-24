.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

.text
.align 4
.global cachefunc
cachefunc:
    jr $ra
     addiu $v0, $zero, 1
