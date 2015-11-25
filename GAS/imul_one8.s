.section .text
.global _start
_start:
    movq $0x7fffffffffffffff, %rax
    movq $4, %rcx
    imulq %rcx
