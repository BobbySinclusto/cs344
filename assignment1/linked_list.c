#include "linked_list.h"

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
        ll->last = new;
    }
    else {
        ll->last->next = new;
    }
    ll->size += 1;
}

void linked_list_free(struct linked_list* ll) {
    struct node* current = ll->head;
    
    while (current != NULL) {
        struct node* tmp = current;
        current = current->next;
        free(tmp);
    }

    free(ll);
}