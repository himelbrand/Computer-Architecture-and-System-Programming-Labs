task1 : task1.o
	gcc -m32 -Wall -o task1 task1.o

task1.o : task1.c
	gcc -g -m32 -l -Wall -ansi -c -o task1.o task1.c

.PHONY: clean

clean:
	rm -f *.o task1