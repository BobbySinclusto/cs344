#ifndef MOVIES_H
#define MOVIES_H
#include "linked_list.h"

struct movie {
    char* title;
    int year;
    char languages[5][20]; // We can assume there will be no more than 5 languages
    double rating;
};

struct linked_list* load_from_csv(char* csv_file);
void free_movies(struct linked_list* movies);

#endif