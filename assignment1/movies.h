/***************************************************************************
** File: movies.h
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: Functions for working with the movie data
**************************************************************************/

#ifndef MOVIES_H
#define MOVIES_H
#include "linked_list.h"

// struct for storing movie information
struct movie {
    char *title;
    int year;
    char languages[5][20]; // We can assume there will be no more than 5 languages
    double rating;
};

// loads movie information from file and outputs a linked list of movies
struct linked_list* load_from_csv(char *csv_file);

// frees all memory allocated for movies
void free_movies(struct linked_list *movies);

#endif