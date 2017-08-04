        global printer
        extern WorldWidth, WorldLength,pTime,gTime,state, system_call, numberOfCells ,resume, debug

        ;; /usr/include/asm/unistd_32.h
sys_write:      equ   4
stdout:         equ   1

section .rodata:
        newLineString: DB "",10,0
        lengthStr:
        DB "length=",0
        widthStr:
        DB "width=",0
        genNumStr:
        DB "number of generations=",0
        pFreqStr:
        DB "print frequency=",0

section .data
counter dd 0
counterWidth dd 0
tempAge dd 0
timeval:
    tv_sec  dd 0
    tv_usec dd 0
section .text
%macro str_len 1
    push ebx
    mov ebx, dword[ebp+%1]
    mov ecx, 0
    dec ebx
    %%count:
    inc ecx
    inc ebx
    cmp byte[ebx],0
    jnz %%count
    dec ecx
    pop ebx
%endmacro
printer:
    mov byte[tempAge +1],0
    mov byte[tempAge +2],0
    mov byte[tempAge +3],0
    cmp dword[debug], 1
    mov edx,1
    jz .debug
    ;;sleep for nice visual
    ; mov dword [tv_sec], 0
    ; mov dword [tv_usec], 1000000
    ; mov eax, 162
    ; mov ebx, timeval
    ; mov ecx, 0
    ; int 0x80
    ;;end of sleep

    .round:
    mov edi,dword[counter]
    mov esi,dword[counterWidth]
    cmp edi,dword[numberOfCells]
    jz .finish
    cmp dword[WorldWidth],esi
    jz .newLine
    push eax
    push 2
    cmp byte[state+edi],0
    jz .deadCell
    mov byte[tempAge],'0'
    push eax
    mov eax,0
    mov al,byte[state+edi]
    add byte[tempAge],al
    pop eax
    push tempAge
    jmp .skip1
    .deadCell:
    mov byte[tempAge],'0'
    push tempAge
    .skip1:
    push edx
    push 4
    call system_call
    add esp,4*4 
    pop eax
    inc dword[counter]
    inc dword[counterWidth]
    jmp .round
    .newLine:
    mov dword[counterWidth],0
    push 2
    push newLineString
    push edx
    push 4
    call system_call
    add esp,4*4
    jmp .round
    .finish:
    mov dword[counter],0
    mov dword[counterWidth],0
    push 2
    push newLineString
    push edx
    push 4
    call system_call
    add esp,4*4

    cmp dword[debug],1
    jz done_debug
    xor ebx, ebx
    call resume             ; resume scheduler
    jmp printer
    .debug:
    ;;print length
    push 8
    push lengthStr
    push 2
    push 4
    call system_call
    add esp,4*4
    str_len 20 ;affects ecx
    push ecx
    push dword[ebp+20]
    push 2
    push 4
    call system_call
    add esp,4*4
    push 2
    push newLineString
    push 2
    push 4
    call system_call
    add esp,4*4
    ;;print width
    push 7
    push widthStr
    push 2
    push 4
    call system_call
    add esp,4*4
    str_len 24 ;affects ecx
    push ecx
    push dword[ebp+24]
    push 2
    push 4
    call system_call
    add esp,4*4
    push 2
    push newLineString
    push 2
    push 4
    call system_call
    add esp,4*4
    ;;print gen
    push 23
    push genNumStr
    push 2
    push 4
    call system_call
    add esp,4*4
    str_len 28 ;affects ecx
    push ecx
    push dword[ebp+28]
    push 2
    push 4
    call system_call
    add esp,4*4
    push 2
    push newLineString
    push 2
    push 4
    call system_call
    add esp,4*4    
    push 17
    push pFreqStr
    push 2
    push 4
    call system_call
    add esp,4*4
    str_len 32 ;affects ecx
    push ecx
    push dword[ebp+32]
    push 2
    push 4
    call system_call
    add esp,4*4
    push 2
    push newLineString
    push 2
    push 4
    call system_call
    add esp,4*4

    mov edx,2
    jmp printer.round
    done_debug:
    push 2
    push newLineString
    push 2
    push 4
    call system_call
    add esp,4*4

    mov edx,1
    mov dword[debug], 0
    jmp printer









