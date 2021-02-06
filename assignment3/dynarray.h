#ifndef DYNARRAY_H
#define DYNARRAY_H

// struct to hold dynarray data
struct dynarray {
    int size;
    int capacity;
    void ** arr;
};

// Initialize the dynarray
struct dynarray* dynarray_init();

// Append an element to the array
void dynarray_append(struct dynarray* arr, void *val);

// Remove an element from the array
void dynarray_remove(struct dynarray* arr, int idx);

// Free the dynamic array
void dynarray_free(struct dynarray* arr);

// Gets an element from the dynarray at a specified index
void* dynarray_get(struct dynarray* arr, int idx);

#endif