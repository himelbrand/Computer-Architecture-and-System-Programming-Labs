
all: myshell

myshell: JobControl.o LineParser.o myshell.o
	gcc -m32  myshell.o LineParser.o JobControl.o -o myshell

LineParser.o: LineParser.h LineParser.c
	gcc -m32 -Wall -ansi -c  LineParser.c -o LineParser.o

JobControl.o: JobControl.h JobControl.c
	gcc -m32 -Wall -ansi -c  JobControl.c -o JobControl.o

myshell.o: task4.c
	gcc -m32 -Wall -ansi -c  task4.c -o myshell.o

clean: 
	rm -f *.o myshell
