.section .text

.global reverse_bits
.type reverse_bits, @function
reverse_bits:
    
    xorl %eax, %eax
    movl $32, %ecx
reverse_bits_start:
    cmpl $0, %ecx
    je reverse_bits_end
    shll $1, %eax
    movl %edi, %esi
    andl $1, %esi
    orl  %esi, %eax
    shrl $1, %edi
    decl %ecx

    jmp reverse_bits_start
reverse_bits_end:
    ret
