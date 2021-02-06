#ifndef DYNARRAY_H
#define DYNARRAY_H

// struct to hold dynarray data
struct dynarray {
    int size;
    int capacity;
    int *arr;
};

// Initialize the dynarray
struct dynarray* dynarray_init();

// Append an element to the array
void dynarray_append(struct dynarray* arr, int val);

// Remove an element from the array
void dynarray_remove(struct dynarray* arr, int idx);

// Free the dynamic array
void dynarray_free(struct dynarray* arr);

// Gets an element from the dynarray at a specified index
int dynarray_get(struct dynarray* arr, int idx);

#endif