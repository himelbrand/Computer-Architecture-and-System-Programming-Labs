all: encoder

encoder: encoder.o
	gcc -g -m32 -Wall -o encoder encoder.o

encoder.o: encoder.c
	gcc -g -m32 -Wall -ansi -c -o encoder.o encoder.c

.PHONY: clean

clean:
	rm -f *.o encoder