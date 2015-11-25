.section .text

.global set_bit
.type set_bit, @function
set_bit:
    movl %esi, %edx
    shrl $3, %edx
    movb (%rdi, %rdx, 1), %r8b
    
    movb $1, %r9b
    movb %sil, %cl
    andb $7, %cl
    shlb %cl, %r9b

    orb %r8b, %r9b
    movb %r9b, (%rdi, %rdx, 1)
    ret

.global clear_bit
.type clear_bit, @function
clear_bit:
    movl %esi, %edx
    shrl $3, %edx
    movb (%rdi, %rdx, 1), %r8b
    
    movb $1, %r9b
    movb %sil, %cl
    andb $7, %cl
    shlb %cl, %r9b

    notb %r9b
    andb %r8b, %r9b
    movb %r9b, (%rdi, %rdx, 1)
    ret

.global test_bit
.type test_bit, @function
test_bit:
    movl %esi, %edx
    shrl $3, %edx
    movb (%rdi, %rdx, 1), %r8b
    
    movb $1, %r9b
    movb %sil, %cl
    andb $7, %cl
    shlb %cl, %r9b
    andb %r8b, %r9b
    movl $0, %eax #返回假
    cmpb $0, %r9b
    je test_bit_false
    movl $1, %eax #返回真
test_bit_false:
    ret

.global assign_bit
.type assign_bit, @function
assign_bit:
    cmpl $0, %edx
    je call_clear_bit
    call set_bit
    jmp assign_bit_end
call_clear_bit:
    call clear_bit
assign_bit_end:
    ret
