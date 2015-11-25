.section .text
.global _start
_start:
    movb $0b01111111, %al #符号位是0
    shrb $3, %al

    movb $0b11111111, %al #符号位是1
    shrb $3, %al

    movw $0x7FFF, %ax
    shrw $3, %ax

    movw $0xFFFF, %ax
    shrw $3, %ax


    movl $0x7FFFFFFF, %eax
    shrl $3, %eax

    movl $0xFFFFFFFF, %eax
    shrl $3, %eax

    movq $0x7FFFFFFFFFFFFFFF, %rax
    shrq $3, %rax

    movq $0xFFFFFFFFFFFFFFFF, %rax
    shrq $3, %rax
