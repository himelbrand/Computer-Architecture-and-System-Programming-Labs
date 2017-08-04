        global _start
        extern init_co, start_co, resume
        extern scheduler, printer
        extern Cell
    
        ;; /usr/include/asm/unistd_32.h
sys_exit:       equ   1
global state
global WorldWidth
global WorldLength
global gTime
global pTime
global numberOfCells
global system_call
global debug
section .rodata
    debugComp: DD "-d",0

section .bss
	state resb 100*100
	init_file resb 101*100
	tempString resb 2
section .data
	ten: dd 10
    test_atoi: db 0,10,0
	x_loc dd 0
	y_loc dd 0
	count dd 0
	WorldWidth dd 1
	WorldLength dd 1
	numberOfCells dd 0
	filename dd 1
	gTime dd 0
	pTime dd 0
	debug dd 0

section .text

_start:
        enter 0, 0
        ;;get argumants 

        mov ecx, [ebp + 4]      ; ecx = argc
        mov edi, dword[ebp+12]
        cmp byte[edi],'-' ;first arg
        jnz .debug_false
        cmp byte[edi +1],'d'
        jnz .debug_false
        mov dword[debug], 1
        mov esi, 4
        jmp .skip1
        .debug_false:
        mov esi, 0
        .skip1:
        mov edi, dword[ebp+12+esi] ;first arg
        mov dword[filename], edi
        mov edi, dword[ebp+16+esi] ;second arg

        pushad
        ;push edi
        omri:
        call atoi
        ;add esp,4
        mov dword[WorldLength], eax
        popad
        mov edi, dword[ebp+20+esi]	;third

        pushad
        call atoi
        mov dword[WorldWidth], eax
        popad
        mov edi, dword[ebp+24+esi] ;fourth

        pushad
        call atoi
        shl eax,1
        mov dword[gTime], eax
        popad
        mov edi, dword[ebp+28+esi] ;fifth
        pushad
        call atoi
        mov dword[pTime], eax
        popad
       
        ;; init sched & printer
        xor ebx, ebx            ; scheduler is co-routine 0
        mov edx, scheduler
        push dword[gTime]
        push dword[pTime]
        call init_co            ; initialize scheduler state
        add esp,2*4
        inc ebx                 ; printer i co-routine 1
        mov edx, printer
        call init_co            ; initialize printer state
        push eax ;calc total number of cells
        push edx
        mov eax, dword[WorldWidth]
        mul dword[WorldLength]
        mov dword[numberOfCells], eax
        pop edx
        pop eax
        ;read init file
        push 0 ;read only flag
        push dword[filename]
        push 5 ;op code - open
        call system_call ;eax holds fd
        add esp,3*4
        push 100*102
        push init_file
        push eax
        push 3 ;op code - read
        call system_call
        add esp,4*4
      
        ;initialize state
        xor ecx,ecx
        init:
        cmp byte[init_file + ecx],"1"
        jz init.cell_alive
        cmp byte[init_file + ecx]," "
        jz init.cell_dead
        cmp byte[init_file + ecx],10
        jz init.continue
        jmp init.done

        .cell_alive:
        push ecx
        mov ecx, dword[count]
        mov byte[state+ecx],1
        inc dword[count]
        pop ecx
        jmp init.continue
        .cell_dead:
        push ecx
        mov ecx, dword[count]
        mov byte[state+ecx],0
        inc dword[count]
        pop ecx
        jmp init.continue
        .continue:
        inc ecx
        jmp init
        .done:
        ;; initialize cell co-re
        mov ebx,2
        init_co_cells:
        ;ebx holds co-index
        ;update next state
        push ebx
        mov edx,0
        mov eax,ebx
        sub eax,2
        mov ebx,dword[WorldWidth]
        div bx
        mov dword[x_loc],edx
        mov dword[y_loc],eax
        pop ebx
        push dword[y_loc]
        push dword[x_loc]

        cmp dword[count],0
        jz init_co_cells.finish
        ; printer i co-routine 1
        mov edx, cell_ro
        call init_co
        inc ebx 
        dec dword[count]
        jmp init_co_cells
        .finish:
        xor ebx, ebx            ; starting co-routine = scheduler
        call start_co           ; start co-routines
        pop     ebp 
        ;; exit
        mov eax, sys_exit
        xor ebx, ebx
        int 80h

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state
    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret 

cell_ro:
	call Cell
    push eax;push return value
	push ebx
	xor ebx, ebx
	call resume
	pop ebx
	pop eax
	sub ebx,2
	cmp al,0
	jz cell_ro.dead
	add byte[state+ebx], al
    cmp byte[state+ebx],10
    jnz cell_ro.finish
    mov byte[state+ebx],9
	jmp cell_ro.finish
	.dead:
	mov byte[state+ebx], al
	.finish:
	add ebx, 2
	push ebx
	xor ebx, ebx
	call resume
	pop ebx
	jmp cell_ro
	
atoi:
    push    ebp
    mov     ebp, esp        ; Entry code - set up ebp and esp
    push ecx
    push edx
    push ebx
atoi_loop:
    xor edx,edx
    cmp byte[edi],0
    jz  atoi_end
    imul dword[ten]
    mov bl,byte[edi]
    sub bl,'0'
    add eax,ebx
    inc edi
    jmp atoi_loop
atoi_end:
    pop ebx                 ; Restore registers
    pop edx
    pop ecx
    mov esp, ebp        ; Function exit code
    pop ebp
    ret
  		