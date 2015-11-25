.section .text
.global _start
_start:
    #movl $7, %eax
    #leal 6(%eax), %edx
    
    #movl $3, %eax
    #movl $4, %ebx
    #leal (%eax, %ebx, 2), %ecx

    #movl $3, %eax
    #movl $4, %ebx
    #leal 1000(%eax, %ebx, 2), %ecx

    #movl $3, %eax
    #movl $4, %ebx
    #leal 1000(, %ebx, 2), %ecx
    
    #movq $8, %rbx
    #leaq (, %rbx, 2), %rax

    #movq $8, %rbx
    #leaq (%rbx, %rbx, 2), %rax

    movq $8, %rbx
    leaq -1(%rbx, %rbx, 2), %rax
