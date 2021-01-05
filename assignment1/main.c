/***************************************************************************
** File: main.c
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: A simple program to read and interact with information from a file
**************************************************************************/

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
        free_movies(movies);
        return 1;
    }

    // Print out initialization text
    printf("Processed file %s and parsed data for %d movie%s.\n", argv[1], movies->length, movies->length == 1 ? "": "s");

    // TODO: add menu thing

    // DEBUG: print out all the movie information in the linked list
    struct node* current = movies->head;
    while (current != NULL) {
        printf("Title: %s\nYear: %d\nLanguages: ", ((struct movie*)current->value)->title, ((struct movie*)current->value)->year);
        for (int i = 0; i < 5 && ((struct movie*)current->value)->languages[i][0] != '\x00'; ++i) {
            printf("%s ", ((struct movie*)current->value)->languages[i]);
        }
        printf("\nRating: %0.1f\n\n", ((struct movie*)current->value)->rating);
        current = current->next;
    }

    // Free memory allocated for all movies
    free_movies(movies);

    return 0;
}