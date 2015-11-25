.section .text
.global _start
_start:
    # (char)4 / (char)2
    movb $8, %al
    movb $2, %dl
    idivb %dl

    movw $8, %ax
    movw $3, %dx
    idivw %dx
