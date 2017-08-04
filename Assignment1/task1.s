section	.rodata
LC0:
	DB	"%s", 10, 0	; Format string

section .bss
LC1:
	RESB	256

section .text
	align 16
	global my_func
	extern printf

my_func:
	push	ebp
	mov	ebp, esp	; Entry code - set up ebp and esp
	pushad			; Save registers

	mov ecx, dword [ebp+8]	; Get argument (pointer to string)

	b0:
	mov edx, LC1
	mov eax, 0
	mov ebx,0
	b1:
	myloop: 
		cmp byte[ecx],0x0a
		je exitloop 
		mov al, byte[ecx]
		
		sub eax,48
		mov bl,16
		mul bl
		inc ecx
		mov bl, byte[ecx]
		b2:
		call checktype
		b3:
		add ebx,eax
		mov [edx], ebx
		b4:
		inc edx
		inc ecx
		jmp myloop


	checktype:
		cmp bl, '9'
		jg ischar
		sub bl, 48
		ret
	ischar:
		cmp bl, 'Z'
		jle capchar
		sub bl, 'W'
		ret
	capchar:
		add bl, 32
		jmp ischar
	exitloop:


	push	LC1		; Call printf with 2 arguments: pointer to str
	push	LC0		; and pointer to format string.
	call	printf
	add 	esp, 8		; Clean up stack after call

	popad			; Restore registers
	mov	esp, ebp	; Function exit code
	pop	ebp
	ret

