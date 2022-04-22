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
 * get.c
 * MGL
 *
 */

#include "glm_context.h"

// these cast a void ptr to a type and value
#define RET_BOOL(__value__) *((GLboolean *)data) = (GLboolean)__value__; break;
#define RET_INT(__value__) *((GLint *)data) = (GLint)__value__; break;
#define RET_FLOAT(__value__) *((GLfloat *)data) = (GLfloat)__value__; break;
#define RET_DOUBLE(__value__) *((GLdouble *)data) = (GLdouble)__value__; break;

enum {
    kBool, kInt, kFloat, kDouble
};

// set value based on type from ctx->state.var
#define RET_TYPE_VAR(__TYPE__, __VALUE__) \
switch(type) {  \
case kBool: RET_BOOL(ctx->state.var.__VALUE__);   \
    case kInt: RET_INT(ctx->state.var.__VALUE__)    \
    case kFloat: RET_FLOAT(ctx->state.var.__VALUE__)    \
    case kDouble: RET_DOUBLE(ctx->state.var.__VALUE__)    \
}

// set count values based on type
#define RET_TYPE_VAR_COUNT(__TYPE__, __VALUE__, __COUNT__) \
for(int i=0, counts[]={1,4,4,8};i<__COUNT__; data+=counts[__TYPE__], i++) \
    switch(type) {  \
        case kBool: RET_BOOL(ctx->state.var.__VALUE__[i])    \
        case kInt: RET_INT(ctx->state.var.__VALUE__[i])    \
        case kFloat: RET_FLOAT(ctx->state.var.__VALUE__[i])    \
        case kDouble: RET_DOUBLE(ctx->state.var.__VALUE__[i])    \
}

// set value based on type from ctx->state not ctx->state.var
#define RET_TYPE(__TYPE__, __VALUE__) \
switch(type) {  \
    case kBool: RET_BOOL(ctx->state.__VALUE__)    \
    case kInt: RET_INT(ctx->state.__VALUE__)    \
    case kFloat: RET_FLOAT(ctx->state.__VALUE__)    \
    case kDouble: RET_DOUBLE(ctx->state.__VALUE__)    \
}

// set count values based on type
#define RET_TYPE_COUNT(__TYPE__, __VALUE__, __COUNT__) \
for(int i=0, counts[]={1,4,4,8};i<__COUNT__; data+=counts[__TYPE__], i++) \
    switch(type) {  \
        case kBool: RET_BOOL(ctx->state.__VALUE__[i])    \
        case kInt: RET_INT(ctx->state.__VALUE__[i])    \
        case kFloat: RET_FLOAT(ctx->state.__VALUE__[i])    \
        case kDouble: RET_DOUBLE(ctx->state.__VALUE__[i])    \
}

static void mglGet(GLMContext ctx, GLenum pname, GLuint type, void *data)
{
    switch(pname)
    {
        case 0x0B11: RET_TYPE_VAR(type, point_size); break; // GL_POINT_SIZE
        case 0x0B12: RET_TYPE_VAR(type, point_size_range); break; // GL_POINT_SIZE_RANGE
        case 0x0B13: RET_TYPE_VAR(type, point_size_granularity); break; // GL_POINT_SIZE_GRANULARITY
        case 0x0B21: RET_TYPE_VAR(type, line_width); break; // GL_LINE_WIDTH
        case 0x0B22: RET_TYPE_VAR(type, line_width_range); break; // GL_LINE_WIDTH_RANGE
        case 0x0B23: RET_TYPE_VAR(type, line_width_granularity); break; // GL_LINE_WIDTH_GRANULARITY
        case 0x0B40: RET_TYPE_VAR(type, polygon_mode); break; // GL_POLYGON_MODE
        case 0x0B45: RET_TYPE_VAR(type, cull_face_mode); break; // GL_CULL_FACE_MODE
        case 0x0B46: RET_TYPE_VAR(type, front_face); break; // GL_FRONT_FACE

        case 0x0B70: RET_TYPE_VAR_COUNT(type, depth_range, 2); break; // GL_DEPTH_RANGE

        case 0x0B72: RET_TYPE_VAR(type, depth_writemask); break; // GL_DEPTH_WRITEMASK
        case 0x0B73: RET_TYPE_VAR(type, depth_clear_value); break; // GL_DEPTH_CLEAR_VALUE
        case 0x0B74: RET_TYPE_VAR(type, depth_func); break; // GL_DEPTH_FUNC
        case 0x0B91: RET_TYPE_VAR(type, stencil_clear_value); break; // GL_STENCIL_CLEAR_VALUE
        case 0x0B92: RET_TYPE_VAR(type, stencil_func); break; // GL_STENCIL_FUNC
        case 0x0B93: RET_TYPE_VAR(type, stencil_value_mask); break; // GL_STENCIL_VALUE_MASK
        case 0x0B94: RET_TYPE_VAR(type, stencil_fail); break; // GL_STENCIL_FAIL
        case 0x0B95: RET_TYPE_VAR(type, stencil_pass_depth_fail); break; // GL_STENCIL_PASS_DEPTH_FAIL
        case 0x0B96: RET_TYPE_VAR(type, stencil_pass_depth_pass); break; // GL_STENCIL_PASS_DEPTH_PASS
        case 0x0B97: RET_TYPE_VAR(type, stencil_ref); break; // GL_STENCIL_REF
        case 0x0B98: RET_TYPE_VAR(type, stencil_writemask); break; // GL_STENCIL_WRITEMASK

        case 0x0BA2: RET_TYPE_COUNT(type, viewport, 4); break; // GL_VIEWPORT

        case 0x0BE0: RET_TYPE_VAR(type, blend_dst_rgb[0]); break; // GL_BLEND_DST
        case 0x0BE1: RET_TYPE_VAR(type, blend_src_rgb[0]); break; // GL_BLEND_SRC

        case 0x0BF0: RET_TYPE_VAR(type, logic_op_mode); break; // GL_LOGIC_OP_MODE
        case 0x0C01: RET_TYPE(type, draw_buffer); break; // GL_DRAW_BUFFER
        case 0x0C02: RET_TYPE(type, read_buffer); break; // GL_READ_BUFFER

        case 0x0C10: RET_TYPE_VAR_COUNT(type, scissor_box, 4); break; // GL_SCISSOR_BOX

        case 0x0C22: RET_TYPE_COUNT(type, color_clear_value, 4); break; // GL_COLOR_CLEAR_VALUE

        case 0x0C23: RET_TYPE_VAR_COUNT(type, color_writemask[0], 4); break; // GL_COLOR_WRITEMASK

        case 0x0D33: RET_TYPE_VAR(type, max_texture_size); break; // GL_MAX_TEXTURE_SIZE
        case 0x0D3A: RET_TYPE_VAR(type, max_viewport_dims); break; // GL_MAX_VIEWPORT_DIMS
        case 0x0D50: RET_TYPE_VAR(type, subpixel_bits); break; // GL_SUBPIXEL_BITS
        case 0x0B00: RET_TYPE_VAR(type, current_color); break; // GL_CURRENT_COLOR
        case 0x0B01: RET_TYPE_VAR(type, current_index); break; // GL_CURRENT_INDEX
        case 0x0B02: RET_TYPE_VAR(type, current_normal); break; // GL_CURRENT_NORMAL
        case 0x0B04: RET_TYPE_VAR(type, current_raster_color); break; // GL_CURRENT_RASTER_COLOR
        case 0x0B05: RET_TYPE_VAR(type, current_raster_index); break; // GL_CURRENT_RASTER_INDEX
        case 0x0B06: RET_TYPE_VAR(type, current_raster_texture_coords); break; // GL_CURRENT_RASTER_TEXTURE_COORDS
        case 0x0B07: RET_TYPE_VAR(type, current_raster_position); break; // GL_CURRENT_RASTER_POSITION
        case 0x0B08: RET_TYPE_VAR(type, current_raster_position_valid); break; // GL_CURRENT_RASTER_POSITION_VALID
        case 0x0B09: RET_TYPE_VAR(type, current_raster_distance); break; // GL_CURRENT_RASTER_DISTANCE
        case 0x0B25: RET_TYPE_VAR(type, line_stipple_pattern); break; // GL_LINE_STIPPLE_PATTERN
        case 0x0B26: RET_TYPE_VAR(type, line_stipple_repeat); break; // GL_LINE_STIPPLE_REPEAT
        case 0x0B30: RET_TYPE_VAR(type, list_mode); break; // GL_LIST_MODE
        case 0x0B31: RET_TYPE_VAR(type, max_list_nesting); break; // GL_MAX_LIST_NESTING
        case 0x0B32: RET_TYPE_VAR(type, list_base); break; // GL_LIST_BASE
        case 0x0B33: RET_TYPE_VAR(type, list_index); break; // GL_LIST_INDEX
        case 0x0B43: RET_TYPE_VAR(type, edge_flag); break; // GL_EDGE_FLAG
        case 0x0B54: RET_TYPE_VAR(type, shade_model); break; // GL_SHADE_MODEL
        case 0x0B55: RET_TYPE_VAR(type, color_material_face); break; // GL_COLOR_MATERIAL_FACE
        case 0x0B56: RET_TYPE_VAR(type, color_material_parameter); break; // GL_COLOR_MATERIAL_PARAMETER
        case 0x0B80: RET_TYPE_VAR(type, accum_clear_value); break; // GL_ACCUM_CLEAR_VALUE
        case 0x0BA0: RET_TYPE_VAR(type, matrix_mode); break; // GL_MATRIX_MODE
        case 0x0BA3: RET_TYPE_VAR(type, modelview_stack_depth); break; // GL_MODELVIEW_STACK_DEPTH
        case 0x0BA4: RET_TYPE_VAR(type, projection_stack_depth); break; // GL_PROJECTION_STACK_DEPTH
        case 0x0BA5: RET_TYPE_VAR(type, texture_stack_depth); break; // GL_TEXTURE_STACK_DEPTH
        case 0x0BA6: RET_TYPE_VAR(type, modelview_matrix); break; // GL_MODELVIEW_MATRIX
        case 0x0BA7: RET_TYPE_VAR(type, projection_matrix); break; // GL_PROJECTION_MATRIX
        case 0x0BB0: RET_TYPE_VAR(type, attrib_stack_depth); break; // GL_ATTRIB_STACK_DEPTH
        case 0x0BC1: RET_TYPE_VAR(type, alpha_test_func); break; // GL_ALPHA_TEST_FUNC
        case 0x0BC2: RET_TYPE_VAR(type, alpha_test_ref); break; // GL_ALPHA_TEST_REF
        case 0x0BF1: RET_TYPE_VAR(type, logic_op); break; // GL_LOGIC_OP
        case 0x0C00: RET_TYPE_VAR(type, aux_buffers); break; // GL_AUX_BUFFERS
        case 0x0C20: RET_TYPE_VAR(type, index_clear_value); break; // GL_INDEX_CLEAR_VALUE
        case 0x0C21: RET_TYPE_VAR(type, index_writemask); break; // GL_INDEX_WRITEMASK
        case 0x0C30: RET_TYPE_VAR(type, index_mode); break; // GL_INDEX_MODE
        case 0x0C31: RET_TYPE_VAR(type, rgba_mode); break; // GL_RGBA_MODE
        case 0x0C40: RET_TYPE_VAR(type, render_mode); break; // GL_RENDER_MODE
        case 0x0CB0: RET_TYPE_VAR(type, pixel_map_i_to_i_size); break; // GL_PIXEL_MAP_I_TO_I_SIZE
        case 0x0CB1: RET_TYPE_VAR(type, pixel_map_s_to_s_size); break; // GL_PIXEL_MAP_S_TO_S_SIZE
        case 0x0CB2: RET_TYPE_VAR(type, pixel_map_i_to_r_size); break; // GL_PIXEL_MAP_I_TO_R_SIZE
        case 0x0CB3: RET_TYPE_VAR(type, pixel_map_i_to_g_size); break; // GL_PIXEL_MAP_I_TO_G_SIZE
        case 0x0CB4: RET_TYPE_VAR(type, pixel_map_i_to_b_size); break; // GL_PIXEL_MAP_I_TO_B_SIZE
        case 0x0CB5: RET_TYPE_VAR(type, pixel_map_i_to_a_size); break; // GL_PIXEL_MAP_I_TO_A_SIZE
        case 0x0CB6: RET_TYPE_VAR(type, pixel_map_r_to_r_size); break; // GL_PIXEL_MAP_R_TO_R_SIZE
        case 0x0CB7: RET_TYPE_VAR(type, pixel_map_g_to_g_size); break; // GL_PIXEL_MAP_G_TO_G_SIZE
        case 0x0CB8: RET_TYPE_VAR(type, pixel_map_b_to_b_size); break; // GL_PIXEL_MAP_B_TO_B_SIZE
        case 0x0CB9: RET_TYPE_VAR(type, pixel_map_a_to_a_size); break; // GL_PIXEL_MAP_A_TO_A_SIZE
        case 0x0D16: RET_TYPE_VAR(type, zoom_x); break; // GL_ZOOM_X
        case 0x0D17: RET_TYPE_VAR(type, zoom_y); break; // GL_ZOOM_Y
        case 0x0D30: RET_TYPE_VAR(type, max_eval_order); break; // GL_MAX_EVAL_ORDER
        case 0x0D31: RET_TYPE_VAR(type, max_lights); break; // GL_MAX_LIGHTS
        case 0x0D32: RET_TYPE_VAR(type, max_clip_planes); break; // GL_MAX_CLIP_PLANES
        case 0x0D34: RET_TYPE_VAR(type, max_pixel_map_table); break; // GL_MAX_PIXEL_MAP_TABLE
        case 0x0D35: RET_TYPE_VAR(type, max_attrib_stack_depth); break; // GL_MAX_ATTRIB_STACK_DEPTH
        case 0x0D36: RET_TYPE_VAR(type, max_modelview_stack_depth); break; // GL_MAX_MODELVIEW_STACK_DEPTH
        case 0x0D37: RET_TYPE_VAR(type, max_name_stack_depth); break; // GL_MAX_NAME_STACK_DEPTH
        case 0x0D38: RET_TYPE_VAR(type, max_projection_stack_depth); break; // GL_MAX_PROJECTION_STACK_DEPTH
        case 0x0D39: RET_TYPE_VAR(type, max_texture_stack_depth); break; // GL_MAX_TEXTURE_STACK_DEPTH
        case 0x0D51: RET_TYPE_VAR(type, index_bits); break; // GL_INDEX_BITS
        case 0x0D52: RET_TYPE_VAR(type, red_bits); break; // GL_RED_BITS
        case 0x0D53: RET_TYPE_VAR(type, green_bits); break; // GL_GREEN_BITS
        case 0x0D54: RET_TYPE_VAR(type, blue_bits); break; // GL_BLUE_BITS
        case 0x0D55: RET_TYPE_VAR(type, alpha_bits); break; // GL_ALPHA_BITS
        case 0x0D56: RET_TYPE_VAR(type, depth_bits); break; // GL_DEPTH_BITS
        case 0x0D57: RET_TYPE_VAR(type, stencil_bits); break; // GL_STENCIL_BITS
        case 0x0D58: RET_TYPE_VAR(type, accum_red_bits); break; // GL_ACCUM_RED_BITS
        case 0x0D59: RET_TYPE_VAR(type, accum_green_bits); break; // GL_ACCUM_GREEN_BITS
        case 0x0D5A: RET_TYPE_VAR(type, accum_blue_bits); break; // GL_ACCUM_BLUE_BITS
        case 0x0D5B: RET_TYPE_VAR(type, accum_alpha_bits); break; // GL_ACCUM_ALPHA_BITS
        case 0x0D70: RET_TYPE_VAR(type, name_stack_depth); break; // GL_NAME_STACK_DEPTH
        case 0x0DD0: RET_TYPE_VAR(type, map1_grid_domain); break; // GL_MAP1_GRID_DOMAIN
        case 0x0DD1: RET_TYPE_VAR(type, map1_grid_segments); break; // GL_MAP1_GRID_SEGMENTS
        case 0x0DD2: RET_TYPE_VAR(type, map2_grid_domain); break; // GL_MAP2_GRID_DOMAIN
        case 0x0DD3: RET_TYPE_VAR(type, map2_grid_segments); break; // GL_MAP2_GRID_SEGMENTS
        case 0x2A00: RET_TYPE_VAR(type, polygon_offset_units); break; // GL_POLYGON_OFFSET_UNITS
        case 0x8038: RET_TYPE_VAR(type, polygon_offset_factor); break; // GL_POLYGON_OFFSET_FACTOR
        case 0x8068: RET_TYPE_VAR(type, texture_binding_1d); break; // GL_TEXTURE_BINDING_1D
        case 0x8069: RET_TYPE_VAR(type, texture_binding_2d); break; // GL_TEXTURE_BINDING_2D
        case 0x0BB1: RET_TYPE_VAR(type, client_attrib_stack_depth); break; // GL_CLIENT_ATTRIB_STACK_DEPTH
        case 0x0D3B: RET_TYPE_VAR(type, max_client_attrib_stack_depth); break; // GL_MAX_CLIENT_ATTRIB_STACK_DEPTH
        case 0x0DF1: RET_TYPE_VAR(type, feedback_buffer_size); break; // GL_FEEDBACK_BUFFER_SIZE
        case 0x0DF2: RET_TYPE_VAR(type, feedback_buffer_type); break; // GL_FEEDBACK_BUFFER_TYPE
        case 0x0DF4: RET_TYPE_VAR(type, selection_buffer_size); break; // GL_SELECTION_BUFFER_SIZE
        case 0x807A: RET_TYPE_VAR(type, vertex_array_size); break; // GL_VERTEX_ARRAY_SIZE
        case 0x807B: RET_TYPE_VAR(type, vertex_array_type); break; // GL_VERTEX_ARRAY_TYPE
        case 0x807C: RET_TYPE_VAR(type, vertex_array_stride); break; // GL_VERTEX_ARRAY_STRIDE
        case 0x807E: RET_TYPE_VAR(type, normal_array_type); break; // GL_NORMAL_ARRAY_TYPE
        case 0x807F: RET_TYPE_VAR(type, normal_array_stride); break; // GL_NORMAL_ARRAY_STRIDE
        case 0x8081: RET_TYPE_VAR(type, color_array_size); break; // GL_COLOR_ARRAY_SIZE
        case 0x8082: RET_TYPE_VAR(type, color_array_type); break; // GL_COLOR_ARRAY_TYPE
        case 0x8083: RET_TYPE_VAR(type, color_array_stride); break; // GL_COLOR_ARRAY_STRIDE
        case 0x8085: RET_TYPE_VAR(type, index_array_type); break; // GL_INDEX_ARRAY_TYPE
        case 0x8086: RET_TYPE_VAR(type, index_array_stride); break; // GL_INDEX_ARRAY_STRIDE
        case 0x8088: RET_TYPE_VAR(type, texture_coord_array_size); break; // GL_TEXTURE_COORD_ARRAY_SIZE
        case 0x8089: RET_TYPE_VAR(type, texture_coord_array_type); break; // GL_TEXTURE_COORD_ARRAY_TYPE
        case 0x808A: RET_TYPE_VAR(type, texture_coord_array_stride); break; // GL_TEXTURE_COORD_ARRAY_STRIDE
        case 0x808C: RET_TYPE_VAR(type, edge_flag_array_stride); break; // GL_EDGE_FLAG_ARRAY_STRIDE
        case 0x806A: RET_TYPE_VAR(type, texture_binding_3d); break; // GL_TEXTURE_BINDING_3D
        case 0x8073: RET_TYPE_VAR(type, max_3d_texture_size); break; // GL_MAX_3D_TEXTURE_SIZE
        case 0x80E8: RET_TYPE_VAR(type, max_elements_vertices); break; // GL_MAX_ELEMENTS_VERTICES
        case 0x80E9: RET_TYPE_VAR(type, max_elements_indices); break; // GL_MAX_ELEMENTS_INDICES
        case 0x846E: RET_TYPE_VAR(type, aliased_line_width_range); break; // GL_ALIASED_LINE_WIDTH_RANGE
        case 0x846D: RET_TYPE_VAR(type, aliased_point_size_range); break; // GL_ALIASED_POINT_SIZE_RANGE
        case 0x84E0: RET_TYPE(type, active_texture); break; // GL_ACTIVE_TEXTURE
        case 0x80AA: RET_TYPE_VAR(type, sample_coverage_value); break; // GL_SAMPLE_COVERAGE_VALUE
        case 0x80AB: RET_TYPE_VAR(type, sample_coverage_invert); break; // GL_SAMPLE_COVERAGE_INVERT
        case 0x8514: RET_TYPE_VAR(type, texture_binding_cube_map); break; // GL_TEXTURE_BINDING_CUBE_MAP
        case 0x851C: RET_TYPE_VAR(type, max_cube_map_texture_size); break; // GL_MAX_CUBE_MAP_TEXTURE_SIZE
        case 0x86A2: RET_TYPE_VAR(type, num_compressed_texture_formats); break; // GL_NUM_COMPRESSED_TEXTURE_FORMATS
        case 0x86A3: RET_TYPE_VAR(type, compressed_texture_formats); break; // GL_COMPRESSED_TEXTURE_FORMATS

        case 0x80C8: RET_TYPE_VAR(type, blend_dst_rgb[0]); break; // GL_BLEND_DST_RGB
        case 0x80C9: RET_TYPE_VAR(type, blend_src_rgb[0]); break; // GL_BLEND_SRC_RGB
        case 0x80CA: RET_TYPE_VAR(type, blend_dst_alpha[0]); break; // GL_BLEND_DST_ALPHA
        case 0x80CB: RET_TYPE_VAR(type, blend_src_alpha[0]); break; // GL_BLEND_SRC_ALPHA

        case 0x84FD: RET_TYPE_VAR(type, max_texture_lod_bias); break; // GL_MAX_TEXTURE_LOD_BIAS

        case 0x8005: RET_TYPE_VAR_COUNT(type, blend_color,4); break; // GL_BLEND_COLOR

        case 0x8894: RET_TYPE_VAR(type, array_buffer_binding); break; // GL_ARRAY_BUFFER_BINDING
        case 0x8895: RET_TYPE_VAR(type, element_array_buffer_binding); break; // GL_ELEMENT_ARRAY_BUFFER_BINDING
        case 0x8009: RET_TYPE_VAR(type, blend_equation_rgb[0]); break; // GL_BLEND_EQUATION_RGB
        case 0x8800: RET_TYPE_VAR(type, stencil_back_func); break; // GL_STENCIL_BACK_FUNC
        case 0x8801: RET_TYPE_VAR(type, stencil_back_fail); break; // GL_STENCIL_BACK_FAIL
        case 0x8802: RET_TYPE_VAR(type, stencil_back_pass_depth_fail); break; // GL_STENCIL_BACK_PASS_DEPTH_FAIL
        case 0x8803: RET_TYPE_VAR(type, stencil_back_pass_depth_pass); break; // GL_STENCIL_BACK_PASS_DEPTH_PASS
        case 0x8824: RET_TYPE_VAR(type, max_draw_buffers); break; // GL_MAX_DRAW_BUFFERS

        case 0x883D: RET_TYPE_VAR(type, blend_equation_alpha[0]); break; // GL_BLEND_EQUATION_ALPHA

        case 0x8869: RET_TYPE(type, max_vertex_attribs); break; // GL_MAX_VERTEX_ATTRIBS
        case 0x8872: RET_TYPE_VAR(type, max_texture_image_units); break; // GL_MAX_TEXTURE_IMAGE_UNITS
        case 0x8B49: RET_TYPE_VAR(type, max_fragment_uniform_components); break; // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
        case 0x8B4A: RET_TYPE_VAR(type, max_vertex_uniform_components); break; // GL_MAX_VERTEX_UNIFORM_COMPONENTS
        case 0x8B4B: RET_TYPE_VAR(type, max_varying_floats); break; // GL_MAX_VARYING_FLOATS
        case 0x8B4C: RET_TYPE_VAR(type, max_vertex_texture_image_units); break; // GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
        case 0x8B4D: RET_TYPE_VAR(type, max_combined_texture_image_units); break; // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
        case 0x8B8D: RET_TYPE_VAR(type, current_program); break; // GL_CURRENT_PROGRAM
        case 0x8CA3: RET_TYPE_VAR(type, stencil_back_ref); break; // GL_STENCIL_BACK_REF
        case 0x8CA4: RET_TYPE_VAR(type, stencil_back_value_mask); break; // GL_STENCIL_BACK_VALUE_MASK
        case 0x8CA5: RET_TYPE_VAR(type, stencil_back_writemask); break; // GL_STENCIL_BACK_WRITEMASK
        case 0x88ED: RET_TYPE_VAR(type, pixel_pack_buffer_binding); break; // GL_PIXEL_PACK_BUFFER_BINDING
        case 0x88EF: RET_TYPE_VAR(type, pixel_unpack_buffer_binding); break; // GL_PIXEL_UNPACK_BUFFER_BINDING
        case 0x821B: RET_TYPE_VAR(type, major_version); break; // GL_MAJOR_VERSION
        case 0x821C: RET_TYPE_VAR(type, minor_version); break; // GL_MINOR_VERSION
        case 0x821D: RET_TYPE_VAR(type, num_extensions); break; // GL_NUM_EXTENSIONS
        case 0x821E: RET_TYPE_VAR(type, context_flags); break; // GL_CONTEXT_FLAGS
        case 0x88FF: RET_TYPE_VAR(type, max_array_texture_layers); break; // GL_MAX_ARRAY_TEXTURE_LAYERS
        case 0x8904: RET_TYPE_VAR(type, min_program_texel_offset); break; // GL_MIN_PROGRAM_TEXEL_OFFSET
        case 0x8905: RET_TYPE_VAR(type, max_program_texel_offset); break; // GL_MAX_PROGRAM_TEXEL_OFFSET
        case 0x8C1C: RET_TYPE_VAR(type, texture_binding_1d_array); break; // GL_TEXTURE_BINDING_1D_ARRAY
        case 0x8C1D: RET_TYPE_VAR(type, texture_binding_2d_array); break; // GL_TEXTURE_BINDING_2D_ARRAY
        case 0x84E8: RET_TYPE_VAR(type, max_renderbuffer_size); break; // GL_MAX_RENDERBUFFER_SIZE
        case 0x8CA6: RET_TYPE_VAR(type, draw_framebuffer_binding); break; // GL_DRAW_FRAMEBUFFER_BINDING
        case 0x8CA7: RET_TYPE_VAR(type, renderbuffer_binding); break; // GL_RENDERBUFFER_BINDING
        case 0x8CAA: RET_TYPE_VAR(type, read_framebuffer_binding); break; // GL_READ_FRAMEBUFFER_BINDING
        case 0x85B5: RET_TYPE_VAR(type, vertex_array_binding); break; // GL_VERTEX_ARRAY_BINDING
        case 0x8C2B: RET_TYPE_VAR(type, max_texture_buffer_size); break; // GL_MAX_TEXTURE_BUFFER_SIZE
        case 0x8C2C: RET_TYPE_VAR(type, texture_binding_buffer); break; // GL_TEXTURE_BINDING_BUFFER
        case 0x84F6: RET_TYPE_VAR(type, texture_binding_rectangle); break; // GL_TEXTURE_BINDING_RECTANGLE
        case 0x84F8: RET_TYPE_VAR(type, max_rectangle_texture_size); break; // GL_MAX_RECTANGLE_TEXTURE_SIZE
        case 0x8F9E: RET_TYPE_VAR(type, primitive_restart_index); break; // GL_PRIMITIVE_RESTART_INDEX
        case 0x8A28: RET_TYPE_VAR(type, uniform_buffer_binding); break; // GL_UNIFORM_BUFFER_BINDING
        case 0x8A29: RET_TYPE_VAR(type, uniform_buffer_start); break; // GL_UNIFORM_BUFFER_START
        case 0x8A2A: RET_TYPE_VAR(type, uniform_buffer_size); break; // GL_UNIFORM_BUFFER_SIZE
        case 0x8A2B: RET_TYPE_VAR(type, max_vertex_uniform_blocks); break; // GL_MAX_VERTEX_UNIFORM_BLOCKS
        case 0x8A2C: RET_TYPE_VAR(type, max_geometry_uniform_blocks); break; // GL_MAX_GEOMETRY_UNIFORM_BLOCKS
        case 0x8A2D: RET_TYPE_VAR(type, max_fragment_uniform_blocks); break; // GL_MAX_FRAGMENT_UNIFORM_BLOCKS
        case 0x8A2E: RET_TYPE_VAR(type, max_combined_uniform_blocks); break; // GL_MAX_COMBINED_UNIFORM_BLOCKS
        case 0x8A2F: RET_TYPE_VAR(type, max_uniform_buffer_bindings); break; // GL_MAX_UNIFORM_BUFFER_BINDINGS
        case 0x8A30: RET_TYPE_VAR(type, max_uniform_block_size); break; // GL_MAX_UNIFORM_BLOCK_SIZE
        case 0x8A31: RET_TYPE_VAR(type, max_combined_vertex_uniform_components); break; // GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS
        case 0x8A32: RET_TYPE_VAR(type, max_combined_geometry_uniform_components); break; // GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS
        case 0x8A33: RET_TYPE_VAR(type, max_combined_fragment_uniform_components); break; // GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS
        case 0x8A34: RET_TYPE_VAR(type, uniform_buffer_offset_alignment); break; // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
        case 0x8C29: RET_TYPE_VAR(type, max_geometry_texture_image_units); break; // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS
        case 0x8DDF: RET_TYPE_VAR(type, max_geometry_uniform_components); break; // GL_MAX_GEOMETRY_UNIFORM_COMPONENTS
        case 0x9122: RET_TYPE_VAR(type, max_vertex_output_components); break; // GL_MAX_VERTEX_OUTPUT_COMPONENTS
        case 0x9123: RET_TYPE_VAR(type, max_geometry_input_components); break; // GL_MAX_GEOMETRY_INPUT_COMPONENTS
        case 0x9124: RET_TYPE_VAR(type, max_geometry_output_components); break; // GL_MAX_GEOMETRY_OUTPUT_COMPONENTS
        case 0x9125: RET_TYPE_VAR(type, max_fragment_input_components); break; // GL_MAX_FRAGMENT_INPUT_COMPONENTS
        case 0x9126: RET_TYPE_VAR(type, context_profile_mask); break; // GL_CONTEXT_PROFILE_MASK
        case 0x8E4F: RET_TYPE_VAR(type, provoking_vertex); break; // GL_PROVOKING_VERTEX
        case 0x9111: RET_TYPE_VAR(type, max_server_wait_timeout); break; // GL_MAX_SERVER_WAIT_TIMEOUT
        case 0x8E59: RET_TYPE_VAR(type, max_sample_mask_words); break; // GL_MAX_SAMPLE_MASK_WORDS
        case 0x9104: RET_TYPE_VAR(type, texture_binding_2d_multisample); break; // GL_TEXTURE_BINDING_2D_MULTISAMPLE
        case 0x9105: RET_TYPE_VAR(type, texture_binding_2d_multisample_array); break; // GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY
        case 0x910E: RET_TYPE_VAR(type, max_color_texture_samples); break; // GL_MAX_COLOR_TEXTURE_SAMPLES
        case 0x910F: RET_TYPE_VAR(type, max_depth_texture_samples); break; // GL_MAX_DEPTH_TEXTURE_SAMPLES
        case 0x9110: RET_TYPE_VAR(type, max_integer_samples); break; // GL_MAX_INTEGER_SAMPLES
        case 0x88FC: RET_TYPE_VAR(type, max_dual_source_draw_buffers); break; // GL_MAX_DUAL_SOURCE_DRAW_BUFFERS
        case 0x8919: RET_TYPE_VAR(type, sampler_binding); break; // GL_SAMPLER_BINDING
        case 0x8E89: RET_TYPE_VAR(type, max_tess_control_uniform_blocks); break; // GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS
        case 0x8E8A: RET_TYPE_VAR(type, max_tess_evaluation_uniform_blocks); break; // GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS
        case 0x8DFA: RET_TYPE_VAR(type, shader_compiler); break; // GL_SHADER_COMPILER
        case 0x8DF8: RET_TYPE_VAR(type, shader_binary_formats); break; // GL_SHADER_BINARY_FORMATS
        case 0x8DF9: RET_TYPE_VAR(type, num_shader_binary_formats); break; // GL_NUM_SHADER_BINARY_FORMATS
        case 0x8DFB: RET_TYPE_VAR(type, max_vertex_uniform_vectors); break; // GL_MAX_VERTEX_UNIFORM_VECTORS
        case 0x8DFC: RET_TYPE_VAR(type, max_varying_vectors); break; // GL_MAX_VARYING_VECTORS
        case 0x8DFD: RET_TYPE_VAR(type, max_fragment_uniform_vectors); break; // GL_MAX_FRAGMENT_UNIFORM_VECTORS
        case 0x87FE: RET_TYPE_VAR(type, num_program_binary_formats); break; // GL_NUM_PROGRAM_BINARY_FORMATS
        case 0x87FF: RET_TYPE_VAR(type, program_binary_formats); break; // GL_PROGRAM_BINARY_FORMATS
        case 0x825A: RET_TYPE_VAR(type, program_pipeline_binding); break; // GL_PROGRAM_PIPELINE_BINDING
        case 0x825B: RET_TYPE_VAR(type, max_viewports); break; // GL_MAX_VIEWPORTS
        case 0x825C: RET_TYPE_VAR(type, viewport_subpixel_bits); break; // GL_VIEWPORT_SUBPIXEL_BITS
        case 0x825D: RET_TYPE_VAR(type, viewport_bounds_range); break; // GL_VIEWPORT_BOUNDS_RANGE
        case 0x825E: RET_TYPE_VAR(type, layer_provoking_vertex); break; // GL_LAYER_PROVOKING_VERTEX
        case 0x825F: RET_TYPE_VAR(type, viewport_index_provoking_vertex); break; // GL_VIEWPORT_INDEX_PROVOKING_VERTEX
        case 0x90BC: RET_TYPE_VAR(type, min_map_buffer_alignment); break; // GL_MIN_MAP_BUFFER_ALIGNMENT
        case 0x92D2: RET_TYPE_VAR(type, max_vertex_atomic_counters); break; // GL_MAX_VERTEX_ATOMIC_COUNTERS
        case 0x92D3: RET_TYPE_VAR(type, max_tess_control_atomic_counters); break; // GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS
        case 0x92D4: RET_TYPE_VAR(type, max_tess_evaluation_atomic_counters); break; // GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS
        case 0x92D5: RET_TYPE_VAR(type, max_geometry_atomic_counters); break; // GL_MAX_GEOMETRY_ATOMIC_COUNTERS
        case 0x92D6: RET_TYPE_VAR(type, max_fragment_atomic_counters); break; // GL_MAX_FRAGMENT_ATOMIC_COUNTERS
        case 0x92D7: RET_TYPE_VAR(type, max_combined_atomic_counters); break; // GL_MAX_COMBINED_ATOMIC_COUNTERS
        case 0x8D6B: RET_TYPE_VAR(type, max_element_index); break; // GL_MAX_ELEMENT_INDEX
        case 0x91BB: RET_TYPE_VAR(type, max_compute_uniform_blocks); break; // GL_MAX_COMPUTE_UNIFORM_BLOCKS
        case 0x91BC: RET_TYPE_VAR(type, max_compute_texture_image_units); break; // GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS
        case 0x8263: RET_TYPE_VAR(type, max_compute_uniform_components); break; // GL_MAX_COMPUTE_UNIFORM_COMPONENTS
        case 0x8264: RET_TYPE_VAR(type, max_compute_atomic_counter_buffers); break; // GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS
        case 0x8265: RET_TYPE_VAR(type, max_compute_atomic_counters); break; // GL_MAX_COMPUTE_ATOMIC_COUNTERS
        case 0x8266: RET_TYPE_VAR(type, max_combined_compute_uniform_components); break; // GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS
        case 0x90EB: RET_TYPE_VAR(type, max_compute_work_group_invocations); break; // GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS
        case 0x91BE: RET_TYPE_VAR(type, max_compute_work_group_count[0]); break; // GL_MAX_COMPUTE_WORK_GROUP_COUNT
        case 0x91BF: RET_TYPE_VAR(type, max_compute_work_group_size[0]); break; // GL_MAX_COMPUTE_WORK_GROUP_SIZE
        case 0x90EF: RET_TYPE_VAR(type, dispatch_indirect_buffer_binding); break; // GL_DISPATCH_INDIRECT_BUFFER_BINDING
        case 0x826C: RET_TYPE_VAR(type, max_debug_group_stack_depth); break; // GL_MAX_DEBUG_GROUP_STACK_DEPTH
        case 0x826D: RET_TYPE_VAR(type, debug_group_stack_depth); break; // GL_DEBUG_GROUP_STACK_DEPTH
        case 0x82E8: RET_TYPE_VAR(type, max_label_length); break; // GL_MAX_LABEL_LENGTH
        case 0x826E: RET_TYPE_VAR(type, max_uniform_locations); break; // GL_MAX_UNIFORM_LOCATIONS
        case 0x9315: RET_TYPE_VAR(type, max_framebuffer_width); break; // GL_MAX_FRAMEBUFFER_WIDTH
        case 0x9316: RET_TYPE_VAR(type, max_framebuffer_height); break; // GL_MAX_FRAMEBUFFER_HEIGHT
        case 0x9317: RET_TYPE_VAR(type, max_framebuffer_layers); break; // GL_MAX_FRAMEBUFFER_LAYERS
        case 0x9318: RET_TYPE_VAR(type, max_framebuffer_samples); break; // GL_MAX_FRAMEBUFFER_SAMPLES
        case 0x90D3: RET_TYPE_VAR(type, shader_storage_buffer_binding); break; // GL_SHADER_STORAGE_BUFFER_BINDING
        case 0x90D4: RET_TYPE_VAR(type, shader_storage_buffer_start); break; // GL_SHADER_STORAGE_BUFFER_START
        case 0x90D5: RET_TYPE_VAR(type, shader_storage_buffer_size); break; // GL_SHADER_STORAGE_BUFFER_SIZE
        case 0x90D6: RET_TYPE_VAR(type, max_vertex_shader_storage_blocks); break; // GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS
        case 0x90D7: RET_TYPE_VAR(type, max_geometry_shader_storage_blocks); break; // GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS
        case 0x90D8: RET_TYPE_VAR(type, max_tess_control_shader_storage_blocks); break; // GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS
        case 0x90D9: RET_TYPE_VAR(type, max_tess_evaluation_shader_storage_blocks); break; // GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS
        case 0x90DA: RET_TYPE_VAR(type, max_fragment_shader_storage_blocks); break; // GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS
        case 0x90DB: RET_TYPE_VAR(type, max_compute_shader_storage_blocks); break; // GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS
        case 0x90DC: RET_TYPE_VAR(type, max_combined_shader_storage_blocks); break; // GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS
        case 0x90DD: RET_TYPE_VAR(type, max_shader_storage_buffer_bindings); break; // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
        case 0x90DF: RET_TYPE_VAR(type, shader_storage_buffer_offset_alignment); break; // GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
        case 0x919F: RET_TYPE_VAR(type, texture_buffer_offset_alignment); break; // GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT
        case 0x82D6: RET_TYPE_VAR(type, vertex_binding_divisor); break; // GL_VERTEX_BINDING_DIVISOR
        case 0x82D7: RET_TYPE_VAR(type, vertex_binding_offset); break; // GL_VERTEX_BINDING_OFFSET
        case 0x82D8: RET_TYPE_VAR(type, vertex_binding_stride); break; // GL_VERTEX_BINDING_STRIDE
        case 0x82D9: RET_TYPE_VAR(type, max_vertex_attrib_relative_offset); break; // GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET
        case 0x82DA: RET_TYPE_VAR(type, max_vertex_attrib_bindings); break; // GL_MAX_VERTEX_ATTRIB_BINDINGS
    }
}

void mglGetBooleanv(GLMContext ctx, GLenum pname, GLboolean *data)
{
    mglGet(ctx, pname, kBool, (void *)data);
}

void mglGetDoublev(GLMContext ctx, GLenum pname, GLdouble *data)
{
    assert(data);

    mglGet(ctx, pname, kDouble, (void *)data);
}

void mglGetFloatv(GLMContext ctx, GLenum pname, GLfloat *data)
{
    assert(data);

    mglGet(ctx, pname, kFloat, (void *)data);
}

void mglGetIntegerv(GLMContext ctx, GLenum pname, GLint *data)
{
    assert(data);

    mglGet(ctx, pname, kInt, (void *)data);
}

const GLubyte *mglGetString(GLMContext ctx, GLenum name)
{
    switch(name)
    {
        case GL_VENDOR:
            return (const GLubyte *)"Mike Larson";

        case GL_RENDERER:
            return (const GLubyte *)"MGL";

        case GL_VERSION:
            return (const GLubyte *)"4.6.0";

        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *)"4.6";

        default:
            assert(0);
    }
}

void mglGetInteger64v(GLMContext ctx, GLenum pname, GLint64 *data)
{
    // Unimplemented function
    assert(0);
}

void mglGetInteger64i_v(GLMContext ctx, GLenum target, GLuint index, GLint64 *data)
{
    // Unimplemented function
    assert(0);
}

const GLubyte  *mglGetStringi(GLMContext ctx, GLenum name, GLuint index)
{
    switch(index)
    {
        case GL_VENDOR: return (const GLubyte *)"Mike Larson";
        case GL_RENDERER: return (const GLubyte *) "MGL";
        case GL_VERSION: return (const GLubyte *) "4.6.0";
        case GL_SHADING_LANGUAGE_VERSION: return (const GLubyte *) "4.6.0";
        case GL_EXTENSIONS: return NULL;

        default:
            assert(0);
    }

    return NULL;
}

void mglGetIntegeri_v(GLMContext ctx, GLenum target, GLuint index, GLint *data)
{
    switch(target)
    {
        case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
            if (index < 3)
            {
                *data = ctx->state.var.max_compute_work_group_count[index];
            }
            break;

        case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
            if (index < 3)
            {
                *data = ctx->state.var.max_compute_work_group_size[index];
            }
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }
}

void mglGetInternalformati64v(GLMContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params)
{
        assert(0);
}

void mglGetInternalformativ(GLMContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params)
{
        assert(0);
}
