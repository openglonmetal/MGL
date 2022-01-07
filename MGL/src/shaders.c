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
 * shaders.c
 * MGL
 *
 */

#include <stdio.h>
#include <string.h>
#include <glslang_c_interface.h>
#include <glslang_c_shader_types.h>

#include <ResourceLimits.h>

#include "shaders.h"
#include "glm_context.h"

const glslang_resource_t* glslang_default_resource(void);

const char *getShaderTypeStr(GLuint type)
{
    static const char *types[] = {"VERTEX_SHADER", "FRAGMENT_SHADER",
        "GEOMETRY_SHADER", "TESS_CONTROL_SHADER", "TESS_EVALUATION_SHADER",
        "COMPUTE_SHADER", "MAX_SHADER_TYPES", NULL};

    assert(type < _MAX_SHADER_TYPES);

    return types[type];
};

GLuint glShaderTypeToGLMType(GLuint type)
{
    switch(type) {
        case GL_VERTEX_SHADER: return _VERTEX_SHADER;
        case GL_FRAGMENT_SHADER: return _FRAGMENT_SHADER;
        case GL_GEOMETRY_SHADER: return _GEOMETRY_SHADER;
        case GL_TESS_CONTROL_SHADER: return _TESS_CONTROL_SHADER;
        case GL_TESS_EVALUATION_SHADER: return _TESS_EVALUATION_SHADER;
        case GL_COMPUTE_SHADER: return _COMPUTE_SHADER;
        default:
            assert(0);
    }
}

glslang_stage_t getGLSLStage(GLuint type)
{
    switch(type) {
        case GL_VERTEX_SHADER: return GLSLANG_STAGE_VERTEX;
        case GL_FRAGMENT_SHADER: return GLSLANG_STAGE_FRAGMENT;
        case GL_GEOMETRY_SHADER: return GLSLANG_STAGE_GEOMETRY;
        case GL_TESS_CONTROL_SHADER: return GLSLANG_STAGE_TESSCONTROL;
        case GL_TESS_EVALUATION_SHADER: return GLSLANG_STAGE_TESSEVALUATION;
        case GL_COMPUTE_SHADER: return GLSLANG_STAGE_COMPUTE;
        default:
            assert(0);
    }

    return 0;
}

void initGLSLInput(GLMContext ctx, GLuint type, const char *src, glslang_input_t *input)
{
    input->language = GLSLANG_SOURCE_GLSL;
    input->stage = getGLSLStage(type);
    input->client = GLSLANG_CLIENT_OPENGL;
    input->client_version = GLSLANG_TARGET_OPENGL_450;
    input->target_language = GLSLANG_TARGET_SPV;
    input->target_language_version = GLSLANG_TARGET_SPV_1_5;

    input->code = src;
    input->default_version = 450;
    input->default_profile = GLSLANG_CORE_PROFILE;
    //input->messages = 0xFFFF & ~GLSLANG_MSG_RELAXED_ERRORS_BIT;
    input->messages = GLSLANG_MSG_DEFAULT_BIT | GLSLANG_MSG_DEBUG_INFO_BIT;
    input->resource = glslang_default_resource();

    input->force_default_version_and_profile = 1;
}

Shader *newShader(GLMContext ctx, GLenum type, GLuint shader)
{
    Shader *ptr;
    char shader_type_name[128];

    ptr = (Shader *)malloc(sizeof(Shader));
    assert(ptr);

    bzero(ptr, sizeof(Shader));

    ptr->name = shader;
    ptr->type = type;
    ptr->glm_type = glShaderTypeToGLMType(type);

    sprintf(shader_type_name, "%s_%d", getShaderTypeStr(ptr->glm_type), shader);
    ptr->mtl_shader_type_name = strdup(shader_type_name);

    return ptr;
}

Shader *getShader(GLMContext ctx, GLenum type, GLuint shader)
{
    Shader *ptr;

    ptr = (Shader *)searchHashTable(&STATE(shader_table), shader);

    if (!ptr)
    {
        ptr = newShader(ctx, type, shader);

        insertHashElement(&STATE(shader_table), shader, ptr);
    }

    return ptr;
}

int isShader(GLMContext ctx, GLuint shader)
{
    Shader *ptr;

    ptr = (Shader *)searchHashTable(&STATE(shader_table), shader);

    if (ptr)
        return 1;

    return 0;
}

Shader *findShader(GLMContext ctx, GLuint shader)
{
    Shader *ptr;

    ptr = (Shader *)searchHashTable(&STATE(shader_table), shader);

    return ptr;
}

GLuint mglCreateShader(GLMContext ctx, GLenum type)
{
    GLuint shader;

    shader = getNewName(&STATE(shader_table));

    getShader(ctx, type, shader);

    return shader;
}

void mglDeleteShader(GLMContext ctx, GLuint shader)
{
    Shader *ptr;

    ptr = findShader(ctx, shader);

    if (!ptr)
    {
        assert(0);

        return;
    }

    deleteHashElement(&STATE(shader_table), shader);

    if (ptr->compiled_glsl_shader)
    {
        glslang_shader_delete(ptr->compiled_glsl_shader);
    }

    if (ptr->mtl_data)
    {
        assert(0);
    }

    free((void *)ptr->mtl_shader_type_name);
    free((void *)ptr->src);
}

GLboolean mglIsShader(GLMContext ctx, GLuint shader)
{
    GLboolean ret = 0;

    // Unimplemented function
    assert(0);
    return ret;
}

void mglShaderSource(GLMContext ctx, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)
{
    size_t len;
    char *src;
    Shader *ptr;

    if (isShader(ctx, shader) == GL_FALSE) {
        assert(0);

        return;
    }

    assert(count);
    assert(string);

    // calculate storage
    if (length) {
        len = 0;
        for(int i=0; i<count; i++) {
            len += length[i];
        }

        assert(len);

        src = (char *)malloc(len);

        *src = 0;
        if (length) {
            for(int i=0; i<count; i++) {
                strncat(src, string[i], length[i]);
            }
        }
    } else {
        assert(*string);

        src = strdup(*string);
        len = strlen(src);
    }

    ptr = findShader(ctx, shader);

    ptr->src_len = len;
    ptr->src = src;
    ptr->dirty_bits |= DIRTY_SHADER;
}

void mglCompileShader(GLMContext ctx, GLuint shader)
{
    Shader *ptr;
    glslang_input_t glsl_input;
    glslang_shader_t *glsl_shader;
    int err;

    ptr = findShader(ctx, shader);

    if (!ptr) {

        assert(0);

        return;
    }

    initGLSLInput(ctx, ptr->type, ptr->src, &glsl_input);

    glsl_shader = glslang_shader_create(&glsl_input);
    if (glsl_shader == NULL)
    {
        assert(0);

        return;
    }

    err = glslang_shader_preprocess(glsl_shader, &glsl_input);
    if (!err)
    {
        printf("glslang_shader_preprocess failed err: %d\n", err);
        printf("glslang_shader_get_preprocessed_code:\n%s\n", glslang_shader_get_preprocessed_code(glsl_shader));
        printf("glslang_shader_get_info_log:\n%s\n", glslang_shader_get_info_log(glsl_shader));
        printf("glslang_shader_get_info_debug_log:\n%s\n", glslang_shader_get_info_debug_log(glsl_shader));
        assert(0);

        return;
    }

    err = glslang_shader_parse(glsl_shader, &glsl_input);
    if (!err)
    {
        printf("glslang_shader_parse failed err: %d\n", err);
        printf("glslang_shader_get_preprocessed_code:\n%s\n", glslang_shader_get_preprocessed_code(glsl_shader));
        printf("glslang_shader_get_info_log:\n%s\n", glslang_shader_get_info_log(glsl_shader));
        printf("glslang_shader_get_info_debug_log:\n%s\n", glslang_shader_get_info_debug_log(glsl_shader));
        assert(0);

        return;
    }

    if (ptr->compiled_glsl_shader) {
        ptr->dirty_bits |= DIRTY_SHADER;
    }

    ptr->compiled_glsl_shader = glsl_shader;
}

void mglGetShaderiv(GLMContext ctx, GLuint shader, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetShaderInfoLog(GLMContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    // Unimplemented function
    assert(0);
}

void mglGetShaderSource(GLMContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    // Unimplemented function
    assert(0);
}

void mglGetVertexAttribdv(GLMContext ctx, GLuint index, GLenum pname, GLdouble *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetVertexAttribfv(GLMContext ctx, GLuint index, GLenum pname, GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetVertexAttribiv(GLMContext ctx, GLuint index, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}
