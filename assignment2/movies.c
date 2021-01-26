/***************************************************************************
** File: movies.c
** Author: Allen Benjamin
** Date: 01/25/2021
** Description: Functions for working with the movie data
**************************************************************************/

#include "movies.h"
#include "linked_list.h"
#include "input_handling.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <limits.h>
#include <fcntl.h>

struct linked_list* load_from_csv(FILE *movies_file) {
    struct linked_list *movies = linked_list_init();

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

void run_movies_menu() {
    while (1) {
        printf("1. Select file to process\n"
            "2. Exit the program\n"
        );
        
        // get choice from user (1-2)
        int choice = get_input_range(1, 2, "Enter a choice 1 or 2: ", "You entered an incorrect choice. Try again: ");
        
        switch (choice) {
            case 1:
                // Display next menu
                process_files_menu();
                break;
            case 2:
                return;
        }
    }
}

void process_files_menu() {
    // Once a movie is succesfully chosen, store the movies in a linked list
    struct linked_list *movies = NULL;

    while (movies == NULL) {
        printf("Which file do you want to process?\n"
            "Enter 1 to pick the largest file\n"
            "Enter 2 to pick the smallest file\n"
            "Enter 3 to specify the name of a file\n"
        );
        
        // get choice from user (1-3)
        int choice = get_input_range(1, 3, "Enter a choice 1 to 3: ", "You entered an incorrect choice. Try again: ");
        
        switch (choice) {
            case 1:
                // Pick largest file
                movies = load_largest_file();
                // If the directory is empty, stop displaying this menu
                if (movies == NULL) {
                    free_movies(movies);
                    return;
                }
                break;
            case 2:
                // Pick smallest file
                movies = load_smallest_file();
                // If the directory is empty, stop displaying this menu
                if (movies == NULL) {
                    free_movies(movies);
                    return;
                }
                break;
            case 3:
                // Ask user for name of file
                movies = load_user_file();
        }
    }

    // Write output to files
    write_output_files(movies);

    // Free memory
    free_movies(movies);
}

int check_name(char *name) {
    regex_t regexp;
    int res = regcomp(&regexp, "movies_.*\\.csv", REG_EXTENDED);
    if (res != 0) {
        printf("regex failed to compile :-(");
        return 0;
    }
    res = regexec(&regexp, name, 0, NULL, 0);
    regfree(&regexp);

    return res == 0;
}

struct linked_list* load_largest_file() {
    // Open the current directory
    DIR *pwd = opendir(".");
    if (pwd == NULL) {
        perror("Error opening current directory.\n");
        return NULL;
    }

    // Keep track of file with maximum size
    int max_size = 0;
    char *csv_file = NULL;

    // Read stuff from current directory
    struct dirent *contents = NULL;
    contents = readdir(pwd);
    struct stat file_info;
    
    while (contents != NULL) {
        // Make sure this is a csv file
        if (check_name(contents->d_name)) {
            // Update max if necessary
            if (stat(contents->d_name, &file_info) != -1) {
                if ((int)file_info.st_size > max_size) {
                    max_size = file_info.st_size;
                    csv_file = contents->d_name;
                }
            }
        }
        contents = readdir(pwd);
    }

    // Check to see if a file was found
    if (csv_file == NULL) {
        printf("No valid files could be found in the current directory.\n");
        closedir(pwd);
        return NULL;
    }
    printf("Now processing the chosen file named %s\n", csv_file);

    FILE *movies_file = fopen(csv_file, "r");

    // Make sure the file opened successfully
    if (movies_file == NULL) {
        printf("Error opening file %s. Try again\n\n", csv_file);
        closedir(pwd);
        return NULL;
    }

    closedir(pwd);
    return load_from_csv(movies_file);
}

struct linked_list* load_smallest_file() {
    // Open the current directory
    DIR *pwd = opendir(".");
    if (pwd == NULL) {
        perror("Error opening current directory.\n");
        return NULL;
    }

    // Keep track of file with maximum size
    int min_size = INT_MAX;
    char *csv_file = NULL;

    // Read stuff from current directory
    struct dirent *contents = NULL;
    contents = readdir(pwd);
    struct stat file_info;
    
    while (contents != NULL) {
        // Make sure this is a csv file
        if (check_name(contents->d_name)) {
            // Update max if necessary
            if (stat(contents->d_name, &file_info) != -1) {
                if ((int)file_info.st_size < min_size) {
                    min_size = file_info.st_size;
                    csv_file = contents->d_name;
                }
            }
        }
        contents = readdir(pwd);
    }

    // Check to see if a file was found
    if (csv_file == NULL) {
        printf("No valid files could be found in the current directory.\n");
        closedir(pwd);
        return NULL;
    }
    printf("Now processing the chosen file named %s\n", csv_file);

    FILE *movies_file = fopen(csv_file, "r");

    // Make sure the file opened successfully
    if (movies_file == NULL) {
        printf("Error opening file %s. Try again\n\n", csv_file);
        closedir(pwd);
        return NULL;
    }

    closedir(pwd);
    return load_from_csv(movies_file);
}

struct linked_list* load_user_file() {
    char *csv_file = NULL;
    size_t n = 0;

    // Get user input
    printf("Enter the complete file name: ");
    getline(&csv_file, &n, stdin);
    // Remove newline character
    csv_file[strcspn(csv_file, "\n")] = '\x00';

    FILE *movies_file = fopen(csv_file, "r");

    // Make sure the file opened successfully
    if (movies_file == NULL) {
        printf("The file %s was not found. Try again\n\n", csv_file);
        free(csv_file);
        return NULL;
    }

    printf("Now processing the chosen file named %s\n", csv_file);

    free(csv_file);
    return load_from_csv(movies_file);
}

void write_output_files(struct linked_list *movies) {
    // Generate random dirname
    // length of string: 8+1+6+1+5+1 = 22
    char dirname[22];
    struct stat dir_info;

    // Make sure directory doesn't already exist
    do {
        snprintf(dirname, 22, "benjamal.movies.%.5d", rand() % 100000);
    } while (stat(dirname, &dir_info) != -1);

    // Create new directory
    mkdir(dirname, 0750);
    printf("Created directory with name %s\n", dirname);

    // Add files to new directory
    // Sort by release date to make this easier
    movies->head = merge_sort(movies->head, movies->length);

    int current_year = 0;
    struct node *current = movies->head;
    FILE *current_file = NULL;

    while (1) {
        // If we're at the end of the list or we're transitioning to a new year, switch to a new file
        if (current == NULL || ((struct movie*)current->value)->year != current_year) {
            if (current == NULL) {
                break;
            }
            // Start a new file
            if (current_file != NULL) {
                fclose(current_file);
            }
            // Directory is 22 characters, +1 for /, +4 for year, +3 for .txt, +1 for null terminator.
            char filepath[31];
            snprintf(filepath, 31, "%s/%.4d.txt", dirname, ((struct movie*)current->value)->year);
            // Set permissions correctly when opening
            int fd = open(filepath, O_CREAT | O_WRONLY, 0640);
            current_file = fdopen(fd, "w");

            // Update current year
            current_year = ((struct movie*)current->value)->year;
        }
        
        // Write this movie to the file
        fprintf(current_file, "%s\n", ((struct movie*)current->value)->title);
        
        current = current->next;
    }
    if (current_file != NULL) {
        fclose(current_file);
    }
}

void free_movies(struct linked_list *movies) {
    if (movies == NULL) {
        return;
    }
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