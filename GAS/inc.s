.section .text
.global _start
_start:
    movb $0x0, %al
    incb %al #8位inc指令

    movw $0x0, %ax
    incw %ax #16位inc指令

    movl $0x0, %eax
    incl %eax #32位inc指令

    movq $0x0, %rax
    incq %rax #64位inc指令
