.section .data
.section .bss
.section .text

.equ SYS_WRITE64, 1
.equ SYS_OPEN64,  2
.equ SYS_CLOSE64, 3
.equ SYS_EXIT, 1
.equ SYS_O_WRONLY_O_TRUNC_O_CREAT, 01101
.equ LINUX_SYSCALL, 0x80

.global _start
_start:
    #从命令行获得文件名，及内容 ./a.out "test" "ni hao"
    cmpl $1, %edi
    je _start_end

    #buffer 长度
    movq 16(%rsi), %rdi
    call strlen
    pushq %rax

    #buffer地址
    pushq 16(%rsi)

    #文件名
    pushq 8(%rsi)
    call write_file
    addl $8, %esp
    
_start_end:
    movl $SYS_EXIT, %eax
    xorl %ebx, %ebx
    int $LINUX_SYSCALL

.type write_file, @function
write_file:
    pushq %rbp
    movq %rsp, %rbp
    #取得第一个参数, 文件名
    movq 16(%rbp), %rdi
    #取得第二个参数, buffer 地址
    movq 24(%rbp), %rsi
    #取得第三个参数, buffer 长度
    movq 32(%rbp), %r8

    pushq %rsi
    #调用open
    movl $SYS_OPEN64, %eax
    movl $SYS_O_WRONLY_O_TRUNC_O_CREAT, %esi
    movl $0644, %edx
    syscall

    #检查返回值
    cmpl $0, %eax
    jl write_file_end

    popq %rsi
    movq %rax, %r9 #fd

    #调用write

    movl $SYS_WRITE64, %eax
    movq %r9, %rdi
    movq %r8, %rdx
    syscall

    #调用close
    movl $SYS_CLOSE64, %eax
    movq %r9, %rdi
    syscall
write_file_end:
    movq %rbp, %rsp
    popq %rbp
    ret
