CC = gcc
CFLAGS = -Wall -ansi -pedantic -Werror -g

PPScheduler : PP.o FileIO.o sorts.o
	$(CC) PP.o FileIO.o sorts.o -o PPScheduler

PP.o : PP.c structs.h sorts.h FileIO.h
	$(CC) $(CFLAGS) PP.c -c

FileIO.o : FileIO.c structs.h
	$(CC) $(CFLAGS) FileIO.c -c

sorts.o : sorts.c structs.h
	$(CC) $(CFLAGS) sorts.c -c

clean :
	rm -rf PPScheduler PP.o FileIO.o sorts.o
