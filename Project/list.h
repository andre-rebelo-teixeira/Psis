#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>

// Node structure for the doubly linked list
typedef struct Node {
    void *data;
    struct Node *prev;
    struct Node *next;
} Node;

// List structure to hold the head and tail of the list
typedef struct List {
    Node *head;
    Node *tail;
} List;

// Create a new list
static inline List *create_list() {
    List *list = (List *)malloc(sizeof(List));
    if (!list) {
        perror("Failed to create list");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

// Insert a new node at the end of the list
static inline void insert_node(List *list, void *data) {
    if (!list) return;

    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("Failed to insert node");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->prev = list->tail;
    new_node->next = NULL;

    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }

    list->tail = new_node;
}

// Remove a specific node from the list
static inline void remove_node(List *list, Node *node, void (*free_data)(void *)) {
    if (!list || !node) return;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    if (free_data) {
        free_data(node->data);
    }
    free(node);
}

// Clean up the entire list
static inline void clean_up_list(List *list, void (*free_data)(void *)) {
    if (!list) return;

    Node *current = list->head;
    while (current) {
        Node *next_node = current->next;
        if (free_data) {
            free_data(current->data);
        }
        free(current);
        current = next_node;
    }

    free(list);
}

#endif // LIST_H
