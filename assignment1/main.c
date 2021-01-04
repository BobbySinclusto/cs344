#include <stdio.h>
#include "movies.h"
#include "linked_list.h"

int main(int argc, char** argv) {
    // Check for correct number of arguments
    if (argc != 2) {
        printf("Invalid number of arguments. Please use ./movies [filename] to run this program.\n");
        return 1;
    }

    // Load movies from the file
    struct linked_list* movies = load_from_csv(argv[1]);

    // Check that there were some movies in the file
    if (movies->length == 0) {
        printf("No movies could be found or the file doesn't exist. Please try again.\n");
        return 1;
    }

    // Print out initialization text
    printf("Processed file %s and parsed data for %d movie%s.\n", argv[1], movies->length, movies->length == 1 ? "": "s");

    // Free memory allocated for all movies
    free_movies(movies);

    return 0;
}