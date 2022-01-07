//
//  hash_table.h
//
//  Created by Michael Larson on 10/27/21.
//

#ifndef hash_table_h
#define hash_table_h

#include <glcorearb.h>

typedef struct {
    GLuint name;
    void *data;
} HashObj;

typedef struct {
    unsigned size;
    GLuint current_name;
    HashObj *keys;
} HashTable;

HashTable *createHashTable(GLuint size);
void initHashTable(HashTable *ptr, GLuint size);
GLuint getNewName(HashTable *table);
void insertHashElement(HashTable *table, GLuint name, void *data);
void *searchHashTable(HashTable *table, GLuint name);
void deleteHashElement(HashTable *table, GLuint name);

#endif /* hash_table_h */
