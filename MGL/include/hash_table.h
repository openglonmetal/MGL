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
 * hash_table.h
 * MGL
 *
 */

#ifndef hash_table_h
#define hash_table_h

#include "glcorearb.h"

typedef struct HashObj_t {
    struct HashObj_t *next;
    GLuint name;
    GLboolean valid;
    void *data;
} HashObj;

typedef struct {
    size_t size;
    GLuint num_free_names;
    GLuint current_name;
    HashObj *keys;
    HashObj *free_keys;
} HashTable;

HashTable *createHashTable(GLuint size);
void initHashTable(HashTable *ptr, GLuint size);
GLuint getNewName(HashTable *table);

void insertHashElement(HashTable *table, GLuint name, void *data);
void deleteHashElement(HashTable *table, GLuint name);

bool isValidKey(HashTable *table, GLuint name);
void *getKeyData(HashTable *table, GLuint name);

#endif /* hash_table_h */
