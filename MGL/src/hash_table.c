/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * hash_table.c
 * MGL
 *
 */

#include <stdlib.h>
#include <strings.h>
#include <assert.h>

#include "hash_table.h"

#include "unimpl.h"

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
    UNIMPL;
}
