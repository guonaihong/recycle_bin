.type ho_strlen, @function

.global ho_strlen

ho_strlen:

	xor   %ecx, %ecx

ho_string_loop_begin:

	movb (%rdi), %al
	cmp   $0,   %al
	je    ho_string_loop_end
	inc  %ecx
	inc  %rdi

	jmp ho_string_loop_begin

ho_string_loop_end:
	mov   %ecx, %eax
	ret
