/***************************************************************************
** File: main.c
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: A simple program to read and interact with information from a file
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "movies.h"
#include "linked_list.h"

int main() {
    // Seed random generator
    srand(time(NULL));

    /*
    // Check for correct number of arguments
    if (argc != 2) {
        printf("Invalid number of arguments.\nExample usage: ./movies movies_sample_1.csv\n");
        return EXIT_FAILURE;
    }

    // Load movies from the file
    struct linked_list* movies = load_from_csv(argv[1]);

    // Check that there were some movies in the file
    if (movies->length == 0) {
        printf("No movies could be found or the file doesn't exist. Please try again.\n");
        free_movies(movies);
        return EXIT_FAILURE;
    }

    // Print out initialization text
    printf("Processed file %s and parsed data for %d movie%s.\n\n", argv[1], movies->length, movies->length == 1 ? "": "s");
    */

    // run the menu
    run_movies_menu();

    return EXIT_SUCCESS;
}