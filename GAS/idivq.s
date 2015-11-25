.section .text
.global _start
_start:
    movq $0, %rax
    movq %rax, %rdx
    shrq $63, %rdx
    movq $5, %rcx
    idivq %rcx
