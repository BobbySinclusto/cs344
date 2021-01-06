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

// Displays movies menu and handles user choices
void run_movies_menu(struct linked_list* movies);

// frees all memory allocated for movies
void free_movies(struct linked_list *movies);

// Sorts movies by year released
struct node* merge_sort(struct node *head, int length);

// Show movies released in a specified year
void filter_by_year(struct linked_list *movies, int year);

// Show the highest ranked movie for each year
void show_highest_ranked(struct linked_list *movies);

// Show all movies for a specified language
void filter_by_language(struct linked_list *movies, char language[20]);

#endif