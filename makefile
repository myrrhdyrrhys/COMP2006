CC = gcc
CFLAGS = -Wall -ansi -pedantic -Werror -g
OBJ = Parent.o
EXEC = Scheduler

$(EXEC) : $(OBJ) FileIO.o sorts.o 
	$(CC) $(OBJ) FileIO.o sorts.o -pthread -o $(EXEC)

Parent.o : Parent.c structs.h sorts.h FileIO.h
	$(CC) $(CFLAGS) Parent.c -c

FileIO.o : FileIO.c structs.h
	$(CC) $(CFLAGS) FileIO.c -c

sorts.o : sorts.c structs.h
	$(CC) $(CFLAGS) sorts.c -c

clean :
	rm -rf Scheduler Parent.o FileIO.o sorts.o
