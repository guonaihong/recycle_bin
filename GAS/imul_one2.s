.section .text
.global _start
_start:
    movw $0x7fff, %ax
    movw $4, %cx
    imulw %cx
