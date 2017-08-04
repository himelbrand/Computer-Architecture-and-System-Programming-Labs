all: task2

task2: menu-map.o
	gcc -m32 -Wall -g  -o task2 menu-map.o 
menu-map.o: menu-map.c
	gcc -m32 -Wall -ansi -g -c -o menu-map.o menu-map.c
.PHONY: clean
clean:
	rm -f *.o task2