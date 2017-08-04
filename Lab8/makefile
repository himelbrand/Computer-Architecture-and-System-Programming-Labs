All: myELF
	
myELF: myELF.o
	gcc -m32 -Wall -g -o myELF myELF.o

myELF.o: task3.c
	gcc -m32 -Wall -c -g -ansi -o myELF.o task3.c

clean: 
	rm -f myELF.o myELF
