.section .text
.global _start
_start:
    movw $3, %cx
    imulw $4, %cx

    movl  $3, %ecx
    imull $4, %ecx

    movq  $3, %rcx
    imulq $4, %rcx
