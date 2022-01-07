//
//  textures.c
//  mgl
//
//  Created by Michael Larson on 11/1/21.
//

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <Accelerate/Accelerate.h>

#include "glm_context.h"
#include "pixel_utils.h"
#include "utils.h"

#ifndef MAX
#define MAX(_A_, _B_)   (_A_ > _B_ ? _A_ : _B_)
#endif

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
    assert(ptr);

    bzero(ptr, sizeof(Texture));

    ptr->name = TEX_OBJ_RES_NAME;
    ptr->target = target;
    ptr->index = index;

    float black_colr[] = {0,0,0,0};

    ptr->params.depth_stencil_mode = GL_DEPTH_COMPONENT;
    ptr->params.base_level = 0;
    memcpy(ptr->params.border_color, black_colr, 4 * sizeof(float));
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

static int isTexuture(GLMContext ctx, GLuint texture)
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
        (assert(ptr));
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
    STATE(dirty_textures) |= (0x1 << index);
}


void mglDeleteTextures(GLMContext ctx, GLsizei n, const GLuint *textures)
{
    // Unimplemented function
    assert(0);
}

GLboolean mglIsTexture(GLMContext ctx, GLuint texture)
{
    return isTexuture(ctx, texture);
}

void mglInvalidateTexImage(GLMContext ctx, GLuint texture, GLint level)
{
    // Unimplemented function
    assert(0);
}

void mglBindSamplers(GLMContext ctx, GLuint first, GLsizei count, const GLuint *samplers)
{
    // Unimplemented function
    assert(0);
}

void mglSampleCoverage(GLMContext ctx, GLfloat value, GLboolean invert)
{
    // Unimplemented function
    assert(0);
}

void mglBindImageTextures(GLMContext ctx, GLuint first, GLsizei count, const GLuint *textures)
{
    // Unimplemented function
    assert(0);
}

void mglClientActiveTexture(GLMContext ctx, GLenum texture)
{
    // Unimplemented function
    assert(0);
}

void mglActiveTexture(GLMContext ctx, GLenum texture)
{
    if ((texture - GL_TEXTURE0) > STATE_VAR(max_combined_texture_image_units))
    {
        assert(0);
    }

    STATE(active_texture) = texture - GL_TEXTURE0;
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
        ctx->mtl_funcs.mtlDeleteMTLTexture(ctx, tex->mtl_data);
    }

    for(int i=0; i<tex->num_levels; i++)
    {
        if (tex->levels[i].complete)
        {
            if (tex->levels[i].data)
            {
                vm_deallocate(mach_host_self(), tex->levels[i].data, tex->levels[i].data);
            }
        }
    }

    free(tex->levels);

    bzero(tex, sizeof(Texture));
}

void initBaseTexLevel(GLMContext ctx, Texture *tex, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    tex->mipmapped = 0;
    tex->num_levels = ilog2(MAX(width, height));
    tex->levels = (TextureLevel *)calloc(tex->num_levels, sizeof(TextureLevel));

    tex->width = width;
    tex->height = height;
    tex->depth = depth;
    tex->internalformat = internalformat;
    tex->complete = false;

    for(int i=0; i<tex->num_levels; i++)
    {
        tex->levels[i].complete = false;
    }
}

bool checkTexLevelParams(GLMContext ctx, Texture *tex, GLint level, GLsizei width, GLsizei height, GLsizei depth)
{
    GLuint base_width, base_height, base_depth;

    base_width = tex->width;
    base_height = tex->height;
    base_depth = tex->depth;

    while(level--)
    {
        base_width >>= 1;
        base_height >>= 1;
        base_depth>>= 1;
    }

    if (width != base_width || height != base_height || depth != base_depth)
    {
        return false;
    }

    return true;
}

void createTextureLevel(GLMContext ctx, Texture *tex, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    if (level == 0)
    {
        if (tex->num_levels == 0)
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
    else if (checkTexLevelParams(ctx, tex, level, width, height, depth) == false)
    {
        // umm leave?
        ERROR_RETURN(GL_INVALID_OPERATION);
    }

    tex->levels[level].internalformat = internalformat;
    tex->levels[level].format = format;
    tex->levels[level].type = type;
    tex->levels[level].width = width;
    tex->levels[level].height = height;
    tex->levels[level].depth = depth;

    kern_return_t err;
    vm_address_t texture_data;
    size_t pixel_size;
    size_t internal_size;
    size_t texture_size;

    pixel_size = sizeForInternalFormat(internalformat, format, type);
    ERROR_CHECK_RETURN(pixel_size, GL_INVALID_ENUM);

    if (ctx->state.unpack.row_length)
    {
        size_t alignment;

        ERROR_CHECK_RETURN((ctx->state.unpack.row_length >> level) >= width, GL_INVALID_VALUE);

        alignment = ctx->state.unpack.alignment;
        if (alignment)
        {
            if (pixel_size >= alignment)
            {
                tex->levels[level].pitch = pixel_size * (ctx->state.unpack.row_length >> level);
            }
            else if (height)
            {
                size_t pitch;

                pitch = alignment / pixel_size;

                pitch = pitch * pixel_size * ctx->state.unpack.row_length * height;

                tex->levels[level].pitch = pitch / alignment;
            }
            else
            {
                size_t pitch;

                pitch = alignment / pixel_size;

                pitch = pitch * pixel_size * ctx->state.unpack.row_length;

                tex->levels[level].pitch = pitch / alignment;
            }
        }
        else
        {
            tex->levels[level].pitch = pixel_size * (ctx->state.unpack.row_length >> level);
        }
    }
    else
    {
        tex->levels[level].pitch = pixel_size * width;
    }

    internal_size = tex->levels[level].pitch;

    if (height)
        internal_size *= height;

    if (depth)
        internal_size *= depth;

    texture_size = page_size_align(internal_size);
    assert(texture_size);

    // Allocate directly from VM
    err = vm_allocate((vm_map_t) mach_task_self(),
                      (vm_address_t*) &texture_data,
                      texture_size,
                      VM_FLAGS_ANYWHERE);
    assert(err == 0);
    assert(texture_data);

    tex->levels[0].data_size = texture_size;
    tex->levels[0].data = (vm_address_t)texture_data;

    if (pixels)
    {
        GLsizei src_size;

        src_size = width * sizeForFormatType(format, type);

        if (height)
            src_size *= height;

        if (depth)
            src_size *= depth;

        if (ctx->state.buffers[_PIXEL_UNPACK_BUFFER])
        {
            Buffer *ptr;

            ptr = ctx->state.buffers[_PIXEL_UNPACK_BUFFER];

            if ((GLsizei)pixels + src_size > ptr->size)
            {
                ERROR_RETURN(GL_INVALID_OPERATION);
            }

            GLubyte *buffer_data;

            buffer_data = (GLubyte *)ptr->data.buffer_data;
            assert(buffer_data);

            buffer_data += (GLsizei)pixels;

            memcpy((void *)texture_data, pixels, src_size);
        }
        else
        {
            memcpy((void *)texture_data, pixels, src_size);
        }

        tex->dirty_bits |= DIRTY_TEXTURE_DATA;
    };

    tex->dirty_bits |= DIRTY_TEXTURE_LEVEL;
    STATE(dirty_bits) |= DIRTY_TEX;
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
        assert(ptr);
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


bool verifyFormatType(GLMContext ctx, GLint internalformat, GLenum format, GLenum type)
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

        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            ERROR_CHECK_RETURN_VALUE(format == GL_DEPTH_COMPONENT, GL_INVALID_OPERATION, false);
            break;

        default:
            ERROR_RETURN_VALUE(GL_INVALID_ENUM, false);
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
            break;

        default:
            ERROR_RETURN_VALUE(GL_INVALID_ENUM, false);
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
            ERROR_CHECK_RETURN_VALUE((format == GL_RGBA || format == GL_BGRA), GL_INVALID_OPERATION, false);
            break;
    }

    return true;
}

void mglTexImage1D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLuint index;
    Texture *tex;

    switch(target)
    {
        case GL_TEXTURE_1D:
        case GL_PROXY_TEXTURE_1D:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(level >=0, GL_INVALID_VALUE);

    // verifyFormatType sets the error
    ERROR_CHECK_RETURN(verifyFormatType(ctx, internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >=0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(border == 0, GL_INVALID_VALUE);

    if (ctx->state.buffers[_PIXEL_UNPACK_BUFFER])
    {
        Buffer *ptr;

        ptr = ctx->state.buffers[_PIXEL_UNPACK_BUFFER];

        ERROR_CHECK_RETURN(ptr->mapped == false, GL_INVALID_OPERATION);
    }

    index = textureIndexFromTarget(ctx, target);

    tex = currentTexture(ctx, index);

    if (tex == NULL)
    {
        assert(0);

        return;
    }

    createTextureLevel(ctx, tex, level, internalformat, width, 1, 1, 0, format, type, pixels);
}


void mglTexImage2D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLuint index;
    Texture *tex;

    index = textureIndexFromTarget(ctx, target);

    tex = currentTexture(ctx, index);

    if (tex == NULL)
    {
        assert(0);

        return;
    }

    createTextureLevel(ctx, tex, level, internalformat, width, height, 1, 0, format, type, pixels);
}

void mglTexImage3D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLuint index;
    Texture *tex;

    index = textureIndexFromTarget(ctx, target);

    tex = currentTexture(ctx, index);

    if (tex == NULL)
    {
        assert(0);

        return;
    }

    createTextureLevel(ctx, tex, level, internalformat, width, height, depth, 0, format, type, pixels);
}

void mglTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}
void mglTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglClearTexImage(GLMContext ctx, GLuint texture, GLint level, GLenum format, GLenum type, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglClearTexSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)
{
    // Unimplemented function
    assert(0);
}
void mglCopyTexImage1D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTexImage2D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglTexStorage1D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    // Unimplemented function
    assert(0);
}

void mglTexStorage2D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglTexStorage3D(GLMContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexImage3D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexImage2D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexImage1D(GLMContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexSubImage3D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexSubImage2D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTexSubImage1D(GLMContext ctx, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglGetCompressedTexImage(GLMContext ctx, GLenum target, GLint level, void *img)
{
    // Unimplemented function
    assert(0);
}
void mglTexImage2DMultisample(GLMContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    // Unimplemented function
    assert(0);
}

void mglTexImage3DMultisample(GLMContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    // Unimplemented function
    assert(0);
}
void mglGetnCompressedTexImage(GLMContext ctx, GLenum target, GLint lod, GLsizei bufSize, void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglGetCompressedTextureSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglBindImageTexture(GLMContext ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    // Unimplemented function
    assert(0);
}


void mglInvalidateTexSubImage(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    // Unimplemented function
    assert(0);
}

void mglTextureView(GLMContext ctx, GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    // Unimplemented function
    assert(0);
}

void mglCreateTextures(GLMContext ctx, GLenum target, GLsizei n, GLuint *textures)
{
    // Unimplemented function
    assert(0);
}

void mglTextureBuffer(GLMContext ctx, GLuint texture, GLenum internalformat, GLuint buffer)
{
    // Unimplemented function
    assert(0);
}

void mglTextureBufferRange(GLMContext ctx, GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    // Unimplemented function
    assert(0);
}

void mglTextureStorage1D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    // Unimplemented function
    assert(0);
}

void mglTextureStorage2D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglTextureStorage3D(GLMContext ctx, GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    // Unimplemented function
    assert(0);
}

void mglTextureStorage2DMultisample(GLMContext ctx, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    // Unimplemented function
    assert(0);
}

void mglTextureStorage3DMultisample(GLMContext ctx, GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    // Unimplemented function
    assert(0);
}

void mglTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCompressedTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTextureSubImage1D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTextureSubImage2D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglCopyTextureSubImage3D(GLMContext ctx, GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglGenerateTextureMipmap(GLMContext ctx, GLuint texture)
{
    // Unimplemented function
    assert(0);
}

void mglGetCompressedTextureImage(GLMContext ctx, GLuint texture, GLint level, GLsizei bufSize, void *pixels)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureLevelParameterfv(GLMContext ctx, GLuint texture, GLint level, GLenum pname, GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureLevelParameteriv(GLMContext ctx, GLuint texture, GLint level, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureParameterfv(GLMContext ctx, GLuint texture, GLenum pname, GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureParameterIiv(GLMContext ctx, GLuint texture, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureParameterIuiv(GLMContext ctx, GLuint texture, GLenum pname, GLuint *params)
{
    // Unimplemented function
    assert(0);
}

void mglGetTextureParameteriv(GLMContext ctx, GLuint texture, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglCreateSamplers(GLMContext ctx, GLsizei n, GLuint *samplers)
{
    // Unimplemented function
    assert(0);
}
