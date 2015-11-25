.section .text
.global _start
_start:
    movb $0b11111111, %al
    salb $3, %al

    movw $0b11111111, %ax
    salw $3, %ax

    movl $0b11111111, %eax
    sall $3, %eax
    
    movq $0b11111111, %rax
    salq $3, %rax
