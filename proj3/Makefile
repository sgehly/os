#Start of Makefile

main: main.o
	gcc -o maze main.o -pthread

main.o: main.c
	gcc -c main.c

clean: 
	-rm maze *.o