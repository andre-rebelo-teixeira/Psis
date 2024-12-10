#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <stdlib.h>

typedef struct {
    void *value;
    void *key;
} hashNode;

typedef struct { 
    hashNode *nodes;
    unsigned int size;
    unsigned int max_allowed_size;
    unsigned int capacity;
} hashTable;

hashTable *createHashTable(unsigned int capacity, unsigned int max_allowed_size);
void insert(hashTable *ht, void *key, void *value);
void *search(hashTable *ht, void *key);
void delete(hashTable *ht, void *key);
void freeHashTable(hashTable *ht);


#endif // HASH_TABLE_H