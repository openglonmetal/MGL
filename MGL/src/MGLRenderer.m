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
 * MGLRenderer.m
 * MGL
 *
 */

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#import <simd/simd.h>
#import <MetalKit/MetalKit.h>

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

// Header shared between C code here, which executes Metal API commands, and .metal files, which
// uses these types as inputs to the shaders.
//#import "AAPLShaderTypes.h"

#import "MGLRenderer.h"
#import "glm_context.h"

#define TRACE_FUNCTION()    printf("%s\n", __FUNCTION__);

extern void mglDrawBuffer(GLMContext ctx, GLenum buf);

// for resource types SPVC_RESOURCE_TYPE_UNIFORM_BUFFER..
#import "spirv_cross_c.h"

typedef struct SyncList_t {
    GLuint count;
    GLuint  size;
    Sync **list;
} SyncList;

MTLPixelFormat mtlPixelFormatForGLTex(Texture * gl_tex);

typedef struct MGLDrawable_t {
    GLuint width;
    GLuint height;
    id<MTLTexture> drawbuffer;
    id<MTLTexture> depthbuffer;
    id<MTLTexture> stencilbuffer;
} MGLDrawable;

enum {
    _FRONT,
    _BACK,
    _FRONT_LEFT,
    _FRONT_RIGHT,
    _BACK_LEFT,
    _BACK_RIGHT,
    _MAX_DRAW_BUFFERS
};

// Main class performing the rendering
@implementation MGLRenderer
{
    NSView *_view;

    CAMetalLayer *_layer;
    id<CAMetalDrawable> _drawable;

    GLMContext  ctx;    // context macros need this exact name

    id<MTLDevice> _device;

    MGLDrawable _drawBuffers[_MAX_DRAW_BUFFERS];

    MTLBlendFactor _src_blend_rgb_factor[MAX_COLOR_ATTACHMENTS];
    MTLBlendFactor _dst_blend_rgb_factor[MAX_COLOR_ATTACHMENTS];
    MTLBlendFactor _src_blend_alpha_factor[MAX_COLOR_ATTACHMENTS];
    MTLBlendFactor _dst_blend_alpha_factor[MAX_COLOR_ATTACHMENTS];
    MTLBlendOperation _rgb_blend_operation[MAX_COLOR_ATTACHMENTS];
    MTLBlendOperation _alpha_blend_operation[MAX_COLOR_ATTACHMENTS];
    MTLColorWriteMask _color_mask[MAX_COLOR_ATTACHMENTS];

    // The command queue used to pass commands to the device.
    id<MTLCommandQueue> _commandQueue;

    // The render pipeline generated from the vertex and fragment shaders in the .metal shader file.
    id<MTLRenderPipelineState> _pipelineState;

    // render pass descriptor containts the binding information for VAO's and such
    MTLRenderPassDescriptor *_renderPassDescriptor;

    // each pass a new command buffer is created
    id<MTLCommandBuffer> _currentCommandBuffer;
    SyncList  *_currentCommandBufferSyncList;

    id<MTLRenderCommandEncoder> _currentRenderEncoder;

    GLuint _blitOperationComplete;

    id<MTLEvent> _currentEvent;
    GLsizei _currentSyncName;
}

MTLVertexFormat glTypeSizeToMtlType(GLuint type, GLuint size, bool normalized)
{
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
            if (normalized)
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUCharNormalized;
                    case 2: return MTLVertexFormatUChar2Normalized;
                    case 3: return MTLVertexFormatUChar3Normalized;
                    case 4: return MTLVertexFormatUChar4Normalized;
                }
            }
            else
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUChar;
                    case 2: return MTLVertexFormatUChar2;
                    case 3: return MTLVertexFormatUChar3;
                    case 4: return MTLVertexFormatUChar4;
                }
            }
            break;

        case GL_BYTE:
            if (normalized)
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatCharNormalized;
                    case 2: return MTLVertexFormatChar2Normalized;
                    case 3: return MTLVertexFormatChar3Normalized;
                    case 4: return MTLVertexFormatChar4Normalized;
                }
            }
            else
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatChar;
                    case 2: return MTLVertexFormatChar2;
                    case 3: return MTLVertexFormatChar3;
                    case 4: return MTLVertexFormatChar4;
                }
            }
            break;

        case GL_UNSIGNED_SHORT:
            if (normalized)
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUShortNormalized;
                    case 2: return MTLVertexFormatUShort2Normalized;
                    case 3: return MTLVertexFormatUShort3Normalized;
                    case 4: return MTLVertexFormatUShort4Normalized;
                }
            }
            else
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUShort;
                    case 2: return MTLVertexFormatUShort2;
                    case 3: return MTLVertexFormatUShort3;
                    case 4: return MTLVertexFormatUShort4;
                }
            }
            break;

        case GL_SHORT:
            if (normalized)
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUShortNormalized;
                    case 2: return MTLVertexFormatShort2Normalized;
                    case 3: return MTLVertexFormatShort3Normalized;
                    case 4: return MTLVertexFormatShort4Normalized;
                }
            }
            else
            {
                switch(size)
                {
                    case 1: return MTLVertexFormatUShort;
                    case 2: return MTLVertexFormatShort2;
                    case 3: return MTLVertexFormatShort3;
                    case 4: return MTLVertexFormatShort4;
                }
            }
            break;

            case GL_HALF_FLOAT:
                switch(size)
                {
                    case 1: return MTLVertexFormatHalf;
                    case 2: return MTLVertexFormatHalf2;
                    case 3: return MTLVertexFormatHalf3;
                    case 4: return MTLVertexFormatHalf4;
                }
                break;

            case GL_FLOAT:
                switch(size)
                {
                    case 1: return MTLVertexFormatFloat;
                    case 2: return MTLVertexFormatFloat2;
                    case 3: return MTLVertexFormatFloat3;
                    case 4: return MTLVertexFormatFloat4;
                }
                break;

            case GL_INT:
                switch(size)
                {
                    case 1: return MTLVertexFormatInt;
                    case 2: return MTLVertexFormatInt2;
                    case 3: return MTLVertexFormatInt3;
                    case 4: return MTLVertexFormatInt4;
                }
                break;

            case GL_UNSIGNED_INT:
                switch(size)
                {
                    case 1: return MTLVertexFormatUInt;
                    case 2: return MTLVertexFormatUInt2;
                    case 3: return MTLVertexFormatUInt3;
                    case 4: return MTLVertexFormatUInt4;
                }
                break;

            case GL_RGB10:
                if (normalized)
                    return MTLVertexFormatInt1010102Normalized;
                break;

            case GL_UNSIGNED_INT_10_10_10_2:
                if (normalized)
                    return MTLVertexFormatInt1010102Normalized;
                break;
        }

    return MTLVertexFormatInvalid;
}

#pragma mark debug code
void printDirtyBit(unsigned dirty_bits, unsigned dirty_flag, const char *name)
{
    if (dirty_bits & dirty_flag)
        printf("%s", name);
}

void logDirtyBits(GLMContext ctx)
{
    if(ctx->state.dirty_bits)
    {
        if (ctx->state.dirty_bits & DIRTY_ALL_BIT)
        {
            printDirtyBit(ctx->state.dirty_bits, DIRTY_ALL_BIT, "DIRTY_ALL_BIT set");
        }
        else
        {
            printDirtyBit(ctx->state.dirty_bits, DIRTY_VAO, "DIRTY_VAO ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_STATE, "DIRTY_STATE ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_BUFFER, "DIRTY_BUFFER ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_TEX, "DIRTY_TEX ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_TEX_PARAM, "DIRTY_TEX_PARAM ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_TEX_BINDING, "DIRTY_TEX_BINDING ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_SAMPLER, "DIRTY_SAMPLER ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_SHADER, "DIRTY_SHADER ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_PROGRAM, "DIRTY_PROGRAM ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_FBO, "DIRTY_FBO ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_DRAWABLE, "DIRTY_DRAWABLE ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_RENDER_STATE, "DIRTY_RENDER_STATE ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_ALPHA_STATE, "DIRTY_ALPHA_STATE ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_IMAGE_UNIT_STATE, "DIRTY_IMAGE_UNIT_STATE ");
            printDirtyBit(ctx->state.dirty_bits, DIRTY_BUFFER_BASE_STATE, "DIRTY_BUFFER_BASE_STATE ");
        }
        printf("\n");
    }
}

#pragma mark buffer objects
- (void) bindMTLBuffer:(Buffer *) ptr
{
    MTLResourceOptions options;

    if (ptr->storage_flags & GL_CLIENT_STORAGE_BIT)
    {
        options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeManaged;

        // ways we will only write to this
        if ((ptr->storage_flags & GL_MAP_READ_BIT) == 0)
        {
            options |= MTLResourceCPUCacheModeWriteCombined;
        }

        id<MTLBuffer> buffer = [_device newBufferWithBytesNoCopy: (void *)(ptr->data.buffer_data)
                                                     length: ptr->data.buffer_size
                                                    options: options
                                                deallocator: ^(void *pointer, NSUInteger length)
                                                {
                                                    kern_return_t err;
                                                    err = vm_deallocate((vm_map_t) mach_task_self(),
                                                                        (vm_address_t) pointer,
                                                                        length);
                                                    assert(err == 0);
                                                }];

        ptr->data.mtl_data = (void *)CFBridgingRetain(buffer);
    }
    else
    {
        // need to figure this out for non-client storage
        options = MTLResourceCPUCacheModeDefaultCache | MTLResourceStorageModeManaged;

        // we will only write to this
        if ((ptr->storage_flags & GL_MAP_READ_BIT) == 0)
        {
            options |= MTLResourceCPUCacheModeWriteCombined;
        }

        id<MTLBuffer> buffer = [_device newBufferWithLength: ptr->data.buffer_size
                                                    options: options];
        assert(buffer);

        // a backing can allocated initially delete it and point the
        // backing data to the MTL buffer
        if (ptr->data.buffer_data)
        {
            void *data;

            data = buffer.contents;
            memcpy(data, (void *)ptr->data.buffer_data, ptr->data.buffer_size);

            [buffer didModifyRange:NSMakeRange(0, ptr->data.buffer_size)];

            kern_return_t err;
            err = vm_deallocate((vm_map_t) mach_task_self(),
                                (vm_address_t) ptr->data.buffer_data,
                                ptr->data.buffer_size);

            assert(err == 0);

            ptr->data.buffer_data = (vm_address_t)data;
        }

        ptr->data.mtl_data = (void *)CFBridgingRetain(buffer);
    }
}

- (bool) mapGLBuffersToMTLBufferMap:(BufferMapList *)buffer_map stage: (int) stage
{
    int count;
    int mapped_buffers;
    struct {
        int spvc_type;
        int gl_buffer_type;
    } mapped_types[4] = {
        {SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, _UNIFORM_BUFFER},
        {SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT, _UNIFORM_CONSTANT},
        {SPVC_RESOURCE_TYPE_STORAGE_BUFFER, _SHADER_STORAGE_BUFFER},
        {SPVC_RESOURCE_TYPE_ATOMIC_COUNTER, _ATOMIC_COUNTER_BUFFER}
    };

    buffer_map->count = 0;

    for(int type=0; type<4; type++)
    {
        int spvc_type;
        int gl_buffer_type;

        spvc_type = mapped_types[type].spvc_type;
        gl_buffer_type = mapped_types[type].gl_buffer_type;

        count = [self getProgramBindingCount: stage type: spvc_type];
        if (count)
        {
            int buffers_to_be_mapped = count;

            for (int i=0; buffers_to_be_mapped; i++)
            {
                GLuint spirv_binding;
                Buffer *buf;

                // get the ubo binding from spirv
                spirv_binding = [self getProgramBinding:stage type:spvc_type index: i];

                buf = ctx->state.buffer_base[gl_buffer_type].buffers[spirv_binding].buf;

                if (buf)
                {
                    buffer_map->buffers[buffer_map->count].attribute_mask = 0;
                    buffer_map->buffers[buffer_map->count].buffer_base_index = spirv_binding;
                    buffer_map->buffers[buffer_map->count].buf = buf;
                    buffer_map->count++;
                    buffers_to_be_mapped--;
                }
                else
                {
                    ctx->error_func(ctx, __FUNCTION__, GL_INVALID_OPERATION);

                    return false;
                }

                // endless loop
                RETURN_FALSE_ON_FAILURE(i < MAX_ATTRIBS);
            }
        }
    }

    // bind vao attribs to buffers (attribs can share the same buffer)
    if (stage == _VERTEX_SHADER)
    {
        int vao_buffer_start;

        count = [self getProgramBindingCount: stage type: SPVC_RESOURCE_TYPE_STAGE_INPUT];
        mapped_buffers = 0;

        // vao buffers start after the uniforms and shader buffers
        vao_buffer_start = buffer_map->count;
        assert(buffer_map->count < ctx->state.max_vertex_attribs);
        buffer_map->buffers[vao_buffer_start].buf = NULL;

        // create attribute map
        //
        // we need to cache this mapping, its called on each draw command
        //
        for(int att=0;att<ctx->state.max_vertex_attribs; att++)
        {
            if (VAO_STATE(enabled_attribs) & (0x1 << att))
            {
                // shouldn't get here, validateVAO should have failed
                assert(VAO_ATTRIB_STATE(att).buffer);

                // check all the buffers for metal objects
                Buffer *gl_buffer;
                Buffer *map_buffer;

                gl_buffer = VAO_ATTRIB_STATE(att).buffer;

                // check start for map... then check
                map_buffer = buffer_map->buffers[vao_buffer_start].buf;

                // empty slot map it here, only works on first buffer..
                if (map_buffer == NULL)
                {
                    // map the buffer object to a metal vertex index
                    assert(buffer_map->count < ctx->state.max_vertex_attribs);
                    buffer_map->buffers[vao_buffer_start].attribute_mask |= (0x1 << att);
                    buffer_map->buffers[vao_buffer_start].buf = gl_buffer;
                    buffer_map->count++;

                    mapped_buffers++;
                }
                else
                {
                    bool found_buffer = false;

                    // find vao attrib with same buffer
                    for (int map=vao_buffer_start;
                         (found_buffer == false) && map<buffer_map->count;
                         map++)
                    {
                        // we need to check name and target, not pointers..
                        if ((map_buffer->name == gl_buffer->name) &&
                            (map_buffer->target == gl_buffer->target))
                        {
                            // include it the list of attributes
                            buffer_map->buffers[map].attribute_mask |= (0x1 << att);
                            found_buffer = true;
                            mapped_buffers++;
                            break;
                        }
                    }

                    if (found_buffer == false)
                    {
                        // map the next buffer object to a metal vertex index
                        assert(buffer_map->count < ctx->state.max_vertex_attribs);
                        buffer_map->buffers[buffer_map->count].attribute_mask = (0x1 << att);
                        buffer_map->buffers[buffer_map->count].buf = gl_buffer;
                        buffer_map->count++;

                        mapped_buffers++;
                    }
                }
            }

            if ((VAO_STATE(enabled_attribs) >> (att+1)) == 0)
                break;
        }

        assert(mapped_buffers == count);
    }
    else if (stage == _COMPUTE_SHADER)
    {
    }

    return true;
}

- (bool) mapBuffersToMTL
{
    if ([self mapGLBuffersToMTLBufferMap: &ctx->state.vertex_buffer_map_list stage:_VERTEX_SHADER] == false)
        return false;

    if ([self mapGLBuffersToMTLBufferMap: &ctx->state.fragment_buffer_map_list stage:_FRAGMENT_SHADER] == false)
        return false;

    return true;
}

- (bool) updateDirtyBuffer:(Buffer *)ptr
{
    if (ptr->data.dirty_bits & DIRTY_BUFFER_ADDR)
    {
        if (ptr->data.mtl_data == NULL)
        {
            [self bindMTLBuffer: ptr];
            RETURN_FALSE_ON_NULL(ptr->data.mtl_data);

            // clear dirty bits
            ptr->data.dirty_bits = 0;
        }
    }
    else if (ptr->data.dirty_bits & DIRTY_BUFFER_DATA)
    {
        if (ptr->data.mtl_data == NULL)
        {
            [self bindMTLBuffer: ptr];
            RETURN_FALSE_ON_NULL(ptr->data.mtl_data);

            // clear dirty bits
            ptr->data.dirty_bits = 0;

            // we had to create a buffer so no need to update data
            return true;
        }

        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(ptr->data.mtl_data);
        assert(buffer);

        // clear dirty bits if not mapped as coherent
        // this will cause us to keep loading the buffer and keep the GPU
        // contents in check for EVERY drawing operation
        if (ptr->access & GL_MAP_COHERENT_BIT)
        {
            [buffer didModifyRange: NSMakeRange(ptr->mapped_offset, ptr->mapped_length)];

            ptr->data.dirty_bits = DIRTY_BUFFER_DATA;
        }
        else
        {
            [buffer didModifyRange: NSMakeRange(0, ptr->data.buffer_size)];

            ptr->data.dirty_bits = 0;
        }
    }
    else
    {
        assert(0);
    }

    return true;
}

- (bool) updateDirtyBaseBufferList: (BufferMapList *)buffer_map_list
{
    // update vbos, some vbos may not have metal buffers yet
    for(int i=0;i<buffer_map_list->count; i++)
    {
        Buffer *gl_buffer;

        gl_buffer = buffer_map_list->buffers[i].buf;

        if (gl_buffer)
        {
            if (gl_buffer->data.dirty_bits)
            {
                RETURN_FALSE_ON_FAILURE([self updateDirtyBuffer: gl_buffer]);;
            }
        }
    }

    return true;
}

- (bool) bindBuffersToCurrentRenderEncoder
{
    assert(_currentRenderEncoder);

    for(int i=0; i<ctx->state.vertex_buffer_map_list.count; i++)
    {
        Buffer *ptr;

        ptr = ctx->state.vertex_buffer_map_list.buffers[i].buf;

        assert(ptr);
        assert(ptr->data.mtl_data);

        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(ptr->data.mtl_data);
        assert(buffer);

        [_currentRenderEncoder setVertexBuffer:buffer offset:0 atIndex:i ];
    }

    for(int i=0; i<ctx->state.fragment_buffer_map_list.count; i++)
    {
        Buffer *ptr;

        ptr = ctx->state.fragment_buffer_map_list.buffers[i].buf;

        assert(ptr);
        assert(ptr->data.mtl_data);

        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(ptr->data.mtl_data);
        assert(buffer);

        [_currentRenderEncoder setFragmentBuffer:buffer offset:0 atIndex:i ];
    }

    return true;
}

- (int) getVertexBufferIndexWithAttributeSet: (int) attribute
{
    for(int i=0; i<ctx->state.vertex_buffer_map_list.count; i++)
    {
        if (ctx->state.vertex_buffer_map_list.buffers[i].attribute_mask & (0x1 << attribute))
            return i;
    }

    assert(0);

    return 0;
}

#pragma mark textures

- (void)swizzleTexDesc:(MTLTextureDescriptor *)tex_desc forTex:(Texture*)tex
{
    unsigned channel_r, channel_g, channel_b, channel_a;

    channel_r = channel_g = channel_b = channel_a = 0;

    switch(tex->params.swizzle_r)
    {
        case GL_RED: channel_r = MTLTextureSwizzleRed; break;
        case GL_GREEN: channel_r = MTLTextureSwizzleGreen; break;
        case GL_BLUE: channel_r = MTLTextureSwizzleBlue; break;
        case GL_ALPHA: channel_r = MTLTextureSwizzleAlpha; break;
        default: assert(0); break;
    }

    switch(tex->params.swizzle_g)
    {
        case GL_RED: channel_g = MTLTextureSwizzleRed; break;
        case GL_GREEN: channel_g = MTLTextureSwizzleGreen; break;
        case GL_BLUE: channel_g = MTLTextureSwizzleBlue; break;
        case GL_ALPHA: channel_g = MTLTextureSwizzleAlpha; break;
        default: assert(0); break;
    }

    switch(tex->params.swizzle_b)
    {
        case GL_RED: channel_b = MTLTextureSwizzleRed; break;
        case GL_GREEN: channel_b = MTLTextureSwizzleGreen; break;
        case GL_BLUE: channel_b = MTLTextureSwizzleBlue; break;
        case GL_ALPHA: channel_b = MTLTextureSwizzleAlpha; break;
        default: assert(0); break;
    }

    switch(tex->params.swizzle_a)
    {
        case GL_RED: channel_a = MTLTextureSwizzleRed; break;
        case GL_GREEN: channel_a = MTLTextureSwizzleGreen; break;
        case GL_BLUE: channel_a = MTLTextureSwizzleBlue; break;
        case GL_ALPHA: channel_a = MTLTextureSwizzleAlpha; break;
        default: assert(0); break;
    }

    tex_desc.swizzle = MTLTextureSwizzleChannelsMake(channel_r, channel_g, channel_b, channel_a);
}

- (id<MTLTexture>) createMTLTextureFromGLTexture:(Texture *) tex
{
    NSUInteger width, height, depth;

    MTLTextureDescriptor *tex_desc;
    MTLTextureType tex_type;
    MTLPixelFormat pixelFormat;
    uint num_faces;
    BOOL mipmapped;
    BOOL is_array;

    num_faces = 1;
    is_array = false;

    switch(tex->target)
    {
//        case GL_TEXTURE_1D: tex_type = MTLTextureType1D; break;
        case GL_TEXTURE_1D: tex_type = MTLTextureType2D; break;
        case GL_RENDERBUFFER: tex_type = MTLTextureType2D; break;
        case GL_TEXTURE_1D_ARRAY: tex_type = MTLTextureType1DArray; is_array = true; break;
        case GL_TEXTURE_2D: tex_type = MTLTextureType2D; break;
        case GL_TEXTURE_2D_ARRAY: tex_type = MTLTextureType2DArray; is_array = true; break;
        // case GL_TEXTURE_2D_MULTISAMPLE: tex_type = MTLTextureType2DMultisample; break;

        case GL_TEXTURE_CUBE_MAP:
            num_faces = 6;
            tex_type = MTLTextureTypeCube;
            break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
            num_faces = 6;
            tex_type = MTLTextureTypeCubeArray;
            break;

        case GL_TEXTURE_3D: tex_type = MTLTextureType3D; break;
        // case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: tex_type = MTLTextureType2DMultisampleArray;  is_array = true; break;
        // case GL_TEXTURE_BUFFER: tex_type = MTLTextureTypeTextureBuffer; break;

        default:
            assert(0);
            break;
    }

    // verify completeness of texture when used
    if (tex->num_levels > 1)
    {
        // mipmapped texture
        if (tex->num_levels != tex->mipmap_levels)
        {
            return NULL;
        }

        for(int face=0; face<num_faces; face++)
        {
            for (int i=0; i<tex->num_levels; i++)
            {
                // incomplete texture
                if (tex->faces[face].levels[i].complete == false)
                    return NULL;
            }
        }

        tex->mipmapped = true;
    }
    else if (tex->num_levels == 1)
    {
        // single level texture
        // incomplete texture
        for(int face=0; face<num_faces; face++)
        {
            if (tex->faces[face].levels[0].complete == false)
                return NULL;
        }
    }
    else
    {
        // not sure how we got here
        assert(0);
        return NULL;
    }
    tex->complete = true;

    pixelFormat = mtlPixelFormatForGLTex(tex);
    assert(pixelFormat != MTLPixelFormatInvalid);

    width = tex->width;
    height = tex->height;
    depth = tex->depth;
    mipmapped = tex->mipmapped == 1;

    tex_desc = [[MTLTextureDescriptor alloc] init];
    tex_desc.textureType = tex_type;
    tex_desc.pixelFormat = pixelFormat;
    tex_desc.width = width;
    tex_desc.height = height;

    if (tex->mtl_requires_private_storage)
    {
        tex_desc.storageMode = MTLStorageModePrivate;
    }

    if (is_array)
    {
        tex_desc.arrayLength = depth;
        tex_desc.depth = 1;
    }
    else
    {
        tex_desc.depth = depth;
    }

    if (mipmapped)
    {
        tex_desc.mipmapLevelCount = tex->mipmap_levels;
    }

    switch(tex->access)
    {
        case GL_READ_ONLY:
            tex_desc.usage = MTLTextureUsageShaderRead; break;
        case GL_WRITE_ONLY:
            tex_desc.usage = MTLTextureUsageShaderWrite; break;
        case GL_READ_WRITE:
            tex_desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite; break;
        default:
            assert(0);
            break;
    }

    if (tex->is_render_target)
    {
        tex_desc.usage |= MTLTextureUsageRenderTarget;
    }

    assert(tex_desc);

    if (tex->params.swizzled)
    {
        [self swizzleTexDesc:tex_desc forTex:tex];
    }

    id<MTLTexture> texture = [_device newTextureWithDescriptor:tex_desc];
    assert(texture);

    if (tex->dirty_bits & DIRTY_TEXTURE_DATA)
    {
        MTLRegion region;

        for(int face=0; face<num_faces; face++)
        {
            for (int level=0; level<tex->num_levels; level++)
            {
                width = tex->faces[face].levels[level].width;
                height = tex->faces[face].levels[level].height;
                depth = tex->faces[face].levels[level].depth;

                if (depth > 1)
                    region = MTLRegionMake3D(0,0,0,width,height,depth);
                else if (height > 1)
                    region = MTLRegionMake2D(0,0,width,height);
                else
                    region = MTLRegionMake1D(0,width);

                NSUInteger bytesPerRow;
                NSUInteger bytesPerImage;

                if (tex_type == MTLTextureType3D)
                {
                    // ogl considers an image a "row".. metal must be different
                    bytesPerRow = tex->faces[face].levels[level].pitch;
                    assert(bytesPerRow);

                    bytesPerImage = bytesPerRow * height;

                    [texture replaceRegion:region mipmapLevel:level slice:0 withBytes:(void *)tex->faces[face].levels[level].data bytesPerRow:bytesPerRow bytesPerImage:bytesPerImage];
                }
                else
                {
                    bytesPerRow = tex->faces[face].levels[level].pitch;
                    assert(bytesPerRow);

                    bytesPerImage = tex->faces[face].levels[level].data_size;
                    assert(bytesPerImage);

                    if (is_array)
                    {
                        GLuint num_layers;
                        size_t offset;
                        GLubyte *tex_data;

                        num_layers = tex->depth;

                        // adjust GL to metal bytesPerImage
                        bytesPerImage /= num_layers;

                        if (depth > 1) // 2d array
                            region = MTLRegionMake3D(0,0,0,width,height,1);
                        else if (height >= 1) // 1d array
                            region = MTLRegionMake2D(0,0,width,1);
                        else // ?
                            assert(0);

                        for(int layer=0; layer<num_layers; layer++)
                        {
                            offset = bytesPerImage * layer;

                            tex_data = (GLubyte *)tex->faces[face].levels[level].data;
                            tex_data += offset;

                            [texture replaceRegion:region mipmapLevel:level slice:layer withBytes:(void *)tex_data bytesPerRow:bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage];
                        }
                    }
                    else
                    {
                        printf("tex id data update %d\n", tex->name);

                        [texture replaceRegion:region mipmapLevel:level slice:face withBytes:(void *)tex->faces[face].levels[level].data bytesPerRow:bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage];
                    }
                }
            }
        }
    }

    tex->dirty_bits = 0;

    return texture;
}

- (id<MTLSamplerState>) createMTLSamplerForTexParam:(TextureParameter *)tex_param target:(GLuint)target
{
    MTLSamplerDescriptor *samplerDescriptor;

    samplerDescriptor = [MTLSamplerDescriptor new];
    assert(samplerDescriptor);

    switch(tex_param->min_filter)
    {
        case GL_NEAREST:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            break;

        case GL_LINEAR:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            samplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
            break;

        case GL_LINEAR_MIPMAP_NEAREST:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
            samplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
            break;

        case GL_NEAREST_MIPMAP_LINEAR:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            samplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
            break;

        case GL_LINEAR_MIPMAP_LINEAR:
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
            samplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
            break;

        default:
            assert(0);
            break;
    }

    switch(tex_param->mag_filter)
    {
        case GL_NEAREST:
            samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
            break;

        case GL_LINEAR:
            samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
            break;

        default:
            assert(0);
            break;
    }

    //     @property (nonatomic) NSUInteger maxAnisotropy;
    if (tex_param->max_anisotropy > 1.0)
    {
        samplerDescriptor.maxAnisotropy = tex_param->max_anisotropy;
    }

    //    @property (nonatomic) MTLSamplerAddressMode sAddressMode;
    //    @property (nonatomic) MTLSamplerAddressMode tAddressMode;
    //    @property (nonatomic) MTLSamplerAddressMode rAddressMode;
    for (int i=0; i<3; i++)
    {
        MTLSamplerAddressMode mode = 0;
        GLenum type = 0;

        switch(i)
        {
            case 0: type = tex_param->wrap_s; break;
            case 1: type = tex_param->wrap_t; break;
            case 2: type = tex_param->wrap_r; break;
        }

        switch(type)
        {
            case GL_CLAMP_TO_EDGE:
                mode = MTLSamplerAddressModeClampToEdge;
                break;

            case GL_CLAMP_TO_BORDER:
                mode = MTLSamplerAddressModeClampToBorderColor;
                break;

            case GL_MIRRORED_REPEAT:
                mode = MTLSamplerAddressModeMirrorRepeat;
                break;

            case GL_REPEAT:
                mode = MTLSamplerAddressModeRepeat;
                break;

            case GL_MIRROR_CLAMP_TO_EDGE:
                mode = MTLSamplerAddressModeMirrorClampToEdge;
                break;

    //        case GL_CLAMP_TO_ZERO_MGL_EXT:
    //            mode = MTLSamplerAddressModeClampToZero;
    //            break;

            default:
                assert(0);
                break;
        }

        switch(i)
        {
            case 0: samplerDescriptor.sAddressMode = mode; break;
            case 1: samplerDescriptor.tAddressMode = mode; break;
            case 2: samplerDescriptor.rAddressMode = mode; break;
        }
    }

    if ((tex_param->border_color[0] == 0.0) &&
        (tex_param->border_color[1] == 0.0) &&
        (tex_param->border_color[2] == 0.0))
    {
        if (tex_param->border_color[3] == 0.0)
        {
            samplerDescriptor.borderColor = MTLSamplerBorderColorTransparentBlack;
        }
        else if (tex_param->border_color[3] == 1.0)
        {
            samplerDescriptor.borderColor = MTLSamplerBorderColorOpaqueBlack;
        }
    }
    else    if ((tex_param->border_color[0] == 1.0) &&
                (tex_param->border_color[1] == 1.0) &&
                (tex_param->border_color[2] == 1.0) &&
                (tex_param->border_color[3] == 1.0))
    {
        samplerDescriptor.borderColor = MTLSamplerBorderColorOpaqueWhite;
    }
    else
    {
        assert(0);
    }

    if (target == GL_TEXTURE_RECTANGLE)
    {
        if ((tex_param->wrap_s == GL_CLAMP_TO_EDGE) &&
            (tex_param->wrap_t == GL_CLAMP_TO_EDGE) &&
            (tex_param->wrap_r == GL_CLAMP_TO_EDGE))
        {
            samplerDescriptor.normalizedCoordinates = false;
        }
        else
        {
            printf("Non-normalized coordinates should only be used with 1D and 2D textures with the ClampToEdge wrap mode, otherwise the results of sampling are undefined.");
        }
    }

    // @property (nonatomic) BOOL lodAverage API_AVAILABLE(ios(9.0), macos(11.0), macCatalyst(14.0));


    // @property (nonatomic) MTLCompareFunction compareFunction API_AVAILABLE(macos(10.11), ios(9.0));
    switch(tex_param->compare_func)
    {
        case GL_LEQUAL:
            samplerDescriptor.compareFunction = MTLCompareFunctionLessEqual;
            break;

        case GL_GEQUAL:
            samplerDescriptor.compareFunction = MTLCompareFunctionGreaterEqual;
            break;

        case GL_LESS:
            samplerDescriptor.compareFunction = MTLCompareFunctionLess;
            break;

        case GL_GREATER:
            samplerDescriptor.compareFunction = MTLCompareFunctionGreater;
            break;

        case GL_EQUAL:
            samplerDescriptor.compareFunction = MTLCompareFunctionEqual;
            break;

        case GL_NOTEQUAL:
            samplerDescriptor.compareFunction = MTLCompareFunctionNotEqual;
            break;

        case GL_ALWAYS:
            samplerDescriptor.compareFunction = MTLCompareFunctionAlways;
            break;

        case GL_NEVER:
            samplerDescriptor.compareFunction = MTLCompareFunctionNever;
            break;

        default:
            assert(0);
            break;
    }

    id<MTLSamplerState> sampler = [_device newSamplerStateWithDescriptor:samplerDescriptor];
    assert(sampler);

    return sampler;
}

- (bool) bindTexturesToCurrentRenderEncoder
{
    GLuint count;

    // iterate shader storage buffers
    count = [self getProgramBindingCount: _FRAGMENT_SHADER type: SPVC_RESOURCE_TYPE_SAMPLED_IMAGE];
    if (count)
    {
        int textures_to_be_mapped = count;

        // something is very wrong..
        assert(textures_to_be_mapped < TEXTURE_UNITS);

        for (int i=0; textures_to_be_mapped > 0; i++)
        {
            RETURN_FALSE_ON_FAILURE(i < count);

            GLuint spirv_binding;
            Texture *ptr;

            spirv_binding = [self getProgramBinding:_FRAGMENT_SHADER type:SPVC_RESOURCE_TYPE_SAMPLED_IMAGE index: i];

            ptr = STATE(active_textures[spirv_binding]);

            if (ptr)
            {
                id<MTLTexture> texture;

                RETURN_FALSE_ON_FAILURE([self bindMTLTexture: ptr]);
                assert(ptr->mtl_data);

                texture = (__bridge id<MTLTexture>)(ptr->mtl_data);
                assert(texture);

                id<MTLSamplerState> sampler;

                // late binding of texture samplers.. but its better than scanning all texture_samplers
                // texture samplers take priority over texture parameters
                if(STATE(texture_samplers[spirv_binding]))
                {
                    Sampler *gl_sampler;

                    gl_sampler = STATE(texture_samplers[spirv_binding]);

                    // delete existing sampler if dirty
                    if (gl_sampler->dirty_bits)
                    {
                        if (gl_sampler->mtl_data)
                        {
                            CFBridgingRelease(gl_sampler->mtl_data);
                            gl_sampler->mtl_data = NULL;
                        }
                    }

                    if (gl_sampler->mtl_data == NULL)
                    {
                        gl_sampler->mtl_data = (void *)CFBridgingRetain([self createMTLSamplerForTexParam:&gl_sampler->params target:ptr->target]);
                        gl_sampler->dirty_bits = 0;
                    }

                    sampler = (__bridge id<MTLSamplerState>)(gl_sampler->mtl_data);
                    assert(sampler);
                }
                else
                {
                    sampler = (__bridge id<MTLSamplerState>)(ptr->params.mtl_data);
                    assert(sampler);
                }

                [_currentRenderEncoder setFragmentTexture:texture atIndex:spirv_binding];
                [_currentRenderEncoder setFragmentSamplerState:sampler atIndex:spirv_binding];

                textures_to_be_mapped--;
            }
        }
    }

    return true;
}

#pragma mark framebuffers

extern bool isColorAttachment(GLMContext ctx, GLuint attachment);
extern FBOAttachment *getFBOAttachment(GLMContext ctx, Framebuffer *fbo, GLenum attachment);

-(void)mtlBlitFramebuffer:(GLMContext)glm_ctx srcX0:(size_t)srcX0 srcY0:(size_t)srcY0 srcX1:(size_t)srcX1 srcY1:(size_t)srcY1 dstX0:(size_t)dstX0 dstY0:(size_t)dstY0 dstX1:(size_t)dstX1 dstY1:(size_t)dstY1 mask:(size_t)mask filter:(GLuint)filter
{
    Framebuffer * readfbo, * drawfbo;
    //int readtex, drawtex;

    readfbo = ctx->state.readbuffer;
    assert(readfbo);

    id<MTLTexture> readtexid;

    if (readfbo==NULL) {
        assert(_drawable);
        readtexid = _drawable.texture;
    } else {
        assert(readfbo);
        FBOAttachment * fboa = getFBOAttachment(ctx, readfbo, STATE(read_buffer));
        assert(fboa);
        Texture * readtexobj;
        if (fboa->textarget == GL_RENDERBUFFER)
        {
            readtexobj = fboa->buf.rbo->tex;
        }
        else
        {
            readtexobj = fboa->buf.tex;
        }
        assert(readtexobj);
        readtexid = (__bridge id<MTLTexture>)(readtexobj->mtl_data);
        assert(readtexid);
    }


    drawfbo = ctx->state.framebuffer;

    id<MTLTexture> drawtexid;
    if (drawfbo==NULL) {
        assert(_drawable);
        drawtexid = _drawable.texture;
    } else {
        assert(drawfbo);
        FBOAttachment * fboa = getFBOAttachment(ctx, drawfbo, STATE(draw_buffer));
        assert(fboa);
        Texture * drawtexobj;
        if (fboa->textarget == GL_RENDERBUFFER)
        {
            drawtexobj = fboa->buf.rbo->tex;
        }
        else
        {
            drawtexobj = fboa->buf.tex;
        }
        assert(drawtexobj);
        drawtexid = (__bridge id<MTLTexture>)(drawtexobj->mtl_data);
        assert(drawtexid);
    }


    // end encoding on current render encoder
    [self endRenderEncoding];

    // start blit encoder
    id<MTLBlitCommandEncoder> blitCommandEncoder;
    blitCommandEncoder = [_currentCommandBuffer blitCommandEncoder];
    [blitCommandEncoder
        copyFromTexture:readtexid sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(srcX0, srcY0, 0) sourceSize:MTLSizeMake(srcX1-srcX0, srcY1-srcY0, 1)
        toTexture:drawtexid destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(dstX0, dstY0, 0) /*destinationSize:MTLSizeMake(dstX1, dstY1, 0)*/ ];
    [blitCommandEncoder endEncoding];

}

void mtlBlitFramebuffer(GLMContext glm_ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlBlitFramebuffer:glm_ctx srcX0:srcX0 srcY0:srcY0 srcX1:srcX1 srcY1:srcY1 dstX0:dstX0 dstY0:dstY0 dstX1:dstX1 dstY1:dstY1 mask:mask filter:filter];
}

- (Texture *)framebufferAttachmentTexture: (FBOAttachment *)fbo_attachment
{
    Texture *tex;

    if (fbo_attachment->textarget == GL_RENDERBUFFER)
    {
        tex = fbo_attachment->buf.rbo->tex;
    }
    else
    {
        tex = fbo_attachment->buf.tex;
    }
    assert(tex);

    return tex;
}

- (bool)bindMTLTexture:(Texture *)tex
{
    if (tex->dirty_bits)
    {
        // release mtl data
        if (tex->mtl_data)
        {
            CFBridgingRelease(tex->mtl_data);
            tex->mtl_data = NULL;
        }

        if (tex->params.mtl_data)
        {
            CFBridgingRelease(tex->params.mtl_data);
            tex->params.mtl_data = NULL;
        }
    }

    if (tex->mtl_data == NULL)
    {
        tex->mtl_data = (void *)CFBridgingRetain([self createMTLTextureFromGLTexture: tex]);
        assert(tex->mtl_data);

        tex->params.mtl_data = (void *)CFBridgingRetain([self createMTLSamplerForTexParam:&tex->params target:tex->target]);
        assert(tex->params.mtl_data);
    }

    return true;
}

- (bool)bindActiveTexturesToMTL
{
    // search through active_texture_mask for enabled bits
    // 128 bits long.. do it on 4 parts
    for(int i=0; i<4; i++)
    {
        unsigned mask = STATE(active_texture_mask[i]);

        if (mask)
        {
            for(int bitpos=0; bitpos<32; bitpos++)
            {
                if (mask & (0x1 << bitpos))
                {
                    Texture *tex;

                    tex = STATE(active_textures[i*32+bitpos]);
                    assert(tex);

                    RETURN_FALSE_ON_FAILURE([self bindMTLTexture: tex]);
                }

                // early out
                if ((mask >> (bitpos + 1)) == 0)
                    break;
            }
        }
    }

    return true;
}

- (bool)bindFramebufferTexture:(FBOAttachment *)fbo_attachment isDrawBuffer:(bool) isDrawBuffer
{
    Texture *tex;

    tex = [self framebufferAttachmentTexture: fbo_attachment];
    assert(tex);

    tex->is_render_target = isDrawBuffer;

    RETURN_FALSE_ON_FAILURE([self bindMTLTexture: tex]);

    return true;
}


#pragma mark programs
- (int) getProgramBindingCount: (int) stage type: (int) type
{
    Program *ptr;

    assert(stage < _MAX_SPIRV_RES);
    switch(type)
    {
        case SPVC_RESOURCE_TYPE_UNIFORM_BUFFER:
        case SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT:
        case SPVC_RESOURCE_TYPE_STORAGE_BUFFER:
        case SPVC_RESOURCE_TYPE_ATOMIC_COUNTER:
        case SPVC_RESOURCE_TYPE_STAGE_INPUT:
        case SPVC_RESOURCE_TYPE_SAMPLED_IMAGE:
        case SPVC_RESOURCE_TYPE_STORAGE_IMAGE:
            break;

        default:
           assert(0);
    }

    ptr = ctx->state.program;
    assert(ptr);

    return ptr->spirv_resources_list[stage][type].count;
}

- (int) getProgramBinding: (int) stage type: (int) type index: (int) index
{
    Program *ptr;

    assert(stage < _MAX_SPIRV_RES);
    switch(type)
    {
       case SPVC_RESOURCE_TYPE_UNIFORM_BUFFER:
       case SPVC_RESOURCE_TYPE_UNIFORM_CONSTANT:
       case SPVC_RESOURCE_TYPE_STORAGE_BUFFER:
       case SPVC_RESOURCE_TYPE_ATOMIC_COUNTER:
       case SPVC_RESOURCE_TYPE_STAGE_INPUT:
       case SPVC_RESOURCE_TYPE_SAMPLED_IMAGE:
       case SPVC_RESOURCE_TYPE_STORAGE_IMAGE:
           break;

       default:
          assert(0);
    }

    ptr = ctx->state.program;
    assert(ptr);

    assert(index < ptr->spirv_resources_list[stage][type].count);

    return ptr->spirv_resources_list[stage][type].list[index].binding;
}

- (int) getProgramLocation: (int) stage type: (int) type index: (int) index
{
    Program *ptr;

    assert(stage < _MAX_SPIRV_RES);
    switch(type)
    {
       case SPVC_RESOURCE_TYPE_UNIFORM_BUFFER:
       case SPVC_RESOURCE_TYPE_STORAGE_BUFFER:
       case SPVC_RESOURCE_TYPE_ATOMIC_COUNTER:
       case SPVC_RESOURCE_TYPE_STAGE_INPUT:
       case SPVC_RESOURCE_TYPE_SAMPLED_IMAGE:
       case SPVC_RESOURCE_TYPE_STORAGE_IMAGE:
           break;

       default:
          assert(0);
    }

    ptr = ctx->state.program;
    assert(ptr);

    assert(index < ptr->spirv_resources_list[stage][type].count);
    
    return ptr->spirv_resources_list[stage][type].list[index].location;
}

- (id<MTLLibrary>) compileShader: (const char *) str
{
    id<MTLLibrary> library;
    __autoreleasing NSError *error = nil;

    library = [_device newLibraryWithSource: [NSString stringWithUTF8String: str] options: nil error: &error];
    if(!library) NSLog(@" error compiling shader => %@ ", [error localizedDescription] );
    assert(library);

    return library;
}

-(bool)bindMTLProgram:(Program *)ptr
{
    if (ptr->dirty_bits & DIRTY_PROGRAM)
    {
        // release mtl shaders
        for(int i=_VERTEX_SHADER; i<_MAX_SHADER_TYPES; i++)
        {
            Shader *shader;
            shader = ptr->shader_slots[i];

            if (shader)
            {
                if (shader->mtl_data.library)
                {
                    CFBridgingRelease(shader->mtl_data.library);
                    CFBridgingRelease(shader->mtl_data.function);
                    shader->mtl_data.library = NULL;
                    shader->mtl_data.function = NULL;
                }
            }
        }

        ptr->dirty_bits &= ~DIRTY_PROGRAM;
    }

    // bind mtl functions to shaders
    for(int i=_VERTEX_SHADER; i<_MAX_SHADER_TYPES; i++)
    {
        Shader *shader;
        shader = ptr->shader_slots[i];

        if (shader)
        {
            if (shader->mtl_data.library == NULL)
            {
                id<MTLLibrary> library;
                id<MTLFunction> function;
                
                library = [self compileShader: ptr->spirv[i].msl_str];
                assert(library);
                function = [library newFunctionWithName:[NSString stringWithUTF8String: shader->entry_point]];
                assert(function);
                shader->mtl_data.library = (void *)CFBridgingRetain(library);
                shader->mtl_data.function = (void *)CFBridgingRetain(function);
            }
        }
    }

    return true;
}

#pragma mark draw buffers
- (id)newDrawBuffer:(MTLPixelFormat)pixelFormat isDepthStencil:(bool)depthStencil
{
    id<MTLTexture> texture;
    MTLTextureDescriptor *tex_desc;
    NSRect frame;

    assert(_layer);
    frame = [_layer frame];

    tex_desc = [[MTLTextureDescriptor alloc] init];
    tex_desc.width = frame.size.width;
    tex_desc.height = frame.size.height;
    tex_desc.width = frame.size.width;
    tex_desc.pixelFormat = pixelFormat;
    tex_desc.usage = MTLTextureUsageRenderTarget;

    if (depthStencil)
    {
        tex_desc.storageMode = MTLStorageModePrivate;
    }

    texture = [_device newTextureWithDescriptor:tex_desc];
    assert(texture);

    return texture;
}

- (id)newDrawBufferWithCustomSize:(MTLPixelFormat)pixelFormat isDepthStencil:(bool)depthStencil customSize:(CGSize)size
{
    id<MTLTexture> texture;
    MTLTextureDescriptor *tex_desc;

    tex_desc = [[MTLTextureDescriptor alloc] init];
    tex_desc.width = size.width;
    tex_desc.height = size.height;
    tex_desc.width = size.width;
    tex_desc.pixelFormat = pixelFormat;
    tex_desc.usage = MTLTextureUsageRenderTarget;

    if (depthStencil)
    {
        tex_desc.storageMode = MTLStorageModePrivate;
    }

    texture = [_device newTextureWithDescriptor:tex_desc];
    assert(texture);

    return texture;
}

- (bool) checkDrawBufferSize:(GLuint) index;
{
    NSRect frame;
    NSSize size;

    frame = [_view frame];
    size = frame.size;

    if (size.width != _drawBuffers[index].width)
        return false;

    if (size.height != _drawBuffers[index].height)
        return false;

    return true;
}

#pragma mark render encoder and command buffer init code
- (MTLStencilOperation) mtlStencilOpForGLOp:(GLenum) op
{
    MTLStencilOperation stencil_op;

    switch(ctx->state.var.stencil_fail)
    {
        case GL_KEEP: stencil_op = MTLStencilOperationKeep; break;
        case GL_ZERO: stencil_op = MTLStencilOperationZero; break;
        case GL_REPLACE: stencil_op = MTLStencilOperationReplace; break;
        case GL_INCR: stencil_op = MTLStencilOperationIncrementClamp; break;
        case GL_INCR_WRAP: stencil_op = MTLStencilOperationDecrementClamp; break;
        case GL_DECR: stencil_op = MTLStencilOperationInvert; break;
        case GL_DECR_WRAP: stencil_op = MTLStencilOperationIncrementWrap; break;
        case GL_INVERT: stencil_op = MTLStencilOperationDecrementWrap; break;
        default:
            assert(0);
    }

    return stencil_op;
}

- (void) updateCurrentRenderEncoder
{
    if (ctx->state.caps.depth_test ||
        ctx->state.caps.stencil_test)
    {
        MTLDepthStencilDescriptor *dsDesc = [[MTLDepthStencilDescriptor alloc] init];

        // mtl maps directly to gl
        if (ctx->state.caps.depth_test)
        {
            MTLCompareFunction depthCompareFunction;

            depthCompareFunction = ctx->state.var.depth_func - GL_NEVER;
            dsDesc.depthCompareFunction = depthCompareFunction;

            dsDesc.depthWriteEnabled = ctx->state.var.depth_writemask;
        }

        if (ctx->state.caps.stencil_test)
        {
            // mtl maps directly to gl
            if (ctx->state.var.stencil_func != GL_NEVER)
            {
                MTLStencilDescriptor *frontSDesc = [[MTLStencilDescriptor alloc] init];

                frontSDesc.stencilCompareFunction = ctx->state.var.stencil_func - GL_NEVER;
                frontSDesc.stencilFailureOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_fail ];
                frontSDesc.depthFailureOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_pass_depth_fail];
                frontSDesc.depthStencilPassOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_pass_depth_pass];
                frontSDesc.writeMask = ctx->state.var.stencil_writemask;
                frontSDesc.readMask = ctx->state.var.stencil_value_mask;    // ????

                dsDesc.frontFaceStencil = frontSDesc;
            }

            if (ctx->state.var.stencil_func != GL_NEVER)
            {
                MTLStencilDescriptor *backSDesc = [[MTLStencilDescriptor alloc] init];

                backSDesc.stencilCompareFunction = ctx->state.var.stencil_back_func - GL_NEVER;
                backSDesc.stencilFailureOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_back_fail ];
                backSDesc.depthFailureOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_back_pass_depth_fail];
                backSDesc.depthStencilPassOperation = [self mtlStencilOpForGLOp:ctx->state.var.stencil_back_pass_depth_pass];
                backSDesc.writeMask = ctx->state.var.stencil_back_writemask;
                backSDesc.readMask = ctx->state.var.stencil_back_value_mask;    // ????

                dsDesc.backFaceStencil = backSDesc;
            }
        }

        id <MTLDepthStencilState> dsState = [_device
                                  newDepthStencilStateWithDescriptor:dsDesc];

        [_currentRenderEncoder setDepthStencilState: dsState];
    }

    if (ctx->state.caps.scissor_test)
    {
        MTLScissorRect rect;

        rect.x = ctx->state.var.scissor_box[0];
        rect.y = ctx->state.var.scissor_box[1];
        rect.width = ctx->state.var.scissor_box[2];
        rect.height = ctx->state.var.scissor_box[3];

        [_currentRenderEncoder setScissorRect:rect];
    }

    [_currentRenderEncoder setViewport:(MTLViewport){ctx->state.viewport[0], ctx->state.viewport[1],
                                        ctx->state.viewport[2], ctx->state.viewport[3],
                                        ctx->state.var.depth_range[0], ctx->state.var.depth_range[1]}];

    if (ctx->state.caps.cull_face)
    {
        MTLCullMode cull_mode;

        switch(ctx->state.var.cull_face_mode)
        {
            case GL_BACK: cull_mode = MTLCullModeBack; break;
            case GL_FRONT: cull_mode = MTLCullModeFront; break;
            default:
                cull_mode = MTLCullModeNone;
        }

        [_currentRenderEncoder setCullMode:cull_mode];

        MTLWinding winding;

        winding = ctx->state.var.front_face - GL_CW;

        [_currentRenderEncoder setFrontFacingWinding:winding];
    }

    if (ctx->state.caps.depth_clamp)
    {
        [_currentRenderEncoder setDepthClipMode: MTLDepthClipModeClamp];
    }

    if (ctx->state.var.polygon_mode == GL_LINES)
    {
        [_currentRenderEncoder setTriangleFillMode: MTLTriangleFillModeLines];
    }
}

- (bool) newRenderEncoder
{
    @autoreleasepool {
    
    if (VAO() == NULL)
        return true;    // not really a failure we just can't setup a new desc

    // end encoding on current render encoder
    [self endRenderEncoding];

    // grab the next drawable from CAMetalLayer
    if (_drawable == NULL)
    {
        assert(_layer);
        
        _drawable = [_layer nextDrawable];

        // late init of gl scissor box on attachment to window system
        NSRect frame;
        frame = [_layer frame];

        ctx->state.var.scissor_box[2] = frame.size.width;
        ctx->state.var.scissor_box[3] = frame.size.height;
    }

    _renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    assert(_renderPassDescriptor);

    if (ctx->state.framebuffer)
    {
        Framebuffer *fbo;

        fbo = ctx->state.framebuffer;

        for (int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
        {
            if (fbo->color_attachments[i].texture)
            {
                Texture *tex;

                tex = [self framebufferAttachmentTexture: &fbo->color_attachments[i]];
                assert(tex);

                assert(tex->mtl_data);
                _renderPassDescriptor.colorAttachments[i].texture = (__bridge id<MTLTexture> _Nullable)(tex->mtl_data);

                if (fbo->color_attachments[i].buf.rbo->is_draw_buffer)
                {
                    GLuint width, height;

                    width = tex->width;
                    height = tex->height;

                    _renderPassDescriptor.renderTargetWidth = width;
                    _renderPassDescriptor.renderTargetHeight = height;
                }
            }

            // early out
            if ((fbo->color_attachment_bitfield >> (i+1)) == 0)
                break;
        }

        // depth attachment
        if (fbo->depth.texture)
        {
            Texture *tex;

            tex = [self framebufferAttachmentTexture: &fbo->depth];
            assert(tex);

            _renderPassDescriptor.depthAttachment.texture = (__bridge id<MTLTexture> _Nullable)(tex->mtl_data);
        }

        // stencil attachment
        if (fbo->stencil.texture)
        {
            Texture *tex;

            tex = [self framebufferAttachmentTexture: &fbo->stencil];
            assert(tex);

            _renderPassDescriptor.stencilAttachment.texture = (__bridge id<MTLTexture> _Nullable)(tex->mtl_data);
        }
    }
    else
    {
        GLuint mgl_drawbuffer;
        id<MTLTexture> texture, depth_texture, stencil_texture;
        
        switch(ctx->state.draw_buffer)
        {
            case GL_FRONT: mgl_drawbuffer = _FRONT; break;
            case GL_BACK: mgl_drawbuffer = _BACK; break;
            case GL_FRONT_LEFT: mgl_drawbuffer = _FRONT_LEFT; break;
            case GL_FRONT_RIGHT: mgl_drawbuffer = _FRONT_RIGHT; break;
            case GL_BACK_LEFT: mgl_drawbuffer = _BACK_LEFT; break;
            case GL_BACK_RIGHT: mgl_drawbuffer = _BACK_RIGHT; break;
            default:
                assert(0);
        }

        if([self checkDrawBufferSize:mgl_drawbuffer])
        {
            _drawBuffers[mgl_drawbuffer].drawbuffer = NULL;
            _drawBuffers[mgl_drawbuffer].depthbuffer = NULL;
            _drawBuffers[mgl_drawbuffer].stencilbuffer = NULL;
        }

        // attach color buffer
        if (mgl_drawbuffer == _FRONT)
        {
            texture = _drawable.texture;

            // sleep mode will return a null texture
            RETURN_FALSE_ON_NULL(texture);
        }
        else if(_drawBuffers[mgl_drawbuffer].drawbuffer)
        {
            texture = _drawBuffers[mgl_drawbuffer].drawbuffer;
        }
        else
        {
            texture = [self newDrawBuffer: ctx->pixel_format.mtl_pixel_format isDepthStencil:false];
            _drawBuffers[mgl_drawbuffer].drawbuffer = texture;
        }

        // attach depth
        if (ctx->depth_format.mtl_pixel_format &&
            ctx->state.caps.depth_test)
        {
            if(_drawBuffers[mgl_drawbuffer].depthbuffer)
            {
                depth_texture = _drawBuffers[mgl_drawbuffer].depthbuffer;
            }
            else
            {
                depth_texture = [self newDrawBufferWithCustomSize:ctx->depth_format.mtl_pixel_format isDepthStencil:true customSize: CGSizeMake(texture.width, texture.height) ];
                _drawBuffers[mgl_drawbuffer].depthbuffer = depth_texture;
            }
        }

        // attach stencil
        if (ctx->stencil_format.mtl_pixel_format &&
            ctx->state.caps.stencil_test)
        {
            if(_drawBuffers[mgl_drawbuffer].stencilbuffer)
            {
                stencil_texture = _drawBuffers[mgl_drawbuffer].stencilbuffer;
            }
            else
            {
                stencil_texture = [self newDrawBufferWithCustomSize:ctx->depth_format.mtl_pixel_format isDepthStencil:true customSize: CGSizeMake(texture.width, texture.height) ];
                _drawBuffers[mgl_drawbuffer].stencilbuffer = stencil_texture;
            }
        }

        _renderPassDescriptor.colorAttachments[0].texture = texture;
        _renderPassDescriptor.depthAttachment.texture = depth_texture;
        _renderPassDescriptor.stencilAttachment.texture = stencil_texture;

        _renderPassDescriptor.renderTargetWidth = texture.width;
        _renderPassDescriptor.renderTargetHeight = texture.height;
    }

    // in case one of the framebuffers should be cleared
    if (true) // ctx->state.clear_bitmask)
    {
        if (ctx->state.clear_bitmask & GL_COLOR_BUFFER_BIT)
        {
            _renderPassDescriptor.colorAttachments[0].clearColor =
                MTLClearColorMake(STATE(color_clear_value[0]),
                                  STATE(color_clear_value[1]),
                                  STATE(color_clear_value[2]),
                                  STATE(color_clear_value[3]));

            _renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        }
        else
        {
            _renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
        }

        if (ctx->state.framebuffer) {
            Framebuffer * fbo = ctx->state.framebuffer;
            for(int i=0; i<STATE(max_color_attachments);i++) {
                FBOAttachment * fboa;
                fboa = &fbo->color_attachments[i];
                if (fboa->clear_bitmask & GL_COLOR_BUFFER_BIT) {
                    _renderPassDescriptor.colorAttachments[i].clearColor =
                        MTLClearColorMake(fboa->clear_color[0],
                                        fboa->clear_color[1],
                                        fboa->clear_color[2],
                                        fboa->clear_color[3]);

                    _renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
                } else {
                    _renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
                }
            }
        }

        if (ctx->state.clear_bitmask & GL_DEPTH_BUFFER_BIT)
        {
            _renderPassDescriptor.depthAttachment.clearDepth = STATE_VAR(depth_clear_value);

            _renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        }
        else
        {
            _renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
        }

        if (ctx->state.clear_bitmask & GL_STENCIL_BUFFER_BIT)
        {
            _renderPassDescriptor.stencilAttachment.clearStencil = STATE_VAR(stencil_clear_value);

            _renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
        }
        else
        {
            _renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
        }

        ctx->state.clear_bitmask = 0;
    }
    else
    {
        _renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
        _renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
        _renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
    }

    _renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    // create a render encoder from the renderpass descriptor
    _currentRenderEncoder = [_currentCommandBuffer renderCommandEncoderWithDescriptor: _renderPassDescriptor];
    assert(_currentRenderEncoder);
    _currentRenderEncoder.label = @"GL Render Encoder";

    // apply all state that isn't included in a renderPassDescriptor into the render encoder
    [self updateCurrentRenderEncoder];

    if ([self bindBuffersToCurrentRenderEncoder] == false)
    {
        printf("buffer binding failed\n");

        return false;
    }

    if ([self bindTexturesToCurrentRenderEncoder] == false)
    {
        printf("texture binding failed\n");

        return false;
    }

    return true;
    }
}

- (bool) newCommandBuffer
{
    if (_currentEvent)
    {
        assert(_currentSyncName);

        [_currentCommandBuffer encodeWaitForEvent: _currentEvent value: _currentSyncName];

        _currentEvent = NULL;
        _currentSyncName = 0;
    }

    // remove any events waiting on current command buffer
    if (_currentCommandBufferSyncList)
    {
        GLuint count;

        count = _currentCommandBufferSyncList->count;

        for(GLuint i=0; i<count; i++)
        {
            Sync *sync;

            sync = _currentCommandBufferSyncList->list[i];

            CFBridgingRelease(sync->mtl_event);
        }

        _currentCommandBufferSyncList->count = 0;
    }

    _currentCommandBuffer = [_commandQueue commandBuffer];
    assert(_currentCommandBuffer);

    return true;
}

- (bool) newCommandBufferAndRenderEncoder
{
    RETURN_FALSE_ON_FAILURE([self newCommandBuffer]);

    RETURN_FALSE_ON_FAILURE([self newRenderEncoder]);

    return true;
}

#pragma mark pipeline descriptor
-(MTLRenderPipelineDescriptor *)generatePipelineDescriptor
{
    MTLRenderPipelineDescriptor *pipelineStateDescriptor;
    Program *program;
    Shader *vertex_shader, *fragment_shader;
    id<MTLFunction> vertexFunction;
    id<MTLFunction> fragmentFunction;

    if (ctx->state.dirty_bits & DIRTY_PROGRAM)
    {
        program = ctx->state.program;

        if (program)
        {
            if ([self bindMTLProgram:program] == false)
                return NULL;
        }
        else
        {
            return NULL;
        }
    }

    program = ctx->state.program;
    vertex_shader = program->shader_slots[_VERTEX_SHADER];
    fragment_shader = program->shader_slots[_FRAGMENT_SHADER];
    assert(vertex_shader);
    assert(fragment_shader);

    vertexFunction = (__bridge id<MTLFunction>)(vertex_shader->mtl_data.function);
    fragmentFunction = (__bridge id<MTLFunction>)(fragment_shader->mtl_data.function);
    assert(vertexFunction);
    assert(fragmentFunction);

    // Configure a pipeline descriptor that is used to create a pipeline state.
    pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    assert(pipelineStateDescriptor);
    pipelineStateDescriptor.label = @"GLSL Pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;

    if (ctx->state.framebuffer)
    {
        Framebuffer *fbo;

        fbo = ctx->state.framebuffer;

        for (int i=0; i<STATE(max_color_attachments); i++)
        {
            if (fbo->color_attachments[i].texture)
            {
                Texture *tex;

                tex = [self framebufferAttachmentTexture: &fbo->color_attachments[i]];
                assert(tex);

                RETURN_NULL_ON_FAILURE([self bindMTLTexture: tex]);
                assert(tex->mtl_data);

                pipelineStateDescriptor.colorAttachments[i].pixelFormat = mtlPixelFormatForGLTex(tex);
            }

            // early out
            if ((fbo->color_attachment_bitfield >> (i+1)) == 0)
                break;
        }

        // depth attachment
        if (fbo->depth.texture
        //    && ctx->state.caps.depth_test // otherwise, the render pipeline's pixelFormat is MTLPixelFormatInvalid
        )
        {
            Texture *tex;

            tex = [self framebufferAttachmentTexture: &fbo->depth];
            assert(tex);

            RETURN_NULL_ON_FAILURE([self bindMTLTexture: tex]);
            assert(tex->mtl_data);

            pipelineStateDescriptor.depthAttachmentPixelFormat = mtlPixelFormatForGLTex(tex);
        }

        // stencil attachment
        if (fbo->stencil.texture
        //    && ctx->state.caps.stencil_test // otherwise, the render pipeline's pixelFormat is MTLPixelFormatInvalid
        )
        {
            Texture *tex;

            tex = [self framebufferAttachmentTexture: &fbo->stencil];
            assert(tex);

            RETURN_NULL_ON_FAILURE([self bindMTLTexture: tex]);
            assert(tex->mtl_data);

            pipelineStateDescriptor.stencilAttachmentPixelFormat = mtlPixelFormatForGLTex(tex);
        }
    }
    else
    {
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = ctx->pixel_format.mtl_pixel_format;

        if (ctx->depth_format.format &&
            ctx->state.caps.depth_test)
            pipelineStateDescriptor.depthAttachmentPixelFormat = ctx->depth_format.mtl_pixel_format;

        if (ctx->stencil_format.format &&
            ctx->state.caps.stencil_test)
            pipelineStateDescriptor.stencilAttachmentPixelFormat = ctx->stencil_format.mtl_pixel_format;
    }

    return pipelineStateDescriptor;
}

#pragma mark vertex descriptor
- (MTLVertexDescriptor *)generateVertexDescriptor
{
    MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    assert(vertexDescriptor);

    [vertexDescriptor reset]; // ??? debug

    // we can bind a new vertex descriptor without creating a new renderbuffer
    for(int i=0;i<ctx->state.max_vertex_attribs; i++)
    {
        if (VAO_STATE(enabled_attribs) & (0x1 << i))
        {
            MTLVertexFormat format;

            if (VAO_ATTRIB_STATE(i).buffer == NULL)
            {
                NSLog(@"Error: Invalid VAO defined enabled but no buffer bound\n");
                return NULL;
            }

            format = glTypeSizeToMtlType(VAO_ATTRIB_STATE(i).type,
                                         VAO_ATTRIB_STATE(i).size,
                                         VAO_ATTRIB_STATE(i).normalized);

            if (format == MTLVertexFormatInvalid)
            {
                NSLog(@"Error: unable to map gl type / size / normalize to format\n");
                return false;
            }

            int mapped_buffer_index;

            mapped_buffer_index = [self getVertexBufferIndexWithAttributeSet: i];

            vertexDescriptor.attributes[i].bufferIndex = mapped_buffer_index;
            vertexDescriptor.attributes[i].offset = ctx->state.vao->attrib[i].base_plus_relative_offset;
            vertexDescriptor.attributes[i].format = format;

            vertexDescriptor.layouts[mapped_buffer_index].stride = VAO_ATTRIB_STATE(i).stride;
            if (ctx->state.vao->attrib[i].divisor)
                vertexDescriptor.layouts[mapped_buffer_index].stepRate = ctx->state.vao->attrib[i].divisor;
            else
                vertexDescriptor.layouts[mapped_buffer_index].stepRate = 1;
            vertexDescriptor.layouts[mapped_buffer_index].stepFunction = MTLVertexStepFunctionPerVertex;
        }

        // early out
        if ((VAO_STATE(enabled_attribs) >> (i+1)) == 0)
            break;
    }

    // clear all dirty bits as they have been translated into a vertex descriptor
    ctx->state.vao->dirty_bits = 0;

    return vertexDescriptor;
}

#pragma mark utility funcs for processGLState
- (MTLBlendFactor) blendFactorFromGL:(GLenum)gl_blend
{
    MTLBlendFactor factor;

    switch(gl_blend)
    {
        case GL_ZERO: factor = MTLBlendFactorZero; break;
        case GL_ONE: factor = MTLBlendFactorOne; break;
        case GL_SRC_COLOR: factor = MTLBlendFactorSourceColor; break;
        case GL_ONE_MINUS_SRC_COLOR: factor = MTLBlendFactorOneMinusSourceColor; break;
        case GL_DST_COLOR: factor = MTLBlendFactorDestinationColor; break;
        case GL_ONE_MINUS_DST_COLOR: factor = MTLBlendFactorOneMinusDestinationColor; break;
        case GL_SRC_ALPHA: factor = MTLBlendFactorSourceAlpha; break;
        case GL_ONE_MINUS_SRC_ALPHA: factor = MTLBlendFactorOneMinusSourceAlpha; break;
        case GL_DST_ALPHA: factor = MTLBlendFactorDestinationAlpha; break;
        case GL_ONE_MINUS_DST_ALPHA: factor = MTLBlendFactorOneMinusDestinationAlpha; break;
        case GL_CONSTANT_COLOR: factor = MTLBlendFactorSource1Color; break;
        case GL_ONE_MINUS_CONSTANT_COLOR: factor = MTLBlendFactorOneMinusSource1Color; break;
        case GL_CONSTANT_ALPHA: factor = MTLBlendFactorSource1Alpha; break;
        case GL_ONE_MINUS_CONSTANT_ALPHA: factor = MTLBlendFactorOneMinusSource1Alpha; break;

        default:
            assert(0);
    }

    return factor;
}

- (MTLBlendOperation) blendOperationFromGL:(GLenum)gl_blend_op
{
    MTLBlendOperation op;

    switch(gl_blend_op)
    {
        case GL_FUNC_ADD: op = MTLBlendOperationAdd; break;
        case GL_FUNC_SUBTRACT: op = MTLBlendOperationSubtract; break;
        case GL_FUNC_REVERSE_SUBTRACT: op = MTLBlendOperationReverseSubtract; break;
        case GL_MIN: op = MTLBlendOperationMin; break;
        case GL_MAX: op = MTLBlendOperationMax; break;

        default:
            assert(0);
    }

    return op;
}

- (void) updateBlendStateCache
{
    for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        _src_blend_rgb_factor[i] = [self blendFactorFromGL:ctx->state.var.blend_src_rgb[i]];
        _src_blend_alpha_factor[i] = [self blendFactorFromGL:ctx->state.var.blend_src_alpha[i]];

        _dst_blend_rgb_factor[i] = [self blendFactorFromGL:ctx->state.var.blend_dst_rgb[i]];
        _dst_blend_alpha_factor[i] = [self blendFactorFromGL:ctx->state.var.blend_dst_alpha[i]];

        _rgb_blend_operation[i] = [self blendOperationFromGL: ctx->state.var.blend_equation_rgb[i]];
        _alpha_blend_operation[i] = [self blendOperationFromGL: ctx->state.var.blend_equation_alpha[i]];

        if (ctx->state.caps.use_color_mask[i])
        {
            _color_mask[i] = MTLColorWriteMaskNone;

            if (ctx->state.var.color_writemask[i][0])
                _color_mask[i] |= MTLColorWriteMaskRed;

            if (ctx->state.var.color_writemask[i][1])
                _color_mask[i] |= MTLColorWriteMaskGreen;

            if (ctx->state.var.color_writemask[i][2])
                _color_mask[i] |= MTLColorWriteMaskBlue;

            if (ctx->state.var.color_writemask[i][3])
                _color_mask[i] |= MTLColorWriteMaskAlpha;
        }
        else
        {
            _color_mask[i] = MTLColorWriteMaskRed | MTLColorWriteMaskGreen | MTLColorWriteMaskBlue | MTLColorWriteMaskAlpha;
        }
    }
}

-(void)bindBlendStateToPipelineStateDescriptor:(MTLRenderPipelineDescriptor *)pipelineStateDescriptor
{
    for(int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        if (pipelineStateDescriptor.colorAttachments[i].pixelFormat != MTLPixelFormatInvalid)
        {
            pipelineStateDescriptor.colorAttachments[i].blendingEnabled = true;

            pipelineStateDescriptor.colorAttachments[i].sourceRGBBlendFactor = _src_blend_rgb_factor[i];
            pipelineStateDescriptor.colorAttachments[i].destinationRGBBlendFactor = _dst_blend_rgb_factor[i];
            pipelineStateDescriptor.colorAttachments[i].sourceAlphaBlendFactor = _src_blend_alpha_factor[i];
            pipelineStateDescriptor.colorAttachments[i].destinationAlphaBlendFactor = _dst_blend_alpha_factor[i];

            pipelineStateDescriptor.colorAttachments[i].rgbBlendOperation = _rgb_blend_operation[i];
            pipelineStateDescriptor.colorAttachments[i].alphaBlendOperation = _alpha_blend_operation[i];

            pipelineStateDescriptor.colorAttachments[i].writeMask = _color_mask[i];
        }
    }
}

-(bool)bindFramebufferAttachmentTextures
{
    Framebuffer *fbo;

    fbo = ctx->state.framebuffer;

    for (int i=0; i<MAX_COLOR_ATTACHMENTS; i++)
    {
        if (fbo->color_attachments[i].texture)
        {
            if ([self bindFramebufferTexture: &fbo->color_attachments[i] isDrawBuffer: (fbo->color_attachments[i].buf.rbo->is_draw_buffer)] == false)
            {
                printf("Failed Framebuffer Attachment\n");
                return false;
            }
        }

        // early out
        if ((fbo->color_attachment_bitfield >> (i+1)) == 0)
            break;
    }

    // depth attachment
    if (fbo->depth.texture)
    {
        if ([self bindFramebufferTexture: &fbo->depth isDrawBuffer: true] == false)
        {
            printf("Failed Framebuffer Attachment\n");
            return false;
        }
    }

    // stencil attachment
    if (fbo->stencil.texture)
    {
        if ([self bindFramebufferTexture: &fbo->stencil isDrawBuffer: true] == false)
        {
            printf("Failed Framebuffer Attachment\n");
            return false;
        }
    }

    return true;
}

- (void) endRenderEncoding
{
    if (_currentRenderEncoder)
    {
        [_currentRenderEncoder endEncoding];
        _currentRenderEncoder = NULL;
    }
}

#pragma mark ------------------------------------------------------------------------------------------
#pragma mark processGLState for resolving opengl state into metal state
#pragma mark ------------------------------------------------------------------------------------------

- (bool) processGLState: (bool) draw_command
{
    assert(_device);
    assert(_commandQueue);

    //logDirtyBits(ctx);
    
    // since a clear is embedded into a render encoder
    if (VAO() == NULL)
    {
        if (draw_command)
        {
            NSLog(@"Error: No VAO defined for ctx\n");

            // quietly return if we are not in a draw command with no vao defined
            // like a clear or init call
            return false;
        }

        // for a clear flush sequence...
        if (ctx->state.dirty_bits & DIRTY_STATE)
        {
            // end encoding on current render encoder
            [self endRenderEncoding];

            RETURN_FALSE_ON_FAILURE([self newRenderEncoder]);
        }

        return true;
    }

    // only draw commands need a functioning render encoder
    // this can mess up a transition between compute and rendering on a flush
    // so just return
    // we may have to create a blank render encoder to safely run compute and
    // rendering correctly
    if (draw_command == false)
    {
        return true;
    }

    if (ctx->state.dirty_bits)
    {
        // dirty state covers all rendering attachments and general state
        if (ctx->state.dirty_bits & DIRTY_STATE)
        {
            if (ctx->state.dirty_bits & DIRTY_FBO)
            {
                if (ctx->state.framebuffer)
                {
                    if (ctx->state.framebuffer->dirty_bits & DIRTY_FBO_BINDING)
                    {
                        RETURN_FALSE_ON_FAILURE([self bindFramebufferAttachmentTextures]);

                        ctx->state.framebuffer->dirty_bits &= ~DIRTY_FBO_BINDING;
                    }
                }

                // dirty FBO state can't be cleared just yet its needed below
            }

            ctx->state.dirty_bits &= ~DIRTY_STATE;
        }

        // check for dirty program and vao
        // leave program / vao state dirty, buffers need to be mapped before used below
        // dirty program causes buffers to be remapped
        // dirty vao causes attributes to be remapped to new buffers
        // dirty buffer base causes buffers to be remapped to new indexes
        if (ctx->state.dirty_bits & (DIRTY_PROGRAM | DIRTY_VAO | DIRTY_BUFFER_BASE_STATE))
        {
            // programs are now compiled before execution, we shouldn't get here
            //assert(ctx->state.program->mtl_data); //

            // figure out vertex shader uniforms / buffer mappings
            RETURN_FALSE_ON_FAILURE([self mapBuffersToMTL]);

            ctx->state.dirty_bits &= ~DIRTY_BUFFER_BASE_STATE;
        }

        // dirty tex covers all texture modifications
        if (ctx->state.dirty_bits & (DIRTY_PROGRAM | DIRTY_TEX | DIRTY_TEX_BINDING | DIRTY_SAMPLER))
        {
            RETURN_FALSE_ON_FAILURE([self bindActiveTexturesToMTL]);

            // textures / active textures and samplers are all handled in bindActiveTexturesToMTL
            ctx->state.dirty_bits &= ~(DIRTY_TEX | DIRTY_TEX_BINDING | DIRTY_SAMPLER);
        }

        // a dirty vao needs to update the render encoder and buffer list
        if (ctx->state.dirty_bits & DIRTY_VAO)
        {
            // we have a dirty VAO, all the renderbuffer bindings are invalid so we need a new renderbuffer
            // with new renderbuffer bindings

            // always end encoding and start a new encoder and bind new vertex buffers
            // end encoding on current render encoder
            [self endRenderEncoding];

            // updateDirtyBaseBufferList binds new mtl buffers or updates old ones
            RETURN_FALSE_ON_FAILURE([self updateDirtyBaseBufferList: &ctx->state.vertex_buffer_map_list]);
            RETURN_FALSE_ON_FAILURE([self updateDirtyBaseBufferList: &ctx->state.fragment_buffer_map_list]);

            // get a new renderer encoder
            RETURN_FALSE_ON_FAILURE([self newRenderEncoder]);

            // clear dirty render state
            ctx->state.dirty_bits &= ~DIRTY_RENDER_STATE;
        }
        else if (ctx->state.dirty_bits & DIRTY_BUFFER)
        {
            // updateDirtyBaseBufferList binds new mtl buffers or updates old ones
            RETURN_FALSE_ON_FAILURE([self updateDirtyBaseBufferList: &ctx->state.vertex_buffer_map_list]);
            RETURN_FALSE_ON_FAILURE([self updateDirtyBaseBufferList: &ctx->state.fragment_buffer_map_list]);

            ctx->state.dirty_bits &= ~DIRTY_BUFFER;
        }
        else if (ctx->state.dirty_bits & DIRTY_RENDER_STATE)
        {
            if (_currentRenderEncoder == NULL)
            {
                RETURN_FALSE_ON_FAILURE([self newRenderEncoder]);
            }

            // a dirty render state may just be something like alpha changes which don't require a new renderbuffer

            // updateCurrentRenderEncoder will update the renderstate outside of creating a new one
            [self updateCurrentRenderEncoder];

            ctx->state.dirty_bits &= ~DIRTY_RENDER_STATE;
        }

        // new pipeline / vertex / renderbuffer and pipelinestate descriptor, should probably make this a single dirty bit
        if (ctx->state.dirty_bits & (DIRTY_PROGRAM | DIRTY_VAO | DIRTY_FBO | DIRTY_ALPHA_STATE | DIRTY_RENDER_STATE))
        {
            // create pipeline descriptor
            MTLRenderPipelineDescriptor *pipelineStateDescriptor;

            pipelineStateDescriptor = [self generatePipelineDescriptor];
            RETURN_FALSE_ON_NULL(pipelineStateDescriptor);

            // create vertex descriptor
            MTLVertexDescriptor *vertexDescriptor;

            vertexDescriptor = [self generateVertexDescriptor];
            RETURN_FALSE_ON_NULL(vertexDescriptor);

            if (ctx->state.caps.blend)
            {
                // cache these rather than recalculating them each time
                if (ctx->state.dirty_bits & DIRTY_ALPHA_STATE)
                {
                    [self updateBlendStateCache];

                    ctx->state.dirty_bits &= ~DIRTY_ALPHA_STATE;
                }

                [self bindBlendStateToPipelineStateDescriptor: pipelineStateDescriptor];
            }

            pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;

            NSError *error;
            _pipelineState = [_device newRenderPipelineStateWithDescriptor: pipelineStateDescriptor
                                                                     error:&error];

            // Pipeline State creation could fail if the pipeline descriptor isn't set up properly.
            //  If the Metal API validation is enabled, you can find out more information about what
            //  went wrong.  (Metal API validation is enabled by default when a debug build is run
            //  from Xcode.)
            NSAssert(_pipelineState, @"Failed to created pipeline state: %@", error);
            RETURN_FALSE_ON_NULL(_pipelineState);

            ctx->state.dirty_bits &= ~(DIRTY_PROGRAM | DIRTY_VAO | DIRTY_FBO);
        }

        //if (ctx->state.dirty_bits)
        //    logDirtyBits(ctx);

        // clear all bits when the DIRTY ALL bit is set.. kind of a hack but we want to
        // check for dirty bits outside of dirty all
        if (ctx->state.dirty_bits & DIRTY_ALL_BIT)
            ctx->state.dirty_bits = 0;

        // we missed something
        //assert(ctx->state.dirty_bits == 0);
    }

    // Create a render command encoder.
    [_currentRenderEncoder setRenderPipelineState: _pipelineState];

    return true;
}

#pragma mark ----- compute utility ---------------------------------------------------------------------

- (bool) bindBuffersToComputeEncoder:(id <MTLComputeCommandEncoder>) computeCommandEncoder
{
    assert(computeCommandEncoder);

    RETURN_FALSE_ON_FAILURE([self mapGLBuffersToMTLBufferMap: &ctx->state.compute_buffer_map_list stage:_COMPUTE_SHADER]);

    // dirty buffer covers all buffer modifications
    if (ctx->state.dirty_bits & DIRTY_BUFFER)
    {
        // updateDirtyBaseBufferList binds new mtl buffers or updates old ones
        [self updateDirtyBaseBufferList: &ctx->state.compute_buffer_map_list];

        ctx->state.dirty_bits &= ~DIRTY_BUFFER;
    }

    for(int i=0; i<ctx->state.compute_buffer_map_list.count; i++)
    {
        Buffer *ptr;

        ptr = ctx->state.compute_buffer_map_list.buffers[i].buf;

        RETURN_FALSE_ON_NULL(ptr);
        RETURN_FALSE_ON_NULL(ptr->data.mtl_data);

        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(ptr->data.mtl_data);
        assert(buffer);

        [computeCommandEncoder setBuffer:buffer offset:0 atIndex:i ];
    }

    return true;
}

- (bool) bindTexturesToComputeEncoder:(id <MTLComputeCommandEncoder>) computeCommandEncoder
{
    GLuint count;
    enum {
        _TEXTURE,
        _IMAGE_TEXTURE
    };
    struct {
        int spvc_type;
        int gl_texture_type;
    } mapped_types[] = {
        {SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, _TEXTURE},
        {SPVC_RESOURCE_TYPE_STORAGE_IMAGE, _IMAGE_TEXTURE},
        {0,0}
    };

    assert(computeCommandEncoder);

    for(int type=0; mapped_types[type].spvc_type; type++)
    {
        int spvc_type;
        int gl_texture_type;

        spvc_type = mapped_types[type].spvc_type;
        gl_texture_type = mapped_types[type].gl_texture_type;

        // iterate shader storage buffers
        count = [self getProgramBindingCount: _COMPUTE_SHADER type: spvc_type];
        if (count)
        {
            int textures_to_be_mapped = count;

            assert(textures_to_be_mapped < TEXTURE_UNITS);

            for (int i=0; textures_to_be_mapped > 0; i++)
            {
               // GLuint spirv_location;
                GLuint spirv_binding;
                Texture *ptr;

                spirv_binding = [self getProgramLocation:_COMPUTE_SHADER type:spvc_type index: i];
                spirv_binding = [self getProgramBinding:_COMPUTE_SHADER type:spvc_type index: i];

                switch(gl_texture_type)
                {
                    case _TEXTURE: ptr = STATE(active_textures[spirv_binding]); break;
                    case _IMAGE_TEXTURE: ptr = STATE(image_units[spirv_binding].tex); break;
                    default:
                        ptr = NULL;
                        assert(0);
                }

                if (ptr)
                {
                    RETURN_FALSE_ON_FAILURE([self bindMTLTexture: ptr]);
                    assert(ptr->mtl_data);

                    id<MTLTexture> texture;
                    texture = (__bridge id<MTLTexture>)(ptr->mtl_data);
                    assert(texture);

                    id<MTLSamplerState> sampler;

                    // late binding of texture samplers.. but its better than scanning the entire texture_samplers
                    if(STATE(texture_samplers[spirv_binding]))
                    {
                        Sampler *gl_sampler;

                        gl_sampler = STATE(texture_samplers[spirv_binding]);

                        // delete existing sampler if dirty
                        if (gl_sampler->dirty_bits)
                        {
                            if (gl_sampler->mtl_data)
                            {
                                CFBridgingRelease(gl_sampler->mtl_data);
                                gl_sampler->mtl_data = NULL;
                            }
                        }

                        if (gl_sampler->mtl_data == NULL)
                        {
                            gl_sampler->mtl_data = (void *)CFBridgingRetain([self createMTLSamplerForTexParam:&gl_sampler->params target:ptr->target]);
                            gl_sampler->dirty_bits = 0;
                        }

                        sampler = (__bridge id<MTLSamplerState>)(gl_sampler->mtl_data);
                        assert(sampler);
                    }
                    else
                    {
                        sampler = (__bridge id<MTLSamplerState>)(ptr->params.mtl_data);
                        assert(sampler);
                    }

                    [computeCommandEncoder setTexture:texture atIndex:spirv_binding];
                    [computeCommandEncoder setSamplerState: sampler atIndex:spirv_binding];

                    textures_to_be_mapped--;
                }

                RETURN_FALSE_ON_FAILURE((i<TEXTURE_UNITS));
            }

            // texture not found
            if (textures_to_be_mapped)
            {
                printf("No texture bound for fragment shader location\n");

                return false;
            }
        }
    }

    ctx->state.dirty_bits &= ~(DIRTY_TEX_BINDING | DIRTY_SAMPLER | DIRTY_IMAGE_UNIT_STATE);

    return true;
}

#pragma mark ------------------------------------------------------------------------------------------
#pragma mark processCompute
#pragma mark ------------------------------------------------------------------------------------------
-(bool)processCompute:(id <MTLComputeCommandEncoder>) computeCommandEncoder
{
    // from https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Compute-Ctx/Compute-Ctx.html#//apple_ref/doc/uid/TP40014221-CH6-SW1
    Program *program;

    program = ctx->state.program;
    assert(program);

    if (program->dirty_bits)
    {
        [self bindMTLProgram: program];
    }

    Shader *computeShader;
    computeShader = program->shader_slots[_COMPUTE_SHADER];
    assert(computeShader);

    id <MTLFunction> func;
    func = (__bridge id<MTLFunction>)(computeShader->mtl_data.function);
    assert(func);

    id <MTLComputePipelineState> computePipelineState;
    NSError *errors;
    computePipelineState = [_device newComputePipelineStateWithFunction:func error: &errors];
    assert(computePipelineState);

    [computeCommandEncoder setComputePipelineState:computePipelineState];

    RETURN_FALSE_ON_FAILURE([self bindBuffersToComputeEncoder: computeCommandEncoder]);

    //setTexture:atIndex:
    //setTextures:withRange:
    RETURN_FALSE_ON_FAILURE([self bindTexturesToComputeEncoder: computeCommandEncoder]);

    // setSamplerState:atIndex:
    // setSamplerState:lodMinClamp:lodMaxClamp:atIndex:
    // setSamplerStates:withRange:
    // setSamplerStates:lodMinClamps:lodMaxClamps:withRange:

    // [computeCommandEncoder setThreadgroupMemoryLength:atIndex:

    ctx->state.dirty_bits = 0;

    return true;
}

-(void)mtlDispatchCompute:(GLMContext)glm_ctx groupsX:(GLuint)groups_x groupsY:(GLuint)groups_y groupsZ:(GLuint)groups_z
{
    // end encoding on current render encoder
    [self endRenderEncoding];

    id <MTLComputeCommandEncoder> computeCommandEncoder = [_currentCommandBuffer computeCommandEncoder];
    assert(computeCommandEncoder);

    RETURN_ON_FAILURE([self processCompute:computeCommandEncoder]);

    MTLSize numThreadgroups;
    MTLSize threadsPerThreadgroup;

    Program *ptr;
    ptr = glm_ctx->state.program;

    if (ptr->local_workgroup_size.x || ptr->local_workgroup_size.y || ptr->local_workgroup_size.z)
    {
        GLuint mod_x, mod_y, mod_z;
        GLuint size_x, size_y, size_z;

        mod_x = groups_x % ptr->local_workgroup_size.x;
        mod_y = groups_y % ptr->local_workgroup_size.y;
        mod_z = groups_z % ptr->local_workgroup_size.z;

        size_x = groups_x / ptr->local_workgroup_size.x;
        size_y = groups_y / ptr->local_workgroup_size.y;
        size_z = groups_z / ptr->local_workgroup_size.z;

        if (mod_x || mod_y || mod_z)
        {
            if (mod_x)
                size_x++;

            if (mod_y)
                size_y++;

            if (mod_z)
                size_z++;
        }

        numThreadgroups = MTLSizeMake(size_x, size_y, size_z);
        threadsPerThreadgroup = MTLSizeMake(ptr->local_workgroup_size.x,
                                            ptr->local_workgroup_size.y,
                                            ptr->local_workgroup_size.z);

        [computeCommandEncoder dispatchThreadgroups:numThreadgroups
                                        threadsPerThreadgroup:threadsPerThreadgroup];
    }
    else
    {
        numThreadgroups = MTLSizeMake(groups_x, groups_y, groups_z);
        threadsPerThreadgroup = MTLSizeMake(1, 1, 1);

        [computeCommandEncoder dispatchThreadgroups:numThreadgroups
                                        threadsPerThreadgroup:threadsPerThreadgroup];
    }

    [computeCommandEncoder endEncoding];

    glm_ctx->state.dirty_bits = DIRTY_ALL;

    //[self newRenderEncoder];
}

void mtlDispatchCompute(GLMContext glm_ctx, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDispatchCompute: glm_ctx groupsX:num_groups_x groupsY:num_groups_y groupsZ:num_groups_z];
}


-(void)mtlDispatchComputeIndirect:(GLMContext)glm_ctx indirect:(GLintptr)indirect
{

}

void mtlDispatchComputeIndirect(GLMContext glm_ctx, GLintptr indirect)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDispatchComputeIndirect: glm_ctx indirect:indirect];
}


-(bool) processBuffer:(Buffer*)ptr
{
    if (ptr == NULL)
    {
        NSLog(@"Error: processBuffer failed\n");

        return false;
    }

    if (ptr->data.mtl_data == NULL)
    {
        [self bindMTLBuffer: ptr];
        RETURN_FALSE_ON_NULL(ptr->data.mtl_data);
    }

    if (ptr->data.dirty_bits)
    {
        [self updateDirtyBuffer: ptr];
    }

    return true;
}

-(void) flushCommandBuffer: (bool) finish
{
    RETURN_ON_FAILURE([self processGLState: false]);

    // end encoding on current render encoder
    [self endRenderEncoding];

    // Finalize rendering here & push the command buffer to the GPU.
    assert(_currentCommandBuffer);
    [_currentCommandBuffer commit];

    if (finish)
    {
        [_currentCommandBuffer waitUntilCompleted];
    }
    else
    {
        [_currentCommandBuffer waitUntilScheduled];
    }

    // get a new command buffer
    [self newCommandBuffer];
}

#pragma mark C interface to mtlBindBuffer
void mtlBindBuffer(GLMContext glm_ctx, Buffer *ptr)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj bindMTLBuffer:ptr];
}

#pragma mark C interface to mtlBindTexture
void mtlBindTexture(GLMContext glm_ctx, Texture *ptr)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj bindMTLTexture:ptr];
}

#pragma mark C interface to mtlBindProgram
void mtlBindProgram(GLMContext glm_ctx, Program *ptr)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj bindMTLProgram:ptr];
}

#pragma mark C interface to mtlDeleteMTLObj
-(void) mtlDeleteMTLObj:(GLMContext) glm_ctx buffer: (void *)obj
{
    [self flushCommandBuffer: false];

    // this should release it to the GC
    CFBridgingRelease(obj);
}

void mtlDeleteMTLObj (GLMContext glm_ctx, void *obj)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDeleteMTLObj: glm_ctx buffer: obj];
}

#pragma mark C interface to mtlGetSync
-(void) mtlGetSync:(GLMContext) glm_ctx sync: (Sync *)sync
{
    RETURN_ON_FAILURE([self processGLState: false]);

    if (_currentEvent == NULL)
    {
        _currentEvent = [_device newEvent];
        assert(_currentEvent);
    }

    _currentSyncName = sync->name;

    sync->mtl_event = (void *)CFBridgingRetain(_currentEvent);

    if (_currentCommandBufferSyncList == NULL)
    {
        _currentCommandBufferSyncList = (SyncList *)malloc(sizeof(SyncList));
        assert(_currentCommandBufferSyncList);

        _currentCommandBufferSyncList->size = 8;
        _currentCommandBufferSyncList->list = (Sync **)malloc(sizeof(Sync *) * 8);
        assert(_currentCommandBufferSyncList->list);

        _currentCommandBufferSyncList->count = 0;
    }

    _currentCommandBufferSyncList->list[_currentCommandBufferSyncList->count] = sync;
    _currentCommandBufferSyncList->count++;

    if (_currentCommandBufferSyncList->count >= _currentCommandBufferSyncList->size)
    {
        _currentCommandBufferSyncList->size *= 2;
        _currentCommandBufferSyncList->list = (Sync **)realloc(_currentCommandBufferSyncList->list,
                                                                sizeof(Sync *) * _currentCommandBufferSyncList->size);
        assert(_currentCommandBufferSyncList->list);
    }
}

void mtlGetSync (GLMContext glm_ctx, Sync *sync)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlGetSync: glm_ctx sync: sync];
}

#pragma mark C interface to mtlWaitForSync
-(void) mtlWaitForSync:(GLMContext) glm_ctx sync: (Sync *)sync
{
    RETURN_ON_FAILURE([self processGLState: false]);

    assert(sync->mtl_event);

    CFBridgingRelease(sync->mtl_event);

    sync->mtl_event = NULL;
}

void mtlWaitForSync (GLMContext glm_ctx, Sync *sync)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlWaitForSync: glm_ctx sync: sync];
}

#pragma mark C interface to mtlFlush
-(void) mtlFlush:(GLMContext) glm_ctx finish:(bool)finish
{
    [self flushCommandBuffer: finish];
}

void mtlFlush (GLMContext glm_ctx, bool finish)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlFlush:glm_ctx finish:finish];
}

#pragma mark C interface to mtlSwapBuffers
-(void) mtlSwapBuffers:(GLMContext) glm_ctx
{
    if (ctx->state.draw_buffer == GL_FRONT || ctx->state.draw_buffer == GL_COLOR_ATTACHMENT0)
    {
        // clear commands rely on processGLState
        // glClear / glSwap / repeat..
        RETURN_ON_FAILURE([self processGLState: false]);

        [self endRenderEncoding];

        if (_drawable)
        {
            assert(_currentCommandBuffer);
            [_currentCommandBuffer presentDrawable: _drawable];
        }

        _drawable = [_layer nextDrawable];

        [_currentCommandBuffer commit];

        [self newCommandBufferAndRenderEncoder];
    }
}

void mtlSwapBuffers (GLMContext glm_ctx)
{
    // Call the Objective-C method using Objective-C syntax
    @autoreleasepool {
        [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlSwapBuffers: glm_ctx];
    }
}

#pragma mark C interface to mtlClearBuffer
-(void) mtlClearBuffer:(GLMContext) glm_ctx type:(GLuint) type mask:(GLbitfield) mask
{
    RETURN_ON_FAILURE([self processGLState: false]);
}

void mtlClearBuffer (GLMContext glm_ctx, GLuint type, GLbitfield mask)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlClearBuffer: glm_ctx type: type mask: mask];
}

#pragma mark C interface to mtlBufferSubData

-(void) mtlBufferSubData:(GLMContext) glm_ctx buf:(Buffer *)buf offset:(size_t)offset size:(size_t)size ptr:(const void *)ptr
{
    id<MTLBuffer> mtl_buffer;
    void *data;

    if (buf->data.mtl_data == NULL)
    {
        [self bindMTLBuffer:buf];
    }

    mtl_buffer = (__bridge id<MTLBuffer>)(buf->data.mtl_data);
    assert(mtl_buffer);

    data = mtl_buffer.contents;
    memcpy(data+offset, ptr, size);

    [mtl_buffer didModifyRange:NSMakeRange(offset, size)];
}

void mtlBufferSubData(GLMContext glm_ctx, Buffer *buf, size_t offset, size_t size, const void *ptr)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlBufferSubData: glm_ctx buf: buf offset:offset size:size ptr:ptr];
}

#pragma mark C interface to mtlMapUnmapBuffer
-(void *) mtlMapUnmapBuffer:(GLMContext) glm_ctx buf:(Buffer *)buf offset:(size_t) offset size:(size_t) size access:(GLenum) access map:(bool)map
{
    id<MTLBuffer> mtl_buffer;

    if (buf->data.mtl_data == NULL)
    {
        [self bindMTLBuffer:buf];
    }

    mtl_buffer = (__bridge id<MTLBuffer>)(buf->data.mtl_data);

    if (map)
    {
        return mtl_buffer.contents + offset;
    }

    [mtl_buffer didModifyRange:NSMakeRange(offset, size)];

    return NULL;
}

void *mtlMapUnmapBuffer(GLMContext glm_ctx, Buffer *buf, size_t offset, size_t size, GLenum access, bool map)
{
    // Call the Objective-C method using Objective-C syntax
    return [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMapUnmapBuffer: glm_ctx buf: buf offset: offset size: size access: access map: map];
}

#pragma mark C interface to mtlFlushMappedBufferRange
-(void) mtlFlushMappedBufferRange:(GLMContext) glm_ctx buf:(Buffer *)buf offset:(size_t) offset length:(size_t) length
{
    id<MTLBuffer> mtl_buffer;

    mtl_buffer = (__bridge id<MTLBuffer>)(buf->data.mtl_data);

    [mtl_buffer didModifyRange:NSMakeRange(offset, length)];
}

void mtlFlushBufferRange(GLMContext glm_ctx, Buffer *buf, GLintptr offset, GLsizeiptr length)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlFlushMappedBufferRange: glm_ctx buf: buf offset: offset length: length];
}


#pragma mark C interface to mtlGetTexImage
-(void) mtlGetTexImage:(GLMContext) glm_ctx tex: (Texture *)tex pixelBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level slice:(NSUInteger)slice
{
    id<MTLTexture> texture;

    //[self processGLState: false];

    // flush finish
    [self mtlFlush:glm_ctx finish:true];

    if (tex)
    {
        texture = (__bridge id<MTLTexture>)(tex->mtl_data);
        assert(texture);
    }
    else
    {
        if (glm_ctx->state.readbuffer)
        {
            Framebuffer *fbo;
            GLuint drawbuffer;

            fbo = ctx->state.readbuffer;
            drawbuffer = ctx->state.read_buffer - GL_COLOR_ATTACHMENT0;
            assert(drawbuffer >= 0);
            assert(drawbuffer <= STATE(max_color_attachments));

            tex = [self framebufferAttachmentTexture: &fbo->color_attachments[drawbuffer]];
            assert(tex);

            texture = (__bridge id<MTLTexture>)(tex->mtl_data);
            assert(texture);
        }
        else
        {
            GLuint mgl_drawbuffer;
            id<MTLTexture> texture;

            switch(ctx->state.read_buffer)
            {
                case GL_FRONT: mgl_drawbuffer = _FRONT; break;
                case GL_BACK: mgl_drawbuffer = _BACK; break;
                case GL_FRONT_LEFT: mgl_drawbuffer = _FRONT_LEFT; break;
                case GL_FRONT_RIGHT: mgl_drawbuffer = _FRONT_RIGHT; break;
                case GL_BACK_LEFT: mgl_drawbuffer = _BACK_LEFT; break;
                case GL_BACK_RIGHT: mgl_drawbuffer = _BACK_RIGHT; break;
                default:
                    assert(0);
            }

            if (mgl_drawbuffer == _FRONT)
            {
                texture = _drawable.texture;
                assert(texture);
            }
            else if(_drawBuffers[mgl_drawbuffer].drawbuffer)
            {
                texture = _drawBuffers[mgl_drawbuffer].drawbuffer;
            }
            else
            {
                assert(0);
            }
        }
    }

    if ([texture isFramebufferOnly] == NO)
    {
        [texture getBytes:pixelBytes bytesPerRow:bytesPerRow bytesPerImage:bytesPerImage fromRegion:region mipmapLevel:level slice:slice];
    }
    else
    {
        // issue a gl error as we can't read a framebuffer only texture
        NSLog(@"Cannot read from framebuffer only texture\n");
        ctx->error_func(ctx, __FUNCTION__, GL_INVALID_OPERATION);
    }
}

void mtlGetTexImage(GLMContext glm_ctx, Texture *tex, void *pixelBytes, GLuint bytesPerRow, GLuint bytesPerImage, GLint x, GLint y, GLsizei width, GLsizei height, GLuint level, GLuint slice)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlGetTexImage:glm_ctx tex:tex pixelBytes:pixelBytes bytesPerRow:bytesPerRow bytesPerImage:bytesPerImage fromRegion:MTLRegionMake2D(x,y,width,height) mipmapLevel:level slice:slice];
}

#pragma mark C interface to mtlGenerateMipmaps

-(void)mtlGenerateMipmaps:(GLMContext)glm_ctx forTexture:(Texture *) tex
{
    RETURN_ON_FAILURE([self processGLState: false]);

    // end encoding on current render encoder
    [self endRenderEncoding];

    // no failure path..?
    RETURN_ON_FAILURE([self bindMTLTexture:tex]);
    assert(tex->mtl_data);

    id<MTLTexture> texture;

    texture = (__bridge id<MTLTexture>)(tex->mtl_data);
    assert(texture);

    // start blit encoder
    id<MTLBlitCommandEncoder> blitCommandEncoder;
    blitCommandEncoder = [_currentCommandBuffer blitCommandEncoder];

    [blitCommandEncoder generateMipmapsForTexture:texture];
    [blitCommandEncoder endEncoding];
}

void mtlGenerateMipmaps(GLMContext glm_ctx, Texture *tex)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlGenerateMipmaps:glm_ctx forTexture:tex];
}


#pragma mark C interface to mtlTexSubImage

-(void)mtlTexSubImage:(GLMContext)glm_ctx tex:(Texture *)tex buf:(Buffer *)buf src_offset:(size_t)src_offset src_pitch:(size_t)src_pitch src_image_size:(size_t)src_image_size src_size:(size_t)src_size slice:(GLuint)slice level:(GLuint)level width:(size_t)width height:(size_t)height depth:(size_t)depth xoffset:(size_t)xoffset yoffset:(size_t)yoffset zoffset:(size_t)zoffset
{
    // we can deal with a null buffer but we need a texture
    if (buf->data.mtl_data == NULL)
    {
        [self bindMTLBuffer: buf];
        RETURN_ON_NULL(buf->data.mtl_data);
    }

    id<MTLBuffer> buffer;
    buffer = (__bridge id<MTLBuffer>)(buf->data.mtl_data);
    assert(buffer);

    if (tex->mtl_data == NULL)
    {
        [self bindMTLTexture: tex];
        RETURN_ON_NULL(tex->mtl_data);
    }

    id<MTLTexture> texture;
    texture = (__bridge id<MTLTexture>)(tex->mtl_data);
    assert(texture);

    // end encoding on current render encoder
    [self endRenderEncoding];

    // start blit encoder
    id<MTLBlitCommandEncoder> blitCommandEncoder;
    blitCommandEncoder = [_currentCommandBuffer blitCommandEncoder];

    [blitCommandEncoder copyFromBuffer:buffer sourceOffset:src_offset sourceBytesPerRow:src_pitch sourceBytesPerImage:src_image_size sourceSize:MTLSizeMake(width, height, depth) toTexture:texture destinationSlice:zoffset destinationLevel:level destinationOrigin:MTLOriginMake(xoffset, yoffset, 0)
                                options:MTLBlitOptionNone];

    [blitCommandEncoder endEncoding];
}

void mtlTexSubImage(GLMContext glm_ctx, Texture *tex, Buffer *buf, size_t src_offset, size_t src_pitch, size_t src_image_size, size_t src_size, GLuint slice, GLuint level, size_t width, size_t height, size_t depth, size_t xoffset, size_t yoffset, size_t zoffset)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlTexSubImage:glm_ctx tex:tex buf:buf src_offset:src_offset src_pitch:src_pitch src_image_size:src_image_size src_size:src_size slice:slice level:level width:width height:height depth:depth xoffset:xoffset yoffset:yoffset zoffset:zoffset];
}

#pragma mark utility functions for draw commands
MTLPrimitiveType getMTLPrimitiveType(GLenum mode)
{
    const GLuint err = 0xFFFFFFFF;

    switch(mode)
    {
        case GL_POINTS:
            return MTLPrimitiveTypePoint;

        case GL_LINES:
            return MTLPrimitiveTypeLine;

        case GL_LINE_STRIP:
            return MTLPrimitiveTypeLineStrip;

        case GL_TRIANGLES:
            return MTLPrimitiveTypeTriangle;

        case GL_TRIANGLE_STRIP:
            return MTLPrimitiveTypeTriangleStrip;

        case GL_LINE_LOOP:
        case GL_LINE_STRIP_ADJACENCY:
        case GL_LINES_ADJACENCY:
        case GL_TRIANGLE_FAN:
        case GL_TRIANGLE_STRIP_ADJACENCY:
        case GL_PATCHES:
            assert(0);
            break;
    }

    return err;
}

MTLIndexType getMTLIndexType(GLenum type)
{
    const GLuint err = 0xFFFFFFFF;

    switch(type)
    {
        case GL_UNSIGNED_SHORT:
            return MTLIndexTypeUInt16;

        case GL_UNSIGNED_INT:
            return MTLIndexTypeUInt32;
    }

    return err;
}

Buffer *getElementBuffer(GLMContext ctx)
{
    Buffer *gl_element_buffer = VAO_STATE(element_array.buffer);

    return gl_element_buffer;
}

Buffer *getIndirectBuffer(GLMContext ctx)
{
    Buffer *gl_indirect_buffer = ctx->state.buffers[_DRAW_INDIRECT_BUFFER];

    return gl_indirect_buffer;
}

#pragma mark C interface to mtlDrawArrays
-(void) mtlDrawArrays: (GLMContext) ctx mode:(GLenum) mode first: (GLint) first count: (GLsizei) count
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

     [_currentRenderEncoder drawPrimitives: primitiveType
                              vertexStart: first
                              vertexCount: count];
}

void mtlDrawArrays(GLMContext glm_ctx, GLenum mode, GLint first, GLsizei count)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArrays: glm_ctx mode: mode first: first count: count];
}

#pragma mark C interface to mtlDrawElements
-(void) mtlDrawElements: (GLMContext) glm_ctx mode:(GLenum) mode count: (GLsizei) count type: (GLenum) type indices:(const void *)indices
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count indexType:indexType
                                     indexBuffer:indexBuffer indexBufferOffset:0 instanceCount:1];
}

void mtlDrawElements(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElements: glm_ctx mode: mode count: count type: type indices: indices];
}


#pragma mark C interface to mtlDrawRangeElements
-(void) mtlDrawRangeElements: (GLMContext) glm_ctx mode:(GLenum) mode start:(GLuint) start end:(GLuint) end count: (GLsizei) count type: (GLenum) type indices:(const void *)indices
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    // indexBufferOffset is a byte offset
    switch(indexType)
    {
        case MTLIndexTypeUInt16: start <<= 1; break;
        case MTLIndexTypeUInt32: start <<= 2; break;
    }

    // for now lets just ignore the range data and use drawIndexedPrimitives
    //
    // in the future it would be an idea to use temp buffers for large buffers that would wire
    // to much memory down.. like a million point galaxy drawing
    //
    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:end - count indexType:indexType
                                     indexBuffer:indexBuffer indexBufferOffset:offset+start instanceCount:1];
}

void mtlDrawRangeElements(GLMContext glm_ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawRangeElements: glm_ctx mode: mode start: start end: end count: count type: type indices: indices];
}


#pragma mark C interface to mtlDrawArraysInstanced
-(void) mtlDrawArraysInstanced: (GLMContext) glm_ctx mode:(GLenum) mode first: (GLint) first count: (GLsizei) count instancecount:(GLsizei) instancecount
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    [_currentRenderEncoder drawPrimitives:primitiveType vertexStart:first vertexCount:count instanceCount:instancecount];
}

void mtlDrawArraysInstanced(GLMContext glm_ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArraysInstanced: glm_ctx mode: mode first: first count: count instancecount: instancecount];
}


#pragma mark C interface to mtlDrawElementsInstanced
-(void) mtlDrawElementsInstanced: (GLMContext) glm_ctx mode:(GLenum) mode count: (GLsizei) count type: (GLenum) type indices:(const void *)indices instancecount:(GLsizei) instancecount
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    // for now lets just ignore the range data and use drawIndexedPrimitives
    //
    // in the future it would be an idea to use temp buffers for large buffers that would wire
    // to much memory down.. like a million point galaxy drawing
    //
    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count indexType:indexType
                                     indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:instancecount];
}

void mtlDrawElementsInstanced(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsInstanced: glm_ctx mode: mode count: count type: type indices: indices instancecount: instancecount];
}


#pragma mark C interface to mtlDrawElementsBaseVertex
-(void) mtlDrawElementsBaseVertex: (GLMContext) glm_ctx mode:(GLenum) mode count: (GLsizei) count type: (GLenum) type indices:(const void *)indices basevertex:(GLint) basevertex
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    [_currentRenderEncoder drawIndexedPrimitives: primitiveType indexCount:count indexType: indexType indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:1 baseVertex:basevertex baseInstance:0];
}

void mtlDrawElementsBaseVertex(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsBaseVertex: glm_ctx mode: mode count: count type: type indices: indices basevertex: basevertex];
}


#pragma mark C interface to mtlDrawRangeElementsBaseVertex
-(void) mtlDrawRangeElementsBaseVertex: (GLMContext) glm_ctx mode:(GLenum) mode start: (GLuint) start end: (GLuint) end type: (GLenum) type indices:(const void *)indices basevertex:(GLint) basevertex
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    // indexBufferOffset is a byte offset
    switch(indexType)
    {
        case MTLIndexTypeUInt16: start <<= 1; break;
        case MTLIndexTypeUInt32: start <<= 2; break;
    }

    [_currentRenderEncoder drawIndexedPrimitives: primitiveType indexCount:end - start indexType: indexType indexBuffer:indexBuffer indexBufferOffset:offset+start instanceCount:1 baseVertex:basevertex baseInstance:0];
}

void mtlDrawRangeElementsBaseVertex(GLMContext glm_ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawRangeElementsBaseVertex:glm_ctx mode:mode start: start end: end type: type indices: indices basevertex:basevertex];
}


#pragma mark C interface to mtlDrawElementsInstancedBaseVertex
-(void) mtlDrawElementsInstancedBaseVertex: (GLMContext) glm_ctx mode:(GLenum) mode count:(GLuint) count type: (GLenum) type indices:(const void *)indices instancecount:(GLsizei) instancecount basevertex:(GLint) basevertex
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count indexType:indexType indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:instancecount baseVertex:basevertex baseInstance:0];
}

void mtlDrawElementsInstancedBaseVertex(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsInstancedBaseVertex:glm_ctx mode:mode count:count type:type indices:indices instancecount:instancecount basevertex:basevertex];
}

#pragma mark C interface to mtlDrawArraysIndirect
-(void) mtlDrawArraysIndirect: (GLMContext) glm_ctx mode:(GLenum) mode indirect: (const void *) indirect
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    Buffer *gl_indirect_buffer = getIndirectBuffer(ctx);
    assert(gl_indirect_buffer);

    if ([self processBuffer: gl_indirect_buffer] == false)
        return;

    id <MTLBuffer>indirectBuffer = (__bridge id<MTLBuffer>)(gl_indirect_buffer->data.mtl_data);
    assert(indirectBuffer);

    [_currentRenderEncoder drawPrimitives:primitiveType indirectBuffer:indirectBuffer indirectBufferOffset:(DrawArraysIndirectCommand *)indirect - (DrawArraysIndirectCommand *)NULL];
}

void mtlDrawArraysIndirect(GLMContext glm_ctx, GLenum mode, const void *indirect)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArraysIndirect:glm_ctx mode:mode indirect:indirect];
}


#pragma mark C interface to mtlDrawElementsIndirect
-(void) mtlDrawElementsIndirect: (GLMContext) glm_ctx mode:(GLenum) mode type:(GLenum) type indirect: (const void *) indirect
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    // get element buffer
    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    // get indirect buffer
    Buffer *gl_indirect_buffer = getIndirectBuffer(ctx);
    assert(gl_indirect_buffer);

    if ([self processBuffer: gl_indirect_buffer] == false)
        return;

    id <MTLBuffer>indirectBuffer = (__bridge id<MTLBuffer>)(gl_indirect_buffer->data.mtl_data);
    assert(indirectBuffer);

    // draw indexed primitive
    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexType:indexType indexBuffer: indexBuffer indexBufferOffset:0 indirectBuffer:indirectBuffer indirectBufferOffset:(DrawElementsIndirectCommand *)indirect - (DrawElementsIndirectCommand *)NULL];
}

void mtlDrawElementsIndirect(GLMContext glm_ctx, GLenum mode, GLenum type, const void *indirect)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsIndirect:glm_ctx mode:mode type:type indirect:indirect];
}


#pragma mark C interface to mtlDrawArraysInstancedBaseInstance
-(void) mtlDrawArraysInstancedBaseInstance: (GLMContext) glm_ctx mode:(GLenum) mode first: (GLint) first count: (GLsizei) count instancecount:(GLsizei) instancecount baseinstance:(GLuint) baseinstance
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    [_currentRenderEncoder drawPrimitives:primitiveType vertexStart:first vertexCount:count instanceCount:instancecount baseInstance:baseinstance];
}

void mtlDrawArraysInstancedBaseInstance(GLMContext glm_ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArraysInstancedBaseInstance:glm_ctx mode:mode first:first count:count instancecount:instancecount baseinstance:baseinstance];
}


#pragma mark C interface to mtlDrawElementsInstancedBaseInstance
-(void) mtlDrawElementsInstancedBaseInstance: (GLMContext) glm_ctx mode:(GLenum) mode  count: (GLsizei) count type:(GLenum) type indices:(const void *)indices instancecount:(GLsizei) instancecount baseinstance:(GLuint) baseinstance
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    // for now lets just ignore the range data and use drawIndexedPrimitives
    //
    // in the future it would be an idea to use temp buffers for large buffers that would wire
    // to much memory down.. like a million point galaxy drawing
    //
    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count indexType:indexType indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:instancecount baseVertex:0 baseInstance:baseinstance];
}

void mtlDrawElementsInstancedBaseInstance(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsInstancedBaseInstance:glm_ctx mode:mode count:count type:type indices:indices instancecount:instancecount baseinstance:baseinstance];
}


#pragma mark C interface to mtlDrawElementsInstancedBaseVertexBaseInstance
-(void) mtlDrawElementsInstancedBaseVertexBaseInstance: (GLMContext) glm_ctx mode:(GLenum) mode count: (GLsizei) count type:(GLenum) type indices:(const void *)indices
                                                        instancecount:(GLsizei) instancecount basevertex:(GLint) basevertex baseinstance:(GLuint) baseinstance
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    size_t offset = (char *)indices - (char *)NULL;

    // for now lets just ignore the range data and use drawIndexedPrimitives
    //
    // in the future it would be an idea to use temp buffers for large buffers that would wire
    // to much memory down.. like a million point galaxy drawing
    //
    [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count indexType:indexType indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:instancecount baseVertex:basevertex baseInstance:baseinstance];
}

void mtlDrawElementsInstancedBaseVertexBaseInstance(GLMContext glm_ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawElementsInstancedBaseVertexBaseInstance:glm_ctx mode:mode count:count type:type indices:indices instancecount:instancecount basevertex:basevertex baseinstance:baseinstance];
}


#pragma mark C interface to mtlMultiDrawArrays
-(void) mtlMultiDrawArrays: (GLMContext)glm_ctx mode:(GLenum) mode first:(const GLint *)first count:(const GLsizei *)count drawcount:(GLsizei) drawcount
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    for(int i=0; i<drawcount; i++)
    {
         [_currentRenderEncoder drawPrimitives: primitiveType
                                  vertexStart: first[i]
                                  vertexCount: count[i]];
    }
}

void mtlMultiDrawArrays(GLMContext glm_ctx, GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMultiDrawArrays:glm_ctx mode:mode first:first count:count drawcount:drawcount];
}


#pragma mark C interface to mtlMultiDrawElements
-(void) mtlMultiDrawElements: (GLMContext)glm_ctx mode:(GLenum) mode count:(const GLsizei *)count type:(GLenum)type indices:(const void *const*)indices drawcount:(GLsizei) drawcount
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    for(int i=0; i<drawcount; i++)
    {
        size_t offset;

        offset = (char *)indices[i] - (char *)NULL;

        [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count[i] indexType:indexType
                                     indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:1];
    }
}

void mtlMultiDrawElements(GLMContext glm_ctx, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMultiDrawElements: glm_ctx mode: mode count: count type: type indices: indices drawcount: drawcount];
}




#pragma mark C interface to mtlMultiDrawElementsBaseVertex
-(void) mtlMultiDrawElementsBaseVertex: (GLMContext) glm_ctx mode:(GLenum) mode count: (const GLsizei *) count type: (GLenum) type indices:(const void *const *)indices drawcount:(GLsizei) drawcount basevertex:(const GLint *) basevertex
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    // element buffer
    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);


    for(int i=0; i<drawcount; i++)
    {
        size_t offset;

        offset = (char *)indices[i] - (char *)NULL;

        [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexCount:count[i] indexType:indexType
                                     indexBuffer:indexBuffer indexBufferOffset:offset instanceCount:count[i] baseVertex:basevertex[i] baseInstance:1];
    }
}

void mtlMultiDrawElementsBaseVertex(GLMContext glm_ctx, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMultiDrawElementsBaseVertex: glm_ctx mode: mode count: count type: type indices: indices drawcount: drawcount basevertex:basevertex];
}


-(void) mtlMultiDrawArraysIndirect: (GLMContext)glm_ctx mode:(GLenum) mode indirect:(const void *)indirect drawcount:(GLsizei) drawcount stride:(GLsizei)stride
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    Buffer *gl_indirect_buffer = getIndirectBuffer(ctx);
    assert(gl_indirect_buffer);

    if ([self processBuffer: gl_indirect_buffer] == false)
        return;

    id <MTLBuffer>indirectBuffer = (__bridge id<MTLBuffer>)(gl_indirect_buffer->data.mtl_data);
    assert(indirectBuffer);

    for(int i=0; i<drawcount; i++)
    {
        size_t offset;

        if (stride)
        {
            offset = (char *)((char *)indirect + i * stride) - (char *)NULL;
        }
        else
        {
            offset = (char *)indirect + i - (char *)NULL;
        }

        [_currentRenderEncoder drawPrimitives:primitiveType indirectBuffer:indirectBuffer indirectBufferOffset:offset];
    }
}

void mtlMultiDrawArraysIndirect(GLMContext glm_ctx, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMultiDrawArraysIndirect:glm_ctx mode:mode indirect:indirect drawcount:drawcount stride:stride];
}


-(void) mtlMultiDrawElementsIndirect: (GLMContext)glm_ctx mode:(GLenum) mode type:(GLenum)type indirect:(const void *)indirect drawcount:(GLsizei) drawcount stride:(GLsizei)stride
{
    MTLPrimitiveType primitiveType;
    MTLIndexType indexType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

    // get element buffer
    indexType = getMTLIndexType(type);
    assert(indexType != 0xFFFFFFFF);

    Buffer *gl_element_buffer = getElementBuffer(ctx);
    assert(gl_element_buffer);

    if ([self processBuffer: gl_element_buffer] == false)
        return;

    id <MTLBuffer>indexBuffer = (__bridge id<MTLBuffer>)(gl_element_buffer->data.mtl_data);
    assert(indexBuffer);

    // get indirect buffer
    Buffer *gl_indirect_buffer = getIndirectBuffer(ctx);
    assert(gl_indirect_buffer);

    if ([self processBuffer: gl_indirect_buffer] == false)
        return;

    id <MTLBuffer>indirectBuffer = (__bridge id<MTLBuffer>)(gl_indirect_buffer->data.mtl_data);
    assert(indirectBuffer);

    for(int i=0; i<drawcount; i++)
    {
        size_t offset;

        if (stride)
        {
            offset = (char *)((char *)indirect + i * stride) - (char *)NULL;
        }
        else
        {
            offset = (char *)indirect + i - (char *)NULL;
        }

        // draw indexed primitive
        [_currentRenderEncoder drawIndexedPrimitives:primitiveType indexType:indexType indexBuffer: indexBuffer indexBufferOffset:0 indirectBuffer:indirectBuffer indirectBufferOffset:offset];
    }
}

void mtlMultiDrawElementsIndirect(GLMContext glm_ctx, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlMultiDrawElementsIndirect:glm_ctx mode:mode type:type indirect:indirect drawcount:drawcount stride:stride];
}

#pragma mark C interface to context functions

- (void) bindObjFuncsToGLMContext: (GLMContext) glm_ctx
{
    glm_ctx->mtl_funcs.mtlObj = (void *)CFBridgingRetain(self);

    glm_ctx->mtl_funcs.mtlBindBuffer = mtlBindBuffer;
    glm_ctx->mtl_funcs.mtlBindTexture = mtlBindTexture;
    glm_ctx->mtl_funcs.mtlBindProgram = mtlBindProgram;

    glm_ctx->mtl_funcs.mtlDeleteMTLObj = mtlDeleteMTLObj;

    glm_ctx->mtl_funcs.mtlGetSync = mtlGetSync;
    glm_ctx->mtl_funcs.mtlWaitForSync = mtlWaitForSync;
    glm_ctx->mtl_funcs.mtlFlush = mtlFlush;
    glm_ctx->mtl_funcs.mtlSwapBuffers = mtlSwapBuffers;
    glm_ctx->mtl_funcs.mtlClearBuffer = mtlClearBuffer;
    glm_ctx->mtl_funcs.mtlBlitFramebuffer = mtlBlitFramebuffer;

    glm_ctx->mtl_funcs.mtlBufferSubData = mtlBufferSubData;
    glm_ctx->mtl_funcs.mtlMapUnmapBuffer = mtlMapUnmapBuffer;
    glm_ctx->mtl_funcs.mtlFlushBufferRange = mtlFlushBufferRange;

    glm_ctx->mtl_funcs.mtlGetTexImage = mtlGetTexImage;
    glm_ctx->mtl_funcs.mtlGenerateMipmaps = mtlGenerateMipmaps;
    glm_ctx->mtl_funcs.mtlTexSubImage = mtlTexSubImage;

    glm_ctx->mtl_funcs.mtlDrawArrays = mtlDrawArrays;
    glm_ctx->mtl_funcs.mtlDrawElements = mtlDrawElements;
    glm_ctx->mtl_funcs.mtlDrawRangeElements = mtlDrawRangeElements;
    glm_ctx->mtl_funcs.mtlDrawArraysInstanced = mtlDrawArraysInstanced;
    glm_ctx->mtl_funcs.mtlDrawElementsInstanced = mtlDrawElementsInstanced;
    glm_ctx->mtl_funcs.mtlDrawElementsBaseVertex = mtlDrawElementsBaseVertex;
    glm_ctx->mtl_funcs.mtlDrawRangeElementsBaseVertex = mtlDrawRangeElementsBaseVertex;
    glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseVertex = mtlDrawElementsInstancedBaseVertex;
    glm_ctx->mtl_funcs.mtlMultiDrawElementsBaseVertex = mtlMultiDrawElementsBaseVertex;
    glm_ctx->mtl_funcs.mtlDrawArraysIndirect = mtlDrawArraysIndirect;
    glm_ctx->mtl_funcs.mtlDrawElementsIndirect = mtlDrawElementsIndirect;
    glm_ctx->mtl_funcs.mtlDrawArraysInstancedBaseInstance = mtlDrawArraysInstancedBaseInstance;
    glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseInstance = mtlDrawElementsInstancedBaseInstance;
    glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseVertexBaseInstance = mtlDrawElementsInstancedBaseVertexBaseInstance;

    glm_ctx->mtl_funcs.mtlMultiDrawArrays = mtlMultiDrawArrays;
    glm_ctx->mtl_funcs.mtlMultiDrawElements = mtlMultiDrawElements;
    glm_ctx->mtl_funcs.mtlMultiDrawElementsBaseVertex = mtlMultiDrawElementsBaseVertex;
    glm_ctx->mtl_funcs.mtlMultiDrawArraysIndirect = mtlMultiDrawArraysIndirect;
    glm_ctx->mtl_funcs.mtlMultiDrawElementsIndirect = mtlMultiDrawElementsIndirect;

    glm_ctx->mtl_funcs.mtlDispatchCompute = mtlDispatchCompute;
    glm_ctx->mtl_funcs.mtlDispatchComputeIndirect = mtlDispatchComputeIndirect;
}

//MGLRenderer *renderer
void*
CppCreateMGLRendererAndBindToContext (void *window, void *glm_ctx)
{
    assert (window);
    assert (glm_ctx);
    MGLRenderer *renderer = [[MGLRenderer alloc] init];
    assert (renderer);
    NSWindow * w = (__bridge NSWindow *)(window); // just a plain bridge as the autorelease pool will try to release this and crash on exit
    assert (w);
    NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(100, 100, 100, 100)];
    assert (view);
    [view setWantsLayer:YES];
    //assert(w.contentView);
    //[w.contentView addSubview:view];
    [w setContentView:view];
    [renderer createMGLRendererAndBindToContext: glm_ctx view: view];
    return  (__bridge void *)(renderer);
}

- (void) createMGLRendererAndBindToContext: (GLMContext) glm_ctx view: (NSView *) view
{
    ctx = glm_ctx;

    [self bindObjFuncsToGLMContext: glm_ctx];

    _device = MTLCreateSystemDefaultDevice();
    assert(_device);

    // Create the command queue
    _commandQueue = [_device newCommandQueue];
    assert(_commandQueue);

    _view = view;

    _layer = [[CAMetalLayer alloc] init];
    assert(_layer);

    _layer.device = _device;
    _layer.pixelFormat = ctx->pixel_format.mtl_pixel_format;
    _layer.framebufferOnly = NO; // enable blitting to main color buffer
    _layer.frame = view.layer.frame;
    _layer.magnificationFilter = kCAFilterNearest;

    // from https://github.com/bkaradzic/bgfx/issues/2009#issuecomment-581390564
    int scaleFactor = [[NSScreen mainScreen] backingScaleFactor];
    [_layer setContentsScale: scaleFactor];

    // for iOS, it'll be:
    // scaleFactor = [[UIScreen mainScreen] scale];
    // [m_metalLayer setContentScaleFactor: scaleFactor];

    //assert([_view layer]);
    if ([_view layer])
    {
        [[_view layer] addSublayer: _layer];
    }
    else
    {
        [_view setLayer: _layer];
    }

    mglDrawBuffer(glm_ctx, GL_FRONT);

    // not sure if this is still needed
    [self newCommandBuffer];
    
    glm_ctx->mtl_funcs.mtlView = (void *)CFBridgingRetain(view);

    // capture Metal commands in MGL.gputrace
    // necessitates Info.plist in the cwd, see https://stackoverflow.com/a/64172784
    //MTLCaptureDescriptor *descriptor = [self setupCaptureToFile: _device];
    //[self startCapture:descriptor];
}

- (MTLCaptureDescriptor *)setupCaptureToFile: (id<MTLDevice>)device//(nonnull MTLDevice* )device // (nonnull MTKView *)view
{
    MTLCaptureDescriptor *descriptor = [[MTLCaptureDescriptor alloc] init];
    descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
    descriptor.outputURL = [NSURL fileURLWithPath:@"MGL.gputrace"];
    descriptor.captureObject = device; //((MTKView *)view).device;
    
    return descriptor;
}

- (void)startCapture:(MTLCaptureDescriptor *) descriptor
{
    NSError *error = nil;
    BOOL success = [MTLCaptureManager.sharedCaptureManager startCaptureWithDescriptor:descriptor
                                                                                error:&error];
    if (!success) {
        NSLog(@" error capturing mtl => %@ ", [error localizedDescription] );
    }
}

// Stop the capture.
- (void)stopCapture
{
    [MTLCaptureManager.sharedCaptureManager stopCapture];
}

@end
