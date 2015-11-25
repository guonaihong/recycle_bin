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

.type ho_strcmp, @function
.global ho_strcmp
ho_strcmp:

ho_strcmp_loop_begin:

	movb (%rdi), %al
	movb (%rsi), %cl

	cmpb $0, %al
	je ho_strcmp_loop_end

	cmpb %al, %cl
	jne ho_strcmp_loop_end

	inc %rdi
	inc %rsi

	jmp  ho_strcmp_loop_begin

ho_strcmp_loop_end:
	subb %cl, %al
	movsbl %al, %eax
	ret
