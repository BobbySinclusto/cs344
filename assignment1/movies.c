/***************************************************************************
** File: movies.c
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: Functions for working with the movie data
**************************************************************************/

#include "movies.h"
#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct linked_list* load_from_csv(char *csv_file) {
    struct linked_list *movies = linked_list_init();

    FILE *movies_file = fopen(csv_file, "r");

    // Make sure the file opened successfully
    if (movies_file == NULL) {
        return movies;
    }

    char *line = NULL;
    ssize_t n = 0;

    // Skip the first line
    getline(&line, &n, movies_file);

    // Loop through each line in the file
    while (getline(&line, &n, movies_file) != -1) {
        // saveptr for strtok
        char *saveptr;

        // Allocate memory for a new movie struct
        struct movie *m = malloc(sizeof(struct movie));
        
        // First is the title
        char* title = strtok_r(line, ",", &saveptr);
        // allocate memory for the title in the movie struct
        m->title = malloc(sizeof(char) * strlen(title) + 1);
        strcpy(m->title, title);
        
        // Next is the year
        m->year = atoi(strtok_r(NULL, ",", &saveptr));
        
        // Next is the array of languages...
        char* languages_str = strtok_r(NULL, ",", &saveptr);
        // saveptr for languages
        char* saveptr_l;
        for (int i = 0; i < 5; ++i, languages_str = NULL) {
            char* tmp = strtok_r(languages_str, "[;]", &saveptr_l);
            if (tmp != NULL) {
                strcpy(m->languages[i], tmp);
            }
            else {
                // Fill the rest of the languages array with empty strings
                m->languages[i][0] = '\x00';
            }
        }

        // Finally, the rating value
        m->rating = strtod(strtok_r(NULL, "\n", &saveptr), NULL);
        
        // Add the new movie to the linked list
        linked_list_append(movies, m);
    }

    // free memory allocated by getline for the line buffer
    free(line);

    fclose(movies_file);
    return movies;
}

void free_movies(struct linked_list *movies) {
    struct node *current = movies->head;
    
    while (current != NULL) {
        struct node *tmp = current;
        current = current->next;
        // Free memory allocated for the movie title string
        free(((struct movie*)tmp->value)->title);
        // Free memory allocated for the movie struct
        free(tmp->value);
        // Free memory allocated for the linked list node
        free(tmp);
    }

    // Free memory allocated for the linked list
    free(movies);
}