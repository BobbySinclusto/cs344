/***************************************************************************
** File: main.c
** Author: Allen Benjamin
** Date: 01/25/2021
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

    // run the menu
    run_movies_menu();

    return EXIT_SUCCESS;
}