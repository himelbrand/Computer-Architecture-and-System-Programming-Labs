ass3: coroutines.o printer.o scheduler.o cell.o ass3.o 
	ld -m elf_i386  -o ass3 $^
ass3.o: ass3.s
	nasm -f elf ass3.s -o ass3.o
coroutines.o: coroutines.s
	nasm -f elf coroutines.s -o coroutines.o
printer.o: printer.s
	nasm -f elf printer.s -o printer.o
scheduler.o: scheduler.s
	nasm -f elf scheduler.s -o scheduler.o
cell.o: cell.c
	gcc -m32 -Wall -ansi -fno-stack-protector -nostdlib -c  cell.c -o cell.o
clean:
	rm *.o ass3