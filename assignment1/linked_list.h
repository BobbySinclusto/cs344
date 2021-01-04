/***************************************************************************
** File: linked_list.h
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: Basic linked list for holding movies
**************************************************************************/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// Struct for individual nodes of the linked list
struct node {
    void* value;
    struct node* next;
};

// Struct for linked list information
struct linked_list {
    struct node* head;
    struct node* last;
    int length;
};

// initializes the linked list
struct linked_list* linked_list_init();

// adds a value to the end of the linked list
void linked_list_append(struct linked_list* ll, void* value);

// frees all memory for the linked list
void linked_list_free(struct linked_list*);

#endif