.global main

.section .text

main:

    xor %rax, %rax
    mov $fmt, %rdi
    mov %rsp, %rsi

    call printf

    push $4

    mov $fmt, %rdi
    mov %rsp, %rsi
    call printf
    pop %rax

    ret
fmt:
.ascii "%p\n\0"
