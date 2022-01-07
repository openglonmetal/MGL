//
//  draw_buffers.c
//  mgl
//
//  Created by Michael Larson on 11/7/21.
//

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include "glm_context.h"

bool check_draw_modes(GLenum mode)
{
    switch(mode)
    {
        case GL_POINTS:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
        case GL_LINES:
        case GL_LINE_STRIP_ADJACENCY:
        case GL_LINES_ADJACENCY:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP_ADJACENCY:
        case GL_TRIANGLES_ADJACENCY:
        case GL_PATCHES:
            return true;
    }

    // need to verify against geometry shaders when I get there

    return false;
}

bool validate_vao(GLMContext ctx)
{
    if (VAO() == NULL)
        return false;

    if (VAO_STATE(enabled_attribs) == 0)
        return false;

    for(int i=0; i<ctx->state.max_vertex_attribs; i++)
    {
        if (VAO_STATE(enabled_attribs) & (0x1 << i))
        {
            if (VAO_ATTRIB_STATE(i).buffer->mapped)
                return false;
        }
    }

    return true;
}

bool validate_program(GLMContext ctx)
{
    if (ctx->state.program == NULL)
        return false;

    if (ctx->state.program->shader_slots[_GEOMETRY_SHADER])
    {
        assert(0); // check modes
        return false;
    }

    return true;
}

GLsizei getTypeSize(GLenum type)
{
    switch(type)
    {
        case GL_UNSIGNED_SHORT:
            return sizeof(unsigned short);

        case GL_UNSIGNED_INT:
            return sizeof(unsigned int);
    }

    assert(0);

    return 0;
}

static size_t page_size_align(size_t size)
{
    if (size & (4096-1))
    {
        size_t pad_size = 0;

        pad_size = 4096 - (size & (4096-1));

        size += pad_size;
    }

    return size;
}

bool getTempElementIndicesForContext(GLMContext ctx, GLsizei count, GLenum type, const void *indices)
{
    size_t type_size, buffer_size, buffer_page_size;
    kern_return_t err;
    BufferData *buffer;

    type_size = getTypeSize(type);
    buffer_size = count * type_size;
    buffer_page_size = page_size_align(buffer_size);

    if (ctx->temp_element_buffer)
    {
        buffer = ctx->temp_element_buffer;

        if (buffer->buffer_size > buffer_page_size)
        {
            assert(buffer->buffer_data);

            memcpy((void *)buffer->buffer_data, indices, buffer_size);
            buffer->dirty_bits |= DIRTY_BUFFER_DATA;

            return true;
        }
        else
        {
            if (buffer->mtl_data)
            {
                ctx->mtl_funcs.mtlDeleteMTLBuffer(ctx, buffer->mtl_data);
                buffer->mtl_data = NULL;
            }

            vm_deallocate(mach_host_self(), buffer->buffer_data, buffer->buffer_size);

            buffer->buffer_data = 0;
            buffer->buffer_size = 0;
        }
    }

    ctx->temp_element_buffer = (BufferData *)malloc(sizeof(BufferData));
    bzero(ctx->temp_element_buffer, sizeof(BufferData));

    buffer = ctx->temp_element_buffer;

    // Allocate directly from VM
    err = vm_allocate((vm_map_t) mach_task_self(),
                      (vm_address_t*) &buffer->buffer_data,
                      buffer_page_size,
                      VM_FLAGS_ANYWHERE);
    assert(err == 0);

    buffer->buffer_size = buffer_page_size;
    buffer->dirty_bits |= DIRTY_BUFFER_ADDR;

    memcpy((void *)buffer->buffer_data, indices, buffer_size);

    return true;
}

void mglDrawArrays(GLMContext ctx, GLenum mode, GLint first, GLsizei count)
{
    if (check_draw_modes(mode) == false)
    {
        assert(0);
    }

    if (count < 1)
    {
        assert(0);
    }

    if (validate_vao(ctx) == false)
    {
        assert(0);
    }

    if (validate_program(ctx) == false)
    {
        assert(0);
    }

    ctx->mtl_funcs.mtlDrawArrays(ctx, mode, first, count);
}

void mglDrawElements(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    if (check_draw_modes(mode) == false)
    {
        assert(0);
    }

    if (count < 1)
    {
        assert(0);
    }

    if (validate_vao(ctx) == false)
    {
        assert(0);
    }

    if (validate_program(ctx) == false)
    {
        assert(0);
    }

    if (VAO_STATE(element_array.buffer) == NULL)
    {
        // no element array we need to define a temp

    }

    if ((VAO_STATE(element_array.buffer)== NULL) &&
        (STATE(buffers[_ELEMENT_ARRAY_BUFFER]) == NULL))
    {
        bool err;

        err = getTempElementIndicesForContext(ctx, count, type, indices);

        assert(err);
    }

    ctx->mtl_funcs.mtlDrawElements(ctx, mode, count, type, indices);
}

void mglDrawRangeElements(GLMContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    if (check_draw_modes(mode) == false)
    {
        assert(0);
    }

    if (start > end)
    {
        assert(0);
    }

    if ((end - start) >= count)
    {
        assert(0);
    }

    if (count < 1)
    {
        assert(0);
    }

    if (validate_vao(ctx) == false)
    {
        assert(0);
    }

    if (validate_program(ctx) == false)
    {
        assert(0);
    }

    if (VAO_STATE(element_array.buffer) == NULL)
    {
        // no element array we need to define a temp
    }

    if ((VAO_STATE(element_array.buffer) == NULL) &&
        (STATE(buffers[_ELEMENT_ARRAY_BUFFER]) == NULL))
    {
        bool err;

        err = getTempElementIndicesForContext(ctx, count, type, indices);

        assert(err);
    }

    ctx->mtl_funcs.mtlDrawRangeElements(ctx, mode, start, end, count, type, indices);
}


void mglMultiDrawArrays(GLMContext ctx, GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    // Unimplemented function
    assert(0);
}

void mglMultiDrawElements(GLMContext ctx, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount)
{
    // Unimplemented function
    assert(0);
}

void mglDrawArraysInstanced(GLMContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    assert(check_draw_modes(mode));
    assert(first >= 0);
    assert(count);
    assert(instancecount);

    if (validate_vao(ctx) == false)
    {
        assert(0);
    }

    if (validate_program(ctx) == false)
    {
        assert(0);
    }

    ctx->mtl_funcs.mtlDrawArraysInstanced(ctx, mode, first, count, instancecount);
}

void mglDrawElementsInstanced(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
    // Unimplemented function
    assert(0);
}

void mglDrawElementsBaseVertex(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    // Unimplemented function
    assert(0);
}

void mglDrawRangeElementsBaseVertex(GLMContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    // Unimplemented function
    assert(0);
}

void mglDrawElementsInstancedBaseVertex(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    // Unimplemented function
    assert(0);
}

void mglMultiDrawElementsBaseVertex(GLMContext ctx, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    // Unimplemented function
    assert(0);
}

void mglDrawArraysIndirect(GLMContext ctx, GLenum mode, const void *indirect)
{
    // Unimplemented function
    assert(0);
}

void mglDrawElementsIndirect(GLMContext ctx, GLenum mode, GLenum type, const void *indirect)
{
    // Unimplemented function
    assert(0);
}

void mglDrawArraysInstancedBaseInstance(GLMContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    // Unimplemented function
    assert(0);
}

void mglDrawElementsInstancedBaseInstance(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)
{
    // Unimplemented function
    assert(0);
}

void mglDrawElementsInstancedBaseVertexBaseInstance(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    // Unimplemented function
    assert(0);
}

void mglMultiDrawArraysIndirect(GLMContext ctx, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    // Unimplemented function
    assert(0);
}

void mglMultiDrawElementsIndirect(GLMContext ctx, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    // Unimplemented function
    assert(0);
}

