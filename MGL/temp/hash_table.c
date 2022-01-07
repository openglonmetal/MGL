//
//  hash_table.c
//
//  Created by Michael Larson on 10/27/21.
//

#include <stdlib.h>
#include <strings.h>
#include <assert.h>

#include "hash_table.h"

void initHashTable(HashTable *ptr, GLuint size)
{
    size_t len;

    len = sizeof(HashObj) * size;

    ptr->current_name = 1;
    ptr->size = size;
    ptr->keys = (HashObj *)malloc(len);
    assert(ptr->keys);

    bzero(ptr->keys, len);
}

GLuint getNewName(HashTable *table)
{
    return table->current_name++;
}

void *searchHashTable(HashTable *table, GLuint name)
{
    assert(table);
    assert (name < table->size);

    return table->keys[name].data;
}

void insertHashElement(HashTable *table, GLuint name, void *data)
{
    assert(table);

    if (name < table->size)
    {
        assert(table->keys[name].data == NULL);

        table->keys[name].data = data;

        return;
    }

    // some calls allow the user to specifiy a name...
    while(table->size < name)
    {
        table->size *= 2;
        table->keys = (HashObj *)realloc(table->keys, table->size);
    }

    table->keys[name].data = data;
}

void deleteHashElement(HashTable *table, GLuint name)
{
    assert(table);
    assert (name < table->size);

    table->keys[name].data = NULL;

    // need to have metal delete object
    assert(0);
}
