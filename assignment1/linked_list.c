/***************************************************************************
** File: linked_list.c
** Author: Allen Benjamin
** Date: 01/04/2021
** Description: Basic linked list for holding movies
**************************************************************************/

#include "linked_list.h"
#include <stdlib.h>

struct linked_list* linked_list_init() {
    struct linked_list* ll = malloc(sizeof(struct linked_list));
    ll->head = NULL;
    ll->last = NULL;
    ll->length = 0;
}

void linked_list_append(struct linked_list* ll, void* value) {
    struct node* new = malloc(sizeof(struct node));
    new->value = value;
    new->next = NULL;
    
    if (ll->last == NULL) {
        ll->head = new;
    }
    else {
        ll->last->next = new;
    }

    ll->last = new;
    ll->length += 1;
}