.section .text
.global _start
_start:
    movw $8, %ax
    movw %ax, %dx
    shrw $15, %dx
    movw $2, %cx
    idivw %cx
