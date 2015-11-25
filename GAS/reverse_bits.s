.section .text

.equ loop_end, 16
.equ size, 31

# %edi 存放循环的计数器
# %esi 存放mask值
.global reverse_bits
.type reverse_bits, @function
reverse_bits:

    movl %edi, %eax
    xorl %edi, %edi

reverse_bits_start:
    cmpl $loop_end, %edi
    je reverse_bits_end
    
    movl $size, %edx
    subl %edi, %edx

    movl %eax, %r8d


    movl $1, %esi  #esi 设置为1
    movl %edx, %ecx
    shll %cl, %esi #左移mask bit
    shrl %cl, %r8d #右移数据bit
    andl $1, %r8d  #取bit位的值
    movl %edi, %ecx#取左移值
    shll %cl, %r8d #左移
    notl %esi      #
    andl %esi, %eax#清除bit位

    movl %eax, %r9d

    movl $1, %esi #esi 设置为1
    movl %edi, %ecx
    shll %cl, %esi #左移mask bit
    shrl %cl, %r9d #右移数据bit
    andl $1, %r9d
    movl %edx, %ecx
    shll %cl, %r9d
    notl %esi
    andl %esi, %eax#清除bit位

    orl %r8d, %eax
    orl %r9d, %eax

    incl %edi
    jmp reverse_bits_start
reverse_bits_end:
    ret
