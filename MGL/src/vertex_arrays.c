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
 * vertex_arrays.c
 * MGL
 *
 */

#include <strings.h>

#include "glm_context.h"

VertexArray *newVAO(GLMContext ctx, GLuint vao)
{
    VertexArray *ptr;

    ptr = (VertexArray *)malloc(sizeof(VertexArray));
    assert(ptr);

    bzero((void *)ptr, sizeof(VertexArray));

    ptr->name = vao;

    return ptr;
}

VertexArray *getVAO(GLMContext ctx, GLuint vao)
{
    VertexArray *ptr;

    ptr = (VertexArray *)searchHashTable(&STATE(vao_table), vao);

    if (!ptr)
    {
        ptr = newVAO(ctx, vao);

        insertHashElement(&STATE(vao_table), vao, ptr);
    }

    return ptr;
}

int isVAO(GLMContext ctx, GLuint vao)
{
    VertexArray *ptr;

    ptr = (VertexArray *)searchHashTable(&STATE(vao_table), vao);

    if (ptr)
        return 1;

    return 0;
}

void mglGenVertexArrays(GLMContext ctx, GLsizei n, GLuint *arrays)
{
    while(n--)
    {
        *arrays++ = getNewName(&STATE(vao_table));
    }
}

void mglBindVertexArray(GLMContext ctx, GLuint array)
{
    VertexArray *ptr;

    if (array)
    {
        ptr = getVAO(ctx, array);
        (assert(ptr));
    }
    else
    {
        ptr = 0;
    }

    STATE(vao) = ptr;
    STATE(dirty_bits) |= DIRTY_VAO;
}

void mglDeleteVertexArrays(GLMContext ctx, GLsizei n, const GLuint *arrays)
{
    GLuint vao;

    while(n--)
    {
        vao = *arrays++;

        if (isVAO(ctx, vao))
        {
            VertexArray *ptr;

            ptr = (VertexArray *)searchHashTable(&STATE(vao_table), vao);

            if (ptr)
            {
                // remove current VAO if bound
                if (ptr == STATE(vao))
                {
                    mglBindVertexArray(ctx, 0);
                }

                // delete any mtl_data
            }

            deleteHashElement(&STATE(vao_table), vao);
        }
    }
}


GLboolean mglIsVertexArray(GLMContext ctx, GLuint array)
{
    if (isVAO(ctx, array))
    {
        return GL_TRUE;
    }

    return GL_FALSE;
}
