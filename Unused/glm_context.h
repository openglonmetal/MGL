//
//  glm_context.h
//  mgl
//
//  Created by Michael Larson on 10/29/21.
//

#include <mach/vm_types.h>
#include "glslang_c_interface.h"
#include "glslang_c_shader_types.h"

#include "glm_dispatch.h"
#include "hash_table.h"
#include "glm_params.h"

#pragma once

#ifndef glm_context_h
#define glm_context_h

#define MAX_ATTRIBS 16
#define MAX_COLOR_ATTACHMENTS 8

/*
 glActiveTexture selects which texture unit subsequent texture state calls will affect. The number of texture units an implementation supports is implementation dependent, but must be at least 80.

*/
#define TEXTURE_UNITS 128
#define TEX_OBJ_RES_NAME    0xcafebeef  // for tex objects.. renderbuffers
#define MAX_CLIP_DISTANCES  8

enum {
    _TEXTURE_BUFFER = 0, // duplicate of _TEXTURE_BUFFER_TARGET
    _ARRAY_BUFFER,
    _ELEMENT_ARRAY_BUFFER,
    _UNIFORM_BUFFER,
    _SHADER_STORAGE_BUFFER,
    _TRANSFORM_FEEDBACK_BUFFER,
    _QUERY_BUFFER,
    _PIXEL_PACK_BUFFER,
    _PIXEL_UNPACK_BUFFER,
    _ATOMIC_COUNTER_BUFFER,
    _COPY_READ_BUFFER,
    _COPY_WRITE_BUFFER,
    _DISPATCH_INDIRECT_BUFFER,
    _DRAW_INDIRECT_BUFFER,
    _MAX_BUFFER_TYPES
};

enum {
    _TEXTURE_BUFFER_TARGET = 0, // duplicate of _TEXTURE_BUFFER
    _TEXTURE_1D,
    _TEXTURE_2D,
    _TEXTURE_3D,
    _TEXTURE_RECTANGLE,
    _TEXTURE_1D_ARRAY,
    _TEXTURE_2D_ARRAY,
    _TEXTURE_CUBE_MAP,
    _TEXTURE_CUBE_MAP_ARRAY,
    _TEXTURE_2D_MULTISAMPLE,
    _TEXTURE_2D_MULTISAMPLE_ARRAY,
    _RENDERBUFFER,
    _MAX_TEXTURE_TYPES
};

static_assert(_TEXTURE_BUFFER == _TEXTURE_BUFFER_TARGET, "_TEXTURE_BUFFER != _TEXTURE_BUFFER_TARGET");

enum {
    _VERTEX_SHADER = 0,
    _TESS_CONTROL_SHADER,
    _TESS_EVALUATION_SHADER,
    _GEOMETRY_SHADER,
    _FRAGMENT_SHADER,
    _COMPUTE_SHADER,
    _MAX_SHADER_TYPES
};

enum {
    _UNIFORM_BASE = 0,
    _TRANSFORM_FEEDBACK_BASE,
    _SHADER_STORAGE_BASE,
    _ATOMIC_COUNTER_BASE,
    _MAX_BASE_TARGET
};

enum {
    _UNKNOWN_RES = 0,
    _UNIFORM_BUFFER_RES,
    _STORAGE_BUFFER_RES,
    _STAGE_INPUT_RES,
    _STAGE_OUTPUT_RES,
    _STORAGE_OUTPUT_RES,
    _ATOMIC_COUNTER_RES,
    _PUSH_CONSTANT_RES,
    _SEPARATE_IMAGE_RES,
    _SEPARATE_SAMPLERS_RES,
    _ACCEL_STRUCT_RES,
    _RAY_QUERY,
    _MAX_SPIRV_RES
};

#define SHADER_MASK_BIT(_TYPE_)    (0x1 << _TYPE_)
#define VERTEX_SHADER_MASK_BIT  SHADER_MASK_BIT(_VERTEX_SHADER)
#define FRAGMENT_SHADER_MASK_BIT  SHADER_MASK_BIT(_FRAGMENT_SHADER)
#define GEOMETRY_SHADER_MASK_BIT  SHADER_MASK_BIT(_GEOMETRY_SHADER)
#define TESS_CONTROL_SHADER_MASK_BIT  SHADER_MASK_BIT(_TESS_CONTROL_SHADER)
#define TESS_EVALUATION_SHADER_MASK_BIT  SHADER_MASK_BIT(_TESS_EVALUATION_SHADER)
#define COMPUTE_SHADER_MASK_BIT  SHADER_MASK_BIT(_COMPUTE_SHADER)

#define DIRTY_BUFFER_DATA   0x1
#define DIRTY_BUFFER_ADDR   (DIRTY_BUFFER_DATA << 1)

#define DIRTY_TEXTURE_LEVEL 0x1
#define DIRTY_TEXTURE_DATA  (DIRTY_TEXTURE_LEVEL << 1)
#define DIRTY_TEXTURE_PARAM  (DIRTY_TEXTURE_DATA << 1)

#define DIRTY_FBO_BINDING   0x1
#define DIRTY_FBO_TEX      (DIRTY_FBO_BINDING << 1)

#define DIRTY_RENDBUF       0x1
#define DIRTY_RENDBUF_TEX   (DIRTY_RENDBUF << 1)

typedef struct BufferData_t {
    GLuint          dirty_bits;
    size_t          buffer_size;
    vm_address_t    buffer_data;
    void            *mtl_data;
} BufferData;

#define BUFFER_IMMUTABLE_STORAGE_FLAG   0x1
#define BUFFER_MAP_PERSISTENT_BIT       (BUFFER_IMMUTABLE_STORAGE_FLAG << 1)

typedef struct Buffer_t {
    GLuint name;
    GLenum target;
    GLuint index;
    GLsizeiptr size;
    GLenum usage;
    GLenum access;
    GLbitfield access_flags;
    GLboolean immutable_storage; // GL_BUFFER_IMMUTABLE_STORAGE
    GLboolean mapped;
    GLuint storage_flags; // GL_BUFFER_STORAGE_FLAGS
    GLsizeiptr mapped_offset;
    GLsizeiptr mapped_length;
    BufferData data;
} Buffer;

typedef struct BufferBaseTarget_t {
    Buffer      *buffer;
    GLsizeiptr  offset;
    GLsizeiptr  size;
} BufferBaseTarget;

typedef struct BufferBase_t {
    GLuint              max_buffers;
    GLuint              enabled_mask;
    BufferBaseTarget    targets[_MAX_BUFFER_TYPES];
} BufferBase;

typedef struct TextureParameter_t {
    GLenum  depth_stencil_mode;
    GLuint  base_level;
    GLfloat border_color[4];
    GLint   border_color_i[4];
    GLuint   border_color_ui[4];
    GLenum  compare_func;
    GLenum  compare_mode;
    GLfloat lod_bias;
    GLenum  min_filter;
    GLenum  mag_filter;
    GLfloat max_anisotropy;
    GLfloat min_lod;
    GLfloat max_lod;
    GLuint  max_level;
    GLenum  swizzle_r;
    GLenum  swizzle_g;
    GLenum  swizzle_b;
    GLenum  swizzle_a;
    GLenum  wrap_s;
    GLenum  wrap_t;
    GLenum  wrap_r;
    void *mtl_data;
} TextureParameter;

typedef struct TextureLevel_t {
    GLboolean complete;
    GLenum internalformat;
    GLuint format;
    GLenum type;
    GLuint width;
    GLuint height;
    GLuint depth;
    size_t pitch;
    GLuint mtl_format;
    size_t  data_size;
    vm_address_t data;
} TextureLevel;

typedef struct Texture_t {
    GLuint dirty_bits;
    GLuint dirty_on_gpu;
    GLboolean is_render_target;
    GLuint name;
    GLuint target;
    GLuint index;
    GLuint mipmapped;
    TextureParameter params;
    // base level params
    GLuint width;
    GLuint height;
    GLuint depth;
    GLenum internalformat;
    GLboolean complete;
    GLuint max_levels;
    GLuint num_levels;
    TextureLevel *levels;
    void    *mtl_data;
} Texture;

typedef struct TextureUnit_t {
    Texture *textures[_MAX_TEXTURE_TYPES];
} TextureUnit;

typedef struct VertexAttrib_t {
    Buffer  *buffer;
    GLuint  size;
    GLenum  type;
    GLuint  normalized;
    GLuint  stride;
    const void *ptr;
} VertexAttrib;

typedef struct VertexElementArray_t {
    Buffer  *buffer;
    GLenum  type;
    GLuint  size;
    const void *ptr;
} VertexElementArray;

typedef struct VertexArray_t {
    GLuint dirty_bits;
    unsigned name;
    unsigned enabled_attribs;
    VertexAttrib attrib[MAX_ATTRIBS];
    VertexElementArray element_array;
    BufferBase  buffer_base;
    void *mtl_data;
} VertexArray;

typedef struct Shader_t {
    GLuint dirty_bits;
    GLuint name;
    GLuint type;
    GLuint glm_type;
    const char *mtl_shader_type_name;
    size_t src_len;
    const char *src;
    glslang_shader_t *compiled_glsl_shader;
    const char *entry_point;
    void *mtl_data;
} Shader;

typedef struct Spirv_t {
    GLuint stage;
    size_t size;
    unsigned int *ir;
    char *msl_str;
} Spirv;

typedef struct SpirvResource_t {
    GLuint  _id;
    GLuint  base_type_id;
    GLuint  type_id;
    const char *name;
    GLuint  set;
    GLuint  binding;
    GLuint  location;
} SpirvResource;

typedef struct SpirvResourceList_t {
    GLuint  count;
    SpirvResource   *list;
} SpirvResourceList;

enum {
    kUniformBuffer,
    kStageInput
};

typedef struct BufferMap_t {
    unsigned    attribute_mask;
    GLuint      buffer_base_index;
    Buffer      *buffer;
} BufferMap;

typedef struct BufferMapList_t {
    GLuint      count;
    BufferMap   buffers[_MAX_BUFFER_TYPES];
} BufferMapList;

typedef struct Program_t {
    GLuint dirty_bits;
    GLuint name;
    Shader *shader_slots[_MAX_SHADER_TYPES];
    glslang_program_t *linked_glsl_program;
    Spirv spirv[_MAX_SHADER_TYPES];
    SpirvResourceList spirv_resources_list[_MAX_SHADER_TYPES][_MAX_SPIRV_RES];
    void *mtl_data;
} Program;

typedef struct Renderbuffer_t {
    GLuint dirty_bits;
    GLuint  name;
    Texture *tex;
} Renderbuffer;

typedef struct FBOAttachment_t {
    GLuint dirty_bits;
    GLuint textarget;   // GL_RENDERBUFFER for renderbuffers
    GLuint texture;
    GLuint level;
    GLuint layer;
    union {
        Texture *tex;
        Renderbuffer *rbo;
    } buf;
} FBOAttachment;

typedef struct Framebuffer_t {
    GLuint dirty_bits;
    GLuint  name;
    GLbitfield color_attachment_bitfield;
    FBOAttachment color_attachments[MAX_COLOR_ATTACHMENTS];
    FBOAttachment depth;
    FBOAttachment stencil;
} Framebuffer;

typedef struct __GLsync {
    GLsizei name;
    void *mtl_event;
#ifdef __cplusplus
} Sync;
#else
} Sync, *__GLsync;
#endif

typedef struct PixelFormat_t {
    GLuint  format;
    GLuint  type;
    GLuint  mtl_pixel_format;
} PixelFormat;

typedef struct GLSLState_t {
    glslang_resource_t  resrc;
    glslang_limits_t    limits;
} GLSLState;

enum {
    dirtyVAO = 0,
    dirtyState,
    dirtyBuffer,
    dirtyTexture,
    dirtyShader,
    dirtyProgram,
    dirtyFBO,
    dirtyDrawable,
    dirtyRenderState,
    dirtyAlphaState
};

typedef struct PixelStore_t {
    GLboolean   swap_bytes;
    GLboolean   lsb_first;
    GLint row_length;
    GLint image_height;
    GLint skip_rows;
    GLint skip_pixels;
    GLint skip_images;
    GLint alignment;
} PixelStore;


#define DIRTY_VAO       (0x1 << dirtyVAO)
#define DIRTY_STATE     (0x1 << dirtyState)
#define DIRTY_BUFFER    (0x1 << dirtyBuffer)
#define DIRTY_TEX       (0x1 << dirtyTexture)
#define DIRTY_SHADER    (0x1 << dirtyShader)
#define DIRTY_PROGRAM   (0x1 << dirtyProgram)
#define DIRTY_FBO       (0x1 << dirtyFBO)
#define DIRTY_DRAWABLE  (0x1 << dirtyDrawable)
#define DIRTY_RENDER_STATE     (0x1 << dirtyRenderState)
#define DIRTY_ALPHA_STATE     (0x1 << dirtyAlphaState)
#define DIRTY_ALL       (0xFFFFFFFF)

typedef struct {
    GLuint dirty_bits;
    GLuint dirty_buffers;
    GLuint dirty_textures;

    // clear request clear_bitmask from glClear to Metal
    GLbitfield  clear_bitmask;

    // opengl state

    // keep these out of the var struct for debugging and access

    GLenum error;   // glGetError

    GLuint draw_buffer; // GL_DRAW_BUFFER
    GLuint read_buffer; // GL_READ_BUFFER
    GLuint max_color_attachments; // GL_MAX_COLOR_ATTACHMENTS
    GLuint max_vertex_attribs; // GL_MAX_VERTEX_ATTRIBS
    GLuint viewport[4]; // GL_VIEWPORT
    GLfloat color_clear_value[4]; // GL_COLOR_CLEAR_VALUE

    VertexArray *vao;
    Texture     *tex;
    Renderbuffer *renderbuffer;
    Framebuffer *framebuffer;
    Framebuffer *readbuffer;

    GLuint      active_texture; // GL_ACTIVE_TEXTURE
    unsigned    active_texture_mask[4];
    Texture     *active_textures[TEXTURE_UNITS];
    TextureUnit texture_units[TEXTURE_UNITS];

    GLsizei sync_name;

    HashTable vao_table;
    HashTable buffer_table;
    HashTable texture_table;
    HashTable shader_table;
    HashTable program_table;
    HashTable renderbuffer_table;
    HashTable framebuffer_table;

    Shader      *shaders[_MAX_SHADER_TYPES];
    Program     *program;
    Buffer      *buffers[_MAX_BUFFER_TYPES];

    // glsl info
    GLSLState   glsl;

    // pixel pack unpack
    PixelStore  pack;
    PixelStore  unpack;
    
    // metal buffer mappings
    BufferMapList vertex_buffer_map_list;
    BufferMapList fragment_buffer_map_list;

    // enable / disable caps
    GLMCaps     caps;

    // hints
    GLMHints    hints;
    
    // put at end, big chunk of yuck
    GLMParams   var;
} GLMState;

static_assert(TEXTURE_UNITS == 128, "active_texture_mask relies on this");

struct GLMMetalFuncs {
    void *mtlObj;
    void *mtlView;

    void (*mtlDeleteMTLBuffer)(GLMContext glm_ctx, void *buf);
    void (*mtlDeleteMTLTexture)(GLMContext glm_ctx, void *tex);

    void (*mtlGetSync)(GLMContext glm_ctx, Sync *sync);
    void (*mtlWaitForSync)(GLMContext glm_ctx, Sync *sync);

    void (*mtlFlush)(GLMContext glm_ctx, bool finish);
    void (*mtlSwapBuffers)(GLMContext glm_ctx);
    
    void (*mtlClearBuffer)(GLMContext glm_ctx, GLuint type, GLbitfield mask);

    void (*mtlBufferSubData)(GLMContext glm_ctx, Buffer *buf, size_t offset, size_t size, const void *ptr);
    void *(*mtlMapUnmapBuffer)(GLMContext glm_ctx, Buffer *buf, size_t offset, size_t size, GLenum access, bool map);
    void (*mtlFlushBufferRange)(GLMContext glm_ctx, Buffer *buf, GLintptr offset, GLsizeiptr length);

    void (*mtlGetTexImage)(GLMContext glm_ctx, Texture *tex, void *pixelBytes, GLuint bytesPerRow, GLuint bytesPerImage, GLint x, GLint y, GLsizei width, GLsizei height, GLuint level, GLuint slice);

    void (*mtlDrawArrays)(GLMContext ctx, GLenum mode, GLint first, GLsizei count);
    void (*mtlDrawElements)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices);
    void (*mtlDrawRangeElements)(GLMContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
    void (*mtlDrawArraysInstanced)(GLMContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
    void (*mtlDrawElementsInstanced)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
    void (*mtlDrawElementsBaseVertex)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
    void (*mtlDrawRangeElementsBaseVertex)(GLMContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
    void (*mtlDrawElementsInstancedBaseVertex)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
    void (*mtlMultiDrawElementsBaseVertex)(GLMContext ctx, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
    void (*mtlDrawArraysIndirect)(GLMContext ctx, GLenum mode, const void *indirect);
    void (*mtlDrawElementsIndirect)(GLMContext ctx, GLenum mode, GLenum type, const void *indirect);
    void (*mtlDrawArraysInstancedBaseInstance)(GLMContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
    void (*mtlDrawElementsInstancedBaseInstance)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
    void (*mtlDrawElementsInstancedBaseVertexBaseInstance)(GLMContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
} ;

typedef struct GLMContextRec_t {
    GLuint      context_flags;

    struct GLMDispatchTable dispatch;
    struct GLMMetalFuncs mtl_funcs;

    GLMState    state;
    GLboolean   assert_on_error;

    PixelFormat pixel_format;
    PixelFormat depth_format;
    PixelFormat stencil_format;

    BufferData  *temp_element_buffer;

    void (* error_func)(GLMContext ctx, const char *func, GLenum type);
} GLMContextRec;


GLMContext createGLMContext(GLenum format, GLenum type,
                            GLenum depth_format, GLenum depth_type,
                            GLenum stencil_format, GLenum stencil_type);

void MGLsetCurrentContext(GLMContext ctx);

enum {
    MGL_PIXEL_FORMAT,
    MGL_PIXEL_TYPE,
    MGL_DEPTH_FORMAT,
    MGL_DEPTH_TYPE,
    MGL_STENCIL_FORMAT,
    MGL_STENCIL_TYPE,
    MGL_CONTEXT_FLAGS
};

#ifdef __cplusplus
extern "C" {
#endif

GLuint sizeForFormatType(GLenum format, GLenum type);
GLuint bicountForFormatType(GLenum format, GLenum type, GLenum component);
GLMContext MGLgetCurrentContext(void);
void MGLget(GLMContext ctx, GLenum param, GLuint *data);

#ifdef __cplusplus
};
#endif

#endif /* glm_context_h */
