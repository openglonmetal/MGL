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
 * tex_params.c
 * MGL
 *
 */

#include "glm_context.h"

extern GLuint textureIndexFromTarget(GLMContext ctx, GLenum target);
extern Texture *currentTexture(GLMContext ctx, GLuint index);
Texture *getTex(GLMContext ctx, GLuint name, GLenum target);

#pragma mark set params
bool setTexParmi(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *param)
{
    switch(pname)
    {
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            switch(*param)
            {
                case GL_DEPTH_COMPONENT:
                case GL_STENCIL_INDEX:
                    tex_params->depth_stencil_mode = *param;
                    break;

                default:
                    assert(0);
            }
            break;

        case GL_TEXTURE_BASE_LEVEL:
            tex_params->base_level = *param;
            // need to compare this against something...
            break;

        case GL_TEXTURE_COMPARE_FUNC:
            switch(*param)
            {
                case GL_LEQUAL:
                case GL_GEQUAL:
                case GL_LESS:
                case GL_GREATER:
                case GL_EQUAL:
                case GL_NOTEQUAL:
                case GL_ALWAYS:
                case GL_NEVER:
                    tex_params->compare_func = *param;
                    break;

                default:
                    assert(0);
            }
            break;

        case GL_TEXTURE_COMPARE_MODE:
            switch(*param)
            {
                case GL_COMPARE_REF_TO_TEXTURE:
                case GL_NONE:
                    tex_params->compare_mode = *param;
                    break;

                default:
                    assert(0);
            }
            break;

        case GL_TEXTURE_MIN_FILTER:
            switch(*param)
            {
                case GL_NEAREST:
                case GL_LINEAR:
                case GL_NEAREST_MIPMAP_NEAREST:
                case GL_LINEAR_MIPMAP_NEAREST:
                case GL_NEAREST_MIPMAP_LINEAR:
                case GL_LINEAR_MIPMAP_LINEAR:
                    tex_params->min_filter = *param;
                    break;

                default:
                    assert(0);
            }
            break;

        case GL_TEXTURE_MAG_FILTER:
            switch(*param)
            {
                case GL_NEAREST:
                case GL_LINEAR:
                    tex_params->mag_filter = *param;
                    break;

                default:
                    assert(0);
            }
            break;

        case GL_TEXTURE_MIN_LOD:
            tex_params->min_lod = *param;
            break;

        case GL_TEXTURE_MAX_LOD:
            tex_params->max_lod = *param;
            break;

        case GL_TEXTURE_MAX_LEVEL:
            tex_params->max_level = *param;
            break;

        case GL_TEXTURE_SWIZZLE_R:
            tex_params->swizzle_r = *param;
            break;

        case GL_TEXTURE_SWIZZLE_G:
            tex_params->swizzle_g = *param;
            break;

        case GL_TEXTURE_SWIZZLE_B:
            tex_params->swizzle_b = *param;
            break;

        case GL_TEXTURE_SWIZZLE_A:
            tex_params->swizzle_a = *param;
            break;

        case GL_TEXTURE_WRAP_S:
            tex_params->wrap_s = *param;
            break;

        case GL_TEXTURE_WRAP_T:
            tex_params->wrap_t = *param;
            break;

        case GL_TEXTURE_WRAP_R:
            tex_params->wrap_r = *param;
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setTexParamsi(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *params)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                tex_params->border_color[i] = (GLint)params[i];
            break;

        case GL_TEXTURE_SWIZZLE_RGBA:
         if ((params[0] != GL_RED) ||
             (params[1] != GL_GREEN) ||
             (params[2] != GL_BLUE) ||
             (params[3] != GL_ALPHA))
            {
                tex_params->swizzled = true;
                tex_params->swizzle_r = params[0];
                tex_params->swizzle_g = params[1];
                tex_params->swizzle_b = params[2];
                tex_params->swizzle_a = params[3];
            }
            else
            {
                tex_params->swizzled = false;
            }
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setTexParamsIiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLint *params)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                tex_params->border_color_i[i] = params[i];
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setTexParamsIuiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLuint *params)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                tex_params->border_color_ui[i] = params[i];
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setTexParmf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLfloat *param)
{
    switch(pname)
    {
        case GL_TEXTURE_LOD_BIAS:
            tex_params->lod_bias = *param;
            break;

        case GL_TEXTURE_MAX_ANISOTROPY:
            tex_params->max_anisotropy = *param;
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setTexParamsf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, const GLfloat *params)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                tex_params->border_color[i] = params[i];
            break;

        case GL_TEXTURE_SWIZZLE_RGBA:
            tex_params->swizzle_r = (GLint)params[0];
            tex_params->swizzle_g = (GLint)params[1];
            tex_params->swizzle_b = (GLint)params[2];
            tex_params->swizzle_a = (GLint)params[3];
            break;

        default:
            return false;
            break;
    }

    return true;
}

#pragma mark get params
static bool getTexParmi(GLMContext ctx, TextureParameter *tex_params, const GLenum pname, GLint *ret)
{
    switch(pname)
    {
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
            *ret = tex_params->depth_stencil_mode;
            break;

        case GL_TEXTURE_BASE_LEVEL:
            *ret = tex_params->base_level;
            // need to compare this against something...
            break;

        case GL_TEXTURE_COMPARE_FUNC:
            *ret = tex_params->compare_func;
                break;
            break;

        case GL_TEXTURE_COMPARE_MODE:
            *ret = tex_params->compare_mode;
            break;

        case GL_TEXTURE_MIN_FILTER:
            *ret = tex_params->min_filter;
            break;

        case GL_TEXTURE_MAG_FILTER:
            *ret = tex_params->mag_filter;
            break;

        case GL_TEXTURE_MIN_LOD:
            *ret = tex_params->min_lod;
            break;

        case GL_TEXTURE_MAX_LOD:
            *ret = tex_params->max_lod;
            break;

        case GL_TEXTURE_MAX_LEVEL:
            *ret = tex_params->max_level;
            break;

        case GL_TEXTURE_SWIZZLE_R:
            *ret = tex_params->swizzle_r;
            break;

        case GL_TEXTURE_SWIZZLE_G:
            *ret = tex_params->swizzle_g;
            break;

        case GL_TEXTURE_SWIZZLE_B:
            *ret = tex_params->swizzle_b;
            break;

        case GL_TEXTURE_SWIZZLE_A:
            *ret = tex_params->swizzle_a;
            break;

        case GL_TEXTURE_WRAP_S:
            *ret = tex_params->wrap_s;
            break;

        case GL_TEXTURE_WRAP_T:
            *ret = tex_params->wrap_t;
            break;

        case GL_TEXTURE_WRAP_R:
            *ret = tex_params->wrap_r;
            break;

        default:
            return false;
            break;
    }

    return true;
}

#if 0
static bool getTexParamsi(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint *ret)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                ret[i] = tex_params->border_color[i];
            break;

        case GL_TEXTURE_SWIZZLE_RGBA:
            *ret++ = tex_params->swizzle_r;
            *ret++ = tex_params->swizzle_g;
            *ret++ = tex_params->swizzle_b;
            *ret++ = tex_params->swizzle_a;
            break;

        default:
            return false;
            break;
    }

    return true;
}

static bool getTexParamsIiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint *ret)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                ret[i] = tex_params->border_color_i[i];
            break;

        default:
            return false;
            break;
    }

    return true;
}

static bool getTexParamsIuiv(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLuint *ret)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                ret[i] = tex_params->border_color_ui[i];
            break;

        default:
            return false;
            break;
    }

    return true;
}

static bool getTexParamsf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLfloat *ret)
{
    switch(pname)
    {
        case GL_TEXTURE_BORDER_COLOR:
            for(int i=0; i<4; i++)
                ret[i] = tex_params->border_color[i];
            break;

        case GL_TEXTURE_SWIZZLE_RGBA:
            *ret++ = tex_params->swizzle_r;
            *ret++ = tex_params->swizzle_g;
            *ret++ = tex_params->swizzle_b;
            *ret++ = tex_params->swizzle_a;
            break;

        default:
            return false;
            break;
    }

    return true;
}
#endif

static bool getTexParmf(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLfloat *ret)
{
    switch(pname)
    {
        case GL_TEXTURE_LOD_BIAS:
            *ret = tex_params->lod_bias;
            break;

        case GL_TEXTURE_MAX_ANISOTROPY:
            *ret = tex_params->max_anisotropy;
            break;

        default:
            return false;
            break;
    }

    return true;
}

bool setParam(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint iparam, GLfloat fparam)
{
    if (iparam)
    {
        if (setTexParmi(ctx, tex_params, pname, &iparam))
            return true;

        fparam = (float)iparam;
        if (setTexParmf(ctx, tex_params, pname, &fparam))
            return true;
    }
    else
    {
        if (setTexParmf(ctx, tex_params, pname, &fparam))
            return true;

        iparam = (GLint)fparam;
        if (setTexParmi(ctx, tex_params, pname, &iparam))
            return true;
    }

    return false;
}

bool getParam(GLMContext ctx, TextureParameter *tex_params, GLenum pname, GLint *iparam, GLfloat *fparam)
{
    if (iparam)
    {
        if (getTexParmi(ctx, tex_params, pname, iparam))
            return true;

        if (getTexParmf(ctx, tex_params, pname, fparam))
            return true;
    }
    else
    {
        if (getTexParmf(ctx, tex_params, pname, fparam))
            return true;

        if (getTexParmi(ctx, tex_params, pname, iparam))
            return true;
    }

    return false;
}

#pragma tex param gl calls
void mglTexParameterf(GLMContext ctx, GLenum target, GLenum pname, GLfloat param)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setParam(ctx, &tex->params, pname, 0, param))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTexParameterfv(GLMContext ctx, GLenum target, GLenum pname, const GLfloat *params)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

   // more than one param... try setTexParamsf
    if (setTexParamsf(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    if (setParam(ctx, &tex->params, pname, 0, *params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTexParameteri(GLMContext ctx, GLenum target, GLenum pname, GLint param)
{
    GLfloat fparam = 0.0;

    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setParam(ctx, &tex->params, pname, param, fparam))
        return;
}

void mglTexParameteriv(GLMContext ctx, GLenum target, GLenum pname, const GLint *params)
{
    GLfloat fparam = 0.0;

    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    // more than one param... try setTexParamsi
    if (setTexParamsi(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    if (setParam(ctx, &tex->params, pname, *params, fparam))
        return;

    assert(0);
}

void mglTexParameterIiv(GLMContext ctx, GLenum target, GLenum pname, const GLint *params)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setTexParamsIiv(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    // more than one param... try setTexParamsi
    if (setTexParamsi(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    GLfloat fparam = 0.0;
    if (setParam(ctx, &tex->params, pname, *params, fparam))
        return;

    assert(0);
}

void mglTexParameterIuiv(GLMContext ctx, GLenum target, GLenum pname, const GLuint *params)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setTexParamsIuiv(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    // more than one param... try setTexParamsi
    if (setTexParamsi(ctx, &tex->params, pname, (GLint *)params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    GLfloat fparam = 0.0;
    if (setParam(ctx, &tex->params, pname, *params, fparam))
        return;

    assert(0);
}

void mglTextureParameterf(GLMContext ctx, GLuint texture, GLenum pname, GLfloat param)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if(setTexParmf(ctx, &tex->params, pname, &param))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTextureParameterfv(GLMContext ctx, GLuint texture, GLenum pname, const GLfloat *param)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if(setTexParmf(ctx, &tex->params, pname, param))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTextureParameteri(GLMContext ctx, GLuint texture, GLenum pname, GLint param)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if(setTexParmi(ctx, &tex->params, pname, &param))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTextureParameteriv(GLMContext ctx, GLuint texture, GLenum pname, const GLint *param)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if(setTexParmi(ctx, &tex->params, pname, param))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTextureParameterIiv(GLMContext ctx, GLuint texture, GLenum pname, const GLint *params)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setTexParamsIiv(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    // more than one param... try setTexParamsi
    if (setTexParamsi(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    GLfloat fparam = 0.0;
    if (setParam(ctx, &tex->params, pname, *params, fparam))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

void mglTextureParameterIuiv(GLMContext ctx, GLuint texture, GLenum pname, const GLuint *params)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    if (setTexParamsIuiv(ctx, &tex->params, pname, params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    // more than one param... try setTexParamsi
    if (setTexParamsi(ctx, &tex->params, pname, (GLint *)params))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;

        return;
    }

    GLfloat fparam = 0.0;
    if (setParam(ctx, &tex->params, pname, *params, fparam))
    {
        tex->dirty_bits |= DIRTY_TEX_PARAM;
    }
}

#pragma get tex param gl calls
void mglGetTexParameterfv(GLMContext ctx, GLenum target, GLenum pname, GLfloat *params)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    GLint iparam;
    iparam = 0;

    if(getParam(ctx, &tex->params, pname, &iparam, params))
    {
        if (iparam)
        {
            *params = (float)iparam;
        }
    }
}

void mglGetTexParameteriv(GLMContext ctx, GLenum target, GLenum pname, GLint *params)
{
    Texture *tex;

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_NULL_RETURN(tex, GL_INVALID_OPERATION);

    GLfloat fparam;
    fparam = 0.0;

    if(getParam(ctx, &tex->params, pname, params, &fparam))
    {
        if (fparam)
        {
            *params = (float)fparam;
        }
    }
}

void mglGetTexLevelParameterfv(GLMContext ctx, GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTexLevelParameteriv(GLMContext ctx, GLenum target, GLint level, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}
