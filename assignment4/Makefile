CC=gcc
CFLAGS= -std=gnu99 -Wall -g -pthread
exe_file= line_processor

$(exe_file): line_processor.o main.o
	$(CC) $(CFLAGS) line_processor.o main.o -o $(exe_file)

line_processor.o: line_processor.h line_processor.c
	$(CC) $(CFLAGS) -c line_processor.c

main.o: line_processor.h main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean
clean:
	rm -rf *.o $(exe_file) 
