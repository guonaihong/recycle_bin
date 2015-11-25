.section .data

hello_world:
.ascii "hello, world"


.section .text
.globl main

main:

mov $hello_world, %rdi
call puts

ret

