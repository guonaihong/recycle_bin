.section .text
.global _start
_start:
    movb $1, %al
    negb %al
    movw $-1, %ax
    negw %ax
    movl $1, %eax
    negl %eax
    movq $-1, %rax
    negq %rax

