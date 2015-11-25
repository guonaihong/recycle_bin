.section .text
.global _start
_start:
    movb $0b11111111, %al #1个字节
    shlb $3, %al

    movw $0b11111111, %ax #2个字节
    shlw $3, %ax

    movl $0b11111111, %eax #4个字节
    shll $3, %eax

    movq $0b11111111, %rax #8个字节
    shlq $3, %rax
