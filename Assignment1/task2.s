section	.rodata
LC0:
	DB	"%d", 10, 0	; Format integer
LC1:
	DB "x or k, or both are off range",10,0 ;Format not legal input

section .text
	align 16
	global calc_div
	extern printf
	extern check

calc_div:
	push	ebp
	mov	ebp, esp	; Entry code - set up ebp and esp
	pushad			; Save registers

	mov ecx, dword [ebp+12]	; Get argument int k
	mov ebx, dword [ebp+8] ; Get argument int x

	push ecx
	push ebx
	call check
	add esp, 8 ; Clean up stack after call
	
	cmp eax,0 ;Check return value
	je notlegal
	
	mov eax,ebx
	mov ebx,1
	shl ebx,cl
	mov dx,0
	div bx
	push eax
	push LC0
	call printf
	add esp, 8 ; Clean up stack after call
	jmp endCall

notlegal:
	push LC1
	call printf
	add esp, 8 ; Clean up stack after call

endCall:
	popad			; Restore registers
	mov	esp, ebp	; Function exit code
	pop	ebp
	ret





