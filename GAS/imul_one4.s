.section .text
.global _start
_start:
    movl $0x7fffffff, %eax
    movl $4, %ecx
    imull %ecx
