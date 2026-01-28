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

//#include <stdlib.h>
//#include <stdio.h>
//#include <strings.h>
//#include <assert.h>
//#include <stdint.h>  // For SIZE_MAX and UINT_MAX
//#include <limits.h>  // For UINT_MAX fallback


#ifdef __APPLE__
#import <Foundation/Foundation.h>
#include <Metal/Metal.h>
#endif

#include "hash_table.h"
#include "glm_context.h"

void initHashTable(HashTable *ptr, GLuint size)
{
    size_t len;

    len = sizeof(HashObj) * size;

    ptr->free_keys = NULL;
    ptr->current_name = 1;
    ptr->size = size;
    ptr->keys = (HashObj *)malloc(len);
    assert(ptr->keys);

    bzero(ptr->keys, len);
}

bool resizeHashTableToFitName(HashTable *table, GLuint name)
{
    // CRITICAL SECURITY FIX: Prevent integer overflow in hash table resizing
    // some calls allow the user to specify a name...
    while(table->size <= name)
    {
        size_t old_size = table->size;

        // CRITICAL: Check for integer overflow before multiplication
        if (old_size > UINT_MAX / 2)
        {
            // SECURITY: Hash table size would overflow, use maximum safe size
            fprintf(stderr, "MGL SECURITY ERROR: Hash table size would overflow, capping at maximum safe size\n");
        
            // We could return an error here, but for now we'll cap at UINT_MAX to prevent overflow
            // This might limit functionality but prevents critical security vulnerability
            if (old_size == UINT_MAX)
            {
                break; // Can't grow any further
            }
            table->size = UINT_MAX;
        }
        else
        {
            table->size = old_size * 2; // Safe multiplication
        }

        // CRITICAL: Check for overflow in size calculation before malloc
        if (table->size > SIZE_MAX / sizeof(HashObj))
        {
            fprintf(stderr, "MGL SECURITY ERROR: Hash table allocation would overflow size_t, preventing\n");
            // Reset to old size to prevent crash
            table->size = old_size;
        
            return false; // Exit function safely
        }

        table->keys = (HashObj *)realloc(table->keys, table->size * sizeof(HashObj));
        if (!table->keys)
        {
            // CRITICAL: Handle allocation failure to prevent crash
            fprintf(stderr, "MGL SECURITY ERROR: Hash table allocation failed\n");
            table->size = old_size; // Restore old size

            return false;
        }

        // Initialize new entries
        for (size_t i = old_size; i < table->size; i++)
        {
            table->keys[i].valid = false;
            table->keys[i].data = NULL;
        }
    }
    
    return true;
}

GLuint getNewName(HashTable *table)
{
    if (table->free_keys)
    {
        HashObj *obj;
        
        obj = table->free_keys;
        
        table->free_keys = table->free_keys->next;
        
        obj->valid = true;
        return obj->name;
    }
        
    GLuint name;
    
    name = table->current_name++;
    
    // we could walk over the edge of the table.. make sure we resize it
    if (table->current_name >= table->size)
    {
        // resize the table to keep allocating names
        if (resizeHashTableToFitName(table, table->current_name) == false)
        {
            return 0;
        }
    }

    table->keys[name].name = name;
    table->keys[name].valid = true;
    
    return name;
}

bool isValidKey(HashTable *table, GLuint name)
{
    if (name >= table->size)
    {
        return false;
    }
    
    return table->keys[name].valid;
}

void *getKeyData(HashTable *table, GLuint name)
{
    assert(table);
    
    if (name >= table->size)
    {
        fprintf(stderr, "MGL: getKeyData - name %u exceeds table size %zu, returning NULL\n", name, table->size);
        return NULL;
    }

    return table->keys[name].data;
}

void insertHashElement(HashTable *table, GLuint name, void *data)
{
    assert(table);

    if (name < table->size)
    {
        assert(table->keys[name].data == NULL);

        table->keys[name].valid = true;
        table->keys[name].data = data;

        return;
    }



    table->keys[name].valid = true;
    table->keys[name].data = data;
}

void deleteHashElement(HashTable *table, GLuint name)
{
    assert(table);

    if (name >= table->size)
    {
        fprintf(stderr, "MGL: deleteHashElement - name %u exceeds table size %zu\n", name, table->size);
        return;
    }

    table->keys[name].valid = false;
    table->keys[name].data = NULL;
    
    // add to free keys list
    table->keys[name].next = table->free_keys;
    table->free_keys = &table->keys[name];
}
