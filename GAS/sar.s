.section .text
.global _start
_start:
    movb $0b01111111, %al #符号位是0
    sarb $3, %al

    movb $0b11111111, %al #符号位是1
    sarb $3, %al

    movw $0x7FFF, %ax
    sarw $3, %ax

    movw $0xFFFF, %ax
    sarw $3, %ax


    movl $0x7FFFFFFF, %eax
    sarl $3, %eax

    movl $0xFFFFFFFF, %eax
    sarl $3, %eax

    movq $0x7FFFFFFFFFFFFFFF, %rax
    sarq $3, %rax

    movq $0xFFFFFFFFFFFFFFFF, %rax
    sarq $3, %rax
