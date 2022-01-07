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
 * draw_buffers.c
 * MGL
 *
 */

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
        assert(0);
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
        assert(0);
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
        assert(0);
    }

    ctx->mtl_funcs.mtlDrawElementsInstanced(ctx, mode, count, type, indices, instancecount);
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

