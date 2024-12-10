#include "hashTable.h"

#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>

hashTable * createHashTable(unsigned int capacity, unsigned int max_allowed_size) {
    hashTable * ht = (hashTable *) malloc(sizeof(hashTable));

    if (ht == NULL) {
        perror("Malloc in createHashTable");
        exit(EXIT_FAILURE);
    }

    ht->nodes = (hashNode *) malloc(sizeof(hashNode) * capacity);
    if (ht->nodes == NULL) {
        perror("Malloc in createHashTable");
        exit(EXIT_FAILURE);
    }

    ht->size = 0;
    ht->capacity = capacity;
    ht->max_allowed_size = max_allowed_size;

    for (unsigned int i = 0; i < capacity; i++) {
        ht->nodes[i].key = NULL;
        ht->nodes[i].value = NULL;
    }
}

void insert(hashTable * ht, void * key, void * value) {
    if (ht == NULL  || key == NULL || value == NULL) {
        return;
    }

    if (ht->size >= ht->max_allowed_size) {
        return;
    }

    unsigned int index = (unsigned int) key % ht->capacity;


    while (ht->nodes[index].key != NULL) {
        index = (index + 1) % ht->capacity;
    }

    ht->nodes[index].key = key;
    ht->nodes[index].value = value;
    ht->size++;
}
