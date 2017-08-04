        global scheduler
        extern resume, end_co
        extern gTime, pTime, numberOfCells
section .data
	gCounter dd -1
	pCounter dd 0
	currentEbx dd 1
section .text

scheduler:
        mov ebx, 2
.next:
	mov edi,dword[esp]   ;gTime
        mov esi,dword[esp +4]   ;pTime

        .shahar7:
	cmp dword[gCounter],esi
	jz .finish
        call resume
        cmp ebx,1
        jz .skip1
        cmp ebx,2
        jnz .skip3
        inc dword[gCounter]
        .skip3:
        inc dword[pCounter]
        jmp .skip2
        .skip1:
        mov ebx,dword[currentEbx]
        mov dword[pCounter],0
        .skip2:
        cmp dword[pCounter],edi 
        jz .printResume
        jmp .nextCell
        .printResume:
        mov dword[currentEbx],ebx
        mov ebx,1
        jmp .next

        .nextCell:
        inc ebx
        sub ebx,2        
        cmp ebx,dword[numberOfCells]
        jz .resRound
        add ebx, 2
        jmp .next
        .resRound:
        mov ebx,2
        jmp .next

        .finish:
        mov ebx,1 ;last print
        call resume

        call end_co             ; stop co-routines