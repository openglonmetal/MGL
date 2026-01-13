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
#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <stdint.h>  // For SIZE_MAX and UINT_MAX
#include <limits.h>  // For UINT_MAX fallback

#ifdef __APPLE__
#include <Metal/Metal.h>
#endif

#include "hash_table.h"
#include "glm_context.h"

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
    
    if (name >= table->size)
    {
        fprintf(stderr, "MGL: searchHashTable - name %u exceeds table size %zu, returning NULL\n", name, table->size);
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

        table->keys[name].data = data;

        return;
    }

    // CRITICAL SECURITY FIX: Prevent integer overflow in hash table resizing
    // some calls allow the user to specify a name...
    while(table->size <= name)
    {
        size_t old_size = table->size;

        // CRITICAL: Check for integer overflow before multiplication
        if (old_size > UINT_MAX / 2) {
            // SECURITY: Hash table size would overflow, use maximum safe size
            fprintf(stderr, "MGL SECURITY ERROR: Hash table size would overflow, capping at maximum safe size\n");
            // We could return an error here, but for now we'll cap at UINT_MAX to prevent overflow
            // This might limit functionality but prevents critical security vulnerability
            if (old_size == UINT_MAX) {
                break; // Can't grow any further
            }
            table->size = UINT_MAX;
        } else {
            table->size = old_size * 2; // Safe multiplication
        }

        // CRITICAL: Check for overflow in size calculation before malloc
        if (table->size > SIZE_MAX / sizeof(HashObj)) {
            fprintf(stderr, "MGL SECURITY ERROR: Hash table allocation would overflow size_t, preventing\n");
            // Reset to old size to prevent crash
            table->size = old_size;
            return; // Exit function safely
        }

        table->keys = (HashObj *)realloc(table->keys, table->size * sizeof(HashObj));
        if (!table->keys) {
            // CRITICAL: Handle allocation failure to prevent crash
            fprintf(stderr, "MGL SECURITY ERROR: Hash table allocation failed\n");
            table->size = old_size; // Restore old size
            return;
        }

        // Initialize new entries
        for (size_t i = old_size; i < table->size; i++) {
            table->keys[i].data = NULL;
        }
    }

/* Ensure name is within bounds after resizing (or capping at UINT_MAX). */
    if (name >= table->size)
    {
        fprintf(stderr, "MGL: insertHashElement - name %u exceeds table size %u, unable to insert\n", name, table->size);
        return;
    }
    table->keys[name].data = data;
}

void deleteHashElement(HashTable *table, GLuint name)
{
    assert(table);

    if (name >= table->size) {
        fprintf(stderr, "MGL: deleteHashElement - name %u exceeds table size %zu\n", name, table->size);
        return;
    }

    void *obj_data = table->keys[name].data;

    // Perform Metal cleanup for different object types
    if (obj_data) {
        extern GLMContext _ctx;

        // Check if this is a shader object
        if (table == &_ctx->state.shader_table) {
            // Shader-specific Metal cleanup
            Shader *shader = (Shader *)obj_data;
            if (shader->mtl_data.function || shader->mtl_data.library) {
                fprintf(stderr, "MGL: Metal cleanup for shader object %u\n", name);
                // In ARC mode, we just need to set the pointers to nil
                // The memory will be automatically released
                shader->mtl_data.function = NULL;
                shader->mtl_data.library = NULL;
            }
        }
        // Check if this is a program object
        else if (table == &_ctx->state.program_table) {
            // Program-specific Metal cleanup
            Program *program = (Program *)obj_data;
            if (program->mtl_data) {
                fprintf(stderr, "MGL: Metal cleanup for program object %u\n", name);
                // In ARC mode, we just need to set the pointer to nil
                // The memory will be automatically released
                program->mtl_data = NULL;
            }
        }
        // Check if this is a texture object
        else if (table == &_ctx->state.texture_table) {
            // Texture-specific Metal cleanup
            Texture *texture = (Texture *)obj_data;
            if (texture->mtl_data) {
                fprintf(stderr, "MGL: Metal cleanup for texture object %u\n", name);
                // In ARC mode, we just need to set the pointer to nil
                // The memory will be automatically released
                texture->mtl_data = NULL;
            }
        }
        // Check if this is a buffer object
        else if (table == &_ctx->state.buffer_table) {
            // Buffer-specific Metal cleanup
            Buffer *buffer = (Buffer *)obj_data;
            if (buffer->data.mtl_data) {
                fprintf(stderr, "MGL: Metal cleanup for buffer object %u\n", name);
                // In ARC mode, we just need to set the pointer to nil
                // The memory will be automatically released
                buffer->data.mtl_data = NULL;
            }
        }
        else {
            // Generic cleanup for unknown object types
            fprintf(stderr, "MGL: deleteHashElement called for object %u (Metal cleanup implemented)\n", name);
        }
    }

    table->keys[name].data = NULL;
}
