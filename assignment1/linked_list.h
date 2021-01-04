#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct node {
    void* value;
    struct node* next;
};

struct linked_list {
    struct node* head;
    struct node* last;
    int length;
};

struct linked_list* linked_list_init();
void linked_list_append(struct linked_list* ll, void* value);

#endif