
.equ SIGN_SIZE, 4
.equ SIGN, -4

#str2int(const char *str, int base)
.section .text
.global str2int
.type str2int, @function
str2int:
    pushq %rbp
    movq %rsp, %rbp

    subq $SIGN_SIZE, %rsp
    movl $1, SIGN(%rbp)

    #初始化返回值
    movl $0, %eax
    #判断base的值
    cmpl $0, %esi
    je str2int_init_base
    cmpl $2, %esi
    jl str2int_exit
    cmpl $36, %esi
    jg str2int_exit
    jmp str2int_skip_space

str2int_init_base:
    movl $10, %r9d

str2int_skip_space:

    movb (%rdi), %r8b
    cmpb $0, %r8b
    je str2int_exit

    cmpb $' ', %r8b
    je str2int_skip_space_next
    cmpb $'\r', %r8b
    je str2int_skip_space_next
    cmpb $'\n', %r8b
    je str2int_skip_space_next
    cmpb $'\t', %r8b
    je str2int_skip_space_next

    jmp str2int_sign
str2int_skip_space_next:
    incq %rdi
    jmp str2int_skip_space

str2int_sign:
    cmpb $'-', %r8b
    jne str2int_guess_hex
    incq %rdi
    movl $-1, SIGN(%rbp)

str2int_guess_hex:
    cmpb $'0', %r8b
    jne str2int_check_base
    movl $8, %r9d
    incq %rdi
    movb (%rdi), %r8b
    cmpb $0, %r8b
    je str2int_exit

    cmpb $'b', %r8b
    je str2int_guess_hex_2
    cmpb $'B', %r8b
    je str2int_guess_hex_2
    cmpb $'x', %r8b
    je str2int_guess_hex_16
    cmpb $'X', %r8b
    je str2int_guess_hex_16
    jmp str2int_check_base

str2int_guess_hex_2:
    movl $2, %r9d
    incq %rdi
    jmp str2int_check_base

str2int_guess_hex_16:
    movl $16, %r9d
    incq %rdi

str2int_check_base:
    cmpl $0, %esi
    jne str2int_cal
    movl %r9d, %esi


str2int_cal:
    xorl %r9d, %r9d
    movb (%rdi), %r9b
    cmpb $0, %r9b
    je str2int_exit

    orb $0x20, %r9b

    #if (c >= 'a' && c <= 'z')
    cmpb $'a', %r9b
    jl str2int_cal_10
    cmpb $'z', %r9b
    jg str2int_exit
    subb $'a', %r9b
    addb $10, %r9b
    jmp str2int_cal_next

str2int_cal_10:
    #if (c >= '0' && c <= '9')
    cmpb $'0', %r9b
    jl str2int_exit
    cmpb $'9', %r9b
    jg str2int_exit
    subb $'0', %r9b

str2int_cal_next:
    imull %esi, %eax
    addl %r9d, %eax
    incq %rdi
    jmp str2int_cal

str2int_exit:
    imull SIGN(%rbp), %eax
    movq %rbp, %rsp
    popq %rbp
    ret
