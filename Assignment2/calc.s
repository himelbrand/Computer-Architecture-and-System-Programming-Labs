section	.rodata
print_op_form:
	DB	"%x",0	; Format integer
print_op_form_free:
	DB	"free : %x",0	; Format integer
print_op_form_malloc:
	DB	"malloc : %x",0	; Format integer
print_count_form:
	DB	"%d ",0	; Format integer
stackoverflow_form:
	DB "Error: Operand Stack Overflow",10,0 ;Format not legal input
exponent_too_big_form:
	DB "Error: exponent too large",10,0 ;Format not legal input
insufficient_op_form:
	DB "Error: Insufficient Number of Arguments on Stack",10,0 ;Format not legal input
debug_form:
	DB "Debug - %s",0
prompt:
	DB ">>calc: ",0 
pprompt:
	DB ">>",0
f1:
	DB "%c%c, %x",0
newline:
	DB "",10,0
doublez:
	DB "00",0
lead_zero:
	DB "0%x",0
place_holder:
	DB "%s",0
dbg_push: 
	DB "push result: ",0
dbg_dup: 
	DB "dup result: ",0
dbg_add: 
	DB "add result: ",0
dbg_shl: 
	DB "shift left result: ",0
dbg_shr: 
	DB "shift right result: ",0
section .bss
	operand_stack: resd 5 
	input: resb 256 ;;because we used the input for operations we couldnt shift too much I think....
	nibble_conversion_table: resb 256
section .data
	op_esp dw 0
	op_ebp dw 0
	op_index dw 0
	op_count dw 0
	str_len dw 0
	debug_str_len dw 0
	new_str_len dw 0
	first_non_zero dw 0
	add_had_carry_ db 0
	shr_had_carry_ db 0
	shr_has_carry_ db 0
	shahar_temp1 dw 0
	shahar_temp2 dw 0
	first_non_zero1 dw 0
	debug_flag dw 0
	save_operand_add1 dd 0
	save_operand_add2 dd 0
	save_operand_shl dd 0
	save_operand_print dd 0
	save_operand_shr1 dd 0
	save_operand_shr2 dd 0
	save_operand_shrtemp dd 0
	last_operand_save dw 0
	shr_count dw 0

section .text
	align 16
	global main
	extern printf
	extern exit
	extern fprintf
	extern malloc
	extern free
	extern fgets
	extern stderr
	extern stdin
	extern stdout
	first equ operand_stack
	second equ operand_stack+4
	third equ operand_stack+8
	fourth equ operand_stack+12
	last equ operand_stack+16
	
%macro getebp 0
	cmp dword[op_index], 0
	jz %%empty
	cmp dword[op_index], 1
	jz %%first
	cmp dword[op_index], 2
	jz %%second
	cmp dword[op_index], 3
	jz %%third
	cmp dword[op_index], 4
	jz %%fourth
	cmp dword[op_index], 5
	jz %%last
	%%empty:
	mov ebx, 0
	jmp %%done
	%%first:
	mov ebx, [first]
	jmp %%done
	%%second:
	mov ebx, [second]
	jmp %%done
	%%third:
	mov ebx, [third]
	jmp %%done
	%%fourth:
	mov ebx, [fourth]
	jmp %%done
	%%last:
	mov ebx, [last]
	jmp %%done
	%%done:
%endmacro

%macro getesp 0
	cmp dword[op_index], 0
	jz %%empty
	cmp dword[op_index], 1
	jz %%first
	cmp dword[op_index], 2
	jz %%second
	cmp dword[op_index], 3
	jz %%third
	cmp dword[op_index], 4
	jz %%fourth
	cmp dword[op_index], 5
	jz %%last
	%%empty:
	mov edx, first
	jmp %%done
	%%first:
	mov edx, second
	jmp %%done
	%%second:
	mov edx, third
	jmp %%done
	%%third:
	mov edx, fourth
	jmp %%done
	%%fourth:
	mov edx, last
	jmp %%done
	%%last:
	mov edx, 0
	jmp %%done
	%%done:
%endmacro

%macro print_debug 1
	cmp dword[debug_flag],1
	jnz %%skip_debug
	pushad
	pushfd
	getebp ;ebx points to top element
	mov eax, 0 ;will hold the number to print
	mov edx, 0
	mov ecx, 0
	%%push_all:
	mov dl, byte[ebx]
	cmp edx, 10
	jl %%zeros
	jmp %%reg
	%%zeros:
	cmp dword[ebx + 1],0
	jz %%reg
	cmp edx, 0
	jz %%two_zeroes
	push dword edx
	push dword lead_zero
	push dword[stderr]
	jmp %%pushed
	%%two_zeroes:
	push dword doublez
	push dword place_holder
	push dword[stderr]
	jmp %%pushed
	%%reg:
	push dword edx
	push dword print_op_form
	push dword[stderr]
	%%pushed:
	inc eax
	mov dword[debug_str_len], eax
	inc ebx
	cmp dword[ebx], 0
	jz %%start_print
	mov edx, 0
	mov ebx, [ebx]
	jmp %%push_all
	%%start_print:

	push %1
	push debug_form
	push dword[stderr]
	call fprintf
	add esp,3*4

	%%print_operand:
	sub dword[debug_str_len], 1
	call fprintf
	add esp, 3*4
	cmp dword[debug_str_len], 0
	jg %%print_operand
	%%done_print:
	printnewline
	popfd
	popad
	%%skip_debug:
%endmacro

%macro mymalloc 1

	push dword %1
	call malloc
	add esp, 4;clean stack
	test eax,eax
	jz fail_exit
%endmacro

%macro printnewline 0
	push dword newline
	call printf
	add esp, 4
%endmacro

%macro printprompt 0
	push dword prompt
	call printf
	add esp, 4
%endmacro

%macro printpprompt 0
	push dword pprompt
	call printf
	add esp, 4
%endmacro

%macro newval 0
	mov edx, [str_len]
	mov ebx, 0
	mov ecx, 0
	cmp edx,[first_non_zero]
	jle %%alone
	mov cl, byte[input+edx-1]
	mov bl, byte[nibble_conversion_table+ecx]
	shl ebx, 4
	%%alone:
	mov cl, byte[input+edx]
	mov cl, byte[nibble_conversion_table+ecx]
	add ebx, ecx
	dec edx
	mov [str_len], edx
%endmacro

%macro popop 0
	cmp dword[op_index], 0
	jz stackoverflow_err;change
	cmp dword[op_index], 1
	jz %%first
	cmp dword[op_index], 2
	jz %%second
	cmp dword[op_index], 3
	jz %%third
	cmp dword[op_index], 4
	jz %%fourth
	cmp dword[op_index], 5
	jz %%last
	jmp %%done
	%%first:
	push dword[first]
	jmp %%done
	%%second:
	push dword[second]
	jmp %%done
	%%third:
	push dword[third]
	jmp %%done
	%%fourth:
	push dword[fourth]
	jmp %%done
	%%last:
	push dword[last]
	jmp %%done
	%%done:
	sub dword[op_index], 1
%endmacro

%macro clear_input 0
mov ecx,0
%%clear_loop:
mov byte [input+ecx],0
inc ecx
cmp ecx,256
jnz %%clear_loop
%endmacro

%macro free_alloc 0
%%free_loop:
	cmp dword[edx + 1],0
	jz %%last_free
	push eax
	push dword[edx +1]
	push edx
	call free
	add esp,4
	pop edx
	pop eax
	jmp %%free_loop
%%last_free:
	push eax
	push edx
	call free
	add esp,4
	pop eax
%endmacro

main:
	push ebp
	mov ebp, esp
	mov ecx,[esp + 8]
	cmp ecx,2
	jnz no_debug
	mov ecx,[esp + 12]
	mov ecx,[ecx + 4]
	cmp byte[ecx],"-"
	jnz no_debug
	cmp byte[ecx + 1],"d"
	jnz no_debug
	cmp byte[ecx + 2],0
	jnz no_debug
	mov dword[debug_flag],1

	no_debug:
	call setup_conversion_table
	mov ecx,0

	calc_loop:
		pushad
		printprompt
		push dword [stdin]
		push dword 82
		push dword input
		call fgets;eax holds number of chars read
		add esp, 3*4 ;clean stack
		popad
		pushad
		jmp check_opr
		back:
		popad
		inc ecx
		jmp calc_loop
		back_err:
		popad
		jmp calc_loop

push_operand:
	pushad
	cmp dword[op_index], 5
	jge stackoverflow_err
	popad
	mov edx, 0

find_input_len:
	cmp byte[input+edx], 10
	jz found
	inc edx
	jmp find_input_len

found:
	dec edx
	mov [str_len], edx
	mov ecx, 0

find_first_non_zero:	
	cmp byte[input+ecx], "0"
	jnz find_first_non_zero.found
	inc ecx
	cmp ecx, dword[str_len]
	jl find_first_non_zero
	.found:
	mov [first_non_zero],ecx
	jmp first_node
	first_node:
	mymalloc 5
	getesp
	mov dword [edx], eax ;puts pointer to last node added in operand stack
	newval;edx = str_len
	mov byte [eax], bl
	next_node:
	dec edx
	mov [str_len], edx
	mov ebx, eax;ebx holds prev node
	cmp edx, [first_non_zero]
	jl done_push
	mymalloc 5
	inc ebx
	mov dword [ebx], eax
	newval
	mov byte [eax], bl
	jmp next_node 
	done_push:
	mov dword[eax + 1], 0
	add dword[op_index], 1
	print_debug dbg_push
	ret

try_dup:
	pushad
	cmp dword[op_index], 0
	jle insufficient_operands_err
	cmp dword[op_index], 5
	jge stackoverflow_err
	popad
	getebp
	mymalloc 5
	getesp
	mov dword[edx], eax
	.next:
	mov ecx, 0
	mov cl, byte[ebx]
	mov byte[eax], cl
	cmp [ebx+1], dword 0
	jz try_dup.done
	mov ebx, [ebx+1]
	mov edi, eax; save prev node
	mymalloc 5
	inc edi
	mov dword[edi], eax
	jmp try_dup.next
	.done:
	mov dword [eax+1], 0
	add dword[op_index], 1
	print_debug dbg_dup
	ret 

try_print:
	pushad
	cmp dword[op_index], 0
	jle insufficient_operands_err
	popad
	getebp
	mov dword[save_operand_print],ebx
	mov eax, 0 ;will hold the number to print
	mov edx, 0
	mov ecx, 0
	.push_all:
	mov dl, byte[ebx]
	cmp edx, 10
	jl try_print.zeros
	jmp try_print.reg
	.zeros:
	cmp dword[ebx + 1],0
	jz try_print.reg
	cmp edx, 0
	jz try_print.two_zeroes
	push dword edx
	push dword lead_zero
	jmp try_print.pushed
	.two_zeroes:
	push dword doublez
	push dword place_holder
	jmp try_print.pushed
	.reg:
	push dword edx
	push dword print_op_form
	.pushed:
	inc eax
	mov dword[str_len], eax
	inc ebx
	cmp dword[ebx], 0
	jz start_print
	mov edx, 0
	mov ebx, [ebx]
	jmp try_print.push_all
	start_print:
	printpprompt
	print_operand:
	sub dword[str_len], 1
	call printf
	add esp, 2*4
	cmp dword[str_len], 0
	jg print_operand
	done_print:
	printnewline
	sub dword[op_index], 1
	;free alloc
	push edx
	mov edx,dword[save_operand_print]
	free_alloc
	pop edx
	ret

try_add:
	pushad
	cmp dword[op_index], 1
	jle insufficient_operands_err
	popop;to stack
	popop;to stack
	call add_numbers
	add esp,2*4
	popad
	ret

add_numbers:
	mov edi, [esp+4] ;first number
	mov esi, [esp+8] ;second number
	mov dword[save_operand_add1],edi
	mov dword[save_operand_add2],esi
	clear_input
	mov ecx, 255
	mov eax, 0
	mov byte[input+ecx], 0
	dec ecx
	.start_add:
	.nextNode:
	cmp byte[input+ecx], 1
	jz add_numbers.temp
	mov byte[input+ecx], 0
	.temp:
	mov al, 0
	mov ah, 0
	mov al, byte[edi]
	add al, byte[esi]
	daa
	jc add_numbers.got_carry
	jmp add_numbers.not_got_carry
	.got_carry:
	add byte[input+ecx], al
	dec ecx
	mov byte[input+ecx], 0
	add byte[input+ecx], 1
	mov byte [add_had_carry_],1
	jmp add_numbers.move_on
	.not_got_carry:
	mov byte [add_had_carry_],0
	add al,byte[input+ecx]
	daa
	jc add_numbers.add_to_carry
	jmp  add_numbers.not_add_to_carry

	.add_to_carry:
	dec ecx
	mov byte[input+ecx], 0
	add byte[input+ecx], 1
	inc ecx
	mov byte [add_had_carry_],1
	.not_add_to_carry:
	mov byte[input+ecx],al
	dec ecx
	jmp add_numbers.move_on
	.move_on:
	mov ebx,0
	cmp dword[edi + 1],0
	jz add_numbers.edi_jmp
	mov edi,dword[edi + 1]
	jmp add_numbers.edi2_jmp
	.edi_jmp:
	mov byte[edi],0
	add ebx,1
	.edi2_jmp:
	cmp dword[esi + 1],0
	jz add_numbers.esi_jmp
	mov esi,dword[esi + 1]
	jmp add_numbers.esi2_jmp
	.esi_jmp:
	mov byte[esi],0
	add ebx,1
	.esi2_jmp:
	cmp ebx,2
	jnz add_numbers.nextNode

	cmp byte[add_had_carry_],1
	jnz add_numbers.keep_ecx
	dec ecx
	.keep_ecx:
	mov [str_len],ecx
	inc dword[str_len]
	mov ebx,255
	sub ebx,[str_len]
	mov [str_len],ebx
	mov ebx,0
	first_node_push:
	mymalloc 5
	getesp
	mov dword [edx], eax ;puts pointer to last node added in operand stack
	mov edx,[str_len]
	mov ebx,0
	mov esi,254
	mov bl,[input + esi]
	mov byte [eax], bl
	next_node_push:
	dec dword[str_len]
	mov ebx, eax;ebx holds prev node
	cmp dword[str_len], 0
	jle done_push_add
	mymalloc 5
	inc ebx
	dec esi
	mov dword [ebx], eax
	mov cl,byte[input + esi]
	mov byte [eax], cl
	jmp next_node_push
	done_push_add:
	mov dword[eax + 1], 0
	add dword[op_index], 1
	print_debug dbg_add
	;to free malloc you have to use edx reg
	push edx
	mov edx,dword[save_operand_add1]
	free_alloc
	pop edx
	push edx
	mov edx,dword[save_operand_add2]
	free_alloc
	pop edx
	ret

try_shl:
	pushad
	cmp dword[op_index], 1
	jle insufficient_operands_err
	getebp
	cmp dword[ebx+1],0
	jnz exponent_too_big
	popop;to stack
	call shift_left
	add esp,1*4
	popad
	ret

shift_left:
	mov edi, [esp+4] ;shift n times
	mov dword[save_operand_shl],edi
	mov ecx, 0
	mov eax, 0
	mov ebx, 0 ;this will hold low nibble
	mov edx, 0 ; this will hold high nibble
	.loop_shift:
	cmp byte[edi],0
	jz shift_left.check_next_address
	jmp shift_left.sub_shift
	.check_next_address:
	cmp dword[edi +1],0
	jz shift_left.finish
	jmp shift_left.sub_shift
	.continue:
	push edi
	call try_dup
	call try_add
	pop edi
	jmp shift_left.loop_shift
	.sub_shift:
	mov ebx,edi
	mov edx,edi
	mov al,byte[edi]
	sub al,1
	das 
	mov byte[edi],al
	cmp byte[edi],0
	jl shift_left.got_carry
	.finish_shift:
	jmp shift_left.continue
	.got_carry:
	mov edx,ebx
	mov ebx,[ebx + 1] ; address of next node
	cmp dword[edx +1],0
	jnz shift_left.take_carry
	cmp ebx,0 ;;@@ this was added
	jz shift_left.continue
	mov al,byte[ebx]
	sub al,1
	das 
	mov byte[ebx],al
	jmp shift_left.continue
	.take_carry:
	mov byte[edx],0x99
	cmp byte[ebx],0
	jz shift_left.got_carry
	mov al,byte[ebx]
	sub al,1
	das 
	mov byte[ebx],al
	cmp byte[ebx],0
	jnz shift_left.continue
	mov dword[edx +1],0
	jmp shift_left.continue
	.finish:
	print_debug dbg_shl
	push eax
	push edx
	mov edx,dword[save_operand_shl]
	free_alloc
	pop edx
	pop eax
	ret

try_shr:
	pushad
	cmp dword[op_index], 1
	jle insufficient_operands_err
	getebp
	cmp dword[ebx+1],0
	jnz exponent_too_big
	popop;to stack
	popop
	call shift_right
	add esp,2*4
	popad
	ret

shift_right:
	mov edi,[esp+8]
	mov esi, [esp+4]
	mov [save_operand_shrtemp],esi
	mov dword[save_operand_shr1],edi
	mov dword[save_operand_shr2],esi


	mov eax,0
	mov al,byte[edi]
	mov dword[shr_count],eax
	jmp shift_right.first_shift
	.next_shift:
	pop esi
	mov [save_operand_shrtemp],esi
	mov eax,dword[shr_count]
	cmp eax, 0
	jz shift_right.finish
	.first_shift:
	mov dword[new_str_len],0
	.push_to_stack:
	add dword[new_str_len],1
	mov edx,0
	mov dl,byte[esi]
	push edx
	mov edx,esi
	mov esi,[esi+1]
	cmp esi,0
	jnz shift_right.push_to_stack
	mov byte[shr_had_carry_],0
	mov ch,0
	.loop_right:
	cmp dword[shr_count],0
	jl shift_right.finish
	mov edx,0
	pop edx
	dec dword[new_str_len]
	mov dh,dl
	and dl, 0x0f
	and dh, 0xf0
	shr dh,4
	mov byte [shr_has_carry_],0
	mov cl,dh
	and cl,1
	cmp cl,0
	jz shift_right.not_got_carry_dh
	add byte[shr_has_carry_],5
	.not_got_carry_dh:
	shr dh,1
	add dh,byte[shr_had_carry_]
	mov byte[shr_had_carry_],0
	mov cl,dl
	and cl,1
	cmp cl,0
	jz shift_right.not_got_carry_dl
	mov byte[shr_had_carry_],5
	.not_got_carry_dl:
	shr dl,1
	add dl,byte[shr_has_carry_]
	mov ebx, 0
	mov bl, dl
	mov dl,byte[nibble_conversion_table + ebx]
	mov ebx, 0
	mov bl, dh
	mov dh,byte[nibble_conversion_table + ebx]
	mov ebx, 0
	mov bl, ch
	mov[input + ebx],dh
	inc ch
	mov ebx, 0
	mov bl, ch
	mov[input + ebx],dl
	inc ch
	cmp dword[new_str_len], 0
	jz shift_right.last_char
	jmp shift_right.loop_right
	.last_char:
	mov ebx, 0
	mov bl, ch
	mov byte[input+ebx], 10
	call push_operand
	mov eax,dword[shr_count]
	cmp eax,0
	jl shift_right.finish
	pushad
	mov edx,dword[save_operand_shrtemp]
	free_alloc
	popad
	popop
	mov eax,dword[shr_count]
	sub al,1
	das
	mov dword[shr_count],0
	add dword[shr_count],eax
	jmp shift_right.next_shift
	.finish:
 	add dword[op_index], 1
 	print_debug dbg_shr
 	;;to free malloc you have to use edx reg
	pushad
	mov edx,dword[save_operand_shr1]
	free_alloc
	popad
	ret

check_opr:
	cmp byte[input],"+"
	jz try_add_op
	jmp next_1
	try_add_op:
	call try_add
	jmp back
	next_1:
	cmp byte[input],"r"
	jz try_shr_op
	jmp next_2
	try_shr_op:
	call try_shr
	jmp back
	next_2:
	cmp byte[input],"l"
	jz try_shl_op
	jmp next_3
	try_shl_op:
	call try_shl
	jmp back
	next_3:
	cmp byte[input],"d"
	jz try_dup_op
	jmp next_4
	try_dup_op:
	call try_dup
	jmp back
	next_4:
	cmp byte[input],"p"
	jz try_print_op
	jmp next_5
	try_print_op:
	call try_print
	jmp back
	next_5:
	cmp byte[input],"q"
	jz exit_calc
	jmp next_6
	exit_calc:
	push ecx
	push print_count_form
	call printf
	add esp, 2*4
	push 0
	loop_clean_operand_stack:
	cmp dword[op_index],0
	jle finish_loop_clean_operand_stack
	push edx
	popop
	pop edx
	free_alloc
	pop edx
	jmp loop_clean_operand_stack
	finish_loop_clean_operand_stack:
	call exit
	next_6:
	call push_operand
	jmp back

setup_conversion_table:
	mov byte [nibble_conversion_table + '0'], 0
	mov byte [nibble_conversion_table + '1'], 1
	mov byte [nibble_conversion_table + '2'], 2
	mov byte [nibble_conversion_table + '3'], 3
	mov byte [nibble_conversion_table + '4'], 4
	mov byte [nibble_conversion_table + '5'], 5
	mov byte [nibble_conversion_table + '6'], 6
	mov byte [nibble_conversion_table + '7'], 7
	mov byte [nibble_conversion_table + '8'], 8
	mov byte [nibble_conversion_table + '9'], 9
	mov byte [nibble_conversion_table + 0x0 ], '0'
	mov byte [nibble_conversion_table + 0x1 ], '1'
	mov byte [nibble_conversion_table + 0x2 ], '2'
	mov byte [nibble_conversion_table + 0x3 ], '3'
	mov byte [nibble_conversion_table + 0x4 ], '4'
	mov byte [nibble_conversion_table + 0x5 ], '5'
	mov byte [nibble_conversion_table + 0x6 ], '6'
	mov byte [nibble_conversion_table + 0x7 ], '7'
	mov byte [nibble_conversion_table + 0x8 ], '8'
	mov byte [nibble_conversion_table + 0x9 ], '9'
	ret

stackoverflow_err:
	push stackoverflow_form
	call printf
	add esp, 4
	jmp back_err

insufficient_operands_err:
	push insufficient_op_form
	call printf
	add esp, 4
	jmp back_err

exponent_too_big:
	push exponent_too_big_form
	call printf
	add esp,4
	jmp back_err

fail_exit:
	push stackoverflow_form
	call printf
	add esp, 4