;;; This is a simplified co-routines implementation:
;;; CORS contains just stack tops, and we always work
;;; with co-routine indexes.
        global init_co, start_co, end_co, resume,cors


maxcors:        equ 100*100+2         ; maximum number of co-routines
stacksz:        equ 16*1024     ; per-co-routine stack size


section .bss

stacks: resb maxcors * stacksz  ; co-routine stacks
cors:   resd maxcors            ; simply an array with co-routine stack tops
curr:   resd 1                  ; current co-routine
origsp: resd 1                  ; original stack top
tmp:    resd 1                  ; temporary value


section .text

        ;; ebx = co-routine index to initialize
        ;; edx = co-routine start
        ;; other registers will be visible to co-routine after "start_co"
init_co:
        cmp ebx,1
        jz .skip
        mov edi,dword[esp +4] ; pTime | x
        mov esi,dword[esp +8] ; gTime | y
        .skip:
        push eax                ; save eax (on caller's stack)
		push edx
		mov edx,0
		mov eax,stacksz
        imul ebx			    ; eax = co-routine's stack offset in stacks
        pop edx
		add eax, stacks + stacksz ; eax = top of (empty) co-routine's stack
        mov [cors + ebx*4], eax ; store co-routine's stack top
        pop eax                 ; restore eax (from caller's stack)

        mov [tmp], esp          ; save caller's stack top
        mov esp, [cors + ebx*4] ; esp = co-routine's stack top

        cmp ebx,1
        jz .skip1
        push esi ; gTime | y
        push edi ; pTime | x
        .skip1:
        push edx                ; save return address to co-routine stack

        pushf                   ; save flags
        pusha                   ; save all registers
        mov [cors + ebx*4], esp ; update co-routine's stack top

        mov esp, [tmp]          ; restore caller's stack top
        ret                     ; return to caller

        ;; ebx = co-routine index to start
start_co:
        pusha                   ; save all registers (restored in "end_co")
        mov [origsp], esp       ; save caller's stack top
        mov [curr], ebx         ; store current co-routine index
        jmp resume.cont         ; perform state-restoring part of "resume"

        ;; can be called or jumped to
end_co:
        mov esp, [origsp]       ; restore stack top of whoever called "start_co"
        popa                    ; restore all registers
        ret                     ; return to caller of "start_co"

        ;; ebx = co-routine index to switch to
resume:                         ; "call resume" pushed return address
        pushf                   ; save flags to source co-routine stack
        pusha                   ; save all registers
        xchg ebx, [curr]        ; ebx = current co-routine index
        mov [cors + ebx*4], esp ; update current co-routine's stack top
        mov ebx, [curr]         ; ebx = destination co-routine index
.cont:
        mov esp, [cors + ebx*4] ; get destination co-routine's stack top
        popa                    ; restore all registers
        popf                    ; restore flags
        ret                     ; jump to saved return address