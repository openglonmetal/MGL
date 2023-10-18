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

Buffer *findBuffer(GLMContext ctx, GLuint buffer);

GLsizei typeSize(GLenum type)
{
    switch(type)
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return sizeof(char);

        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return sizeof(short);

        case GL_INT:
        case GL_UNSIGNED_INT:
            return sizeof(int);

        case GL_FLOAT:
            return sizeof(float);

        case GL_DOUBLE:
            return sizeof(float);

        case GL_HALF_FLOAT:
            return sizeof(float) >> 1;

        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            return sizeof(int);

        default:
            return 0;
    }

    return 0;
}

GLsizei genStrideFromTypeSize(GLenum type, GLint size)
{
    return typeSize(type) * size;
}


VertexArray *newVAO(GLMContext ctx, GLuint vao)
{
    VertexArray *ptr;

    ptr = (VertexArray *)malloc(sizeof(VertexArray));
    assert(ptr);

    bzero((void *)ptr, sizeof(VertexArray));

    ptr->name = vao;

    for(int i=0; i<MAX_ATTRIBS; i++)
    {
        ptr->attrib[i].size = 4;
        ptr->attrib[i].type = GL_FLOAT;
        ptr->attrib[i].stride = 0;
        ptr->attrib[i].divisor = 1;
        ptr->attrib[i].relativeoffset = 0;
        ptr->attrib[i].base_plus_relative_offset = 0;
        ptr->attrib[i].buffer_bindingindex = 0;
    }

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

    if (array == 0)
    {
        ptr = STATE(default_vao);
    }
    else
    {
        //ERROR_CHECK_RETURN(isVAO(ctx, array), GL_INVALID_VALUE);

        ptr = getVAO(ctx, array);

        ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);
    }

    if (ptr)
    {
        // bind _ELEMENT_ARRAY_BUFFER if vao element array is null, think this is right
        if (ptr->element_array.buffer == NULL)
        {
            ptr->element_array.buffer = STATE(vao)->buffer_bindings[_ELEMENT_ARRAY_BUFFER].buffer;

            ptr->dirty_bits |= DIRTY_VAO_ATTRIB;
        }
    }
    
    if (STATE(vao) != ptr)
    {
        STATE(vao) = ptr;
        STATE(dirty_bits) |= DIRTY_VAO;
    }
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
    return isVAO(ctx, array);
}

void mglGetVertexAttribdv(GLMContext ctx, GLuint index, GLenum pname, GLdouble *params)
{
    VertexArray *vao;

    vao = ctx->state.vao;

    Buffer *buf;

    buf = ctx->state.vao->attrib[index].buffer;

    switch(pname)
    {
        case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
            if (buf)
            {
                *params = buf->name;
            }
            else
            {
                *params = 0;
            }
            break;

        case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
            if (vao->enabled_attribs & (0x1 << index))
            {
                *params = GL_TRUE;
            }
            else
            {
                *params = GL_FALSE;
            }
            break;

        case GL_VERTEX_ATTRIB_ARRAY_SIZE:
            *params = vao->attrib[index].size;
            break;

        case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
            *params = vao->attrib[index].stride;
            break;

        case GL_VERTEX_ATTRIB_ARRAY_TYPE:
            *params = vao->attrib[index].type;
            break;

        case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
            *params = vao->attrib[index].normalized;
            break;

        case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
            ERROR_RETURN(GL_INVALID_ENUM); // unsupported for now
            *params = 0;
            break;

        case GL_CURRENT_VERTEX_ATTRIB:
            ERROR_RETURN(GL_INVALID_ENUM); // unsupported for now, probably never
            *params = 0;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }
}

void mglGetVertexAttribiv(GLMContext ctx, GLuint index, GLenum pname, GLint *params)
{
    double dparams[4];

    if (pname != GL_CURRENT_VERTEX_ATTRIB)
    {
        ERROR_CHECK_RETURN(ctx->state.vao, GL_INVALID_OPERATION);
    }

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    if (params == NULL)
        return;

    mglGetVertexAttribdv(ctx, index, pname, dparams);

    if (pname == GL_CURRENT_VERTEX_ATTRIB)
    {
        for(int i=0; i<4; i++)
            params[i] = (GLint)dparams[i];
    }
    else
    {
        *params = (GLint)dparams[0];
    }
}

void mglGetVertexAttribfv(GLMContext ctx, GLuint index, GLenum pname, GLfloat *params)
{
    double dparams[4];

    if (pname != GL_CURRENT_VERTEX_ATTRIB)
    {
        ERROR_CHECK_RETURN(ctx->state.vao, GL_INVALID_OPERATION);
    }

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    if (params == NULL)
        return;

    mglGetVertexAttribdv(ctx, index, pname, dparams);

    if (pname == GL_CURRENT_VERTEX_ATTRIB)
    {
        for(int i=0; i<4; i++)
            params[i] = (GLfloat)dparams[i];
    }
    else
    {
        *params = (GLfloat)dparams[0];
    }
}

void setVertexAttrib(GLMContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{

    if (stride == 0)
        stride = genStrideFromTypeSize(type, size);

    ERROR_CHECK_RETURN(stride, GL_INVALID_ENUM);

    VAO_ATTRIB_STATE(index).size = size;
    VAO_ATTRIB_STATE(index).type = type;
    VAO_ATTRIB_STATE(index).normalized = normalized;
    VAO_ATTRIB_STATE(index).stride = stride;
    VAO_ATTRIB_STATE(index).relativeoffset = (GLubyte *)pointer - (GLubyte *)NULL;

    // bind current array buffer to attrib
    VAO_ATTRIB_STATE(index).buffer = STATE(vao)->buffer_bindings[_ARRAY_BUFFER].buffer;
    ERROR_CHECK_RETURN(VAO_ATTRIB_STATE(index).buffer, GL_INVALID_OPERATION);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglVertexAttribPointer(GLMContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(stride >= 0, GL_INVALID_VALUE);

    // GL_INVALID_OPERATION is generated if zero is bound to the GL_ARRAY_BUFFER buffer object binding point and the pointer argument is not NULL.

    if (pointer != NULL)
    {
        Buffer *ptr;

        ptr = STATE(vao)->buffer_bindings[_ARRAY_BUFFER].buffer;

        ERROR_CHECK_RETURN(ptr, GL_INVALID_OPERATION);
    }

    switch(type)
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
        case GL_DOUBLE:
        case GL_FIXED:
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    setVertexAttrib(ctx, index, size, type, normalized, stride, pointer);
}

void mglVertexAttribIPointer(GLMContext ctx, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(stride >= 0, GL_INVALID_VALUE);

    // GL_INVALID_OPERATION is generated if zero is bound to the GL_ARRAY_BUFFER buffer object binding point and the pointer argument is not NULL.

    if (pointer != NULL)
    {
        Buffer *ptr;

        ptr = ctx->state.vao->attrib[index].buffer;

        ERROR_CHECK_RETURN(ptr, GL_INVALID_OPERATION);
    }

    switch(type)
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
        case GL_DOUBLE:
        case GL_FIXED:
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    setVertexAttrib(ctx, index, size, type, 0, stride, pointer);
}


void mglVertexAttribLPointer(GLMContext ctx, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(stride >= 0, GL_INVALID_VALUE);

    // GL_INVALID_OPERATION is generated if zero is bound to the GL_ARRAY_BUFFER buffer object binding point and the pointer argument is not NULL.

    if (pointer != NULL)
    {
        Buffer *ptr;

        ptr = ctx->state.vao->attrib[index].buffer;

        ERROR_CHECK_RETURN(ptr, GL_INVALID_OPERATION);
    }

    switch(type)
    {
        case GL_DOUBLE:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    setVertexAttrib(ctx, index, size, type, 0, stride, pointer);
}

void mglGetVertexAttribPointerv(GLMContext ctx, GLuint index, GLenum pname, void **pointer)
{
    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    switch(pname)
    {
        case GL_VERTEX_ATTRIB_ARRAY_POINTER:
            *pointer = (void **)VAO_ATTRIB_STATE(index).relativeoffset;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
            break;
    }
}

/*
 glVertexAttribPointer, glVertexAttribIPointer and glVertexAttribLPointer
 specify the location and data format of the array of generic vertex attributes
 at index index to use when rendering. size specifies the number of components
 per attribute and must be 1, 2, 3, 4, or GL_BGRA. type specifies the data type
 of each component, and stride specifies the byte stride from one attribute to
 the next, allowing vertices and attributes to be packed into a single array
 or stored in separate arrays.
*/

void mglEnableVertexArrayAttrib(GLMContext ctx, GLuint vaobj, GLuint index)
{
    VertexArray *ptr;

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    ptr->enabled_attribs |= (0x1 << index);

    ptr->dirty_bits |= DIRTY_VAO_ATTRIB;
}

void mglDisableVertexArrayAttrib(GLMContext ctx, GLuint vaobj, GLuint index)
{
    VertexArray *ptr;

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    ptr->enabled_attribs &= ~(0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglEnableVertexAttribArray(GLMContext ctx, GLuint index)
{
    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    STATE(vao)->enabled_attribs |= (0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglDisableVertexAttribArray(GLMContext ctx, GLuint index)
{
    ERROR_CHECK_RETURN(VAO(), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(index < MAX_ATTRIBS, GL_INVALID_VALUE);

    STATE(vao)->enabled_attribs &= ~(0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

/*
 glCreateVertexArrays returns n previously unused vertex array object
names in arrays, each representing a new vertex array object initialized to the default state.
 */
void mglCreateVertexArrays(GLMContext ctx, GLsizei n, GLuint *arrays)
{
    ERROR_CHECK_RETURN(arrays, GL_INVALID_VALUE);

    mglGenVertexArrays(ctx, n, arrays);

    for(int i=0; i<n; i++)
    {
        VertexArray *ptr;

        ptr = getVAO(ctx, arrays[i]);

        assert(ptr);
    }
}

/*
 glVertexArrayElementBuffer binds a buffer object with id buffer to the
 element array buffer bind point of a vertex array object with id vaobj.
 If buffer is zero, any existing element array buffer binding to vaobj is removed.
 */
void mglVertexArrayElementBuffer(GLMContext ctx, GLuint vaobj, GLuint buffer)
{
    VertexArray *ptr;
    Buffer *buf_ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    if (buffer == 0)
    {
        ptr->element_array.buffer = NULL;
        return;
    }

    buf_ptr = findBuffer(ctx, buffer);
    ERROR_CHECK_RETURN(buf_ptr, GL_INVALID_VALUE);

    ptr->element_array.buffer = buf_ptr;

    buf_ptr->data.dirty_bits |= DIRTY_BUFFER;
    ptr->dirty_bits |= DIRTY_FBO_BINDING;
}

void setVertexBindingIndex(GLMContext ctx, VertexArray *vao, GLuint attribindex, GLuint bindingindex)
{
    ERROR_CHECK_RETURN(attribindex < MAX_ATTRIBS, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(bindingindex < MAX_BINDABLE_BUFFERS, GL_INVALID_VALUE);

    vao->attrib[attribindex].buffer_bindingindex = bindingindex;

    vao->dirty_bits |= DIRTY_VAO_ATTRIB | DIRTY_VAO_BUFFER_BASE;
}

void mglVertexAttribBinding(GLMContext ctx, GLuint attribindex, GLuint bindingindex)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setVertexBindingIndex(ctx, ptr, attribindex, bindingindex);
}

void mglVertexArrayAttribBinding(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setVertexBindingIndex(ctx, ptr, attribindex, bindingindex);
}

void setAttribFormat(GLMContext ctx, VertexArray *vao, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    ERROR_CHECK_RETURN(attribindex < MAX_ATTRIBS, GL_INVALID_VALUE);

    switch(type)
    {
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            ERROR_CHECK_RETURN(size == 1, GL_INVALID_VALUE);
            break;

        case GL_BYTE:
        case GL_SHORT:
        case GL_INT:
        case GL_FIXED:
        case GL_FLOAT:
        case GL_HALF_FLOAT:
            ERROR_CHECK_RETURN((size >= 1 && size <=4), GL_INVALID_VALUE);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    vao->attrib[attribindex].size = size;
    vao->attrib[attribindex].type = type;
    vao->attrib[attribindex].normalized = normalized;
    vao->attrib[attribindex].relativeoffset = relativeoffset;

    vao->dirty_bits |= DIRTY_VAO_ATTRIB;
}

void mglVertexAttribFormat(GLMContext ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribFormat(ctx, ptr, attribindex, size, type, normalized, relativeoffset);
}

void mglVertexArrayAttribFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribFormat(ctx, ptr, attribindex, size, type, normalized, relativeoffset);
}

void setAttribIFormat(GLMContext ctx, VertexArray *vao, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    ERROR_CHECK_RETURN(attribindex < MAX_ATTRIBS, GL_INVALID_VALUE);

    switch(type)
    {
        case GL_UNSIGNED_INT:
            ERROR_CHECK_RETURN((size >= 1 && size <=4), GL_INVALID_VALUE);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    vao->attrib[attribindex].size = size;
    vao->attrib[attribindex].type = type;
    vao->attrib[attribindex].normalized = 0;
    vao->attrib[attribindex].relativeoffset = relativeoffset;

    vao->dirty_bits |= DIRTY_VAO_ATTRIB;
}

void mglVertexAttribIFormat(GLMContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribIFormat(ctx, VAO(), attribindex, size, type, relativeoffset);
}

void mglVertexArrayAttribIFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribIFormat(ctx, VAO(), attribindex, size, type, relativeoffset);
}

void setAttribLFormat(GLMContext ctx, VertexArray *vao, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    ERROR_CHECK_RETURN(attribindex < MAX_ATTRIBS, GL_INVALID_VALUE);

    switch(type)
    {
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            ERROR_CHECK_RETURN(size == 1, GL_INVALID_VALUE);
            break;

        case GL_UNSIGNED_INT:
            ERROR_CHECK_RETURN((size >= 1 && size <=4), GL_INVALID_VALUE);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    vao->attrib[attribindex].size = size;
    vao->attrib[attribindex].type = type;
    vao->attrib[attribindex].normalized = 0;
    vao->attrib[attribindex].relativeoffset = relativeoffset;

    vao->dirty_bits |= DIRTY_VAO_ATTRIB;
}

void mglVertexAttribLFormat(GLMContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribLFormat(ctx, ptr, attribindex, size, type, relativeoffset);
}

void mglVertexArrayAttribLFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setAttribLFormat(ctx, ptr, attribindex, size, type, relativeoffset);
}

void mglVertexAttribDivisor(GLMContext ctx, GLuint index, GLuint divisor)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    ptr->attrib[index].divisor = divisor;
}

void setBindingDivisor(GLMContext ctx, VertexArray *vao, GLuint bindingindex, GLuint divisor)
{
    ERROR_CHECK_RETURN(bindingindex < MAX_BINDABLE_BUFFERS, GL_INVALID_VALUE);

    vao->buffer_bindings[bindingindex].divisor = divisor;

    vao->dirty_bits |= DIRTY_VAO_ATTRIB | DIRTY_VAO_BUFFER_BASE;
}

void mglVertexBindingDivisor(GLMContext ctx, GLuint bindingindex, GLuint divisor)
{
    VertexArray *ptr;

    ptr = ctx->state.vao;

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setBindingDivisor(ctx, ptr, bindingindex, divisor);
}

void mglVertexArrayBindingDivisor(GLMContext ctx, GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);

    setBindingDivisor(ctx, ptr, bindingindex, divisor);
}



void mglGetVertexArrayiv(GLMContext ctx, GLuint vaobj, GLenum pname, GLint *param)
{
    // Unimplemented function
    assert(0);
}

void mglGetVertexArrayIndexediv(GLMContext ctx, GLuint vaobj, GLuint index, GLenum pname, GLint *param)
{
    // Unimplemented function
    assert(0);
}

void mglGetVertexArrayIndexed64iv(GLMContext ctx, GLuint vaobj, GLuint index, GLenum pname, GLint64 *param)
{
    // Unimplemented function
    assert(0);
}

