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
 * glm_context.c
 * MGL
 *
 */

#include <stdlib.h>
#include <strings.h>
#include <assert.h>

#include "glm_context.h"
#include "MGLRenderer.h"
#include "error.h"

extern void getMacOSDefaults(GLMContext glm_ctx);
extern void init_dispatch(GLMContext ctx);

GLMContext _ctx = NULL;

GLMContext createGLMContext(GLenum format, GLenum type,
                            GLenum depth_format, GLenum depth_type,
                            GLenum stencil_format, GLenum stencil_type)
{
    GLMContext ctx = (GLMContext)malloc(sizeof(GLMContextRec));
    GLMContext save = _ctx;
    int err;

    bzero((void *)ctx, sizeof(GLMContextRec));

    _ctx = ctx;

    ctx->pixel_format.format = format;
    ctx->pixel_format.type = type;

    if ((format == 0) && (type == 0))
    {
        format = GL_UNSIGNED_INT;
        type = GL_UNSIGNED_INT_8_8_8_8_REV;
    }

    ctx->pixel_format.mtl_pixel_format = mtlPixelFormatForGLFormatType(format, type);

    if (depth_format)
    {
        ctx->depth_format.format = depth_format;
        ctx->depth_format.type = depth_type;
        ctx->depth_format.mtl_pixel_format = mtlPixelFormatForGLFormatType(depth_format, depth_type);
    }

    if (stencil_format)
    {
        ctx->stencil_format.format = stencil_format;
        ctx->stencil_format.type = stencil_type;
        ctx->stencil_format.mtl_pixel_format = mtlPixelFormatForGLFormatType(stencil_format, stencil_type);
    }

    // this can probably go away
    //ctx->state.var = default_glm_params;

    // use a CGL context to read guestimates of gl params for installed GPU
    getMacOSDefaults(ctx);

    assert(STATE(max_color_attachments) <= MAX_COLOR_ATTACHMENTS);
    assert(STATE(max_vertex_attribs) <= MAX_ATTRIBS);

    STATE(draw_buffer) = GL_FRONT;
    STATE(read_buffer) = GL_FRONT;
    STATE(active_texture) = 0;

    STATE(pack.swap_bytes) = false;
    STATE(pack.lsb_first) = false;
    STATE(pack.row_length) = 0;
    STATE(pack.image_height) = 0;
    STATE(pack.skip_rows) = 0;
    STATE(pack.skip_pixels) = 0;
    STATE(pack.skip_images) = 0;
    STATE(pack.alignment) = 4;

    STATE(unpack.swap_bytes) = false;
    STATE(unpack.lsb_first) = false;
    STATE(unpack.row_length) = 0;
    STATE(unpack.image_height) = 0;
    STATE(unpack.skip_rows) = 0;
    STATE(unpack.skip_pixels) = 0;
    STATE(unpack.skip_images) = 0;
    STATE(unpack.alignment) = 4;

    STATE(caps.blend) = false;
    STATE(caps.line_smooth) = false;
    STATE(caps.polygon_smooth) = false;
    STATE(caps.cull_face) = false;
    STATE(caps.depth_test) = false;
    STATE(caps.stencil_test) = false;
    STATE(caps.dither) = true;
    STATE(caps.scissor_test) = false;
    STATE(caps.color_logic_op) = false;
    STATE(caps.polygon_offset_point) = false;
    STATE(caps.polygon_offset_line) = false;
    STATE(caps.polygon_offset_fill) = false;
    STATE(caps.index_logic_op) = false;
    STATE(caps.multisample) = true;
    STATE(caps.sample_alpha_to_coverage) = false;
    STATE(caps.sample_alpha_to_one) = false;
    STATE(caps.sample_coverage) = false;
    STATE(caps.rasterizer_discard) = false;
    STATE(caps.framebuffer_srgb) = false;
    STATE(caps.primitive_restart) = false;
    STATE(caps.depth_clamp) = false;
    STATE(caps.texture_cube_map_seamless) = false;
    STATE(caps.sample_mask) = false;
    STATE(caps.sample_shading) = false;
    STATE(caps.primitive_restart_fixed_index) = false;
    STATE(caps.debug_output_synchronous) = false;
    STATE(caps.debug_output) = false;

    STATE(var.cull_face_mode) = GL_BACK;
    STATE(var.front_face) = GL_CCW;

    STATE(hints.line_smooth_hint) = GL_DONT_CARE;
    STATE(hints.polygon_smooth_hint) = GL_DONT_CARE;
    STATE(hints.texture_compression_hint) = GL_DONT_CARE;
    STATE(hints.fragment_shader_derivative_hint) = GL_DONT_CARE;

    STATE(var.line_width) = 1.0f;
    STATE(var.point_size) = 1.0f;
    STATE(var.polygon_mode) = GL_FILL;

    STATE(var.scissor_box[0]) = 0;
    STATE(var.scissor_box[1]) = 0;
    STATE(var.scissor_box[2]) = 0;  // needs to be set on binding to window
    STATE(var.scissor_box[3]) = 0;  // needs to be set on binding to window

    for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        STATE(var.blend_src_rgb[i]) = GL_ONE;
        STATE(var.blend_src_alpha[i]) = GL_ONE;
        STATE(var.blend_dst_rgb[i]) = GL_ZERO;
        STATE(var.blend_dst_alpha[i]) = GL_ZERO;
        STATE(var.blend_equation_rgb[i]) = GL_FUNC_ADD;
        STATE(var.blend_equation_alpha[i]) = GL_FUNC_ADD;
    }

    STATE(var.depth_func) = GL_LESS;
    STATE(var.depth_clear_value) = 1.0;

    STATE(var.logic_op) = GL_COPY;
    STATE(var.stencil_func) = GL_ALWAYS;

    STATE(var.stencil_fail) = GL_KEEP;
    STATE(var.stencil_pass_depth_fail) = GL_KEEP;
    STATE(var.stencil_pass_depth_pass) = GL_KEEP;

    for(int i=0; i<MAX_CLIP_DISTANCES; i++)
    {
        STATE(caps.clip_distances[i]) = false;
    }

    STATE(var.stencil_fail) = GL_KEEP;
    STATE(var.stencil_pass_depth_fail) = GL_KEEP;
    STATE(var.stencil_pass_depth_pass) = GL_KEEP;
    STATE(var.stencil_back_fail) = GL_KEEP;
    STATE(var.stencil_fail) = GL_KEEP;
    STATE(var.stencil_back_pass_depth_fail) = GL_KEEP;
    STATE(var.stencil_back_pass_depth_pass) = GL_KEEP;

    STATE(var.stencil_func) = GL_ALWAYS;
    STATE(var.stencil_ref) = 0;
    STATE(var.stencil_writemask) = 0xFFFFFFFF;

    STATE(var.stencil_back_func) = GL_ALWAYS;
    STATE(var.stencil_back_ref) = 0;
    STATE(var.stencil_back_writemask) = 0xFFFFFFFF;

    STATE(var.max_compute_work_group_invocations) = 1024;

    STATE(var.max_compute_work_group_count[0]) = 65535;
    STATE(var.max_compute_work_group_count[1]) = 65535;
    STATE(var.max_compute_work_group_count[2]) = 65535;

    STATE(var.max_compute_work_group_size[0]) = 1024;
    STATE(var.max_compute_work_group_size[1]) = 1024;
    STATE(var.max_compute_work_group_size[2]) = 256;

    for(int attachment=0; attachment<MAX_COLOR_ATTACHMENTS; attachment++)
    {
        STATE(caps.use_color_mask[attachment]) = false;

        for(int i=0; i<4; i++)
            STATE(var.color_writemask[attachment][i]) = GL_TRUE;
    }


    STATE(var.cull_face_mode) = GL_BACK;


    STATE(sync_name) = 1;

    STATE(dirty_bits) = DIRTY_ALL;
    
    initHashTable(&STATE(vao_table), 32);
    initHashTable(&STATE(buffer_table), 32);
    initHashTable(&STATE(texture_table), 32);
    initHashTable(&STATE(shader_table), 32);
    initHashTable(&STATE(program_table), 32);
    initHashTable(&STATE(renderbuffer_table), 32);
    initHashTable(&STATE(framebuffer_table), 32);
    initHashTable(&STATE(sampler_table), 32);
    
    init_dispatch(ctx);

    ctx->assert_on_error = GL_TRUE;
    ctx->error_func = error_func;

    ctx->temp_element_buffer = NULL;
    
    err = glslang_initialize_process();
    assert(err);
    
    _ctx = save;

    return ctx;
}

void MGLsetCurrentContext(GLMContext ctx)
{
    _ctx = ctx;
}

GLMContext MGLgetCurrentContext(void)
{
    return _ctx;
}

void MGLget(GLMContext ctx, GLenum param, GLuint *data)
{
    switch(param)
    {
        case MGL_PIXEL_FORMAT: *data = ctx->pixel_format.format; break;
        case MGL_PIXEL_TYPE: *data = ctx->pixel_format.type; break;
        case MGL_DEPTH_FORMAT: *data = ctx->depth_format.format; break;
        case MGL_DEPTH_TYPE: *data = ctx->depth_format.type; break;
        case MGL_STENCIL_FORMAT: *data = ctx->stencil_format.format; break;
        case MGL_STENCIL_TYPE: *data = ctx->stencil_format.type; break;
        case MGL_CONTEXT_FLAGS: *data = ctx->context_flags; break;
        default:
            assert(0);
    }
}

void MGLswapBuffers(GLMContext ctx)
{
    if (ctx == NULL)
        return;

    ctx->mtl_funcs.mtlSwapBuffers(ctx);
}

