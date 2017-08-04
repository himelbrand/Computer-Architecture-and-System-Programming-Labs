flame2 : skeleton.o 
	ld -m elf_i386 -o flame2 $^

skeleton.o : skeleton.s
	nasm -f elf skeleton.s -o skeleton.o

.PHONY: clean

clean:
	rm -f skeleton.o flame2
