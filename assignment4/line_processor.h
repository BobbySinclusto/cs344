#ifndef LINE_PROCESSOR_H
#define LINE_PROCESSOR_H

#include <stdbool.h>
#include <pthread.h>

#define NUM_LINES 50
#define LINE_SIZE 1001
#define OUTPUT_SIZE 81

// Functions for dealing with the individual buffers
void get_ib(char copy_to[LINE_SIZE]);

void put_ib(char copy_from[LINE_SIZE]);

void get_sb(char copy_to[LINE_SIZE]);

void put_sb(char copy_from[LINE_SIZE]);

char* put_ob(char *copy_from);

int get_ob(char copy_to[OUTPUT_SIZE]);

// Gets input from the user
void* get_input(void *args);

// Replaces the newlines with spaces
void* replace_newlines(void *args);

// Replaces ++ with ^
void* replace_plus(void *args);

// Writes 80 character long lines to stdout
void* output_stuff(void *args);

// Set up threads and run the line processor
void run_line_processor();

#endif

