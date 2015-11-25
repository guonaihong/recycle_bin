.section .text
.global _start
_start:
    addb $1, %al
    addw $1, %ax
    addl $1, %eax
    addq $1, %rax
