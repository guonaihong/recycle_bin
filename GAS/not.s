.section .text
.global _start
_start:
    movb $0xff, %al
    notb %al #8位的not指令
    
    movw $0xffff, %ax
    notw %ax #16位的not指令

    movl $0xffffffff, %eax
    notl %eax #32位的not指令

    movq $0xffffffffffffffff, %rax
    notq %rax #64位的not指令
