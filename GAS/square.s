.section .data
.section .text

.global _start
fmt:
    .ascii "%d\n\0"

_start:
    pushq $2
    call square
    addq $8, %rsp               #复位%rsp

    movl %eax, %esi        
    movq $fmt, %rdi       
    xorl %eax, %eax        
    call printf            #调用printf

    movl $0, %edi
    call exit              #调用exit

.type square, @function
square:
    pushq %rbp
    movq %rsp, %rbp
    
    movl 16(%rbp), %eax
    imull %eax, %eax

    movq %rbp, %rsp
    popq %rbp
    ret
