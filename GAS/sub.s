.section .text
.global _start
_start:
    subb $1, %bl
    subw $1, %bx
    subl $1, %ebx
    subq $1, %rbx
