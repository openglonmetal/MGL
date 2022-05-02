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
 * framebuffers.c
 * MGL
 *
 */

#include <stdio.h>
#include <string.h>

#include "glm_context.h"
#include "pixel_utils.h"
#include "utils.h"

#define RENDBUF_STATE(_val_)    ctx->state.renderbuffer->_val_

extern GLuint textureIndexFromTarget(GLMContext ctx, GLenum target);
extern Texture *newTexObj(GLMContext ctx, GLenum target);
extern Texture *findTexture(GLMContext ctx, GLuint texture);


#pragma mark renderbuffer logic

static Renderbuffer *newRenderbuffer(GLMContext ctx, GLuint renderbuffer)
{
    Renderbuffer *ptr;

    ptr = (Renderbuffer *)malloc(sizeof(Renderbuffer));
    assert(ptr);

    bzero(ptr, sizeof(Renderbuffer));

    ptr->name = renderbuffer;

    return ptr;
}

static Renderbuffer *getRenderbuffer(GLMContext ctx, GLuint renderbuffer)
{
    Renderbuffer *ptr;

    ptr = (Renderbuffer *)searchHashTable(&STATE(renderbuffer_table), renderbuffer);

    if (!ptr)
    {
        ptr = newRenderbuffer(ctx, renderbuffer);

        insertHashElement(&STATE(renderbuffer_table), renderbuffer, ptr);
    }

    return ptr;
}

static int isRenderBuffer(GLMContext ctx, GLuint renderbuffer)
{
    Renderbuffer *ptr;

    ptr = (Renderbuffer *)searchHashTable(&STATE(renderbuffer_table), renderbuffer);

    if (ptr)
        return 1;

    return 0;
}

Renderbuffer *findRenderbuffer(GLMContext ctx, GLuint renderbuffer)
{
    Renderbuffer *ptr;

    ptr = (Renderbuffer *)searchHashTable(&STATE(renderbuffer_table), renderbuffer);

    return ptr;
}

Framebuffer *currentFBOForType(GLMContext ctx, GLenum target)
{
    switch(target)
    {
        case GL_FRAMEBUFFER:
        case GL_DRAW_FRAMEBUFFER:
            return ctx->state.framebuffer;
            break;

        case GL_READ_FRAMEBUFFER:
            return ctx->state.readbuffer;
            break;

        default: assert(0); break;
    }
}

#pragma mark framebuffer logic
static Framebuffer *newFramebuffer(GLMContext ctx, GLuint framebuffer)
{
    Framebuffer *ptr;

    ptr = (Framebuffer *)malloc(sizeof(Framebuffer));
    assert(ptr);

    bzero(ptr, sizeof(Framebuffer));

    ptr->name = framebuffer;

    return ptr;
}

static Framebuffer *getFramebuffer(GLMContext ctx, GLuint framebuffer)
{
    Framebuffer *ptr;

    ptr = (Framebuffer *)searchHashTable(&STATE(framebuffer_table), framebuffer);

    if (!ptr)
    {
        ptr = newFramebuffer(ctx, framebuffer);

        insertHashElement(&STATE(framebuffer_table), framebuffer, ptr);
    }

    return ptr;
}

static int isFramebuffer(GLMContext ctx, GLuint framebuffer)
{
    Framebuffer *ptr;

    ptr = (Framebuffer *)searchHashTable(&STATE(framebuffer_table), framebuffer);

    if (ptr)
        return 1;

    return 0;
}

Framebuffer *findFrameBuffer(GLMContext ctx, GLuint framebuffer)
{
    Framebuffer *ptr;

    ptr = (Framebuffer *)searchHashTable(&STATE(framebuffer_table), framebuffer);

    return ptr;
}

#pragma mark Framebuffer calls
GLboolean mglIsFramebuffer(GLMContext ctx, GLuint framebuffer)
{
    return isFramebuffer(ctx, framebuffer);
}

void mglGenFramebuffers(GLMContext ctx, GLsizei n, GLuint *framebuffers)
{
    assert(framebuffers);

    while(n--)
    {
        *framebuffers++ = getNewName(&STATE(framebuffer_table));
    }
}

void mglBindFramebuffer(GLMContext ctx, GLenum target, GLuint framebuffer)
{
    Framebuffer *ptr;

    if(framebuffer)
    {
        ptr = getFramebuffer(ctx, framebuffer);
    }
    else
    {
        ptr = NULL;
    }

    switch(target) {
        case GL_DRAW_FRAMEBUFFER:
            ctx->state.framebuffer = ptr;
            break;

        case GL_READ_FRAMEBUFFER:
            ctx->state.readbuffer = ptr;
            break;

        case GL_FRAMEBUFFER:
            ctx->state.framebuffer = ptr;
            ctx->state.readbuffer = ptr;
            break;
    }
    
    STATE(dirty_bits) |= DIRTY_FBO;
}

void mglDeleteFramebuffers(GLMContext ctx, GLsizei n, const GLuint *framebuffers)
{
    // Unimplemented function
    assert(0);
}

GLenum  mglCheckFramebufferStatus(GLMContext ctx, GLenum target)
{
    Framebuffer *fbo;
    Texture *tex;
    GLuint level;
    GLuint width, height;

    fbo = currentFBOForType(ctx, target);

    if (fbo->color_attachments[0].textarget == GL_RENDERBUFFER)
    {
        tex = fbo->color_attachments[0].buf.rbo->tex;
    }
    else
    {
        tex = fbo->color_attachments[0].buf.tex;
    }

    level = fbo->color_attachments[0].level;

    width = tex->faces[0].levels[level].width;
    height = tex->faces[0].levels[level].height;

    for(int i=1; i<STATE(max_color_attachments);i++)
    {
        if (fbo->color_attachments[i].textarget == GL_RENDERBUFFER)
        {
            tex = fbo->color_attachments[i].buf.rbo->tex;
        }
        else
        {
            tex = fbo->color_attachments[i].buf.tex;
        }

        if (tex)
        {
            level = fbo->color_attachments[i].level;
            width = tex->faces[0].levels[level].width;
            height = tex->faces[0].levels[level].height;
        }
    }

    printf("%s need to fix this function\n", __FUNCTION__);

    return GL_FRAMEBUFFER_COMPLETE;
}

#pragma mark Renderbuffer calls
GLboolean mglIsRenderbuffer(GLMContext ctx, GLuint renderbuffer)
{
    return isRenderBuffer(ctx, renderbuffer);
}

void mglGenRenderbuffers(GLMContext ctx, GLsizei n, GLuint *renderbuffers)
{
    assert(renderbuffers);

    while(n--)
    {
        *renderbuffers++ = getNewName(&STATE(renderbuffer_table));
    }
}

void mglBindRenderbuffer(GLMContext ctx, GLenum target, GLuint renderbuffer)
{
    Renderbuffer    *ptr;
    GLuint index;

    // if (ctx->state.framebuffer == NULL)
    // {
    //     // no fbo bound..
    //     assert(0);
    // }

    assert(target == GL_RENDERBUFFER);

    if (renderbuffer)
    {
        ptr = getRenderbuffer(ctx, renderbuffer);
        assert(ptr);
    }
    else
    {
        ptr = NULL;
    }

    index = textureIndexFromTarget(ctx, target);
    if (index == _MAX_TEXTURE_TYPES)
    {
        assert(0);
    }

    ctx->state.renderbuffer = ptr;
    // no dirty state
}

void mglDeleteRenderbuffers(GLMContext ctx, GLsizei n, const GLuint *renderbuffers)
{
    // Unimplemented function
    assert(0);
}

void mglRenderbufferStorage(GLMContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    Texture *tex;

    assert(target == GL_RENDERBUFFER);

    if(ctx->state.renderbuffer == NULL)
    {
        assert(0);
        // no renderbuffer bound
    }

    tex = newTexObj(ctx, target);
    assert(tex);

    //bool createTextureLevel(GLMContext ctx, Texture *tex, GLuint face, GLint level, GLboolean is_array, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void *pixels, GLboolean proxy)
    createTextureLevel(ctx, tex, 0, 0, false, internalformat, width, height, 1, 0, 0, NULL, false);

    tex->access = GL_READ_WRITE;
    tex->is_render_target = true;
    
    ctx->state.renderbuffer->tex = tex;
}

void mglGetRenderbufferParameteriv(GLMContext ctx, GLenum target, GLenum pname, GLint *params)
{
    assert(target == GL_RENDERBUFFER);

    assert(params);

    // cant get here without a storage call
    assert(RENDBUF_STATE(tex));

    switch(pname)
    {
        case GL_RENDERBUFFER_WIDTH:
            *params = RENDBUF_STATE(tex->width); break;

        case GL_RENDERBUFFER_HEIGHT:
            *params = RENDBUF_STATE(tex->height); break;

        case GL_RENDERBUFFER_INTERNAL_FORMAT:
            *params = RENDBUF_STATE(tex->internalformat); break;

        // for now renderbuffers inherit the pixel format from the context..
        case GL_RENDERBUFFER_RED_SIZE:
            *params = bicountForFormatType(ctx->pixel_format.format, ctx->pixel_format.type, GL_RED); break;

        case GL_RENDERBUFFER_GREEN_SIZE:
            *params = bicountForFormatType(ctx->pixel_format.format, ctx->pixel_format.type, GL_GREEN); break;

        case GL_RENDERBUFFER_BLUE_SIZE:
            *params = bicountForFormatType(ctx->pixel_format.format, ctx->pixel_format.type, GL_BLUE); break;

        case GL_RENDERBUFFER_ALPHA_SIZE:
            *params = bicountForFormatType(ctx->pixel_format.format, ctx->pixel_format.type, GL_ALPHA); break;

        case GL_RENDERBUFFER_DEPTH_SIZE:
            assert(0); break;

        case GL_RENDERBUFFER_STENCIL_SIZE:
            assert(0); break;

        case GL_RENDERBUFFER_SAMPLES:
            assert(0); break;

        default: assert(0); break;
    }

}

#pragma mark Framebuffer Texture Bind calls
FBOAttachment *getFBOAttachment(GLMContext ctx, Framebuffer *fbo, GLenum attachment)
{
    switch(attachment)
    {
        case GL_DEPTH_ATTACHMENT:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            return &fbo->depth;
            break;

        case GL_STENCIL_ATTACHMENT:
            return &fbo->stencil;
            break;

        default:
            attachment = attachment - GL_COLOR_ATTACHMENT0;
            return &fbo->color_attachments[attachment];
            break;
    }
}

bool isColorAttachment(GLMContext ctx, GLuint attachment)
{
    return ((attachment >= GL_COLOR_ATTACHMENT0) &&
            (attachment <= (GL_COLOR_ATTACHMENT0 + STATE(max_color_attachments))));
}

bool isCubeMapTarget(GLMContext ctx, GLuint textarget)
{
    return ((textarget >= GL_TEXTURE_CUBE_MAP_POSITIVE_X) &&
            (textarget <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z));
}

void framebufferTexture(GLMContext ctx, GLenum target, GLenum attachment_type, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
    Framebuffer *fbo;
    Texture *tex;
    FBOAttachment *fbo_attachment_ptr;

    fbo = currentFBOForType(ctx, target);

    switch(attachment)
    {
        case GL_DEPTH_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            break;

        default:
            if (isColorAttachment(ctx, attachment))
            {
                GLuint index;

                index = attachment - GL_COLOR_ATTACHMENT0;
                if (texture)
                {
                    fbo->color_attachment_bitfield |= (0x1 << index);
                }
                else
                {
                    fbo->color_attachment_bitfield &= ~(0x1 << index);
                }
                break;
            }

            assert(attachment < STATE(max_color_attachments));
    }

    if (texture)
    {
        tex = findTexture(ctx, texture);
        (assert(tex));

        switch(textarget)
        {
            case GL_TEXTURE_BUFFER:
            case GL_TEXTURE_1D:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_3D:
            case GL_TEXTURE_RECTANGLE:
            case GL_TEXTURE_1D_ARRAY:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_2D_MULTISAMPLE:
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                break;

            default:
                if (tex->target == GL_TEXTURE_CUBE_MAP)
                {
                    if (isCubeMapTarget(ctx, textarget))
                    {
                        break;
                    }
                }

                assert(0);

                break;
        }

        if (level >= tex->mipmap_levels)
        {
            assert(0);
        }

        if (level > 0)
        {
            switch(textarget)
            {
                // If textarget is GL_TEXTURE_RECTANGLE, GL_TEXTURE_2D_MULTISAMPLE, or GL_TEXTURE_2D_MULTISAMPLE_ARRAY, then level must be zero.
                case GL_TEXTURE_RECTANGLE:
                case GL_TEXTURE_2D_MULTISAMPLE:
                case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                    assert(0);
                    return;

                // if textarget is GL_TEXTURE_3D, then level must be greater than or equal to zero and less than or equal to $log_2$ of the value of GL_MAX_3D_TEXTURE_SIZE.
                case GL_TEXTURE_3D:
                    if (level >= ilog2(STATE_VAR(max_texture_size)))
                    {
                        assert(0);
                        return;
                    }
                    break;

                default:
                    if (tex->target == GL_TEXTURE_CUBE_MAP)
                    {
                        // if textarget is one of GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, then level must be greater than or equal to zero and less than or equal to $log_2$ of the value of GL_MAX_CUBE_MAP_TEXTURE_SIZE.

                        if (isCubeMapTarget(ctx, textarget))
                        {
                            if (level >=0 && level <= ilog2(STATE_VAR(max_texture_size)))
                            {
                                break;
                            }

                            assert(0);
                            return;
                        }
                    }
                    else if (level >= ilog2(STATE_VAR(max_texture_size)))
                    {
                        // For all other values of textarget, level must be greater than or equal to zero and less than or equal to $log_2$ of the value of GL_MAX_TEXTURE_SIZE.


                        assert(0);
                        return;
                    }
                    break;
            }
        }
    }
    else
    {
        // ignore all error checking
        tex = NULL;
    }

    fbo_attachment_ptr = getFBOAttachment(ctx, fbo, attachment);

    fbo_attachment_ptr->texture = texture;
    fbo_attachment_ptr->textarget = textarget;
    fbo_attachment_ptr->level = level;
    fbo_attachment_ptr->layer = layer;
    fbo_attachment_ptr->clear_bitmask = 0;
    fbo_attachment_ptr->clear_color[0] = 0.f;
    fbo_attachment_ptr->clear_color[1] = 0.f;
    fbo_attachment_ptr->clear_color[2] = 0.f;
    fbo_attachment_ptr->clear_color[3] = 0.f;
    fbo_attachment_ptr->buf.tex = tex;

    if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
    {
        fbo->stencil = fbo->depth;
    }

    fbo->dirty_bits |= DIRTY_FBO_BINDING;
}

/*
 target
 Specifies the target to which the framebuffer is bound for all commands except glNamedFramebufferTexture.

 framebuffer
 Specifies the name of the framebuffer object for glNamedFramebufferTexture.

 attachment
 Specifies the attachment point of the framebuffer.

 textarget
 For glFramebufferTexture1D, glFramebufferTexture2D and glFramebufferTexture3D, specifies what type of texture is expected in the texture parameter, or for cube map textures, which face is to be attached.

 texture
 Specifies the name of an existing texture object to attach.

 level
 Specifies the mipmap level of the texture object to attach.
 */

void mglFramebufferTexture(GLMContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    framebufferTexture(ctx, target, GL_NONE, attachment, GL_NONE, texture, level, 0);
}


void mglFramebufferTexture1D(GLMContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    assert(textarget == GL_TEXTURE_1D);

    framebufferTexture(ctx, target, GL_TEXTURE_1D, attachment, textarget, texture, level, 0);
}

void mglFramebufferTexture2D(GLMContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    switch(textarget)
    {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_2D_MULTISAMPLE:
            break;

        default:
            if (isCubeMapTarget(ctx, textarget))
            {
                break;
            }

            assert(0);

            return;
    }

    framebufferTexture(ctx, target, GL_TEXTURE_2D, attachment, textarget, texture, level, 0);
}

void mglFramebufferTexture3D(GLMContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    assert(textarget == GL_TEXTURE_3D);

    framebufferTexture(ctx, target, GL_TEXTURE_3D, attachment, textarget, texture, level, zoffset);
}

void mglFramebufferTextureLayer(GLMContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    framebufferTexture(ctx, target, GL_TEXTURE_3D, attachment, GL_TEXTURE_3D, texture, level, layer);
}


void mglFramebufferRenderbuffer(GLMContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    Framebuffer *fbo;
    Renderbuffer *rbo;
    FBOAttachment *fbo_attachment_ptr;

    fbo = currentFBOForType(ctx, target);

    switch(attachment)
    {
        case GL_DEPTH_ATTACHMENT:
        case GL_DEPTH_STENCIL_ATTACHMENT:
        case GL_STENCIL_ATTACHMENT:
            break;

        default:
            if (isColorAttachment(ctx, attachment))
            {
                GLuint index;

                index = attachment - GL_COLOR_ATTACHMENT0;
                if (renderbuffer)
                {
                    fbo->color_attachment_bitfield |= (0x1 << index);
                }
                else
                {
                    fbo->color_attachment_bitfield &= ~(0x1 << index);
                }
                break;
            }

            assert(attachment < STATE(max_color_attachments));
    }

    if (renderbuffer)
    {
        rbo = findRenderbuffer(ctx, renderbuffer);
        (assert(rbo));
    }
    else
    {
        rbo = NULL;
    }

    fbo_attachment_ptr = getFBOAttachment(ctx, fbo, attachment);

    fbo_attachment_ptr->textarget = GL_RENDERBUFFER;
    fbo_attachment_ptr->texture = renderbuffer;
    fbo_attachment_ptr->level = 0;
    fbo_attachment_ptr->buf.rbo = rbo;
    fbo_attachment_ptr->buf.rbo->is_draw_buffer = GL_FALSE;

    if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
    {
        fbo->stencil = fbo->depth;
    }

    fbo->dirty_bits |= DIRTY_FBO_BINDING;
}

#pragma mark =====

void getFramebufferAttachmentParameteriv(GLMContext ctx, GLuint framebuffer, GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    Framebuffer *fbo;
    FBOAttachment *fbo_attachment_ptr;

    switch(target)
    {
        case GL_DRAW_FRAMEBUFFER:
        case GL_FRAMEBUFFER:
            fbo = ctx->state.framebuffer;
            break;

        case GL_READ_FRAMEBUFFER:
            fbo = ctx->state.readbuffer;
            break;

        default:
            // target will be zero for mglGetNamedFramebufferAttachmentParameteriv
            fbo = findFrameBuffer(ctx, framebuffer);
            assert(0);
            break;
    }

    if (fbo)
    {
        GLuint level;
        Texture *tex;
        GLenum target;

        if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
        {
            FBOAttachment *depth_attachment_ptr;
            FBOAttachment *stencil_attachment_ptr;

            depth_attachment_ptr = getFBOAttachment(ctx, fbo, GL_DEPTH_ATTACHMENT);
            stencil_attachment_ptr = getFBOAttachment(ctx, fbo, GL_STENCIL_ATTACHMENT);

            if ((depth_attachment_ptr != NULL) &&
                (stencil_attachment_ptr != NULL) &&
                (depth_attachment_ptr == stencil_attachment_ptr))
            {
                *params = GL_NONE;

                return;
            }

        }

        fbo_attachment_ptr = getFBOAttachment(ctx, fbo, attachment);

        if (fbo_attachment_ptr == NULL)
        {
            *params = GL_NONE;
            return;
        }

        level = fbo_attachment_ptr->level;
        target = fbo_attachment_ptr->textarget;

        if (target == GL_RENDERBUFFER)
        {
            tex = fbo_attachment_ptr->buf.rbo->tex;
        }
        else
        {
            tex = fbo_attachment_ptr->buf.tex;
        }

        switch(pname)
        {
            case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
                *params = bitcountForInternalFormat(tex->internalformat, GL_RED);
                return;

            case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
                *params = bitcountForInternalFormat(tex->internalformat, GL_GREEN);
                return;

            case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
                *params = bitcountForInternalFormat(tex->internalformat, GL_BLUE);
                return;

            case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
                *params = bitcountForInternalFormat(tex->internalformat, GL_ALPHA);
                return;

            case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
            case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
                *params = bitcountForInternalFormat(tex->internalformat, GL_NONE);
                return;

            case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                assert(0);
                // need to fill these in

            default:
                return;
        }
    }
    else
    {
        // default framebuffer
        switch(attachment)
        {
            case GL_FRONT_LEFT:
            case GL_FRONT_RIGHT:
            case GL_BACK_LEFT:
            case GL_BACK_RIGHT:
            case GL_DEPTH:
            case GL_STENCIL:
                assert(0);
                break;

            default:
                assert(0);
                return;
        }
    }
}

void mglGetFramebufferAttachmentParameteriv(GLMContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    getFramebufferAttachmentParameteriv(ctx, 0, target, attachment, pname, params);
}

void mglGetNamedFramebufferAttachmentParameteriv(GLMContext ctx, GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)
{
    getFramebufferAttachmentParameteriv(ctx, framebuffer, 0, attachment, pname, params);
}


void mglBlitFramebuffer(GLMContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    ctx->mtl_funcs.mtlBlitFramebuffer(ctx, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void mglRenderbufferStorageMultisample(GLMContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglFramebufferParameteri(GLMContext ctx, GLenum target, GLenum pname, GLint param)
{
    // Unimplemented function
    assert(0);
}

void mglGetFramebufferParameteriv(GLMContext ctx, GLenum target, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

void mglInvalidateFramebuffer(GLMContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    // Unimplemented function
    assert(0);
}

void mglInvalidateSubFramebuffer(GLMContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglCreateFramebuffers(GLMContext ctx, GLsizei n, GLuint *framebuffers)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferRenderbuffer(GLMContext ctx, GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferParameteri(GLMContext ctx, GLuint framebuffer, GLenum pname, GLint param)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferTexture(GLMContext ctx, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferTextureLayer(GLMContext ctx, GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferDrawBuffer(GLMContext ctx, GLuint framebuffer, GLenum buf)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferDrawBuffers(GLMContext ctx, GLuint framebuffer, GLsizei n, const GLenum *bufs)
{
    // Unimplemented function
    assert(0);
}

void mglNamedFramebufferReadBuffer(GLMContext ctx, GLuint framebuffer, GLenum src)
{
    // Unimplemented function
    assert(0);
}

void mglInvalidateNamedFramebufferData(GLMContext ctx, GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments)
{
    // Unimplemented function
    assert(0);
}

void mglInvalidateNamedFramebufferSubData(GLMContext ctx, GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglClearNamedFramebufferiv(GLMContext ctx, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value)
{
    // Unimplemented function
    assert(0);
}

void mglClearNamedFramebufferuiv(GLMContext ctx, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    // Unimplemented function
    assert(0);
}

void mglClearNamedFramebufferfv(GLMContext ctx, GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    // Unimplemented function
    assert(0);
}

void mglClearNamedFramebufferfi(GLMContext ctx, GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    // Unimplemented function
    assert(0);
}

void mglBlitNamedFramebuffer(GLMContext ctx, GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    // Unimplemented function
    assert(0);
}

GLenum  mglCheckNamedFramebufferStatus(GLMContext ctx, GLuint framebuffer, GLenum target)
{
    GLenum ret = (GLenum)0;

    // Unimplemented function
    assert(0);
    return ret;
}

void mglGetNamedFramebufferParameteriv(GLMContext ctx, GLuint framebuffer, GLenum pname, GLint *param)
{
    // Unimplemented function
    assert(0);
}

void mglCreateRenderbuffers(GLMContext ctx, GLsizei n, GLuint *renderbuffers)
{
    // Unimplemented function
    assert(0);
}

void mglNamedRenderbufferStorage(GLMContext ctx, GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglNamedRenderbufferStorageMultisample(GLMContext ctx, GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    // Unimplemented function
    assert(0);
}

void mglGetNamedRenderbufferParameteriv(GLMContext ctx, GLuint renderbuffer, GLenum pname, GLint *params)
{
    // Unimplemented function
    assert(0);
}

