//
//  vertex_arrays.c
//  mgl
//
//  Created by Michael Larson on 11/3/21.
//

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
