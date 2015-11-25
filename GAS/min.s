.global main

.section .text
min:
    mov   %rdi, %rax
    cmp   %rsi, %rax
    cmovg %rsi, %rax
    ret

main:
    
    mov   $6,   %rdi
    mov   $3,   %rsi
    call  min
    mov   %rax, %rdx

    xor   %rax, %rax
    mov   $fmt, %rdi
    mov   %rdx, %rsi
    call printf

    xor   %rax, %rax
    ret
fmt:
    .ascii "%ld\n\0"
