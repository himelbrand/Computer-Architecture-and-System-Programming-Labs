All: hexeditplus
	
hexeditplus: hexeditplus.o
	gcc -m32 -Wall -g -o hexeditplus hexeditplus.o

hexeditplus.o: task1.c
	gcc -m32 -Wall -c -g -ansi -o hexeditplus.o task1.c

clean: 
	rm -f *.o hexeditplus
