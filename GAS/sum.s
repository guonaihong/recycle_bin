.global sum

.section .text
sum:

    mov $-1, %rax
    cmp $0, %rsi
    jl end
    mov $0, %rax
next:

    add (%rdi), %rax
    add $8, %rdi 
    dec %rsi

    jnz next

end:
    ret
