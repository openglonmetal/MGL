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
 * textures.c
 * MGL
 *
 */

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdio.h>

// fails in 26.2 Tahoe
//#include <Accelerate/Accelerate.h>

#include "pixel_utils.h"
#include "utils.h"
#include "glm_context.h"

extern void *getBufferData(GLMContext ctx, Buffer *ptr);

bool texSubImage(GLMContext ctx, Texture *tex, GLuint face, GLuint level, GLuint xoffset, GLuint yoffset, GLuint zoffset, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, void *pixels);

GLuint textureIndexFromTarget(GLMContext ctx, GLenum target)
{
    switch(target)
    {
        case GL_TEXTURE_BUFFER: return _TEXTURE_BUFFER_TARGET;
        case GL_TEXTURE_1D: return _TEXTURE_1D;
        case GL_TEXTURE_2D: return _TEXTURE_2D;
        case GL_TEXTURE_3D: return _TEXTURE_3D;
        case GL_TEXTURE_RECTANGLE: return _TEXTURE_RECTANGLE;
        case GL_TEXTURE_1D_ARRAY: return _TEXTURE_1D_ARRAY;
        case GL_TEXTURE_2D_ARRAY: return _TEXTURE_2D_ARRAY;
        case GL_TEXTURE_CUBE_MAP: return _TEXTURE_CUBE_MAP;
        case GL_TEXTURE_CUBE_MAP_ARRAY: return _TEXTURE_CUBE_MAP_ARRAY;
        case GL_TEXTURE_2D_MULTISAMPLE: return _TEXTURE_2D_MULTISAMPLE;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return _TEXTURE_2D_MULTISAMPLE_ARRAY;
        case GL_RENDERBUFFER: return _RENDERBUFFER;

        default:
            assert(0);
    }

    assert(0);

    return _MAX_TEXTURE_TYPES;
}

Texture *currentTexture(GLMContext ctx, GLuint index)
{
    GLuint active_texture;

    active_texture = STATE(active_texture);

    return STATE(texture_units[active_texture].textures[index]);
}

Texture *newTexObj(GLMContext ctx, GLenum target)
{
    Texture *ptr;
    GLuint index;

    index = textureIndexFromTarget(ctx, target);
    if (index == _MAX_TEXTURE_TYPES)
    {
        assert(0);
    }

    ptr = (Texture *)malloc(sizeof(Texture));
    // CRITICAL SECURITY FIX: Check malloc result instead of using assert()
    if (!ptr) {
        fprintf(stderr, "MGL SECURITY ERROR: Failed to allocate memory for texture\n");
        STATE(error) = GL_OUT_OF_MEMORY;
        return NULL;
    }

    bzero(ptr, sizeof(Texture));

    ptr->name = TEX_OBJ_RES_NAME;
    ptr->target = target;
    ptr->index = index;

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

Texture *newTexture(GLMContext ctx, GLenum target, GLuint texture)
{
    Texture *ptr;
    GLuint index;

    index = textureIndexFromTarget(ctx, target);
    if (index == _MAX_TEXTURE_TYPES)
    {
        assert(0);
    }

    ptr = newTexObj(ctx, target);

    ptr->name = texture;

    return ptr;
}

static Texture *getTexture(GLMContext ctx, GLenum target, GLuint texture)
{
    Texture *ptr;

    ptr = (Texture *)searchHashTable(&STATE(texture_table), texture);

    if (!ptr)
    {
        ptr = newTexture(ctx, target, texture);

        insertHashElement(&STATE(texture_table), texture, ptr);
    }

    return ptr;
}

static GLboolean isTexture(GLMContext ctx, GLuint texture)
{
    Texture *ptr;

    ptr = (Texture *)searchHashTable(&STATE(texture_table), texture);

    if (ptr)
        return 1;

    return 0;
}

Texture *findTexture(GLMContext ctx, GLuint texture)
{
    Texture *ptr;

    ptr = (Texture *)searchHashTable(&STATE(texture_table), texture);

    return ptr;
}

Texture *getTex(GLMContext ctx, GLuint name, GLenum target)
{
    GLuint index;
    Texture *ptr;

    if (name == 0)
    {
        index = textureIndexFromTarget(ctx, target);
        assert(index != _MAX_TEXTURE_TYPES);

        ptr = currentTexture(ctx, index);
        
        // Create default texture if none exists for this target
        if (!ptr) {
            GLuint active_texture = STATE(active_texture);
            ptr = newTexObj(ctx, target);
            assert(ptr);
            STATE(texture_units[active_texture].textures[index]) = ptr;
            fprintf(stderr, "MGL: Created default texture for target 0x%x\n", target);
        }
    }
    else
    {
        ptr = findTexture(ctx, name);
        assert(ptr);
        
        target = ptr->target;

        index = textureIndexFromTarget(ctx, target);
        assert(index != _MAX_TEXTURE_TYPES);
    }

    return ptr;
}

bool checkInternalFormatForMetal(GLMContext ctx, GLuint internalformat)
{
    // see if we can actually use this internal format
    GLenum mtl_format;
    mtl_format = mtlFormatForGLInternalFormat(internalformat);

    if (mtl_format == MTLPixelFormatInvalid)
    {
        // Only warn once per format to reduce log spam during capability probing
        static unsigned warned_formats[64] = {0};
        static int warned_count = 0;
        int already_warned = 0;
        for (int i = 0; i < warned_count && i < 64; i++) {
            if (warned_formats[i] == internalformat) { already_warned = 1; break; }
        }
        if (!already_warned && warned_count < 64) {
            warned_formats[warned_count++] = internalformat;
            // Only warn for standard GL format ranges (not internal Mesa/Gallium enums)
            // Skip 0x2xxx (GL get parameters), 0x8Dxx-0x9xxx (internal enums)
            if (internalformat >= 0x8040 && internalformat < 0x8D70) {
                fprintf(stderr, "MGL: checkInternalFormatForMetal - internalformat 0x%x has no Metal equivalent\n", internalformat);
            }
        }
        return false;
    }

    return true;
}


#pragma mark basic tex calls bind / delete / gen...
void mglGenTextures(GLMContext ctx, GLsizei n, GLuint *textures)
{
    assert(textures);

    while(n--)
    {
        *textures++ = getNewName(&STATE(texture_table));

        // TEX_OBJ_RES_NAME has special name.. skip it
        if (STATE(texture_table.current_name) == TEX_OBJ_RES_NAME)
            getNewName(&STATE(texture_table));
    }
}

void mglCreateTextures(GLMContext ctx, GLenum target, GLsizei n, GLuint *textures)
{
    mglGenTextures(ctx, n, textures);

    while(n--)
    {
        // create a texture object
        assert(getTexture(ctx, target, *textures++));
    }
}

void mglBindTexture(GLMContext ctx, GLenum target, GLuint texture)
{
    GLuint active_texture;
    GLint index;
    Texture *ptr;

    index = textureIndexFromTarget(ctx, target);
    if (index == _MAX_TEXTURE_TYPES)
    {
        assert(0);
    }

    if (texture)
    {
        ptr = getTexture(ctx, target, texture);
        assert(ptr);
    }
    else
    {
        ptr = NULL;
    }

    active_texture = STATE(active_texture);

    GLuint mask_index = active_texture / 32;
    GLuint mask = (0x1 << active_texture);

    if (ptr)
    {
        STATE(active_texture_mask[mask_index]) |= mask;
    }
    else
    {
        STATE(active_texture_mask[mask_index]) &= ~mask;
    }

    STATE(active_textures[active_texture]) = ptr;
    STATE(texture_units[active_texture].textures[index]) = ptr;
    STATE(dirty_bits) |= DIRTY_TEX;
}

void mglBindImageTexture(GLMContext ctx, GLuint unit, GLuint texture, GLuint level, GLboolean layered, GLint layer, GLenum access, GLenum internalformat)
{
    Texture *ptr;

    // ERROR_CHECK_RETURN(unit < TEXTURE_UNITS, GL_INVALID_VALUE);
    if (unit >= TEXTURE_UNITS) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: unit >= TEXTURE_UNITS (%d)\n", unit);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    ptr = getTex(ctx, texture, 0);

    // ERROR_CHECK_RETURN(ptr, GL_INVALID_VALUE);
    if (!ptr) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: texture %d not found\n", texture);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    // ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);
    if (level < 0) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: level < 0 (%d)\n", level);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    // ERROR_CHECK_RETURN(layered >= 0, GL_INVALID_VALUE);
    if (layered < 0) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: layered < 0 (%d)\n", layered);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    switch(access)
    {
        case GL_READ_ONLY:
        case GL_WRITE_ONLY:
        case GL_READ_WRITE:
            break;

        default:
            fprintf(stderr, "MGL Error: mglBindImageTexture: invalid access 0x%x\n", access);
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    // ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_ENUM);
    if (!checkInternalFormatForMetal(ctx, internalformat)) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: invalid internalformat 0x%x\n", internalformat);
        ERROR_RETURN(GL_INVALID_ENUM);
    }

    // ERROR_CHECK_RETURN(ptr->internalformat == internalformat, GL_INVALID_VALUE);
    if (ptr->internalformat != internalformat) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: internalformat mismatch (tex=0x%x req=0x%x)\n", ptr->internalformat, internalformat);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    // ERROR_CHECK_RETURN(level < ptr->num_levels, GL_INVALID_VALUE);
    if (level >= ptr->num_levels) {
        fprintf(stderr, "MGL Error: mglBindImageTexture: level >= num_levels (%d >= %d)\n", level, ptr->num_levels);
        ERROR_RETURN(GL_INVALID_VALUE);
    }

    
    ImageUnit unit_params;

    if (ptr->access != access)
    {
        ptr->dirty_bits |= DIRTY_TEXTURE_ACCESS;
        ptr->access = access;
    }

    unit_params.texture = texture;
    unit_params.level = level;
    unit_params.layered = layered;
    unit_params.layer = layer;
    unit_params.access = access;
    unit_params.internalformat = internalformat;
    unit_params.tex = ptr;

    ctx->state.image_units[unit] = unit_params;

    ctx->state.dirty_bits |= DIRTY_IMAGE_UNIT_STATE;
}

void mglDeleteTextures(GLMContext ctx, GLsizei n, const GLuint *textures)
{
    while(n--)
    {
        GLuint name;

        name = *textures++;

        Texture *tex;

        tex = findTexture(ctx, name);

        if(tex)
        {
            for(int i=0; i<TEXTURE_UNITS; i++)
            {
                if(ctx->state.active_textures[i] == tex)
                {
                    ctx->state.active_textures[i] = NULL;

                    ctx->state.dirty_bits |= DIRTY_TEX_BINDING;
                }
            }

            for(int i=0; i<TEXTURE_UNITS; i++)
            {
                if(ctx->state.image_units[i].texture == name)
                {
                    bzero(&ctx->state.image_units[i], sizeof(ImageUnit));

                    ctx->state.dirty_bits |= DIRTY_IMAGE_UNIT_STATE;
                }
            }

            if (tex->mtl_data)
            {
                ctx->mtl_funcs.mtlDeleteMTLObj(ctx, tex->mtl_data);
            }
        }
    }
}

GLboolean mglIsTexture(GLMContext ctx, GLuint texture)
{
    return isTexture(ctx, texture);
}

void mglInvalidateTexImage(GLMContext ctx, GLuint texture, GLint level)
{
    // Stub - invalidation is just a hint, safe to ignore
    fprintf(stderr, "MGL: glInvalidateTexImage called (stub) - texture=%u level=%d\n", texture, level);
}

void mglInvalidateTexSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    // Stub - invalidation is just a hint, safe to ignore
    fprintf(stderr, "MGL: glInvalidateTexSubImage called (stub)\n");
}

void mglBindImageTextures(GLMContext ctx, GLuint first, GLsizei count, const GLuint *textures)
{
    fprintf(stderr, "MGL: glBindImageTextures called first=%u count=%d\n", first, count);
    // Bind multiple image textures
    for (GLsizei i = 0; i < count; i++) {
        GLuint tex_name = textures ? textures[i] : 0;
        if (tex_name != 0) {
            Texture *tex = findTexture(ctx, tex_name);
            if (tex) {
                mglBindImageTexture(ctx, first + i, tex_name, 0, GL_FALSE, 0, tex->access ? tex->access : GL_READ_ONLY, tex->internalformat);
            }
        }
    }
}

void mglClientActiveTexture(GLMContext ctx, GLenum texture)
{
    // Legacy function for fixed-function pipeline, safe to ignore in modern GL
    fprintf(stderr, "MGL: glClientActiveTexture called (stub) - legacy function\n");
}

void mglActiveTexture(GLMContext ctx, GLenum texture)
{
    texture -= GL_TEXTURE0;

    if (texture > STATE_VAR(max_combined_texture_image_units))
    {
        ERROR_RETURN(GL_INVALID_INDEX);
    }

    STATE(active_texture) = texture;
    ctx->state.dirty_bits |= DIRTY_TEX_BINDING;
}

void mglBindTextures(GLMContext ctx, GLuint first, GLsizei count, const GLuint *textures)
{
    for (int i=0; i < count; i++)
    {
        GLuint texture;

        if (textures == NULL)
        {
            texture = 0;
        }
        else
        {
            texture = textures[i];
        }

        STATE(active_texture) = first + i - GL_TEXTURE0;

        if (texture != 0)
        {
            Texture *ptr;
            GLenum target;

            ptr = findTexture(ctx, textures[i]);
            assert(ptr);

            target = ptr->target;

            mglBindTexture(ctx, target, textures[i]);
        }
        else
        {
            for(GLenum target=0; target<_MAX_TEXTURE_TYPES; target++)
            {
                mglBindTexture(ctx, target, 0);
            }
        }
    }

}

void mglBindTextureUnit(GLMContext ctx, GLuint unit, GLuint texture)
{
    // bind texture without changing the current active unit
    // my guess

    Texture *ptr;
    GLenum target;

    ptr = findTexture(ctx, texture);
    assert(ptr);

    unit = unit - GL_TEXTURE0;
    assert(unit < TEXTURE_UNITS);

    target = ptr->target;

    STATE(texture_units[unit].textures[target]) = ptr;
}

void generateMipmaps(GLMContext ctx, GLuint texture, GLenum target)
{
    Texture *ptr;

    ptr = getTex(ctx, texture, target);

    ERROR_CHECK_RETURN(ptr, GL_INVALID_OPERATION);

    // level 0 needs to be filled out for mipmap geneation
    ERROR_CHECK_RETURN(ptr->faces[0].levels[0].complete, GL_INVALID_OPERATION);

    ptr->mipmapped = true;
    ptr->genmipmaps = true;

    ptr->dirty_bits |= DIRTY_TEXTURE_LEVEL;

    ctx->mtl_funcs.mtlGenerateMipmaps(ctx, ptr);
}

void mglGenerateMipmap(GLMContext ctx, GLenum target)
{
    switch(target)
    {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    generateMipmaps(ctx, 0, target);
}

void mglGenerateTextureMipmap(GLMContext ctx, GLuint texture)
{
    generateMipmaps(ctx, texture, 0);
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

void invalidateTexture(GLMContext ctx, Texture *tex)
{
    if (tex->mtl_data)
    {
        ctx->mtl_funcs.mtlDeleteMTLObj(ctx, tex->mtl_data);
    }

    for(int face=0; face<_CUBE_MAP_MAX_FACE; face++)
    {
        for(GLuint i=0; i<tex->num_levels; i++)
        {
            if (tex->faces[face].levels[i].complete)
            {
                if (tex->faces[face].levels[i].data)
                {
                    vm_deallocate(mach_host_self(), tex->faces[face].levels[i].data, tex->faces[face].levels[i].data);
                }
            }
        }
    }

    for(int i=0; i<6; i++)
    {
        if (tex->faces[i].levels)
            free(tex->faces[i].levels);
    }

    bzero(tex, sizeof(Texture));
}

void initBaseTexLevel(GLMContext ctx, Texture *tex, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    tex->mipmapped = 0;
    tex->mipmap_levels = ilog2(MAX(width, height)) + 1;

    for(int face=0; face<_CUBE_MAP_MAX_FACE; face++)
    {
        // CRITICAL SECURITY FIX: Prevent integer overflow in mipmap allocation
        if ((size_t)tex->mipmap_levels > SIZE_MAX / sizeof(TextureLevel)) {
            fprintf(stderr, "MGL SECURITY ERROR: Mipmap levels %d would cause allocation overflow\n", tex->mipmap_levels);
            // CRITICAL FIX: Handle gracefully instead of crashing
            STATE(error) = GL_OUT_OF_MEMORY;
            return;
        }

        tex->faces[face].levels = (TextureLevel *)calloc(tex->mipmap_levels, sizeof(TextureLevel));
        if (!tex->faces[face].levels) {
            fprintf(stderr, "MGL SECURITY ERROR: calloc failed for face %d with %d levels\n", face, tex->mipmap_levels);
            // CRITICAL FIX: Handle gracefully instead of crashing
            STATE(error) = GL_OUT_OF_MEMORY;
            return;
        }
    }

    tex->internalformat = internalformat;
    tex->width = width;
    tex->height = height;
    tex->depth = depth;
    tex->complete = false;

    for(GLuint face=0; face<_CUBE_MAP_MAX_FACE; face++)
    {
        for(GLuint i=0; i<tex->mipmap_levels; i++)
        {
            tex->faces[face].levels[i].complete = false;
        }
    }
}

bool checkTexLevelParams(GLMContext ctx, Texture *tex, GLuint level, GLuint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type)
{
    GLsizei base_width, base_height;

    if (level >= tex->mipmap_levels)
    {
        return false;
    }

    if (tex->target == GL_TEXTURE_2D)
    {
        if (level != 0)
        {
            base_width = tex->width;
            base_height = tex->height;

            while(level--)
            {
                base_width >>= 1;
                base_height >>= 1;
            }

            if (width != base_width || height != base_height)
            {
                return false;
            }
        }
    }

    if (internalformat)
    {
        // internal formats don't jive
        if (internalformat != tex->internalformat)
        {
            return false;
        }
    }
    else
    {
        GLuint temp_internalformat;

        // check if we are expected to convert data
        temp_internalformat = internalFormatForGLFormatType(format, type);

        if (temp_internalformat != tex->internalformat)
        {
            return false;
        }
    }

    if (checkInternalFormatForMetal(ctx, internalformat))
    {
        return false;
    }

    return true;
}


bool verifyInternalFormatAndFormatType(GLMContext ctx, GLint internalformat, GLenum format, GLenum type)
{
    switch(internalformat)
    {
        // unsized formats
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
            break;

        // sized formats
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_R16F:
        case GL_RG16F:
        case GL_RGB16F:
        case GL_RGBA16F:
        case GL_R32F:
        case GL_RG32F:
        case GL_RGB32F:
        case GL_RGBA32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        // Missing SNORM/UI formats used by virgl
        case 0x9014: // GL_ALPHA8_SNORM
        case 0x9016: // GL_LUMINANCE8_ALPHA8_SNORM
        case 0x9018: // GL_ALPHA16_SNORM
        case 0x901a: // GL_LUMINANCE16_ALPHA16_SNORM
        case 0x8d7e: // GL_ALPHA8UI_EXT
            break;

        // compressed types
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            break;

        // Legacy alpha/luminance formats (deprecated but still used)
        case 0x803c: // GL_ALPHA8
        case 0x803e: // GL_ALPHA16
        case 0x8040: // GL_LUMINANCE8
        case 0x8042: // GL_LUMINANCE16
        case 0x8045: // GL_LUMINANCE8_ALPHA8
        case 0x8048: // GL_LUMINANCE16_ALPHA16
        case 0x8816: // GL_ALPHA16F_ARB
        case 0x8818: // GL_LUMINANCE16F_ARB
        case 0x8819: // GL_LUMINANCE_ALPHA16F_ARB
        case 0x881c: // GL_ALPHA32F_ARB
        case 0x881e: // GL_LUMINANCE32F_ARB
        case 0x881f: // GL_LUMINANCE_ALPHA32F_ARB
            break;

        // ASTC compressed formats
        case 0x93b0: // GL_COMPRESSED_RGBA_ASTC_4x4_KHR
        case 0x93b1: // GL_COMPRESSED_RGBA_ASTC_5x4_KHR
        case 0x93b2: // GL_COMPRESSED_RGBA_ASTC_5x5_KHR
        case 0x93b3: // GL_COMPRESSED_RGBA_ASTC_6x5_KHR
        case 0x93b4: // GL_COMPRESSED_RGBA_ASTC_6x6_KHR
        case 0x93b5: // GL_COMPRESSED_RGBA_ASTC_8x5_KHR
        case 0x93b6: // GL_COMPRESSED_RGBA_ASTC_8x6_KHR
        case 0x93b7: // GL_COMPRESSED_RGBA_ASTC_8x8_KHR
        case 0x93b8: // GL_COMPRESSED_RGBA_ASTC_10x5_KHR
        case 0x93b9: // GL_COMPRESSED_RGBA_ASTC_10x6_KHR
        case 0x93ba: // GL_COMPRESSED_RGBA_ASTC_10x8_KHR
        case 0x93bb: // GL_COMPRESSED_RGBA_ASTC_10x10_KHR
        case 0x93bc: // GL_COMPRESSED_RGBA_ASTC_12x10_KHR
        case 0x93bd: // GL_COMPRESSED_RGBA_ASTC_12x12_KHR
        case 0x93d0: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
        case 0x93d1: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
        case 0x93d2: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
        case 0x93d3: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
        case 0x93d4: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
        case 0x93d5: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
        case 0x93d6: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
        case 0x93d7: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
        case 0x93d8: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
        case 0x93d9: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
        case 0x93da: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
        case 0x93db: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
        case 0x93dc: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
        case 0x93dd: // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR
            break;

        // ETC2/EAC compressed formats
        case 0x9270: // GL_COMPRESSED_R11_EAC
        case 0x9271: // GL_COMPRESSED_SIGNED_R11_EAC
        case 0x9272: // GL_COMPRESSED_RG11_EAC
        case 0x9273: // GL_COMPRESSED_SIGNED_RG11_EAC
        case 0x9274: // GL_COMPRESSED_RGB8_ETC2
        case 0x9275: // GL_COMPRESSED_SRGB8_ETC2
        case 0x9276: // GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
        case 0x9277: // GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
        case 0x9278: // GL_COMPRESSED_RGBA8_ETC2_EAC
        case 0x9279: // GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
            break;

        // S3TC/DXT compressed formats
        case 0x83f0: // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        case 0x83f1: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        case 0x83f2: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        case 0x83f3: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        case 0x8c4c: // GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
        case 0x8c4d: // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
        case 0x8c4e: // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
        case 0x8c4f: // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
            break;

        // Additional integer formats (alternate enum values used by some implementations)
        case 0x8d72: // alternate GL_RGBA8I
        case 0x8d75: // alternate GL_RGB8I
        case 0x8d78: // alternate GL_RGBA8UI
        case 0x8d7a: // alternate GL_RGB8UI
        case 0x8d7b: // GL_ALPHA8I_EXT
        // case 0x8d7e: // alternate GL_RGBA32UI - Duplicate of GL_ALPHA8UI_EXT
        case 0x8d80: // alternate GL_RGB32UI
        case 0x8d81: // GL_ALPHA32I_EXT
        case 0x8d84: // alternate GL_RGBA16I
        case 0x8d86: // alternate GL_RGB16I
        case 0x8d87: // GL_ALPHA16I_EXT
        case 0x8d8a: // alternate GL_RGBA32I
        case 0x8d8c: // alternate GL_RGB32I
        case 0x8d8d: // GL_ALPHA32I_EXT
        case 0x8d90: // alternate GL_RGBA16UI
        case 0x8d92: // alternate GL_RGB16UI
        case 0x8d93: // GL_ALPHA16UI_EXT
            break;

        // SNORM formats
        case 0x8f9b: // GL_SIGNED_NORMALIZED
        case 0x8fbd: // GL_RGB10_A2UI (alternate)
        case 0x8fbe: // GL_RGBA16_SNORM
            break;

        // Depth/stencil special formats
        // case 0x9014: // GL_DEPTH_COMPONENT16_NONLINEAR_NV - Duplicate of GL_ALPHA8_SNORM
        // case 0x9016: // GL_TEXTURE_2D_MULTISAMPLE - Duplicate of GL_LUMINANCE8_ALPHA8_SNORM
        // case 0x9018: // GL_TEXTURE_2D_MULTISAMPLE_ARRAY - Duplicate of GL_ALPHA16_SNORM
        // case 0x901a: // GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY - Duplicate of GL_LUMINANCE16_ALPHA16_SNORM
            break;

        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
            ERROR_CHECK_RETURN_VALUE(format == GL_DEPTH_COMPONENT, GL_INVALID_OPERATION, false);
            break;
            
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
            ERROR_CHECK_RETURN_VALUE(format == GL_DEPTH_STENCIL, GL_INVALID_OPERATION, false);
            break;
            
        case GL_STENCIL_INDEX8:
            ERROR_CHECK_RETURN_VALUE(format == GL_STENCIL_INDEX, GL_INVALID_OPERATION, false);
            break;
            
        case GL_RGB565:
            break;

        default:
            // Log warning but don't error - many formats work even if not explicitly listed
            fprintf(stderr, "MGL WARNING: verifyInternalFormat unknown internalformat 0x%x\n", internalformat);
            break;
    }

    switch(format)
    {
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_BGR:
        case GL_RGBA:
        case GL_BGRA:
        case GL_RED_INTEGER:
        case GL_RG_INTEGER:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        // Legacy formats (deprecated but still used by virglrenderer)
        case 0x1906: // GL_ALPHA
        case 0x1909: // GL_LUMINANCE
        case 0x190a: // GL_LUMINANCE_ALPHA
        case 0x8000: // GL_COLOR_INDEX (legacy)
        case 0x8d97: // GL_ALPHA_INTEGER
        case 0x8d9c: // GL_LUMINANCE_INTEGER_EXT
        case 0x8d9d: // GL_LUMINANCE_ALPHA_INTEGER_EXT
            break;

        default:
            // Allow unknown formats with warning - virglrenderer may use nonstandard values
            fprintf(stderr, "MGL WARNING: verifyFormat unknown format 0x%x, allowing\n", format);
            break;
    }

    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_HALF_FLOAT:
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            ERROR_CHECK_RETURN_VALUE(format == GL_RGB,GL_INVALID_OPERATION, false);
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            // Allow RGBA, BGRA, and integer variants (RGBA_INTEGER, RGB_INTEGER, BGRA_INTEGER)
            ERROR_CHECK_RETURN_VALUE((format == GL_RGBA || format == GL_BGRA || 
                                      format == GL_RGBA_INTEGER || format == GL_RGB_INTEGER ||
                                      format == GL_BGRA_INTEGER), GL_INVALID_OPERATION, false);
            break;
            
        case GL_UNSIGNED_INT_24_8:
            ERROR_CHECK_RETURN_VALUE(format == GL_DEPTH_STENCIL, GL_INVALID_OPERATION, false);
            break;
            
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            ERROR_CHECK_RETURN_VALUE(format == GL_DEPTH_STENCIL, GL_INVALID_OPERATION, false);
            break;
        
        // Packed float types for special formats
        case 0x8c3b: // GL_UNSIGNED_INT_10F_11F_11F_REV
        case 0x8c3e: // GL_UNSIGNED_INT_5_9_9_9_REV
            break;
            
        default:
            fprintf(stderr, "MGL WARNING: verifyInternalFormat unknown type 0x%x\n", type);
            break;
    }

    return true;
}


void unpackTexture(GLMContext ctx, Texture *tex, GLuint face, GLuint level, void *src_data, void *dst_data, size_t src_pitch, size_t pixel_size, size_t xoffset, size_t yoffset, size_t zoffset, size_t width, size_t height, size_t depth)
{
    GLubyte *src, *dst;
    size_t dst_pitch;

    src = (GLubyte *)src_data;
    dst = (GLubyte *)dst_data;

    assert(tex);
    dst_pitch = tex->faces[face].levels[level].pitch;
    assert(dst_pitch);

    if (xoffset || yoffset || zoffset)
    {
        size_t xoffset_bytes = xoffset * pixel_size; // num pixels
        size_t yoffset_bytes = yoffset * dst_pitch; // num lines (rows * bytes_per_row)
        size_t zoffset_bytes = zoffset * dst_pitch * height; // num planes

        dst += xoffset_bytes;
        dst += yoffset_bytes;
        dst += zoffset_bytes;
    }

    if (depth > 1)
    {
        // 3d texture
        for(size_t y=0; y<depth; y++)
        {
            memcpy(dst, src, dst_pitch);
            src += src_pitch;
            dst += dst_pitch;
        }
    }
    else if (height > 1)
    {
        // 2d texture
        size_t copy_size = width * pixel_size;
        
        for(size_t y=0; y<height; y++)
        {
            memcpy(dst, src, copy_size);
            src += src_pitch;
            dst += dst_pitch;
        }
    }
    else
    {
        // 1d texture
        memcpy(dst, src, width * pixel_size);
    }
}

#pragma mark texImage 1D/2D/3D
// Forward declaration
bool texSubImage(GLMContext ctx, Texture *tex, GLuint face, GLuint level, GLuint xoffset, GLuint yoffset, GLuint zoffset, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, void *pixels);

bool createTextureLevel(GLMContext ctx, Texture *tex, GLuint face, GLuint level, GLboolean is_array, GLuint internalformat, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, void *pixels, GLboolean proxy)
{
    // all the levels are created on a tex storage call.. if we get here we should just assert
    if (tex->immutable_storage)
    {
        // Compatibility: Treat glTexImage* on immutable texture as glTexSubImage*
        // This allows guests to update content using glTexImage* which is common in some drivers
        // We pass 0 for offsets. texSubImage will handle validation.
        return texSubImage(ctx, tex, face, level, 0, 0, 0, width, height, depth, format, type, pixels);
    }
    
    if (level == 0)
    {
        if (internalformat == 0)
        {
            internalformat = internalFormatForGLFormatType(format, type);

            if (internalformat == 0)
            {
                ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
            }
        }
        else if (pixels)
        {
            GLuint temp_format;

            // check if format type can be copied directly to the internal format
            temp_format = internalFormatForGLFormatType(format, type);

            // MGL doesn't support pixel format conversion
            // If mismatch, use the format that matches the incoming data
            if (temp_format != internalformat)
            {
                internalformat = temp_format;
            }
        }

        // see if we can actually use this internal format
        if (checkInternalFormatForMetal(ctx, internalformat) == false)
        {
            ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
        }

        if (tex->mipmap_levels == 0)
        {
            // uninitialized tex
            initBaseTexLevel(ctx, tex, internalformat, width, height, depth);
        }
        else if (width != tex->width || height != tex->height || internalformat != tex->internalformat)
        {
            // invalidate texture because the base level width / height / internal format are being changed...
            invalidateTexture(ctx, tex);

            initBaseTexLevel(ctx, tex, internalformat, width, height, depth);
        }
    }
    else if (checkTexLevelParams(ctx, tex, level, internalformat, width, height, depth, format, type) == false)
    {
        ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
    }

    if (STATE(buffers[_PIXEL_UNPACK_BUFFER]))
    {
        Buffer *ptr;

        ptr = STATE(buffers[_PIXEL_UNPACK_BUFFER]);

        ERROR_CHECK_RETURN(ptr->mapped == false, GL_INVALID_OPERATION);

        GLubyte *buffer_data;
        buffer_data = getBufferData(ctx, ptr);

        // if a pixel buffer is the src, pixels is the offset
        // need to check offset against size
        size_t offset;
        offset = (size_t)pixels;

        pixels = &buffer_data[offset];
    }

    tex->num_levels = MAX(tex->num_levels, level + 1);
    tex->faces[face].levels[level].width = width;
    tex->faces[face].levels[level].height = height;
    tex->faces[face].levels[level].depth = depth;

    kern_return_t err;
    vm_address_t texture_data;
    size_t pixel_size;
    size_t internal_size;
    size_t texture_size;
    size_t src_pitch;

    pixel_size = sizeForInternalFormat(internalformat, format, type);
    ERROR_CHECK_RETURN_VALUE(pixel_size, GL_INVALID_ENUM, false);

    assert(width);
    assert(height);
    assert(depth);

    tex->faces[face].levels[level].pitch = pixel_size * width;

    if (depth > 1)
    {
        // 3d texture
        internal_size = pixel_size * width * height * depth;
    }
    else if (height > 1)
    {
        // 2d texture
        internal_size = pixel_size * width * height;
    }
    else
    {
        // 1d texture
        internal_size = pixel_size * width;
    }

    texture_size = page_size_align(internal_size);
    assert(texture_size);

    switch(mtlFormatForGLInternalFormat(internalformat))
    {
        case MTLPixelFormatDepth16Unorm:
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatDepth24Unorm_Stencil8:
        case MTLPixelFormatDepth32Float_Stencil8:
            tex->mtl_requires_private_storage = true;
            break;

        default:
            tex->mtl_requires_private_storage = false;
            break;
    }

    if (tex->mtl_requires_private_storage == false)
    {
        // Allocate directly from VM
        err = vm_allocate((vm_map_t) mach_task_self(),
                          (vm_address_t*) &texture_data,
                          texture_size,
                          VM_FLAGS_ANYWHERE);
        assert(err == 0);
        assert(texture_data);

        tex->faces[face].levels[level].data_size = texture_size;
        tex->faces[face].levels[level].data = (vm_address_t)texture_data;

        if (pixels)
        {
            GLsizei src_size;

            src_size = width * sizeForFormatType(format, type);

            if (ctx->state.unpack.row_length)
            {
                size_t alignment;

                if (ctx->state.unpack.row_length < width) {
                    ERROR_RETURN_VALUE(GL_INVALID_VALUE, false);
                }

                alignment = ctx->state.unpack.alignment;
                if (alignment)
                {
                    /* row_length is in pixels, so multiply by pixel_size to get bytes per row */
                    size_t row_bytes = ctx->state.unpack.row_length * pixel_size;
                    if (row_bytes >= alignment)
                    {
                        src_pitch = row_bytes;
                        assert(src_pitch);
                    }
                    else if (depth > 1)
                    {
                        // 3d texture
                        src_pitch = alignment / src_size;

                        src_pitch = src_pitch * src_size * ctx->state.unpack.row_length * height;

                        src_pitch = src_pitch / alignment;
                        assert(src_pitch);
                    }
                    else
                    {
                        src_pitch = alignment / src_size;

                        src_pitch = src_pitch * src_size * ctx->state.unpack.row_length;

                        src_pitch = src_pitch / alignment;
                        assert(src_pitch);
                    }
                }
                else
                {
                    src_pitch = src_size * ctx->state.unpack.row_length;
                    assert(src_pitch);
                }
            }
            else
            {
                src_pitch = tex->faces[face].levels[level].pitch;
                assert(src_pitch);
            }

            // unpack from pixel buffer
            if (STATE(buffers[_PIXEL_UNPACK_BUFFER]))
            {
                Buffer *ptr;

                ptr = STATE(buffers[_PIXEL_UNPACK_BUFFER]);

                ERROR_CHECK_RETURN(ptr->mapped == false, GL_INVALID_OPERATION);

                GLubyte *buffer_data;
                buffer_data = getBufferData(ctx, ptr);

                // if a pixel buffer is the src, pixels is the offset
                // need to check offset against size
                size_t offset;
                offset = (size_t)pixels;

                pixels = &buffer_data[offset];
            }

            unpackTexture(ctx, tex, face, level, (void *)pixels, (void *)texture_data, src_pitch, pixel_size, 0, 0, 0, width, height, depth);

            tex->dirty_bits |= DIRTY_TEXTURE_DATA;
        };
    }

    tex->faces[face].levels[level].complete = true;

    tex->dirty_bits |= DIRTY_TEXTURE_LEVEL;
    STATE(dirty_bits) |= DIRTY_TEX;

    return true;
}

void mglTexImage1D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;
    bool proxy;

    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_1D:
            break;

        case GL_PROXY_TEXTURE_1D:
            proxy = true;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    // verifyFormatType sets the error
    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(border == 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    tex->access = GL_READ_ONLY;

    createTextureLevel(ctx, tex, 0, level, false, internalformat, width, 1, 1, format, type, (void *)pixels, proxy);
}

void mglTexImage2D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;
    GLuint face;
    GLboolean is_array;
    GLboolean proxy;

    fprintf(stderr, "MGL: mglTexImage2D called - target=0x%x, level=%d, internalformat=0x%x, width=%d, height=%d, format=0x%x, type=0x%x\n",
            target, level, internalformat, width, height, format, type);

    face = 0;
    is_array = false;
    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_2D:
            break;

        case GL_PROXY_TEXTURE_2D:
        case GL_PROXY_TEXTURE_CUBE_MAP:
            proxy = true;
            break;

        case GL_PROXY_TEXTURE_1D_ARRAY:
            is_array = true;
            proxy = true;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;

        case GL_PROXY_TEXTURE_RECTANGLE:
            proxy = true;
            ERROR_CHECK_RETURN(level==0, GL_INVALID_OPERATION);
            break;

        case GL_TEXTURE_RECTANGLE:
            ERROR_CHECK_RETURN(level==0, GL_INVALID_OPERATION);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    // verifyFormatType sets the error
    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(border == 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    tex->access = GL_READ_ONLY;

    createTextureLevel(ctx, tex, face, level, is_array, internalformat, width, height, 1, format, type, (void *)pixels, proxy);
}

void mglTexImage2DMultisample(GLMContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    // Multisample textures are used by virglrenderer for capability probing.
    // Apple Silicon handles MSAA differently - we silently succeed to allow
    // the rendering pipeline to proceed without MSAA.
    fprintf(stderr, "MGL: mglTexImage2DMultisample (stub) - target=0x%x samples=%d internalformat=0x%x %dx%d\n",
            target, samples, internalformat, width, height);
    (void)ctx; (void)target; (void)samples; (void)internalformat;
    (void)width; (void)height; (void)fixedsamplelocations;
    // Don't set error - allow probing to "succeed" so virglrenderer continues
}

void mglTexImage3D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;
    GLboolean is_array;
    GLboolean proxy;

    is_array = false;
    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_3D:
            break;

        case GL_PROXY_TEXTURE_3D:
            proxy = true;
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
            is_array = true;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    // verifyFormatType sets the error
    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(depth >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(border == 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    tex->access = GL_READ_ONLY;

    createTextureLevel(ctx, tex, 0, level, is_array, internalformat, width, height, depth, format, type, (void *)pixels, proxy);
}

void mglTexImage3DMultisample(GLMContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    // Multisample array textures - silently succeed like 2D multisample
    fprintf(stderr, "MGL: mglTexImage3DMultisample (stub) - target=0x%x samples=%d internalformat=0x%x %dx%dx%d\n",
            target, samples, internalformat, width, height, depth);
    (void)ctx; (void)target; (void)samples; (void)internalformat;
    (void)width; (void)height; (void)depth; (void)fixedsamplelocations;
    // Don't set error
}

#pragma mark texSubImage
bool texSubImage(GLMContext ctx, Texture *tex, GLuint face, GLuint level, GLuint xoffset, GLuint yoffset, GLuint zoffset, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, void *pixels)
{
    // Debug: Log large texture uploads (VM framebuffer size)
    if (width >= 640 && height >= 400)
    {
        fprintf(stderr, "MGL DEBUG: texSubImage tex_id=%u %dx%d at (%d,%d) pixels=%p\n",
                tex ? tex->name : 0, width, height, xoffset, yoffset, pixels);
    }
    
    // ERROR_CHECK_RETURN_VALUE(tex != NULL, GL_INVALID_OPERATION, false);
    if (tex == NULL)
    {
        fprintf(stderr, "MGL Error: texSubImage: tex is NULL\n");
        ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
    }

    // ERROR_CHECK_RETURN_VALUE(level <= tex->num_levels, GL_INVALID_OPERATION, false);
    if (level > tex->num_levels)
    {
        fprintf(stderr, "MGL Error: texSubImage: level %d > num_levels %d\n", level, tex->num_levels);
        ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
    }
    
    if (!tex->faces[face].levels)
    {
        fprintf(stderr, "MGL Error: texSubImage: levels is NULL\n");
        ERROR_CHECK_RETURN_VALUE(false, GL_INVALID_OPERATION, false);
    }
    
    // ERROR_CHECK_RETURN_VALUE(tex->faces[face].levels[level].complete, GL_INVALID_OPERATION, false);
    if (!tex->faces[face].levels[level].complete)
    {
        fprintf(stderr, "MGL Error: texSubImage: level %d not complete\n", level);
        ERROR_RETURN_VALUE(GL_INVALID_OPERATION, false);
    }

    // unpack from pixel buffer
    if (STATE(buffers[_PIXEL_UNPACK_BUFFER]))
    {
        Buffer *ptr;

        ptr = STATE(buffers[_PIXEL_UNPACK_BUFFER]);

        // ERROR_CHECK_RETURN(ptr->mapped == false, GL_INVALID_OPERATION);
        if (ptr->mapped) {
            fprintf(stderr, "MGL Error: texSubImage: pixel unpack buffer is mapped\n");
            ERROR_RETURN(GL_INVALID_OPERATION);
        }

        GLubyte *buffer_data;
        buffer_data = getBufferData(ctx, ptr);

        // if a pixel buffer is the src, pixels is the offset
        // need to check offset against size
        size_t offset;
        offset = (size_t)pixels;

        pixels = &buffer_data[offset];
    }

    // no src data.. return
    ERROR_CHECK_RETURN(pixels, GL_INVALID_OPERATION);

    size_t pixel_size;
    size_t src_size;
    size_t src_pitch;

    pixel_size = sizeForFormatType(format, type);
    src_size = width * pixel_size;

    if (ctx->state.unpack.row_length)
    {
        size_t alignment;

        // ERROR_CHECK_RETURN((ctx->state.unpack.row_length >> level) >= width, GL_INVALID_VALUE);
        // Fix: row_length applies to the source data for the current level, do not shift by level
        if (ctx->state.unpack.row_length < width)
        {
             ERROR_RETURN_VALUE(GL_INVALID_VALUE, false);
        }

        alignment = ctx->state.unpack.alignment;
        if (alignment)
        {
            if (src_size >= alignment)
            {
                size_t row_bytes = (ctx->state.unpack.row_length >> level) * pixel_size;
                src_pitch = row_bytes;
                assert(src_pitch);
            }
            else if (depth > 1)
            {
                // 3d texture
                src_pitch = alignment / src_size;

                src_pitch = src_pitch * src_size * ctx->state.unpack.row_length * height;

                src_pitch = src_pitch / alignment;
                assert(src_pitch);
            }
            else
            {
                src_pitch = alignment / src_size;

                src_pitch = src_pitch * src_size * ctx->state.unpack.row_length;

                src_pitch = src_pitch / alignment;
                assert(src_pitch);
            }
        }
        else
        {
            size_t row_bytes = (ctx->state.unpack.row_length >> level) * pixel_size;
            src_pitch = row_bytes;
            assert(src_pitch);
        }
    }
    else
    {
        src_pitch = src_size;
        assert(src_pitch);
    }

    void *texture_data;

    texture_data = (void *)tex->faces[face].levels[level].data;
    
    unpackTexture(ctx, tex, face, level, pixels, texture_data, src_pitch, pixel_size, xoffset, yoffset, zoffset, width, height, depth);

    // use a blit command to update data
    do
    {
        Buffer *buf;

        buf = STATE(buffers[_PIXEL_UNPACK_BUFFER]);

        if (buf == NULL)
            continue;

        if (tex->mtl_data == NULL)
            continue;

        size_t src_offset;
        size_t src_image_size;
        size_t src_size;

        src_offset = (size_t)0;

        src_image_size = src_pitch * height;

        src_size = src_image_size * depth;

        ctx->mtl_funcs.mtlTexSubImage(ctx, tex, buf, src_offset, src_pitch, src_image_size, src_size, zoffset, level, width, height, depth, xoffset, yoffset, zoffset);

        return true;
    } while(false);

    // use process gl to upload texture data
    tex->dirty_bits |= DIRTY_TEXTURE_DATA;
    
    return true;
}

#pragma mark texSubImage1D
void texSubImage1D(GLMContext ctx, Texture *tex, GLuint face, GLint level, GLuint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, tex->internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(width + xoffset <= tex->width, GL_INVALID_VALUE);

    texSubImage(ctx, tex, face, level, xoffset, 0, 0, width, 1, 1, format, type, (void *)pixels);
}

void mglTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;

    switch(target)
    {
        case GL_TEXTURE_1D:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texSubImage1D(ctx, tex, 0, level, xoffset, width, format, type, pixels);
}

void mglTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

   texSubImage1D(ctx, tex, 0, level, xoffset, width, format, type, pixels);
}

#pragma mark texSubImage2D
bool texSubImage2D(GLMContext ctx, Texture *tex, GLuint face, GLuint level, GLuint xoffset, GLuint yoffset, GLuint width, GLuint height, GLenum format, GLenum type, const void *pixels)
{
    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);
    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, tex->internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(width + xoffset <= tex->width, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height + yoffset <= tex->height, GL_INVALID_VALUE);

    texSubImage(ctx, tex, face, level, xoffset, yoffset, 0, width, height, 1, format, type, (void *)pixels);

    return true;
}

void mglTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;
    GLuint face;

    face = 0;

    switch(target)
    {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_RECTANGLE:
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    tex = getTex(ctx, 0, target);

    if (!tex) {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    texSubImage2D(ctx, tex, face, level, xoffset, yoffset, width, height, format, type, pixels);
}

void mglTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texSubImage2D(ctx, tex, 0, level, xoffset, yoffset, width, height, format, type, pixels);
}

#pragma mark texSubImage3D
void texSubImage3D(GLMContext ctx, Texture *tex, GLuint level, GLuint xoffset, GLuint yoffset, GLuint zoffset, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, const void *pixels)
{

    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, tex->internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(depth >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(width + xoffset <= tex->width, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height + yoffset <= tex->height, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(depth + zoffset <= tex->depth, GL_INVALID_VALUE);

    texSubImage(ctx, tex, 0, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (void *)pixels);
}

void mglTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;

    switch(target)
    {
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texSubImage3D(ctx, tex, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void mglTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    Texture *tex;

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texSubImage3D(ctx, tex, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

#pragma mark TexStorage

void texStorage(GLMContext ctx, Texture *tex, GLuint faces, GLsizei levels, GLboolean is_array, GLenum internalformat, GLuint width, GLuint height, GLuint depth, GLboolean proxy)
{
    tex->access = GL_READ_ONLY;

    for(GLuint face=0; face<faces; face++)
    {
        GLuint level_width, level_height;

        level_width = width;
        level_height = height;

        for(int level=0; level<levels; level++)
        {
            createTextureLevel(ctx, tex, face, level, is_array, internalformat, level_width, level_height, depth, 0, 0, NULL, proxy);

            level_width >>= 1;
            level_height >>= 1;
            
            // Mipmap dimensions must be at least 1
            if (level_width == 0) level_width = 1;
            if (level_height == 0) level_height = 1;
        }
    }

    // mark it immutable
    tex->immutable_storage = BUFFER_IMMUTABLE_STORAGE_FLAG;

    // bind it to metal
    ctx->mtl_funcs.mtlBindTexture(ctx, tex);

    ERROR_CHECK_RETURN(tex->mtl_data, GL_OUT_OF_MEMORY);
}

void mglTexStorage1D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    Texture *tex;
    GLboolean proxy;

    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_1D:
            break;

        case GL_PROXY_TEXTURE_1D:
            proxy = true;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texStorage(ctx, tex, 1, levels, false, internalformat, width, 1, 1, proxy);
}

void mglTextureStorage1D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    Texture *tex;

    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, texture, 0);

    ERROR_CHECK_RETURN(tex != NULL, GL_INVALID_OPERATION);

    texStorage(ctx, tex, 1, levels, false, internalformat, width, 1, 1, false);
}

void mglTexStorage2D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    Texture *tex;
    GLboolean is_array;
    GLboolean proxy;
    GLuint num_faces;

    is_array = false;
    proxy = false;
    num_faces = 1;

    switch(target)
    {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            break;

        case GL_PROXY_TEXTURE_2D:
        case GL_PROXY_TEXTURE_RECTANGLE:
            proxy = true;
            break;

        case GL_TEXTURE_CUBE_MAP:
            num_faces = 6;
            proxy = true;
            break;

        case GL_PROXY_TEXTURE_CUBE_MAP:
            num_faces = 6;
            proxy = true;
            break;

        case GL_TEXTURE_1D_ARRAY:
            is_array = true;
            break;

        case GL_PROXY_TEXTURE_1D_ARRAY:
            is_array = true;
            proxy = true;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);
    
    fprintf(stderr, "MGL: mglTexStorage2D target=0x%x levels=%d internalformat=0x%x %dx%d tex=%p\n",
            target, levels, internalformat, width, height, tex);
    fflush(stderr);

    texStorage(ctx, tex, num_faces, levels, is_array, internalformat, width, height, 1, proxy);
}


void mglTextureStorage2D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    Texture *tex;

    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, texture, 0);

    texStorage(ctx, tex, 1, levels, false, internalformat, width, height, 1, false);
}

void mglTextureStorage2DMultisample(GLMContext ctx, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    fprintf(stderr, "MGL WARNING: glTextureStorage2DMultisample called (stub) - MSAA not fully supported\n");
    // Fall back to non-MSAA storage
    mglTextureStorage2D(ctx, texture, 1, internalformat, width, height);
}

void mglTexStorage3D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    Texture *tex;
    GLboolean is_array;
    GLboolean proxy;

    is_array = false;
    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_3D:
            break;

        case GL_PROXY_TEXTURE_3D:
            proxy = true;
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            is_array = true;
            break;

        case GL_PROXY_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
            is_array = true;
            proxy = true;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(checkMaxLevels(levels, width, height, depth), GL_INVALID_OPERATION);
    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(depth > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    texStorage(ctx, tex, 1, levels, is_array, internalformat, width, height, depth, proxy);
}

void mglTextureStorage3D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    Texture *tex;

    ERROR_CHECK_RETURN(levels > 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(checkInternalFormatForMetal(ctx, internalformat), GL_INVALID_OPERATION);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(depth > 0, GL_INVALID_VALUE);

    tex = getTex(ctx, texture, 0);

    createTextureLevel(ctx, tex, 0, 0, false, internalformat, width, height, depth, 0, 0, NULL, false);
}

void mglTextureStorage3DMultisample(GLMContext ctx, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    fprintf(stderr, "MGL WARNING: glTextureStorage3DMultisample called (stub) - MSAA not fully supported\n");
    // Fall back to non-MSAA storage
    mglTextureStorage3D(ctx, texture, 1, internalformat, width, height, depth);
}


#pragma mark clear tex image
void mglClearTexImage(GLMContext ctx, GLuint texture, GLuint level, GLenum format, GLenum type, const void *data)
{
    fprintf(stderr, "MGL: glClearTexImage called - texture=%u level=%d\n", texture, level);
    
    Texture *tex = getTex(ctx, texture, 0);
    if (!tex) {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    // For now, use texSubImage to clear - fill with the clear data
    GLuint width = tex->width >> level;
    GLuint height = tex->height >> level;
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    
    // If data is NULL, clear to zero
    if (data == NULL)
    {
        size_t pixel_size = sizeForFormatType(format, type);

        // CRITICAL SECURITY FIX: Prevent integer overflow in texture clear allocation
        if (width > SIZE_MAX / height / pixel_size)
        {
            fprintf(stderr, "MGL SECURITY ERROR: Texture clear allocation would overflow: %dx%dx%zu\n", width, height, pixel_size);
            STATE(error) = GL_OUT_OF_MEMORY;
            return;
        }

        size_t size = width * height * pixel_size;
        void *clear_data = calloc(1, size);
        if (clear_data)
        {
            texSubImage(ctx, tex, 0, level, 0, 0, 0, width, height, 1, format, type, clear_data);
            free(clear_data);
        }
    }
    else
    {
        // Fill entire texture with the provided clear value
        size_t pixel_size = sizeForFormatType(format, type);

        // CRITICAL SECURITY FIX: Prevent integer overflow in texture fill allocation
        if (width > SIZE_MAX / height / pixel_size)
        {
            fprintf(stderr, "MGL SECURITY ERROR: Texture fill allocation would overflow: %dx%dx%zu\n", width, height, pixel_size);
            STATE(error) = GL_OUT_OF_MEMORY;
            return;
        }

        size_t size = width * height * pixel_size;
        void *fill_data = malloc(size);
        if (fill_data)
        {
            // Replicate the clear value across the entire buffer
            for (size_t i = 0; i < width * height; i++)
            {
                memcpy((char*)fill_data + i * pixel_size, data, pixel_size);
            }
            texSubImage(ctx, tex, 0, level, 0, 0, 0, width, height, 1, format, type, fill_data);
            free(fill_data);
        }
    }
}

void mglClearTexSubImage(GLMContext ctx, GLuint texture, GLuint level, GLuint xoffset, GLint yoffset, GLuint zoffset, GLuint width, GLuint height, GLuint depth, GLenum format, GLenum type, const void *data)
{
    fprintf(stderr, "MGL: glClearTexSubImage called - texture=%u %dx%dx%d at (%d,%d,%d)\n",
            texture, width, height, depth, xoffset, yoffset, zoffset);
    
    Texture *tex = getTex(ctx, texture, 0);
    if (!tex)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    size_t pixel_size = sizeForFormatType(format, type);

    // CRITICAL SECURITY FIX: Prevent integer overflow in texture subimage allocation
    if (width > SIZE_MAX / height / depth / pixel_size)
    {
        fprintf(stderr, "MGL SECURITY ERROR: Texture subimage allocation would overflow: %dx%dx%dx%zu\n", width, height, depth, pixel_size);
        STATE(error) = GL_OUT_OF_MEMORY;
        return;
    }

    size_t size = width * height * depth * pixel_size;
    
    if (data == NULL)
    {
        void *clear_data = calloc(1, size);
        if (clear_data)
        {
            texSubImage(ctx, tex, 0, level, xoffset, yoffset, zoffset, width, height, depth, format, type, clear_data);
            free(clear_data);
        }
    }
    else
    {
        void *fill_data = malloc(size);
        if (fill_data)
        {
            for (size_t i = 0; i < width * height * depth; i++) {
                memcpy((char*)fill_data + i * pixel_size, data, pixel_size);
            }
            texSubImage(ctx, tex, 0, level, xoffset, yoffset, zoffset, width, height, depth, format, type, fill_data);
            free(fill_data);
        }
    }
}

#pragma mark compressed tex image
void mglCompressedTexImage3D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexImage3D called (stub) - compressed textures not supported\n");
}

void mglCompressedTexImage2D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexImage2D called (stub) - compressed textures not supported\n");
}

void mglCompressedTexImage1D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexImage1D called (stub) - compressed textures not supported\n");
}

void mglCompressedTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexSubImage3D called (stub) - compressed textures not supported\n");
}

void mglCompressedTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexSubImage2D called (stub) - compressed textures not supported\n");
}

void mglCompressedTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    // Stub - compressed textures not fully supported yet
    fprintf(stderr, "MGL WARNING: glCompressedTexSubImage1D called (stub) - compressed textures not supported\n");
}

#pragma mark copy tex
void mglCopyTexImage1D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    // Stub - not commonly used
    fprintf(stderr, "MGL WARNING: glCopyTexImage1D called (stub)\n");
}

void mglCopyTexImage2D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    fprintf(stderr, "MGL: glCopyTexImage2D called - target=0x%x level=%d %dx%d\n", target, level, width, height);
    
    // Get or create texture
    Texture *tex = getTex(ctx, 0, target);
    if (!tex)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    // Initialize texture storage if needed
    if (tex->mipmap_levels == 0)
    {
        initBaseTexLevel(ctx, tex, internalformat, width, height, 1);
    }
    
    // Copy from framebuffer to texture
    ctx->mtl_funcs.mtlCopyTexSubImage(ctx, tex, level, 0, 0, x, y, width, height);
}

void mglCopyTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    fprintf(stderr, "MGL: glCopyTexSubImage1D called (stub)\n");
    // Stub - 1D textures rarely used
}

void mglCopyTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    fprintf(stderr, "MGL: glCopyTexSubImage2D called - target=0x%x %dx%d at (%d,%d) from (%d,%d)\n",
            target, width, height, xoffset, yoffset, x, y);
    
    // Get the bound texture
    Texture *tex = getTex(ctx, 0, target);
    if (!tex)
    {
        fprintf(stderr, "MGL ERROR: glCopyTexSubImage2D - no texture bound\n");
        return;
    }
    
    // This copies from the current read framebuffer to the texture
    // For now, use the Metal blit function
    ctx->mtl_funcs.mtlCopyTexSubImage(ctx, tex, level, xoffset, yoffset, x, y, width, height);
}

void mglCopyTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    fprintf(stderr, "MGL: glCopyTexSubImage3D called - stub, only 2D copy supported\n");
    // For now just do 2D copy, ignoring zoffset
    Texture *tex = getTex(ctx, 0, target);
    if (tex)
    {
        ctx->mtl_funcs.mtlCopyTexSubImage(ctx, tex, level, xoffset, yoffset, x, y, width, height);
    }
}

void mglCopyTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    fprintf(stderr, "MGL: glCopyTextureSubImage1D called (stub)\n");
}

void mglCopyTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    fprintf(stderr, "MGL: glCopyTextureSubImage2D called - texture=%u %dx%d\n", texture, width, height);
    Texture *tex = getTex(ctx, texture, 0);
    if (tex)
    {
        ctx->mtl_funcs.mtlCopyTexSubImage(ctx, tex, level, xoffset, yoffset, x, y, width, height);
    }
}

void mglCopyTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    fprintf(stderr, "MGL: glCopyTextureSubImage3D called (stub)\n");
    Texture *tex = getTex(ctx, texture, 0);
    if (tex) {
        ctx->mtl_funcs.mtlCopyTexSubImage(ctx, tex, level, xoffset, yoffset, x, y, width, height);
    }
}

#pragma mark get tex image

void mglGetTexImage(GLMContext ctx, GLenum target, GLuint level, GLenum format, GLenum type, void *pixels)
{
    fprintf(stderr, "MGL: glGetTexImage called - target=0x%x level=%d format=0x%x type=0x%x\n",
            target, level, format, type);
    
    Texture *tex = getTex(ctx, 0, target);
    if (!tex)
    {
        fprintf(stderr, "MGL ERROR: glGetTexImage - no texture bound\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    if (!tex->mtl_data)
    {
        fprintf(stderr, "MGL ERROR: glGetTexImage - texture has no Metal data\n");
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    if (level >= tex->num_levels)
    {
        fprintf(stderr, "MGL ERROR: glGetTexImage - invalid level %d (max %d)\n", level, tex->num_levels);
        ERROR_RETURN(GL_INVALID_VALUE);
    }
    
    // Calculate dimensions for this level
    GLsizei width = tex->width >> level;
    GLsizei height = tex->height >> level;
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    
    // Calculate bytes per row
    size_t pixel_size = sizeForFormatType(format, type);
    size_t bytesPerRow = width * pixel_size;
    size_t bytesPerImage = bytesPerRow * height;
    
    fprintf(stderr, "MGL: glGetTexImage - reading %dx%d, bytesPerRow=%zu\n", width, height, bytesPerRow);
    
    // Use the Metal function to read the texture
    ctx->mtl_funcs.mtlGetTexImage(ctx, tex, pixels, (GLuint)bytesPerRow, (GLuint)bytesPerImage, 0, 0, width, height, level, 0);
}

void mglGetTextureImage(GLMContext ctx, GLuint texture, GLuint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    fprintf(stderr, "MGL: glGetTextureImage called - texture=%u level=%d\n", texture, level);
    
    Texture *tex = getTex(ctx, texture, 0);
    if (!tex) {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    if (!tex->mtl_data)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    GLuint width = tex->width >> level;
    GLuint height = tex->height >> level;
    
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    
    GLsizei pixel_size = sizeForFormatType(format, type);
    GLsizei bytesPerRow = width * pixel_size;
    GLsizei bytesPerImage = bytesPerRow * height;
    
    if (bytesPerImage > bufSize)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    ctx->mtl_funcs.mtlGetTexImage(ctx, tex, pixels, (GLuint)bytesPerRow, (GLuint)bytesPerImage, 0, 0, (GLuint)width, (GLuint)height, level, 0);
}

void mglGetTextureSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    fprintf(stderr, "MGL: glGetTextureSubImage called - texture=%u\n", texture);
    
    Texture *tex = getTex(ctx, texture, 0);
    if (!tex || !tex->mtl_data)
    {
        ERROR_RETURN(GL_INVALID_OPERATION);
    }
    
    size_t pixel_size = sizeForFormatType(format, type);
    size_t bytesPerRow = width * pixel_size;
    size_t bytesPerImage = bytesPerRow * height;
    
    ctx->mtl_funcs.mtlGetTexImage(ctx, tex, pixels, (GLuint)bytesPerRow, (GLuint)bytesPerImage, xoffset, yoffset, width, height, level, zoffset);
}

void mglGetCompressedTexImage(GLMContext ctx, GLenum target, GLint level, void *img)
{
    fprintf(stderr, "MGL WARNING: glGetCompressedTexImage called (stub) - compressed textures not supported\n");
}

void mglGetnCompressedTexImage(GLMContext ctx, GLenum target, GLint lod, GLsizei bufSize, void *pixels)
{
    fprintf(stderr, "MGL WARNING: glGetnCompressedTexImage called (stub) - compressed textures not supported\n");
}

void mglGetCompressedTextureSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels)
{
    fprintf(stderr, "MGL WARNING: glGetCompressedTextureSubImage called (stub) - compressed textures not supported\n");
}

void mglTextureView(GLMContext ctx, GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    fprintf(stderr, "MGL WARNING: glTextureView called (stub) - texture views not supported\n");
}

void mglTextureBuffer(GLMContext ctx, GLuint texture, GLenum internalformat, GLuint buffer)
{
    fprintf(stderr, "MGL WARNING: glTextureBuffer called (stub)\n");
}

void mglTextureBufferRange(GLMContext ctx, GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    fprintf(stderr, "MGL WARNING: glTextureBufferRange called (stub)\n");
}

void mglCompressedTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    fprintf(stderr, "MGL WARNING: glCompressedTextureSubImage1D called (stub)\n");
}

void mglCompressedTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    fprintf(stderr, "MGL WARNING: glCompressedTextureSubImage2D called (stub)\n");
}

void mglCompressedTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    fprintf(stderr, "MGL WARNING: glCompressedTextureSubImage3D called (stub)\n");
}

void mglGetCompressedTextureImage(GLMContext ctx, GLuint texture, GLint level, GLsizei bufSize, void *pixels)
{
    fprintf(stderr, "MGL WARNING: glGetCompressedTextureImage called (stub)\n");
}

void mglGetTextureLevelParameteriv(GLMContext ctx, GLuint texture, GLint level, GLenum pname, GLint *params)
{
    Texture *tex = getTex(ctx, texture, 0);
    if (!tex) {
        *params = 0;
        return;
    }
    
    GLsizei width = tex->width >> level;
    GLsizei height = tex->height >> level;
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    
    switch (pname) {
        case GL_TEXTURE_WIDTH:
            *params = width;
            break;
        case GL_TEXTURE_HEIGHT:
            *params = height;
            break;
        case GL_TEXTURE_DEPTH:
            *params = tex->depth >> level;
            if (*params < 1) *params = 1;
            break;
        case GL_TEXTURE_INTERNAL_FORMAT:
            *params = tex->internalformat;
            break;
        default:
            fprintf(stderr, "MGL: glGetTextureLevelParameteriv pname=0x%x not implemented\n", pname);
            *params = 0;
            break;
    }
}

void mglGetTextureLevelParameterfv(GLMContext ctx, GLuint texture, GLint level, GLenum pname, GLfloat *params)
{
    GLint iparams;
    mglGetTextureLevelParameteriv(ctx, texture, level, pname, &iparams);
    *params = (GLfloat)iparams;
}

void mglGetTextureParameterfv(GLMContext ctx, GLuint texture, GLenum pname, GLfloat *params)
{
    fprintf(stderr, "MGL: glGetTextureParameterfv called (stub) pname=0x%x\n", pname);
    *params = 0.0f;
}

void mglGetTextureParameterIiv(GLMContext ctx, GLuint texture, GLenum pname, GLint *params)
{
    fprintf(stderr, "MGL: glGetTextureParameterIiv called (stub) pname=0x%x\n", pname);
    *params = 0;
}

void mglGetTextureParameterIuiv(GLMContext ctx, GLuint texture, GLenum pname, GLuint *params)
{
    fprintf(stderr, "MGL: glGetTextureParameterIuiv called (stub) pname=0x%x\n", pname);
    *params = 0;
}

void mglGetTextureParameteriv(GLMContext ctx, GLuint texture, GLenum pname, GLint *params)
{
    fprintf(stderr, "MGL: glGetTextureParameteriv called (stub) pname=0x%x\n", pname);
    *params = 0;
}

void mglGetTexParameterIiv(GLMContext ctx, GLenum target, GLenum pname, GLint *params)
{
    fprintf(stderr, "MGL: glGetTexParameterIiv called (stub) pname=0x%x\n", pname);
    *params = 0;
}

void mglGetTexParameterIuiv(GLMContext ctx, GLenum target, GLenum pname, GLuint *params)
{
    fprintf(stderr, "MGL: glGetTexParameterIuiv called (stub) pname=0x%x\n", pname);
    *params = 0;
}

void mglSampleCoverage(GLMContext ctx, GLfloat value, GLboolean invert)
{
    // Stub - sample coverage is a hint for multisampling
    fprintf(stderr, "MGL: glSampleCoverage called (stub) value=%f invert=%d\n", value, invert);
}

