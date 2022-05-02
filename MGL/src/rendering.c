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
 * rendering.c
 * MGL
 *
 */

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <limits.h>

#include "mgl.h"

#include "pixel_utils.h"
#include "glm_context.h"

void mglClear(GLMContext ctx, GLbitfield mask)
{
    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        assert(0);
    }

    ctx->state.clear_bitmask = mask;
}

void mglClearColor(GLMContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    ctx->state.color_clear_value[0] = red;
    ctx->state.color_clear_value[1] = blue;
    ctx->state.color_clear_value[2] = green;
    ctx->state.color_clear_value[3] = alpha;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglClearStencil(GLMContext ctx, GLint s)
{
    ctx->state.var.stencil_clear_value = s;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglClearDepth(GLMContext ctx, GLdouble depth)
{
    ctx->state.var.depth_clear_value = depth;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglClearBufferfv(GLMContext ctx, GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    Framebuffer * fbo = ctx->state.framebuffer;
    FBOAttachment * fboa;

    switch (buffer) {
        case GL_COLOR:
            fboa = &fbo->color_attachments[drawbuffer];
            fboa->clear_bitmask |= GL_COLOR_BUFFER_BIT;
            fboa->clear_color[0] = value[0];
            fboa->clear_color[1] = value[1];
            fboa->clear_color[2] = value[2];
            fboa->clear_color[3] = value[3];
            break;
        case GL_DEPTH:
            fboa = &fbo->depth;
            fboa->clear_bitmask |= GL_DEPTH_BUFFER_BIT;
            fboa->clear_color[0] = value[0];
            break;
        case GL_STENCIL:
            fboa = &fbo->stencil;
            fboa->clear_bitmask |= GL_STENCIL_BUFFER_BIT;
            fboa->clear_color[0] = value[0];
            break;
        default:
            break;
    }
}

void mglClearBufferfi(GLMContext ctx, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    Framebuffer * fbo = ctx->state.framebuffer;
    FBOAttachment * fboa;

    switch (buffer) {
        case GL_DEPTH_STENCIL:
            fboa = &fbo->depth;
            fboa->clear_bitmask |= GL_DEPTH_BUFFER_BIT;
            fboa->clear_color[0] = depth;

            fboa = &fbo->stencil;
            fboa->clear_bitmask |= GL_STENCIL_BUFFER_BIT;
            fboa->clear_color[0] = stencil;
            break;
        default:
            break;
    }
}

void mglFinish(GLMContext ctx)
{
    ctx->mtl_funcs.mtlFlush(ctx, true);
}

void mglFlush(GLMContext ctx)
{
    ctx->mtl_funcs.mtlFlush(ctx, false);
}

void mglDrawBuffers(GLMContext ctx, GLsizei n, const GLenum *bufs)
{
    for (GLsizei i=0; i<n; ++i) {
        mglDrawBuffer(ctx, bufs[i]);
    }
}

void mglDrawBuffer(GLMContext ctx, GLenum buf)
{
    switch(buf)
    {
        case GL_FRONT:
            break;

        case GL_NONE:
        case GL_FRONT_LEFT:
        case GL_FRONT_RIGHT:
        case GL_BACK_LEFT:
        case GL_BACK_RIGHT:
        case GL_LEFT:
        case GL_RIGHT:
        case GL_FRONT_AND_BACK:
            // not interested in these yet
            assert(0);
            break;
    }

    if ((buf >= GL_COLOR_ATTACHMENT0) &&
        (buf <= (GL_COLOR_ATTACHMENT0 + STATE(max_color_attachments))))
    {
        // probably should validate current fbo..
        Framebuffer * fbo = ctx->state.framebuffer;
        fbo->color_attachments[buf-GL_COLOR_ATTACHMENT0].buf.rbo->is_draw_buffer = GL_TRUE;
    }

    STATE(draw_buffer) = buf;
    STATE(dirty_bits) |= DIRTY_STATE;
}

void mglReadBuffer(GLMContext ctx, GLenum buf)
{
    switch(buf)
    {
        case GL_FRONT:
            break;

        case GL_NONE:
        case GL_FRONT_LEFT:
        case GL_FRONT_RIGHT:
        case GL_BACK_LEFT:
        case GL_BACK_RIGHT:
        case GL_LEFT:
        case GL_RIGHT:
        case GL_FRONT_AND_BACK:
            // not interested in these yet
            assert(0);
            break;
    }

    if ((buf >= GL_COLOR_ATTACHMENT0) &&
        (buf <= (GL_COLOR_ATTACHMENT0 + STATE(max_color_attachments))))
    {
        // probably should validate current fbo..
    }

    STATE(read_buffer) = buf;
    STATE(dirty_bits) |= DIRTY_STATE;
}

void mglPixelStorei(GLMContext ctx, GLenum pname, GLint param)
{
    ERROR_CHECK_RETURN(param >= 0, GL_INVALID_VALUE);

    switch(pname)
    {
        case GL_PACK_SWAP_BYTES:
            ctx->state.pack.swap_bytes = (param != 0 ? true : false);
            break;

        case GL_PACK_LSB_FIRST:
            ctx->state.pack.lsb_first = (param != 0 ? true : false);
            break;

        case GL_PACK_ROW_LENGTH:
            ctx->state.pack.row_length = param;
            break;

        case GL_PACK_IMAGE_HEIGHT:
            ctx->state.pack.image_height = param;
            break;

        case GL_PACK_SKIP_ROWS:
            ctx->state.pack.skip_rows = param;
            break;

        case GL_PACK_SKIP_PIXELS:
            ctx->state.pack.skip_rows = param;
            break;

        case GL_PACK_SKIP_IMAGES:
            ctx->state.pack.skip_images = param;
            break;

        case GL_PACK_ALIGNMENT:
            switch(param)
            {
                case 1:
                case 2:
                case 4:
                case 8:
                    ctx->state.pack.alignment = param;
                    break;

                default:
                    ERROR_RETURN(GL_INVALID_VALUE);
                    break;
            }
            break;

        case GL_UNPACK_SWAP_BYTES:
            ctx->state.unpack.swap_bytes = (param != 0 ? true : false);
            break;

        case GL_UNPACK_LSB_FIRST:
            ctx->state.unpack.lsb_first = (param != 0 ? true : false);
            break;

        case GL_UNPACK_ROW_LENGTH:
            ctx->state.unpack.row_length = param;
            break;

        case GL_UNPACK_IMAGE_HEIGHT:
            ctx->state.unpack.image_height = param;
            break;

        case GL_UNPACK_SKIP_ROWS:
            ctx->state.unpack.skip_rows = param;
            break;

        case GL_UNPACK_SKIP_PIXELS:
            ctx->state.unpack.skip_rows = param;
            break;

        case GL_UNPACK_SKIP_IMAGES:
            ctx->state.unpack.skip_images = param;
            break;

        case GL_UNPACK_ALIGNMENT:
            switch(param)
            {
                case 1:
                case 2:
                case 4:
                case 8:
                    ctx->state.unpack.alignment = param;
                    break;

                default:
                    ERROR_RETURN(GL_INVALID_VALUE);
                    break;
            }
            break;
    }
}

void mglPixelStoref(GLMContext ctx, GLenum pname, GLfloat param)
{
    mglPixelStorei(ctx, pname, (GLint)param);
}

//     void (*mtlGetTexImage)(GLMContext glm_ctx, Texture *tex, void *pixelBytes, GLuint bytesPerRow, GLuint bytesPerImage, GLint x, GLint y, GLsizei width, GLsizei height, GLuint level, GLuint slice);

void mglReadPixels(GLMContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
    GLuint pixel_size;

    pixel_size = sizeForFormatType(format, type);
    ERROR_CHECK_RETURN(pixel_size != 0, GL_INVALID_ENUM);

    ERROR_CHECK_RETURN(width > 0, GL_INVALID_ENUM);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_ENUM);

    switch(format)
    {
        case GL_STENCIL_INDEX:
            ERROR_CHECK_RETURN(ctx->stencil_format.mtl_pixel_format > 0, GL_INVALID_OPERATION);
            break;

        case GL_DEPTH_COMPONENT:
            ERROR_CHECK_RETURN(ctx->depth_format.mtl_pixel_format > 0, GL_INVALID_OPERATION);
            break;

        case GL_DEPTH_STENCIL:
            ERROR_CHECK_RETURN((ctx->depth_format.mtl_pixel_format > 0) ||
                               (ctx->stencil_format.mtl_pixel_format > 0), GL_INVALID_OPERATION);
            switch(type)
            {
                case GL_UNSIGNED_INT_24_8:
                case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                    break;

                default:
                    ERROR_RETURN(GL_INVALID_ENUM);
                    break;
            }
            break;

        default:
            break;
    }

    switch(type)
    {
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            ERROR_CHECK_RETURN(format == GL_RGB || format == GL_BGR, GL_INVALID_OPERATION);
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            ERROR_CHECK_RETURN(format == GL_RGBA || format == GL_BGRA, GL_INVALID_OPERATION);
            break;
    }

    GLuint pitch;
    if (ctx->state.pack.row_length)
    {
        pitch = ctx->state.pack.row_length * pixel_size;
    }
    else
    {
        pitch = (width - x) * pixel_size;
    }

    size_t buffer_size;
    buffer_size = pitch * (height - y);

    if (! ctx->state.buffers[_PIXEL_PACK_BUFFER])
    {
        // in client memory
        ctx->mtl_funcs.mtlGetTexImage(ctx, NULL, (void *)pixels, pitch, (GLuint)buffer_size, x, y, width, height, 0, 0);
    }
    else
    {
        // in PBO
        Buffer *ptr;

        ptr = ctx->state.buffers[_PIXEL_PACK_BUFFER];

        ERROR_CHECK_RETURN(ptr->mapped == false, GL_INVALID_OPERATION);

        size_t size_req;

        size_req = pixel_size * (width - x) * (height - y);

        ERROR_CHECK_RETURN(ptr->size >= size_req, GL_INVALID_OPERATION);

        vm_address_t buffer_data;
        buffer_data = ptr->data.buffer_data;

        if (pixels) {
            // GL_INVALID_OPERATION is generated if a non-zero buffer object name is bound to the GL_PIXEL_PACK_BUFFER target and data is not evenly divisible into the number of bytes needed to store in memory a datum indicated by type.

            uint64_t alignment_test;
            uint32_t alignment;

            buffer_data += (const char*)pixels-(const char*)NULL;

            alignment_test = (uint64_t)buffer_data;
            alignment_test = ~alignment_test;

            alignment = 0;
            // count number of lower zero bits for alignment
            while( ((alignment_test & 0x1) == 1) && alignment >= pixel_size)
            {
                alignment++;
                alignment_test >>= 1;
            }

            ERROR_CHECK_RETURN(alignment >= pixel_size, GL_INVALID_OPERATION);
        }

        //ctx->mtl_funcs.mtlGetTexImage(ctx, NULL, (void *)buffer_data, pitch, (GLuint)buffer_size, x, y, width, height, 0, 0);
        ctx->mtl_funcs.mtlGetTexImageAsync(ctx, NULL, pitch, (GLuint)buffer_size, x, y, width, height, 0, 0);
    }
}

