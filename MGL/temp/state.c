//
//  state.c
//  mgl
//
//  Created by Michael Larson on 12/14/21.
//

#include "mgl.h"
#include "glm_context.h"

#define ENABLE_CAP(_cap_)   ctx->state.caps._cap_ = true; break
#define DISABLE_CAP(_cap_)   ctx->state.caps._cap_ = false; break

void mglDisable(GLMContext ctx, GLenum cap)
{
    switch(cap)
    {
        case GL_BLEND: DISABLE_CAP(blend);
        case GL_LINE_SMOOTH: DISABLE_CAP(line_smooth);
        case GL_POLYGON_SMOOTH: DISABLE_CAP(polygon_smooth);
        case GL_CULL_FACE: DISABLE_CAP(cull_face);
        case GL_DEPTH_TEST: DISABLE_CAP(depth_test);
        case GL_STENCIL_TEST: DISABLE_CAP(stencil_test);
        case GL_DITHER: DISABLE_CAP(dither);
        case GL_SCISSOR_TEST: DISABLE_CAP(scissor_test);
        case GL_COLOR_LOGIC_OP: DISABLE_CAP(color_logic_op);
        case GL_POLYGON_OFFSET_POINT: DISABLE_CAP(polygon_offset_point);
        case GL_POLYGON_OFFSET_LINE: DISABLE_CAP(polygon_offset_line);
        case GL_POLYGON_OFFSET_FILL: DISABLE_CAP(polygon_offset_fill);
        case GL_MULTISAMPLE: DISABLE_CAP(multisample);
        case GL_SAMPLE_ALPHA_TO_COVERAGE: DISABLE_CAP(sample_alpha_to_coverage);
        case GL_SAMPLE_ALPHA_TO_ONE: DISABLE_CAP(sample_alpha_to_one);
        case GL_SAMPLE_COVERAGE: DISABLE_CAP(sample_coverage);
        case GL_RASTERIZER_DISCARD: DISABLE_CAP(rasterizer_discard);
        case GL_FRAMEBUFFER_SRGB: DISABLE_CAP(framebuffer_srgb);
        //case GL_PRIMITIVE_RESTART: DISABLE_CAP(primitive_restart);
        case GL_DEPTH_CLAMP: DISABLE_CAP(depth_clamp);
        case GL_TEXTURE_CUBE_MAP_SEAMLESS: DISABLE_CAP(texture_cube_map_seamless);
        case GL_SAMPLE_MASK: DISABLE_CAP(sample_mask);
        case GL_SAMPLE_SHADING: DISABLE_CAP(sample_shading);
        //case GL_PRIMITIVE_RESTART_FIXED_INDEX: DISABLE_CAP(primitive_restart_fixed_index);
        case GL_DEBUG_OUTPUT_SYNCHRONOUS: DISABLE_CAP(debug_output_synchronous);
        case GL_DEBUG_OUTPUT: DISABLE_CAP(debug_output);
        default:
            ERROR_RETURN(GL_INVALID_ENUM);
            break;
    }

    ctx->state.dirty_bits |= DIRTY_STATE | DIRTY_RENDER_STATE;
}

void mglEnable(GLMContext ctx, GLenum cap)
{
    switch(cap)
    {
        case GL_BLEND: ENABLE_CAP(blend);
        case GL_LINE_SMOOTH: ENABLE_CAP(line_smooth);
        case GL_POLYGON_SMOOTH: ENABLE_CAP(polygon_smooth);
        case GL_CULL_FACE: ENABLE_CAP(cull_face);
        case GL_DEPTH_TEST: ENABLE_CAP(depth_test);
        case GL_STENCIL_TEST: ENABLE_CAP(stencil_test);
        case GL_DITHER: ENABLE_CAP(dither);
        case GL_SCISSOR_TEST: ENABLE_CAP(scissor_test);
        case GL_COLOR_LOGIC_OP: ENABLE_CAP(color_logic_op);
        case GL_POLYGON_OFFSET_POINT: ENABLE_CAP(polygon_offset_point);
        case GL_POLYGON_OFFSET_LINE: ENABLE_CAP(polygon_offset_line);
        case GL_POLYGON_OFFSET_FILL: ENABLE_CAP(polygon_offset_fill);
        case GL_PROGRAM_POINT_SIZE: ENABLE_CAP(program_point_size);
        case GL_MULTISAMPLE: ENABLE_CAP(multisample);
        case GL_SAMPLE_ALPHA_TO_COVERAGE: ENABLE_CAP(sample_alpha_to_coverage);
        case GL_SAMPLE_ALPHA_TO_ONE: ENABLE_CAP(sample_alpha_to_one);
        case GL_SAMPLE_COVERAGE: ENABLE_CAP(sample_coverage);
        case GL_RASTERIZER_DISCARD: ENABLE_CAP(rasterizer_discard);
        case GL_FRAMEBUFFER_SRGB: ENABLE_CAP(framebuffer_srgb);
         //case GL_PRIMITIVE_RESTART: ENABLE_CAP(primitive_restart);
        case GL_DEPTH_CLAMP: ENABLE_CAP(depth_clamp);
        case GL_TEXTURE_CUBE_MAP_SEAMLESS: ENABLE_CAP(texture_cube_map_seamless);
        case GL_SAMPLE_MASK: ENABLE_CAP(sample_mask);
        case GL_SAMPLE_SHADING: ENABLE_CAP(sample_shading);
        //case GL_PRIMITIVE_RESTART_FIXED_INDEX: ENABLE_CAP(primitive_restart_fixed_index);
        case GL_DEBUG_OUTPUT_SYNCHRONOUS: ENABLE_CAP(debug_output_synchronous);
        case GL_DEBUG_OUTPUT: ENABLE_CAP(debug_output);
        default:
            ERROR_RETURN(GL_INVALID_ENUM);
            break;
    }

    ctx->state.dirty_bits |= DIRTY_STATE | DIRTY_RENDER_STATE;
}

void mglCullFace(GLMContext ctx, GLenum mode)
{
    switch(mode)
    {
        case GL_FRONT:
        case GL_BACK:
        case GL_FRONT_AND_BACK:
            ctx->state.var.cull_face_mode = mode;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglFrontFace(GLMContext ctx, GLenum mode)
{
    switch(mode)
    {
        case GL_CW:
        case GL_CCW:
            ctx->state.var.front_face = mode;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

#define HINT(_target_) ctx->state.hints._target_ = mode; break;
void mglHint(GLMContext ctx, GLenum target, GLenum mode)
{
    switch(target)
    {
        case GL_LINE_SMOOTH_HINT: HINT(line_smooth_hint);
        case GL_POLYGON_SMOOTH_HINT: HINT(polygon_smooth_hint)
        case GL_TEXTURE_COMPRESSION_HINT: HINT(texture_compression_hint);
        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT: HINT(fragment_shader_derivative_hint);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglLineWidth(GLMContext ctx, GLfloat width)
{
    ERROR_CHECK_RETURN(width <= 0, GL_INVALID_VALUE);

    ctx->state.var.line_width = width;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglPointSize(GLMContext ctx, GLfloat size)
{
    ERROR_CHECK_RETURN(size <= 0, GL_INVALID_VALUE);

    ctx->state.var.point_size = size;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglPolygonMode(GLMContext ctx, GLenum face, GLenum mode)
{
    ERROR_CHECK_RETURN(face == GL_FRONT_AND_BACK, GL_INVALID_VALUE);

    switch(mode)
    {
        case GL_POINT:
        case GL_LINE:
        case GL_FILL:
            ctx->state.var.polygon_mode = mode;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglScissor(GLMContext ctx, GLint x, GLint y, GLsizei width, GLsizei height)
{
    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);

    ctx->state.var.scissor_box[0] = x;
    ctx->state.var.scissor_box[1] = y;
    ctx->state.var.scissor_box[2] = width;
    ctx->state.var.scissor_box[3] = height;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglLogicOp(GLMContext ctx, GLenum opcode)
{
    switch(opcode)
    {
        case GL_CLEAR:
        case GL_SET:
        case GL_COPY:
        case GL_COPY_INVERTED:
        case GL_NOOP:
        case GL_AND:
        case GL_NAND:
        case GL_OR:
        case GL_NOR:
        case GL_XOR:
        case GL_EQUIV:
        case GL_AND_REVERSE:
        case GL_AND_INVERTED:
        case GL_OR_REVERSE:
        case GL_OR_INVERTED:
            ctx->state.var.logic_op = opcode;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglStencilFunc(GLMContext ctx, GLenum func, GLint ref, GLuint mask)
{
    switch(func)
    {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            ctx->state.var.stencil_func = func;
            ctx->state.var.stencil_back_func = func;
            ctx->state.var.stencil_ref = ref;
            ctx->state.var.stencil_back_ref = ref;
            ctx->state.var.stencil_writemask = mask;
            ctx->state.var.stencil_back_writemask = mask;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

static bool validStencilOpSeparate(GLMContext ctx, GLenum op)
{
    switch(op)
    {
        case GL_KEEP:
        case GL_ZERO:
        case GL_REPLACE:
        case GL_INCR:
        case GL_INCR_WRAP:
        case GL_DECR:
        case GL_DECR_WRAP:
        case GL_INVERT:
            return true;
    }

    return false;
}

void mglStencilOp(GLMContext ctx, GLenum fail, GLenum zfail, GLenum zpass)
{
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, fail), GL_INVALID_ENUM);
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, zfail), GL_INVALID_ENUM);
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, zpass), GL_INVALID_ENUM);

    ctx->state.var.stencil_fail = fail;
    ctx->state.var.stencil_pass_depth_fail = zfail;
    ctx->state.var.stencil_pass_depth_pass = zpass;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}


void mglStencilMask(GLMContext ctx, GLuint mask)
{
    ctx->state.var.stencil_value_mask = mask;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglColorMask(GLMContext ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    if (red == false || green == false  || blue == false  || alpha == false)
    {
        for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
        {
            STATE(caps.use_color_mask[i]) = true;

            ctx->state.var.color_writemask[i][0] = red;
            ctx->state.var.color_writemask[i][1] = green;
            ctx->state.var.color_writemask[i][2] = blue;
            ctx->state.var.color_writemask[i][3] = alpha;

        }
    }
    else
    {
        for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
        {
            STATE(caps.use_color_mask[i]) = false;
        }
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglDepthMask(GLMContext ctx, GLboolean flag)
{
    ctx->state.var.depth_writemask = flag;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglStencilOpSeparate(GLMContext ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, sfail), GL_INVALID_ENUM);
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, dpfail), GL_INVALID_ENUM);
    ERROR_CHECK_RETURN(validStencilOpSeparate(ctx, dppass), GL_INVALID_ENUM);

    switch(face)
    {
        case GL_FRONT:
            ctx->state.var.stencil_fail = sfail;
            ctx->state.var.stencil_pass_depth_fail = dpfail;
            ctx->state.var.stencil_pass_depth_pass = dppass;
            break;

        case GL_BACK:
            ctx->state.var.stencil_back_fail = sfail;
            ctx->state.var.stencil_back_pass_depth_fail = dpfail;
            ctx->state.var.stencil_back_pass_depth_pass = dppass;
            break;

        case GL_FRONT_AND_BACK:
            ctx->state.var.stencil_fail = sfail;
            ctx->state.var.stencil_pass_depth_fail = dpfail;
            ctx->state.var.stencil_pass_depth_pass = dppass;
            ctx->state.var.stencil_back_fail = sfail;
            ctx->state.var.stencil_back_pass_depth_fail = dpfail;
            ctx->state.var.stencil_back_pass_depth_pass = dppass;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglStencilFuncSeparate(GLMContext ctx, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    switch(func)
    {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    switch(face)
    {
        case GL_FRONT:
            ctx->state.var.stencil_func = func;
            ctx->state.var.stencil_ref = ref;
            ctx->state.var.stencil_writemask = mask;
            break;

        case GL_BACK:
            ctx->state.var.stencil_back_func = func;
            ctx->state.var.stencil_back_ref = ref;
            ctx->state.var.stencil_back_writemask = mask;
            break;

        case GL_FRONT_AND_BACK:
            ctx->state.var.stencil_func = func;
            ctx->state.var.stencil_ref = ref;
            ctx->state.var.stencil_writemask = mask;
            ctx->state.var.stencil_back_func = func;
            ctx->state.var.stencil_back_ref = ref;
            ctx->state.var.stencil_back_writemask = mask;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglStencilMaskSeparate(GLMContext ctx, GLenum face, GLuint mask)
{
    switch(face)
    {
        case GL_FRONT:
            ctx->state.var.stencil_writemask = mask;
            break;

        case GL_BACK:
            ctx->state.var.stencil_back_writemask = mask;
            break;

        case GL_FRONT_AND_BACK:
            ctx->state.var.stencil_writemask = mask;
            ctx->state.var.stencil_back_writemask = mask;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglDepthFunc(GLMContext ctx, GLenum func)
{
    switch(func)
    {
        case GL_LEQUAL:
        case GL_GEQUAL:
        case GL_LESS:
        case GL_GREATER:
        case GL_EQUAL:
        case GL_NOTEQUAL:
        case GL_ALWAYS:
        case GL_NEVER:
            ctx->state.var.depth_func = func;
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

static GLdouble _clamp(GLdouble a)
{
    if (a < 0.0)
        a = 0.0;
    else if (a > 1.0)
        a = 1.0;

    return a;
}

void mglDepthRange(GLMContext ctx, GLdouble n, GLdouble f)
{
    n = _clamp(n);
    f = _clamp(f);

    ctx->state.var.depth_range[0] = n;
    ctx->state.var.depth_range[1] = f;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

void mglViewport(GLMContext ctx, GLint x, GLint y, GLsizei width, GLsizei height)
{
    ERROR_CHECK_RETURN(width > 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height > 0, GL_INVALID_VALUE);

    ctx->state.viewport[0] = x;
    ctx->state.viewport[1] = y;
    ctx->state.viewport[2] = width;
    ctx->state.viewport[3] = height;

    ctx->state.dirty_bits |= DIRTY_RENDER_STATE;
}

#define RET_VAR(_VAR_, _DEFAULT_)  return (ctx->state.var._VAR_ == _DEFAULT_)
#define RET_CAP(_CAP_)  return ctx->state.caps._CAP_

GLboolean mglIsEnabled(GLMContext ctx, GLenum cap)
{
    switch(cap)
    {
        case GL_BLEND: RET_CAP(blend);
        case GL_COLOR_LOGIC_OP: RET_CAP(color_logic_op);
        case GL_CULL_FACE: RET_CAP(cull_face);
        case GL_DEPTH_CLAMP: RET_CAP(depth_clamp);
        case GL_DEBUG_OUTPUT: RET_CAP(debug_output);
        case GL_DEBUG_OUTPUT_SYNCHRONOUS: RET_CAP(debug_output_synchronous);
        case GL_DEPTH_TEST: RET_CAP(depth_test);
        case GL_DITHER: RET_CAP(dither);
        case GL_FRAMEBUFFER_SRGB: RET_CAP(framebuffer_srgb);
        case GL_LINE_SMOOTH: RET_CAP(line_smooth);
        case GL_MULTISAMPLE: RET_CAP(multisample);
        case GL_POLYGON_SMOOTH: RET_CAP(polygon_smooth);
        case GL_POLYGON_OFFSET_FILL: RET_CAP(polygon_offset_fill);
        case GL_POLYGON_OFFSET_LINE: RET_CAP(polygon_offset_line);
        case GL_POLYGON_OFFSET_POINT: RET_CAP(polygon_offset_point);
        case GL_PROGRAM_POINT_SIZE: RET_CAP(program_point_size);
        case GL_PRIMITIVE_RESTART: RET_CAP(primitive_restart);
        case GL_SAMPLE_ALPHA_TO_COVERAGE: RET_CAP(sample_alpha_to_coverage);
        case GL_SAMPLE_ALPHA_TO_ONE: RET_CAP(sample_alpha_to_one);
        case GL_SAMPLE_COVERAGE: RET_CAP(sample_coverage);
        case GL_SAMPLE_MASK: RET_CAP(sample_mask);
        case GL_SCISSOR_TEST: RET_CAP(scissor_test);
        case GL_STENCIL_TEST: RET_CAP(stencil_test);
        case GL_TEXTURE_CUBE_MAP_SEAMLESS: RET_CAP(texture_cube_map_seamless);

        default:
            ERROR_RETURN_VALUE(GL_INVALID_ENUM, GL_FALSE);
    }

    return false;
}

void mglEnablei(GLMContext ctx, GLenum target, GLuint index)
{
    if (target >= GL_CLIP_DISTANCE0 &&
        target >= GL_CLIP_DISTANCE7)
    {
        if (index >= 0 &&
            index < ctx->state.var.max_clip_distances)
        {
            ctx->state.caps.clip_distances[index] = true;

            ctx->state.dirty_bits |= DIRTY_RENDER_STATE;

            return;
        }

        ERROR_RETURN(GL_INVALID_VALUE);
    }

    ERROR_RETURN(GL_INVALID_ENUM);
}

void mglDisablei(GLMContext ctx, GLenum target, GLuint index)
{
    if (target >= GL_CLIP_DISTANCE0 &&
        target >= GL_CLIP_DISTANCE7)
    {
        if (index >= 0 &&
            index < ctx->state.var.max_clip_distances)
        {
            ctx->state.caps.clip_distances[index] = false;

            ctx->state.dirty_bits |= DIRTY_RENDER_STATE;

            return;
        }

        ERROR_RETURN(GL_INVALID_VALUE);
    }

    ERROR_RETURN(GL_INVALID_ENUM);
}

GLboolean mglIsEnabledi(GLMContext ctx, GLenum target, GLuint index)
{
    if (target >= GL_CLIP_DISTANCE0 &&
        target >= GL_CLIP_DISTANCE7)
    {
        if (index >= 0 &&
            index < ctx->state.var.max_clip_distances)
        {
            return ctx->state.caps.clip_distances[index];
        }

        ERROR_RETURN_VALUE(GL_INVALID_VALUE, false);
    }

    ERROR_RETURN_VALUE(GL_INVALID_ENUM, false);
}

void mglBlendColor(GLMContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    ctx->state.var.blend_color[0] = red;
    ctx->state.var.blend_color[1] = green;
    ctx->state.var.blend_color[2] = blue;
    ctx->state.var.blend_color[3] = alpha;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendEquation(GLMContext ctx, GLenum mode)
{
    switch(mode)
    {
        case GL_FUNC_ADD:
        case GL_FUNC_SUBTRACT:
        case GL_FUNC_REVERSE_SUBTRACT:
        case GL_MIN:
        case GL_MAX:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        ctx->state.var.blend_equation_rgb[i] = mode;
        ctx->state.var.blend_equation_alpha[i] = mode;
    }

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendEquationi(GLMContext ctx, GLuint buf, GLenum mode)
{
    switch(mode)
    {
        case GL_FUNC_ADD:
        case GL_FUNC_SUBTRACT:
        case GL_FUNC_REVERSE_SUBTRACT:
        case GL_MIN:
        case GL_MAX:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(buf >=0 && buf < MAX_COLOR_ATTACHMENTS, GL_INVALID_VALUE);

    ctx->state.var.blend_equation_rgb[buf] = mode;
    ctx->state.var.blend_equation_alpha[buf] = mode;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendEquationSeparatei(GLMContext ctx, GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    switch(modeRGB)
    {
        case GL_FUNC_ADD:
        case GL_FUNC_SUBTRACT:
        case GL_FUNC_REVERSE_SUBTRACT:
        case GL_MIN:
        case GL_MAX:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    switch(modeAlpha)
    {
        case GL_FUNC_ADD:
        case GL_FUNC_SUBTRACT:
        case GL_FUNC_REVERSE_SUBTRACT:
        case GL_MIN:
        case GL_MAX:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(buf >= 0 && buf < MAX_COLOR_ATTACHMENTS, GL_INVALID_VALUE);

    ctx->state.var.blend_equation_rgb[buf] = modeRGB;
    ctx->state.var.blend_equation_alpha[buf] = modeAlpha;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendFunc(GLMContext ctx, GLenum sfactor, GLenum dfactor)
{
    switch(sfactor)
    {
        case GL_ZERO:
        case GL_ONE:
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
        case GL_DST_COLOR:
        case GL_ONE_MINUS_DST_COLOR:
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
        case GL_DST_ALPHA:
        case GL_ONE_MINUS_DST_ALPHA:
        case GL_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    switch(dfactor)
    {
        case GL_ZERO:
        case GL_ONE:
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
        case GL_DST_COLOR:
        case GL_ONE_MINUS_DST_COLOR:
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
        case GL_DST_ALPHA:
        case GL_ONE_MINUS_DST_ALPHA:
        case GL_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        ctx->state.var.blend_src_rgb[i] = sfactor;
        ctx->state.var.blend_src_alpha[i] = sfactor;
        ctx->state.var.blend_dst_rgb[i] = dfactor;
        ctx->state.var.blend_dst_alpha[i] = dfactor;
    }

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendFunci(GLMContext ctx, GLuint buf, GLenum sfactor, GLenum dfactor)
{
    switch(sfactor)
    {
        case GL_ZERO:
        case GL_ONE:
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
        case GL_DST_COLOR:
        case GL_ONE_MINUS_DST_COLOR:
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
        case GL_DST_ALPHA:
        case GL_ONE_MINUS_DST_ALPHA:
        case GL_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    switch(dfactor)
    {
        case GL_ZERO:
        case GL_ONE:
        case GL_SRC_COLOR:
        case GL_ONE_MINUS_SRC_COLOR:
        case GL_DST_COLOR:
        case GL_ONE_MINUS_DST_COLOR:
        case GL_SRC_ALPHA:
        case GL_ONE_MINUS_SRC_ALPHA:
        case GL_DST_ALPHA:
        case GL_ONE_MINUS_DST_ALPHA:
        case GL_CONSTANT_COLOR:
        case GL_ONE_MINUS_CONSTANT_COLOR:
        case GL_CONSTANT_ALPHA:
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(buf >=0 && buf < MAX_COLOR_ATTACHMENTS, GL_INVALID_VALUE);

    ctx->state.var.blend_src_rgb[buf] = sfactor;
    ctx->state.var.blend_src_alpha[buf] = sfactor;
    ctx->state.var.blend_dst_rgb[buf] = dfactor;
    ctx->state.var.blend_dst_alpha[buf] = dfactor;

    ctx->state.dirty_bits |= DIRTY_STATE;
}

void mglBlendFuncSeparatei(GLMContext ctx, GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    // Unimplemented function
    assert(0);
}

void mglBlendEquationSeparate(GLMContext ctx, GLenum modeRGB, GLenum modeAlpha)
{
    // Unimplemented function
    assert(0);
}


void mglGetPointerv(GLMContext ctx, GLenum pname, void **params)
{
    // Unimplemented function
    assert(0);
}

void mglPolygonOffset(GLMContext ctx, GLfloat factor, GLfloat units)
{
    // Unimplemented function
    assert(0);
}

void mglBlendFuncSeparate(GLMContext ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    // Unimplemented function
    assert(0);
}

void mglPointParameterf(GLMContext ctx, GLenum pname, GLfloat param)
{
    // Unimplemented function
    assert(0);
}

void mglPointParameterfv(GLMContext ctx, GLenum pname, const GLfloat *params)
{
    // Unimplemented function
    assert(0);
}

void mglPointParameteri(GLMContext ctx, GLenum pname, GLint param)
{
    // Unimplemented function
    assert(0);
}

void mglPointParameteriv(GLMContext ctx, GLenum pname, const GLint *params)
{
    // Unimplemented function
    assert(0);
}

