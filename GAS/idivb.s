.section .text
.global _start
_start:
    movw $8, %ax
    movb $2, %cl
    idivb %cl
