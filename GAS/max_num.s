.section .data
data_item:
    .long 1,2,3,4,5,6,7,8,9,10,15, -1, 99
data_item_end:

.equ data_item_len, data_item_end - data_item

fmt:
    .ascii "%d\n\0"

.section .text

.global _start
_start:
    movq $data_item_len, %rax
    movl %eax, %edx
    movl $4, %ebx
    sarl $31, %edx
    divl %ebx

    pushq %rax       #数组长度
    pushq $data_item #数组元素首地址
    call max_num

    addq $16, %rsp

    movl %eax, %esi
    movl $fmt, %edi
    xorl %eax, %eax
    call printf

    xorl %edi, %edi
    call exit

.type max_num, @function
max_num:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi #第一个参数
    movl 24(%rbp), %esi #第二个参数
    
    movl -4(%rdi, %rsi, 4), %eax
    leaq -4(%rdi, %rsi, 4), %rdx
    jmp max_num_end

max_num_loop_start:
    movl (%rdx), %ecx
    subl $1, %esi
    cmpl  %ecx, %eax
    cmovl %ecx, %eax
    subq $4, %rdx

max_num_end:
    testl %esi, %esi
    jg max_num_loop_start

    movq %rbp, %rsp
    popq %rbp
    ret

