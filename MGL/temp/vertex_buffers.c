//
//  vertex_buffers.c
//  mgl
//
//  Created by Michael Larson on 11/2/21.
//

#include <assert.h>
#include <strings.h>

#include "glm_context.h"

extern Buffer *getBuffer(GLMContext ctx, GLenum target, GLuint buffer);
extern VertexArray *getVAO(GLMContext ctx, GLuint vao);
extern void mglGenVertexArrays(GLMContext ctx, GLsizei n, GLuint *arrays);


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
            assert(0);
    }

    return 0;
}

GLsizei genStrideFromTypeSize(GLenum type, GLint size)
{
    return typeSize(type) * size;
}

void mglBindVertexBuffer(GLMContext ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    Buffer *buf_ptr;

    assert(ctx->state.vao);

    if (buffer == 0) {
        VAO_ATTRIB_STATE(bindingindex).buffer = NULL;
        return;
    }

    if (buffer)
    {
        buf_ptr = getBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, buffer);
        assert(buf_ptr);
    }
    else
    {
        buf_ptr = NULL;
    }

    assert(bindingindex < ctx->state.max_vertex_attribs);
    VAO_ATTRIB_STATE(bindingindex).buffer = buf_ptr;
    VAO_ATTRIB_STATE(bindingindex).ptr = (void *)offset;
    VAO_ATTRIB_STATE(bindingindex).stride = stride;

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglBindVertexBuffers(GLMContext ctx, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)
{
    Buffer *buf_ptr;

    assert(ctx->state.vao);

    for(int i=0; i<count; i++)
    {
        GLuint buffer;
        GLintptr offset;
        GLsizei stride;

        buffer = buffers[i];

        if (buffer == 0) {
            VAO_ATTRIB_STATE(first + i).buffer = NULL;
            continue;
        }

        offset = offsets[i];
        stride = strides[i];

        if (buffer)
        {
            buf_ptr = getBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, buffer);
            assert(buf_ptr);
        }
        else
        {
            buf_ptr = NULL;
        }

       assert(first + i < ctx->state.max_vertex_attribs);
        VAO_ATTRIB_STATE(first + i).buffer = buf_ptr;
        VAO_ATTRIB_STATE(first + i).ptr = (void *)offset;
        VAO_ATTRIB_STATE(first + i).stride = stride;

        VAO_STATE(dirty_bits) |= DIRTY_VAO;
    }
}

void mglVertexAttribPointer(GLMContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    if (VAO() == NULL)
    {
        assert(0);

        return;
    }

    if (index > ctx->state.max_vertex_attribs)
    {
        assert(0);

        return;
    }

    if (stride == 0)
        stride = genStrideFromTypeSize(type, size);

    VAO_ATTRIB_STATE(index).size = size;
    VAO_ATTRIB_STATE(index).type = type;
    VAO_ATTRIB_STATE(index).normalized = normalized;
    VAO_ATTRIB_STATE(index).stride = stride;
    VAO_ATTRIB_STATE(index).ptr = pointer;

    // bind current array buffer to attrib
    VAO_ATTRIB_STATE(index).buffer = STATE(buffers[_ARRAY_BUFFER]);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglGetVertexAttribPointerv(GLMContext ctx, GLuint index, GLenum pname, void **pointer)
{
    if (VAO() == NULL)
    {
        assert(0);

        return;
    }

    if (index > ctx->state.max_vertex_attribs)
    {
        assert(0);

        return;
    }

    switch(pname) {
        case GL_VERTEX_ATTRIB_ARRAY_POINTER:
            *pointer = (void **)VAO_ATTRIB_STATE(index).ptr;
        default:
            assert(0);
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

    ptr = getVAO(ctx, vaobj);

    assert(ptr);

    ptr->enabled_attribs |= (0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglDisableVertexArrayAttrib(GLMContext ctx, GLuint vaobj, GLuint index)
{
    VertexArray *ptr;

    ptr = getVAO(ctx, vaobj);

    assert(ptr);

    ptr->enabled_attribs &= ~(0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglEnableVertexAttribArray(GLMContext ctx, GLuint index)
{
    assert(STATE(vao));

    assert(index < ctx->state.max_vertex_attribs);

    STATE(vao)->enabled_attribs |= (0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

void mglDisableVertexAttribArray(GLMContext ctx, GLuint index)
{
    assert(STATE(vao));

    assert(index < ctx->state.max_vertex_attribs);

    STATE(vao)->enabled_attribs &= ~(0x1 << index);

    VAO_STATE(dirty_bits) |= DIRTY_VAO;
}

/*
 glCreateVertexArrays returns n previously unused vertex array object
names in arrays, each representing a new vertex array object initialized to the default state.
 */
void mglCreateVertexArrays(GLMContext ctx, GLsizei n, GLuint *arrays)
{
    assert(arrays);

    mglGenVertexArrays(ctx, n, arrays);

    for(int i=0; i<n; i++) {
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
    assert(ptr);

    if (buffer == 0) {
        ptr->element_array.buffer = NULL;
        return;
    }

    buf_ptr = getBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, buffer);
    assert(buf_ptr);

    ptr->element_array.buffer = buf_ptr;
    // what about type and size?

    buf_ptr->data.dirty_bits |= DIRTY_BUFFER;
}

/*
glBindVertexBuffer and glVertexArrayVertexBuffer bind the buffer named buffer
to the vertex buffer binding point whose index is given by bindingindex.
glBindVertexBuffer modifies the binding of the currently bound vertex array
object, whereas glVertexArrayVertexBuffer allows the caller to specify ID of
the vertex array object with an argument named vaobj, for which the binding
should be modified. offset and stride specify the offset of the first element
within the buffer and the distance between elements within the buffer,
respectively, and are both measured in basic machine units. bindingindex
must be less than the value of GL_MAX_VERTEX_ATTRIB_BINDINGS. offset and
stride must be greater than or equal to zero. If buffer is zero, then any
buffer currently bound to the specified binding point is unbound.

If buffer is not the name of an existing buffer object, the GL first creates
a new state vector, initialized with a zero-sized memory buffer and comprising
all the state and with the same initial values as in case of glBindBuffer.
buffer is then attached to the specified bindingindex of the vertex array object.
 */

void mglVertexArrayVertexBuffer(GLMContext ctx, GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    VertexArray *ptr;
    Buffer *buf_ptr;

    ptr = getVAO(ctx, vaobj);
    assert(ptr);

    if (buffer == 0) {
        ptr->attrib[bindingindex].buffer = NULL;
        return;
    }

    buf_ptr = getBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, buffer);
    assert(buf_ptr);

    assert(bindingindex < ctx->state.max_vertex_attribs);
    ptr->attrib[bindingindex].buffer = buf_ptr;
    ptr->attrib[bindingindex].ptr = (void *)offset;
    ptr->attrib[bindingindex].stride = stride;

    ptr->dirty_bits |= DIRTY_VAO;
}

void mglVertexArrayVertexBuffers(GLMContext ctx, GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)
{
    VertexArray *ptr;
    Buffer *buf_ptr;

    ptr = getVAO(ctx, vaobj);
    assert(ptr);

    for(int i=0; i<count; i++)
    {
        GLuint buffer;
        GLintptr offset;
        GLsizei stride;

        buffer = buffers[i];

        if (buffer == 0) {
            assert(first + i < ctx->state.max_vertex_attribs);
            ptr->attrib[first + i].buffer = NULL;
            continue;
        }

        offset = offsets[i];
        stride = strides[i];

        buf_ptr = getBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, buffer);
        assert(buf_ptr);

        assert(first + i < ctx->state.max_vertex_attribs);
        ptr->attrib[first + i].buffer = buf_ptr;
        ptr->attrib[first + i].ptr = (void *)offset;
        ptr->attrib[first + i].stride = stride;
    }

    ptr->dirty_bits |= DIRTY_VAO;
}

void mglVertexArrayAttribBinding(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
    // Unimplemented function
    assert(0);
}

void mglVertexArrayAttribFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    // Unimplemented function
    assert(0);
}

void mglVertexArrayAttribIFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    // Unimplemented function
    assert(0);
}

void mglVertexArrayAttribLFormat(GLMContext ctx, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    // Unimplemented function
    assert(0);
}

void mglVertexArrayBindingDivisor(GLMContext ctx, GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
    // Unimplemented function
    assert(0);
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
