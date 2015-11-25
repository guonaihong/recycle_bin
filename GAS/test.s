.section .text
.global _start
_start:
    movw $4, %ax
    movw $2, %dx
    idivw %dx
