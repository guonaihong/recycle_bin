.section .text
.global _start
_start:
    movb $0x7f, %al
    movb $4, %cl
    imulb %cl
