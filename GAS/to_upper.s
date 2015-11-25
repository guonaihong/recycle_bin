.section .data
.section .bss

.equ BUFFER_SIZE, 512
.lcomm BUFFER, BUFFER_SIZE
.section .text

.equ SYS_READ64, 0
.equ SYS_WRITE64, 1
.equ SYS_OPEN64, 2
.equ SYS_CLOSE64, 3

.equ ST_BUFFER_SIZE, 16
.equ ARGV1, 8
.equ ARGV2, 16
.equ ST_FD_IN, -8
.equ ST_FD_OUT, -16

.equ OFFSET, 'A' - 'a'

.equ O_RDONLY, 00
.equ O_WRONLY_CREAT_TRUNC, 01101
.global main
main:

    cmpl $3, %edi
    jl main_error

    pushq %rbp
    movq %rsp, %rbp
    subq $ST_BUFFER_SIZE, %rsp
    movl $-1, ST_FD_IN(%rbp)
    movl $-1, ST_FD_OUT(%rbp)

    pushq %rsi
open_fd_in:
    movq $SYS_OPEN64, %rax
    movq ARGV1(%rsi), %rdi
    movl $O_RDONLY, %esi
    syscall
    cmpl $0, %eax
    jl main_error
    movl %eax, ST_FD_IN(%rbp)
    
    popq %rsi
open_fd_out:
    movq $SYS_OPEN64, %rax
    movq ARGV2(%rsi), %rdi
    movl $O_WRONLY_CREAT_TRUNC, %esi
    movl $0644, %edx
    syscall
    cmpl $0, %eax
    jl main_end
    movl %eax, ST_FD_OUT(%rbp)

read_write_begin:

    movq $SYS_READ64, %rax
    movl ST_FD_IN(%rbp), %edi
    movq $BUFFER, %rsi
    movl $BUFFER_SIZE, %edx
    syscall
    cmpl $0, %eax
    jle main_end

    movq %rax, %r8
    pushq %rax
    pushq $BUFFER
    call to_upper
    addq $16, %rsp

    movq $SYS_WRITE64, %rax
    movl ST_FD_OUT(%rbp), %edi
    movq $BUFFER, %rsi
    movq %r8, %rdx
    syscall
    cmpl $0, %eax
    jle main_end
    jmp read_write_begin

main_end:
    movq $SYS_CLOSE64, %rax
    movl ST_FD_IN(%rbp), %edi
    syscall

    movq $SYS_CLOSE64, %rax
    movl ST_FD_OUT(%rbp), %edi
    syscall
    
    xorl %eax, %eax
    movq %rbp, %rsp
    popq %rbp
    ret
main_error:
    movl $1, %eax
    ret

#大写转小写函数
.type to_upper, @function
to_upper:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi
    movl 24(%rbp), %esi

    cmpl $0, %esi
    jle to_upper_end

    xor %rcx, %rcx
to_upper_begin:
    cmpq %rcx, %rsi
    jle to_upper_end

    movb (%rdi, %rcx, 1), %bl
    incq %rcx
    
    cmpb $'a', %bl
    jl to_upper_begin
    cmpb $'z', %bl
    jg to_upper_begin
    
    addb $OFFSET, %bl
    movb %bl, -1(%rdi, %rcx, 1)
    jmp to_upper_begin

to_upper_end:
    movq %rbp, %rsp
    popq %rbp
    ret
