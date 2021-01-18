/***************************************************************************
** File: movies.c
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: Functions for working with the movie data
**************************************************************************/

#include "movies.h"
#include "linked_list.h"
#include "input_handling.h"
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
    size_t n = 0;

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

void run_movies_menu(struct linked_list* movies) {
    // Sort movies by year first
    movies->head = merge_sort(movies->head, movies->length);

    // Store input for language
    char language[20];

    while (1) {
        printf("1. Show movies released in the specified year\n"
            "2. Show highest rated movie for each year\n"
            "3. Show the title and year of release of all movies in a specific language\n"
            "4. Exit from the program\n\n"
            "Enter a choice from 1 to 4: ");
        
        // get choice from user (1-4)
        int choice = get_input_range(1, 4, "", "You entered an incorrect choice. Try again: ");
        
        switch (choice) {
            case 1:
                // get year from user, must be between 1900 and 2021 (inclusive)
                filter_by_year(movies, get_input_range(1900, 2021, "Enter the year for which you want to see movies: ", "You entered an invalid year. Try again: "));
                break;
            case 2:
                show_highest_ranked(movies);
                break;
            case 3:
                // get language from user
                printf("Enter the language for which you want to see movies: ");
                fgets(language, 20, stdin);
                // Remove possible newline character (https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input)
                language[strcspn(language, "\n")] = '\x00';
                filter_by_language(movies, language);
                break;
            case 4:
                return;
        }
    }
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

void filter_by_year(struct linked_list *movies, int year) {
    char found = 0;
    struct node *current = movies->head;
    while (current != NULL) {
        if (((struct movie*)current->value)->year == year) {
            printf("%s\n", ((struct movie*)current->value)->title);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No data about movies released in the year %d\n", year);
    }
    printf("\n");
}

void show_highest_ranked(struct linked_list *movies) {
    int current_max = 0;
    int current_year = 0;
    struct movie *current_max_movie = NULL;

    struct node *current = movies->head;
    while (1) {
        // If we're at the end of the list or we're transitioning to a new year, print the max
        if (current == NULL || ((struct movie*)current->value)->year != current_year) {
            // Make sure this isn't the first movie in the list before printing information about max for the current year
            if (current_max_movie != NULL) {
                printf("%d %0.1f %s\n", current_max_movie->year, current_max_movie->rating, current_max_movie->title);
            }
            if (current == NULL) {
                break;
            }
            // Update things for the next year
            current_max = ((struct movie*)current->value)->rating;
            current_year = ((struct movie*)current->value)->year;
            current_max_movie = (struct movie*)current->value;
        }
        else if (((struct movie*)current->value)->rating > current_max) {
            current_max = ((struct movie*)current->value)->rating;
            current_year = ((struct movie*)current->value)->year;
            current_max_movie = (struct movie*)current->value;
        }
        current = current->next;
    }
    printf("\n");
}

void filter_by_language(struct linked_list *movies, char language[20]) {
    struct node *current = movies->head;
    char found = 0;
    while (current != NULL) {
        // Loop through every language
        for (int i = 0; i < 5 && ((struct movie*)current->value)->languages[i][0] != '\x00'; ++i) {
            if (strcmp(language, ((struct movie*)current->value)->languages[i]) == 0) {
                printf("%d %s\n", ((struct movie*)current->value)->year, ((struct movie*)current->value)->title);
                found = 1;
            }
        }
        current = current->next;
    }
    if (!found) {
        printf("No data about movies released in %s\n", language);
    }
    printf("\n");
}

struct node* merge(struct node *first, struct node *second) {
    struct node *new_head = NULL;
    struct node *current = NULL;
    while (first != NULL || second != NULL) {
        struct node *next = NULL;
        // Determine if first or second should be the next element
        if (first != NULL && second != NULL && ((struct movie*)first->value)->year <= ((struct movie*)second->value)->year) {
            next = first;
            first = first->next;
        }
        else if (first != NULL && second != NULL && ((struct movie*)second->value)->year < ((struct movie*)first->value)->year) {
            next = second;
            second = second->next;
        }
        else if (first == NULL) {
            next = second;
            second = second->next;
        }
        else {
            next = first;
            first = first->next;
        }

        if (current == NULL) {
            new_head = next;
        }
        else {
            current->next = next;
        }
        current = next;
    }
    // set the last element of the list to point to NULL
    current->next = NULL;

    return new_head;
}

struct node* merge_sort(struct node *head, int length) {
    // If list contains one element or less it is sorted already
    if (length <= 1) {
        return head;
    }
    
    // Find middle element
    struct node *right = head;
    for (int i = 0; i < length / 2; ++i) {
        if (i == length / 2 - 1) {
            struct node *tmp = right->next;
            right->next = NULL;
            right = tmp;
        }
        else {
            right = right->next;
        }
    }

    return merge(merge_sort(head, length / 2), merge_sort(right, (length + 1) / 2));
}