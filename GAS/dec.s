.section .text
.global _start
_start:
    movb $0xff, %al
    decb %al #8位dec指令

    movw $0xffff, %ax
    decw %ax #16位dec指令

    movl $0xffffffff, %eax
    decl %eax #32位dec指令

    movq $0xffffffffffffffff, %rax
    decq %rax #64位dec指令
