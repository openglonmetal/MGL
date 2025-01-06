/*
 * Copyright (C) Michael Larson on on 1/6/25.
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
 * uniforms.c
 * MGL
 *
 */

#include <stdio.h>
#include <string.h>
#include "spirv_cross_c.h"

#include "shaders.h"
#include "programs.h"
#include "buffers.h"
#include "glm_context.h"


#pragma mark uniforms

GLint  mglGetUniformLocation(GLMContext ctx, GLuint program, const GLchar *name)
{
    if (isProgram(ctx, program) == GL_FALSE)
    {
        ERROR_RETURN(GL_INVALID_OPERATION); // also may be GL_INVALID_VALUE ????

        return -1;
    }

    Program *ptr;

    ptr = getProgram(ctx, program);
    assert(program);

    if (ptr->linked_glsl_program == NULL)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);

        return -1;
    }

    for (int stage=_VERTEX_SHADER; stage<_MAX_SHADER_TYPES; stage++)
    {
        int count;

        count = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT].count;

        for (int i=0; i<count; i++)
        {
            const char *str = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT].list[i].name;

            if (!strcmp(str, name))
            {
                GLuint binding;

                binding = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT].list[i].binding;

                return binding;
            }
        }
    }
    
    return -1;
}

void mglGetUniformfv(GLMContext ctx, GLuint program, GLint location, GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetUniformiv(GLMContext ctx, GLuint program, GLint location, GLint *params)
{
    // Unimplemented function
    assert(0);
}


void mglGetUniformIndices(GLMContext ctx, GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)
{
    // Unimplemented function
    assert(0);
}

void mglGetActiveUniformsiv(GLMContext ctx, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetActiveUniformName(GLMContext ctx, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    // Unimplemented function
    assert(0);
}

GLuint  mglGetUniformBlockIndex(GLMContext ctx, GLuint program, const GLchar *uniformBlockName)
{
    if (isProgram(ctx, program) == GL_FALSE)
    {
        assert(0);

        return 0;
    }

    Program *ptr;

    ptr = getProgram(ctx, program);
    assert(program);

    if (ptr->linked_glsl_program == NULL)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);

        return -1;
    }

    for (int stage=_VERTEX_SHADER; stage<_MAX_SHADER_TYPES; stage++)
    {
        int count;

        count = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_BUFFER].count;

        for (int i=0; i<count; i++)
        {
            const char *str = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_BUFFER].list[i].name;

            if (!strcmp(str, uniformBlockName))
            {
                GLuint binding;

                binding = ptr->spirv_resources_list[stage][SPVC_RESOURCE_TYPE_UNIFORM_BUFFER].list[i].binding;

                return binding;
            }
        }
    }

    assert(0);

    return 0xFFFFFFFF;
}

void mglGetActiveUniformBlockiv(GLMContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetActiveUniformBlockName(GLMContext ctx, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
    // Unimplemented function
    assert(0);
}

void mglUniformBlockBinding(GLMContext ctx, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    // Unimplemented function
    assert(0);
}

bool checkUniformParams(GLMContext ctx, GLint location)
{
    Program* ptr = ctx->state.program;
    
    ERROR_CHECK_RETURN_VALUE(ptr, GL_INVALID_OPERATION, false)

    ERROR_CHECK_RETURN_VALUE(location >= 0, GL_INVALID_OPERATION, false)
        
    ERROR_CHECK_RETURN_VALUE(location < MAX_BINDABLE_BUFFERS, GL_INVALID_OPERATION, false)

    return true;
}

void mglUniform(GLMContext ctx, GLint location, void *ptr, GLsizei size)
{
    assert(checkUniformParams(ctx, location));
    
    Buffer *buf = ctx->state.buffer_base[_UNIFORM_CONSTANT].buffers[location].buf;
    
    if(buf == NULL)
    {
        ctx->state.buffer_base[_UNIFORM_CONSTANT].buffers[location].buf = newBuffer(ctx, GL_UNIFORM_BUFFER, location);
        buf = ctx->state.buffer_base[_UNIFORM_CONSTANT].buffers[location].buf;
    }
    
    initBufferData(ctx, buf, size, ptr, true);
}

void mglUniform1d(GLMContext ctx, GLint location, GLdouble x)
{
    mglUniform(ctx, location, &x, sizeof(GLdouble));
}

void mglUniform1dv(GLMContext ctx, GLint location, GLsizei count, const GLdouble *value)
{
    mglUniform(ctx, location, (void *)value, count * sizeof(GLdouble));
}

void mglUniform1f(GLMContext ctx, GLint location, GLfloat v0)
{
    mglUniform(ctx, location, &v0, sizeof(GLfloat));
}

void mglUniform1fv(GLMContext ctx, GLint location, GLsizei count, const GLfloat *value)
{
    mglUniform(ctx, location, (void *)value, count * sizeof(GLfloat));
}

void mglUniform1i(GLMContext ctx, GLint location, GLint v0)
{
    mglUniform(ctx, location, &v0, sizeof(GLint));
}

void mglUniform1iv(GLMContext ctx, GLint location, GLsizei count, const GLint *value)
{
    mglUniform(ctx, location, (void *)value, count * sizeof(GLint));
}

void mglUniform1ui(GLMContext ctx, GLint location, GLuint v0)
{
    mglUniform(ctx, location, &v0, sizeof(GLuint));
}

void mglUniform1uiv(GLMContext ctx, GLint location, GLsizei count, const GLuint *value)
{
    mglUniform(ctx, location, (void *)value, count * sizeof(GLuint));
}

void mglUniform2d(GLMContext ctx, GLint location, volatile GLdouble x, volatile GLdouble y)
{
    GLdouble data[] = {x, y};
    
    mglUniform(ctx, location, data, 2 * sizeof(GLdouble));
}

void mglUniform2dv(GLMContext ctx, GLint location, GLsizei count, const GLdouble *value)
{
    mglUniform(ctx, location, (void *)value, 2 * count * sizeof(GLuint));
}

void mglUniform2f(GLMContext ctx, GLint location, GLfloat v0, GLfloat v1)
{
    GLfloat data[] = {v0, v1};
    
    mglUniform(ctx, location, data, 2 * sizeof(GLfloat));
}

void mglUniform2fv(GLMContext ctx, GLint location, GLsizei count, const GLfloat *value)
{
    mglUniform(ctx, location, (void *)value, 2 * count * sizeof(GLfloat));
}

void mglUniform2i(GLMContext ctx, GLint location, GLint v0, GLint v1)
{
    GLint data[] = {v0, v1};
    
    mglUniform(ctx, location, data, 2 * sizeof(GLint));
}

void mglUniform2iv(GLMContext ctx, GLint location, GLsizei count, const GLint *value)
{
    mglUniform(ctx, location, (void *)value, 2 * count * sizeof(GLint));
}

void mglUniform2ui(GLMContext ctx, GLint location, GLuint v0, GLuint v1)
{
    GLuint data[] = {v0, v1};
    
    mglUniform(ctx, location, data, 2 * sizeof(GLuint));
}

void mglUniform2uiv(GLMContext ctx, GLint location, GLsizei count, const GLuint *value)
{
    mglUniform(ctx, location, (void *)value, 2 * count * sizeof(GLuint));
}

void mglUniform3d(GLMContext ctx, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    GLdouble data[] = {x, y, z};
    
    mglUniform(ctx, location, data, 3 * sizeof(GLdouble));
}

void mglUniform3dv(GLMContext ctx, GLint location, GLsizei count, const GLdouble *value)
{
    mglUniform(ctx, location, (void *)value, 3 * count * sizeof(GLdouble));
}

void mglUniform3f(GLMContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GLfloat data[] = {v0, v1, v2};
    
    mglUniform(ctx, location, data, 3 * sizeof(GLfloat));
}

void mglUniform3fv(GLMContext ctx, GLint location, GLsizei count, const GLfloat *value)
{
    mglUniform(ctx, location, (void *)value, 3 * count * sizeof(GLfloat));
}

void mglUniform3i(GLMContext ctx, GLint location, GLint v0, GLint v1, GLint v2)
{
    GLint data[] = {v0, v1, v2};
    
    mglUniform(ctx, location, data, 3 * sizeof(GLfloat));
}

void mglUniform3iv(GLMContext ctx, GLint location, GLsizei count, const GLint *value)
{
    mglUniform(ctx, location, (void *)value, 3 * count * sizeof(GLint));
}

void mglUniform3ui(GLMContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    GLuint data[] = {v0, v1, v2};
    
    mglUniform(ctx, location, (void *)data, 3 * sizeof(GLuint));
}

void mglUniform3uiv(GLMContext ctx, GLint location, GLsizei count, const GLuint *value)
{
    mglUniform(ctx, location, (void *)value, 3 * count * sizeof(GLuint));
}

void mglUniform4d(GLMContext ctx, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    GLdouble data[] = {x, y, z, 2};
    
    mglUniform(ctx, location, data, 4 * sizeof(GLdouble));
}

void mglUniform4dv(GLMContext ctx, GLint location, GLsizei count, const GLdouble *value)
{
    mglUniform(ctx, location, (void *)value, 4 * count * sizeof(GLdouble));
}

void mglUniform4f(GLMContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GLfloat data[] = {v0, v1, v2, v3};
    
    mglUniform(ctx, location, (void *)data, 4 * sizeof(GLfloat));
}

void mglUniform4fv(GLMContext ctx, GLint location, GLsizei count, const GLfloat *value)
{
    mglUniform(ctx, location, (void *)value, 4 * count * sizeof(GLfloat));
}

void mglUniform4i(GLMContext ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    GLint data[] = {v0, v1, v2, v3};
    
    mglUniform(ctx, location, data, 4 * sizeof(GLint));
}

void mglUniform4iv(GLMContext ctx, GLint location, GLsizei count, const GLint *value)
{
    mglUniform(ctx, location, (void *)value, 4 * count * sizeof(GLint));
}

void mglUniform4ui(GLMContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    GLuint data[] = {v0, v1, v2, v3};
    
    mglUniform(ctx, location, data, 4 * sizeof(GLuint));
}

void mglUniform4uiv(GLMContext ctx, GLint location, GLsizei count, const GLuint *value)
{
    mglUniform(ctx, location, (void *)value, 4 * count * sizeof(GLuint));
}


// Macro to define matrix types
#define DEFINE_MATRIX_TYPE(_type_, _rows_, _cols_, _name_) \
typedef struct { \
    _type_ d[_rows_][_cols_]; \
} _name_;

// Macro to define transpose functions
#define DEFINE_TRANSPOSE_FUNC(_type_, _rows_, _cols_, _name_, _transposed_name_) \
void _name_##Transpose (const _name_ *matrix, _transposed_name_ *result) { \
    for (int i = 0; i < _rows_; i++) { \
        for (int j = 0; j < _cols_; j++) { \
            result->d[j][i] = matrix->d[i][j]; \
        } \
    } \
}

// Generalized function for uniform matrix upload
#define HANDLE_MATRIX_TRANSPOSE(_type_, _src_type_, _dst_type_, _transpose_func_) \
    if (transpose) { \
        const _src_type_ *src = (const _src_type_ *)value; \
        _dst_type_ *dst = (_dst_type_ *)malloc(count * sizeof(_dst_type_)); \
        for (int i = 0; i < count; i++) { \
            _transpose_func_(&src[i], &dst[i]); \
        } \
        mglUniform(ctx, location, (void *)dst, count * sizeof(_dst_type_)); \
        free(dst); \
    } else { \
        mglUniform(ctx, location, (void *)value, count * sizeof(_src_type_)); \
    }

DEFINE_MATRIX_TYPE(GLdouble, 2, 2, Mat2x2dv)       // 2x2 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 2, 2, Mat2x2dvTrans) // Transposed matrix type (same dimensions for 2x2)
DEFINE_TRANSPOSE_FUNC(GLdouble, 2, 2, Mat2x2dv, Mat2x2dvTrans)

void mglUniformMatrix2dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat2x2dv,          // Source matrix type
                            Mat2x2dvTrans,     // Destination matrix type
                            Mat2x2dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 2, 2, Mat2x2fv)       // 2x2 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 2, 2, Mat2x2fvTrans) // Transposed matrix type (same dimensions for 2x2)
DEFINE_TRANSPOSE_FUNC(GLfloat, 2, 2, Mat2x2fv, Mat2x2fvTrans)

void mglUniformMatrix2fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat2x2fv,          // Source matrix type
                            Mat2x2fvTrans,     // Destination matrix type
                            Mat2x2fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 2, 3, Mat2x3dv)       // 2x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 2, 3, Mat2x3dvTrans) // Transposed matrix type (same dimensions for 2x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 2, 3, Mat2x3dv, Mat2x3dvTrans)

void mglUniformMatrix2x3dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,         // Element type
                            Mat2x3dv,          // Source matrix type
                            Mat2x3dvTrans,     // Destination matrix type
                            Mat2x3dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 2, 3, Mat2x3fv)       // 2x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 2, 3, Mat2x3fvTrans) // Transposed matrix type (same dimensions for 2x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 2, 3, Mat2x3fv, Mat2x3fvTrans)

void mglUniformMatrix2x3fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat2x3fv,          // Source matrix type
                            Mat2x3fvTrans,     // Destination matrix type
                            Mat2x3fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 2, 4, Mat2x4dv)       // 2x4 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 2, 4, Mat2x4dvTrans) // Transposed matrix type (same dimensions for 2x4)
DEFINE_TRANSPOSE_FUNC(GLdouble, 2, 4, Mat2x4dv, Mat2x4dvTrans)

void mglUniformMatrix2x4dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat2x4dv,          // Source matrix type
                            Mat2x4dvTrans,     // Destination matrix type
                            Mat2x4dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 2, 4, Mat2x4fv)       // 2x4 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 2, 4, Mat2x4fvTrans) // Transposed matrix type (same dimensions for 2x4)
DEFINE_TRANSPOSE_FUNC(GLfloat, 2, 4, Mat2x4fv, Mat2x4fvTrans)

void mglUniformMatrix2x4fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat2x4fv,          // Source matrix type
                            Mat2x4fvTrans,     // Destination matrix type
                            Mat2x4fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 3, 3, Mat3x3dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 3, 3, Mat3x3dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 3, 3, Mat3x3dv, Mat3x3dvTrans)

void mglUniformMatrix3dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat3x3dv,          // Source matrix type
                            Mat3x3dvTrans,     // Destination matrix type
                            Mat3x3dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 3, 3, Mat3x3fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 3, 3, Mat3x3fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 3, 3, Mat3x3fv, Mat3x3fvTrans)

void mglUniformMatrix3fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat3x3fv,          // Source matrix type
                            Mat3x3fvTrans,     // Destination matrix type
                            Mat3x3fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 3, 2, Mat3x2dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 3, 2, Mat3x2dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 3, 2, Mat3x2dv, Mat3x2dvTrans)

void mglUniformMatrix3x2dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat3x2dv,          // Source matrix type
                            Mat3x2dvTrans,     // Destination matrix type
                            Mat3x2dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 3, 2, Mat3x2fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 3, 2, Mat3x2fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 3, 2, Mat3x2fv, Mat3x2fvTrans)

void mglUniformMatrix3x2fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat3x2fv,          // Source matrix type
                            Mat3x2fvTrans,     // Destination matrix type
                            Mat3x2fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 3, 4, Mat3x4dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 3, 4, Mat3x4dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 3, 4, Mat3x4dv, Mat3x4dvTrans)

void mglUniformMatrix3x4dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat3x4dv,          // Source matrix type
                            Mat3x4dvTrans,     // Destination matrix type
                            Mat3x4dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 3, 4, Mat3x4fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 3, 4, Mat3x4fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 3, 4, Mat3x4fv, Mat3x4fvTrans)

void mglUniformMatrix3x4fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat3x4fv,          // Source matrix type
                            Mat3x4fvTrans,     // Destination matrix type
                            Mat3x4fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 4, 4, Mat4x4dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 4, 4, Mat4x4dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 4, 4, Mat4x4dv, Mat4x4dvTrans)

void mglUniformMatrix4dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat4x4dv,          // Source matrix type
                            Mat4x4dvTrans,     // Destination matrix type
                            Mat4x4dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 4, 4, Mat4x4fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 4, 4, Mat4x4fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 4, 4, Mat4x4fv, Mat4x4fvTrans)

void mglUniformMatrix4fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat4x4fv,          // Source matrix type
                            Mat4x4fvTrans,     // Destination matrix type
                            Mat4x4fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 4, 2, Mat4x2dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 4, 2, Mat4x2dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 4, 2, Mat4x2dv, Mat4x2dvTrans)

void mglUniformMatrix4x2dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat4x4dv,          // Source matrix type
                            Mat4x4dvTrans,     // Destination matrix type
                            Mat4x4dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 4, 2, Mat4x2fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 4, 2, Mat4x2fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 4, 2, Mat4x2fv, Mat4x2fvTrans)

void mglUniformMatrix4x2fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat4x2fv,          // Source matrix type
                            Mat4x2fvTrans,     // Destination matrix type
                            Mat4x2fvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLdouble, 4, 3, Mat4x3dv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLdouble, 4, 3, Mat4x3dvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLdouble, 4, 3, Mat4x3dv, Mat4x3dvTrans)

void mglUniformMatrix4x3dv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLdouble,        // Element type
                            Mat4x3dv,          // Source matrix type
                            Mat4x3dvTrans,     // Destination matrix type
                            Mat4x3dvTranspose  // Transpose function
        );
}

DEFINE_MATRIX_TYPE(GLfloat, 4, 3, Mat4x3fv)       // 3x3 matrix type
DEFINE_MATRIX_TYPE(GLfloat, 4, 3, Mat4x3fvTrans) // Transposed matrix type (same dimensions for 3x3)
DEFINE_TRANSPOSE_FUNC(GLfloat, 4, 3, Mat4x3fv, Mat4x3fvTrans)

void mglUniformMatrix4x3fv(GLMContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    HANDLE_MATRIX_TRANSPOSE(
                            GLfloat,        // Element type
                            Mat4x3fv,          // Source matrix type
                            Mat4x3fvTrans,     // Destination matrix type
                            Mat4x3fvTranspose  // Transpose function
        );
}

