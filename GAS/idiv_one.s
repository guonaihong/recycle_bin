.section .text
.global _start
_start:
    movl $8, %eax
    movl %eax, %edx
    shrl $31, %edx
    movl $2, %ecx
    idivl %ecx
