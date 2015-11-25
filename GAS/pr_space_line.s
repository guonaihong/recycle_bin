
.section .bss
.lcomm buf, 512

.section .text
.global pr_space_line
.global main

pr_space_line:
    cmp $0,  %rdi
    mov $-1, %rax
    jle pr_fail

pr_loop:
    push %rdi
    mov  $str_zero, %rdi
    call puts
    pop  %rdi
    dec  %rdi
    jnz  pr_loop

pr_fail:
    ret

str_zero:
.asciz ""

main:
    mov $buf, %rdi
    mov $512, %rsi
    mov stdin, %rdx
    call fgets

    mov $buf, %rdi
    call atoi
    mov %rax, %rdi
    call pr_space_line
    ret
