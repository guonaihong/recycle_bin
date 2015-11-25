.equ BUFFER_SIZE, 512
.equ SYS_READ, 3
.equ SYS_WRITE, 4
.equ SYS_EXIT, 1
.equ STDIN, 0
.equ STDOUT, 1
.equ OFFSET, 'a' -  'A'
.equ LINUX_SYSCALL, 0x80

.section .data
.section .bss
.lcomm BUFFER, BUFFER_SIZE
.section .text

.global _start
_start:
    #read(0, BUFFER, BUFFER_SIZE)
    movl $SYS_READ, %eax
    movl $STDIN, %ebx
    movl $BUFFER, %ecx
    movl $BUFFER_SIZE, %edx
    int $LINUX_SYSCALL

    #判断返回值
    cmpl $0, %eax
    jle _start_end

    #调用to_lower
    pushq %rax
    pushq $BUFFER
    call to_lower
    #复位rsp
    addq $16, %rsp

    #打印到屏幕上
    movl $SYS_WRITE, %eax
    movl $STDOUT, %ebx
    movl $BUFFER, %ecx
    movl $BUFFER_SIZE, %edx
    int $LINUX_SYSCALL

_start_end:
    #调用exit
    movl $SYS_EXIT, %eax
    xorl %ebx, %ebx
    int $LINUX_SYSCALL


.type to_lower, @function
to_lower:
    pushq %rbp
    movq %rsp, %rbp
    #获取第一个参数
    movl 16(%rbp), %edi
    #获取第二个参数
    movl 24(%rbp), %esi
    
    #edi为下标
    xorl %edx, %edx

to_lower_begin:
    #esi 下标等于长度时退出循环
    cmpl %edx, %esi
    jle to_lower_end

    #取出当前字节
    movb (%edi, %edx, 1), %cl
    incl %edx
    #如果是'A' ~ 'Z' 改变buffer里面的值
    cmpb $'A', %cl
    jl to_lower_begin
    cmpb $'Z', %cl
    jg to_lower_begin
    
    addb $OFFSET, %cl
    movb %cl, -1(%edi, %edx, 1)
    jmp to_lower_begin

to_lower_end:
    movq %rbp, %rsp
    popq %rbp
    ret
