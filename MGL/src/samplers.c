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
 * samplers.c
 * MGL
 *
 */

#include <strings.h>
#include "glm_context.h"

bool setTexParmi(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *param);
bool setTexParamsi(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *params);
bool setTexParamsIiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *params);
bool setTexParamsIuiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLuint *params);
bool setTexParmf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLfloat *param);
bool setTexParamsf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLfloat *params);
bool setParam(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint iparam, GLfloat fparam);


bool getParam(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint *iparam, GLfloat *fparam);

Sampler *newSampler(GLMContext ctx, GLuint sampler)
{
    Sampler *ptr;

    ptr = (Sampler *)malloc(sizeof(Sampler));
    assert(ptr);

    bzero(ptr, sizeof(Sampler));

    ptr->name = sampler;

    float black_color[] = {0,0,0,0};

    ptr->params.depth_stencil_mode = GL_DEPTH_COMPONENT;
    ptr->params.base_level = 0;
    memcpy(ptr->params.border_color, black_color, 4 * sizeof(float));
    ptr->params.compare_func = GL_NEVER;
    ptr->params.compare_mode = GL_ALWAYS;
    ptr->params.lod_bias = 0.0;
    ptr->params.min_filter = GL_NEAREST;
    ptr->params.mag_filter = GL_NEAREST;
    ptr->params.max_anisotropy = 0.0;
    ptr->params.min_lod = -1000;
    ptr->params.max_lod = 1000;
    ptr->params.max_level = 1000;
    ptr->params.swizzle_r = GL_RED;
    ptr->params.swizzle_g = GL_GREEN;
    ptr->params.swizzle_b = GL_BLUE;
    ptr->params.swizzle_a = GL_ALPHA;
    ptr->params.wrap_s = GL_REPEAT;
    ptr->params.wrap_t = GL_REPEAT;
    ptr->params.wrap_r = GL_REPEAT;

    return ptr;
}

Sampler *getSampler(GLMContext ctx, GLuint sampler)
{
    Sampler *ptr;

    ptr = (Sampler *)searchHashTable(&STATE(sampler_table), sampler);

    if (!ptr)
    {
        ptr = newSampler(ctx, sampler);

        insertHashElement(&STATE(sampler_table), sampler, ptr);
    }

    return ptr;
}

bool isSampler(GLMContext ctx, GLuint sampler)
{
    Sampler *ptr;

    ptr = (Sampler *)searchHashTable(&STATE(sampler_table), sampler);

    if (ptr)
        return true;

    return false;
}

Sampler *findSampler(GLMContext ctx, GLuint sampler)
{
    Sampler *ptr;

    ptr = (Sampler *)searchHashTable(&STATE(sampler_table), sampler);

    return ptr;
}

GLboolean mglIsSampler(GLMContext ctx, GLuint sampler)
{
    return isSampler(ctx, sampler);
}

void mglGenSamplers(GLMContext ctx, GLsizei count, GLuint *samplers)
{
    while(count--)
    {
        *samplers++ = getNewName(&ctx->state.sampler_table);
    }
}

void mglBindSampler(GLMContext ctx, GLuint unit, GLuint sampler)
{
    Sampler *ptr;

    unit = unit - GL_TEXTURE0;

    if (unit > STATE_VAR(max_combined_texture_image_units))
    {
        ERROR_RETURN(GL_INVALID_INDEX);
    }

    if (sampler)
    {
        ERROR_CHECK_RETURN(isSampler(ctx, sampler), GL_INVALID_OPERATION);

        ptr = findSampler(ctx, sampler);

        if(ptr == NULL)
        {
            ptr = getSampler(ctx, sampler);
            assert(ptr);
        }
    }
    else
    {
        ptr = NULL;
    }
    
    ctx->state.texture_samplers[unit] = ptr;
    ctx->state.dirty_bits  |= DIRTY_SAMPLER;
}

void mglDeleteSamplers(GLMContext ctx, GLsizei count, const GLuint *samplers)
{
    while(count--)
    {
        GLuint sampler;

        sampler = *samplers++;

        if (isSampler(ctx, sampler))
        {
            Sampler *ptr;

            ptr = findSampler(ctx, sampler);
            assert(ptr);

            // remove any references to this sampler
            for(int i=0; i<TEXTURE_UNITS; i++)
            {
                if (ctx->state.texture_samplers[i] == ptr)
                {
                    ctx->state.texture_samplers[i] = NULL;
                }
            }

            deleteHashElement(&ctx->state.sampler_table, sampler);

            if (ptr->mtl_data)
            {
                ctx->mtl_funcs.mtlDeleteMTLObj(ctx, ptr->mtl_data);
            }

            free(ptr);
        }
    }
}

void mglCreateSamplers(GLMContext ctx, GLsizei n, GLuint *samplers)
{
    mglGenSamplers(ctx, n, samplers);

    while(n--)
    {
        GLuint name;

        name = *samplers++;

        assert(getSampler(ctx, name));
    }
}

void mglBindSamplers(GLMContext ctx, GLuint first, GLsizei count, const GLuint *samplers)
{
    while(count--)
    {
        mglBindSampler(ctx, first, *samplers++);
        first++;
    }
}

void mglSamplerParameterf(GLMContext ctx, GLuint sampler, GLenum pname, GLfloat param)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setParam(ctx, &ptr->params, pname, 0, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;
    }
}

void mglSamplerParameterfv(GLMContext ctx, GLuint sampler, GLenum pname, const GLfloat *param)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setTexParamsf(ctx, &ptr->params, pname, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setParam(ctx, &ptr->params, pname, 0, *param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }
}

void mglSamplerParameteri(GLMContext ctx, GLuint sampler, GLenum pname, GLint param)
{
    Sampler *ptr;

    ptr = getSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setParam(ctx, &ptr->params, pname, 0, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;
    }
}

void mglSamplerParameteriv(GLMContext ctx, GLuint sampler, GLenum pname, const GLint *param)
{
    GLfloat fparam = 0.0;
    Sampler *ptr;

    ptr = getSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setTexParamsi(ctx, &ptr->params, pname, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setParam(ctx, &ptr->params, pname, *param, fparam))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;
    }
}

void mglSamplerParameterIiv(GLMContext ctx, GLuint sampler, GLenum pname, const GLint *param)
{
    GLfloat fparam = 0.0;
    Sampler *ptr;

    ptr = getSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setTexParamsIiv(ctx, &ptr->params, pname, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setTexParamsi(ctx, &ptr->params, pname, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setParam(ctx, &ptr->params, pname, *param, fparam))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;
    }
}

void mglSamplerParameterIuiv(GLMContext ctx, GLuint sampler, GLenum pname, const GLuint *param)
{
    GLfloat fparam = 0.0;
    Sampler *ptr;

    ptr = getSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    if (setTexParamsIuiv(ctx, &ptr->params, pname, param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setTexParamsi(ctx, &ptr->params, pname, (GLint *)param))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;

        return;
    }

    if (setParam(ctx, &ptr->params, pname, *param, fparam))
    {
        ptr->dirty_bits |= DIRTY_SAMPLER_PARAM;
    }
}

void mglGetSamplerParameterIiv(GLMContext ctx, GLuint sampler, GLenum pname, GLint *params)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    assert(0);
}

void mglGetSamplerParameterIuiv(GLMContext ctx, GLuint sampler, GLenum pname, GLuint *params)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    assert(0);
}

void mglGetSamplerParameterfv(GLMContext ctx, GLuint sampler, GLenum pname, GLfloat *params)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    GLint iparam;
    iparam = 0;

    if(getParam(ctx, &ptr->params, pname, &iparam, params))
    {
        if (iparam)
        {
            *params = (float)iparam;
        }
    }
}

void mglGetSamplerParameteriv(GLMContext ctx, GLuint sampler, GLenum pname, GLint *params)
{
    Sampler *ptr;

    ptr = findSampler(ctx, sampler);

    ERROR_CHECK_NULL_RETURN(ptr, GL_INVALID_OPERATION);

    GLfloat fparam;
    fparam = 0.0;

    if(getParam(ctx, &ptr->params, pname, params, &fparam))
    {
        if (fparam)
        {
            *params = (float)fparam;
        }
    }
}
