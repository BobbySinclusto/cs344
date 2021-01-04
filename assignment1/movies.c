#include "movies.h"
#include "linked_list.h"
#include <stdlib.h>

struct linked_list* load_from_csv(char* csv_file) {
    struct linked_list* movies = linked_list_init();

    // test data, remove when this is done
    struct movie* tmp = malloc(sizeof(struct movie));
    
    linked_list_append(movies, tmp);

    return movies;
}

void free_movies(struct linked_list* movies) {
    struct node* current = movies->head;
    
    while (current != NULL) {
        struct node* tmp = current;
        current = current->next;
        // Free memory allocated for the movie struct
        free(tmp->value);
        // Free memory allocated for the linked list node
        free(tmp);
    }

    // Free memory allocated for the linked list
    free(movies);
}