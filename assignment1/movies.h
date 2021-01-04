#ifndef MOVIES_H
#define MOVIES_H
#include "linked_list.h"

struct movie {
    char* title;
    int year;
    char*[5] languages; // We can assume there will be no more than 5 languages
    double rating;
};



#endif