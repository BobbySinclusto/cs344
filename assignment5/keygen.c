#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    // Make sure the right number of command-line arguments were provided
    if (argc != 2) {
        fprintf(stderr, "Usage: keygen keylength\n");
        return 1;
    }
    
    // Seed random generator
    srand(time(NULL));

    // Print a string of random characters for a string of the given length
    for (int i = 0; i < atoi(argv[1]); ++i) {
        char c = rand() % 27;
        printf("%c", c == 26 ? ' ' : c + 'A');
    }
    printf("\n");
    return 0;
}