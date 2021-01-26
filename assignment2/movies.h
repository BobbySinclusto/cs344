/***************************************************************************
** File: movies.h
** Author: Allen Benjamin
** Date: 01/25/2021
** Description: Functions for working with the movie data
**************************************************************************/

#ifndef MOVIES_H
#define MOVIES_H
#include "linked_list.h"
#include "stdio.h"

// struct for storing movie information
struct movie {
    char *title;
    int year;
    char languages[5][20]; // We can assume there will be no more than 5 languages
    double rating;
};

// loads movie information from file and outputs a linked list of movies
struct linked_list* load_from_csv(FILE *movie_file);

// Displays movies menu and handles user choices
void run_movies_menu();

// Displays file processing menu and handles user choices
void process_files_menu();

// frees all memory allocated for movies
void free_movies(struct linked_list *movies);

// Sorts movies by year released
struct node* merge_sort(struct node *head, int length);

// Loads movies from largest csv file in directory, NULL if none exist
struct linked_list* load_largest_file();

// Loads movies from smallest csv file in directory, NULL if none exist
struct linked_list* load_smallest_file();

// Loads movies from user-specified file, NULL if doesn't exist
struct linked_list* load_user_file();

// Write output to files
void write_output_files(struct linked_list *movies);

#endif