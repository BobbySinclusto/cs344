/***************************************************************************
** File: input_handling.c
** Author: Allen Benjamin
** Date: 01/05/2021
** Description: Utility functions for handling user input
**************************************************************************/

#include "input_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int get_input_range(int lower, int upper, const char *prompt, const char *error) {
    printf("%s", prompt);
    char input[11];
    char *endptr = NULL;
    errno = 0;

    fgets(input, 11, stdin);

    // Just in case strtol fails, make sure number is out of range initially
    int result = lower - 1;
    result = (int)strtol(input, &endptr, 10);

    while (errno != 0 || *endptr != '\n' || result < lower || result > upper) {
        // Reset errno and endptr
        errno = 0;
        endptr = NULL;
        // Display error
        printf("%s", error);
        // Get number from user
        fgets(input, 11, stdin);
        // Convert to integer:
        result = (int)strtol(input, &endptr, 10);
    }

    return result;
}