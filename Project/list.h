#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>

/*
    *
    * This file contains the implementation of an abstract doubly linked list
    *
    * The main reason to use a double linked list is the easier removal of node in the middle of the list, without the need of either iterating two time over the list, or using a an extra pointer variable to keep track of the previous node
    *
    * The list is implemented as a structure with pointer to both the tail and the head, but currently we can only add to the tail of the list, for future implementation a function to add data in the head of the list is important
*/

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
    unsigned int size;
} List;

/**
 * @brief Create a list object
 * 
 * @return List* 
 */
static inline List *create_list() {
    List *list = (List *)malloc(sizeof(List));
    if (!list) {
        perror("Failed to create list");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

/**
 * @brief Insert a node in the tail of the list
 * 
 * @param list Pointer to the list
 * @param data Pointer to the data to be stored in the list
 */
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
    list->size++;
}

/**
 * @brief This function is responsible for the removal of a single node from the list
 * 
 * @param list Pointer to the list
 * @param node Pointer to the node to be removed
 * @param free_data Function pointer to a function capable of cleaning all the dynamically allocated memory in the data* of the list
 */
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
    list->size = list->size > 0 ? list->size - 1 : 0;
}

/**
 * @brief Remove all the heap allocated memory from the list
 * @brief Be careful with this function, this will clear all the list, and make the list pointer a NULL
 * 
 * @param list Pointer to the list
 * @param free_data function point to a function capable of cleaning all the dynamically allocated memory in the data* of the list
 */
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
