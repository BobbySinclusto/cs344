#include "dynarray.h"
#include "stdlib.h"

// Initialize the dynarray
struct dynarray* dynarray_init(){
    struct dynarray *arr = malloc(sizeof(struct dynarray));
    arr->size = 0;
    arr->capacity = 1;
    arr->arr = malloc(sizeof(void*));
    return arr;
}

// Append an element to the array
void dynarray_append(struct dynarray* arr, void *val){
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->arr = realloc(arr->arr, sizeof(void*) * arr->capacity);
    }
    arr->arr[arr->size] = val;
    arr->size += 1;
}

// Remove an element from the array
void dynarray_remove(struct dynarray* arr, int idx){
    if (idx >= 0 && idx < arr->size) {
        for (int i = idx; i < arr->size - 1; ++i) {
            arr->arr[i] = arr->arr[i+1];
        }
        arr->size -= 1;
    }
}

// Free the dynamic array
void dynarray_free(struct dynarray* arr){
    free(arr->arr);
    free(arr);
}

// Gets an element from the dynarray at a specified index
void* dynarray_get(struct dynarray* arr, int idx){
    return arr->arr[idx];
}