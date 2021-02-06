CC=gcc
CFLAGS= -std=gnu99 -Wall -g
exe_file= smallsh

$(exe_file): dynarray.o smallsh.o main.o
	$(CC) $(CFLAGS) dynarray.o smallsh.o main.o -o $(exe_file)

dynarray.o: dynarray.h dynarray.c
	$(CC) $(CFLAGS) -c dynarray.c

smallsh.o: dynarray.h smallsh.h smallsh.c
	$(CC) $(CFLAGS) -c smallsh.c

main.o: dynarray.h smallsh.h main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean
clean:
	rm -f *.o $(exe_file)
