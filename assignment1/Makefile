CC=gcc
CFLAGS= -std=gnu99 -Wall -g
exe_file= movies

$(exe_file): linked_list.o input_handling.o movies.o main.o
	$(CC) $(CFLAGS) linked_list.o input_handling.o movies.o main.o -o $(exe_file)

linked_list.o: linked_list.h linked_list.c
	$(CC) $(CFLAGS) -c linked_list.c

input_handling.o: input_handling.h input_handling.c
	$(CC) $(CFLAGS) -c input_handling.c

movies.o: movies.h movies.c
	$(CC) $(CFLAGS) -c movies.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean
clean:
	rm -f *.o $(exe_file)
