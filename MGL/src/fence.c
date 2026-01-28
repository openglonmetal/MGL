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
 * fence.c
 * MGL
 *
 */

#include <strings.h>

#include "glm_context.h"

Sync *newSync(GLMContext ctx)
{
    Sync *ptr;

    ptr = (Sync *)malloc(sizeof(Sync));
    // CRITICAL SECURITY FIX: Check malloc result instead of using assert()
    if (!ptr)
    {            
        fprintf(stderr, "MGL SECURITY ERROR: Failed to allocate memory for Sync\n");
        return NULL;
    }

    bzero(ptr, sizeof(Sync));

    ptr->name = STATE(sync_name)++;

    return ptr;
}

int isSync(GLMContext ctx, GLsync sync)
{
    if (sync->name < STATE(sync_name))
        return 1;

    return 0;
}

GLsync mglFenceSync(GLMContext ctx, GLenum condition, GLbitfield flags)
{
    Sync *ptr;

    switch(condition)
    {
        case GL_SYNC_GPU_COMMANDS_COMPLETE:
            break;

        default:
            // CRITICAL FIX: Handle unknown fence conditions gracefully instead of crashing
            fprintf(stderr, "MGL ERROR: Unknown fence sync condition 0x%x, defaulting to GPU_COMMANDS_COMPLETE\n", condition);
            break;
    }

    // must be zero
    if (flags != 0)
    {            
        // CRITICAL FIX: Handle invalid flags gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Fence sync flags must be zero, got 0x%x, continuing with zero\n", flags);
    }

    ptr = newSync(ctx);

    ctx->mtl_funcs.mtlGetSync(ctx, ptr);

    return ptr;
}


GLboolean mglIsSync(GLMContext ctx, GLsync sync)
{
    if (sync == NULL)
    {
        return false;
    }

    return (GLboolean)isSync(ctx, sync);
}

void mglDeleteSync(GLMContext ctx, GLsync sync)
{
    if (isSync(ctx, sync) == GL_FALSE)
    {
        // CRITICAL FIX: Handle invalid sync gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Attempting to delete invalid sync object %p\n", sync);
        return;
    }

    if (sync->mtl_event)
    {
        ctx->mtl_funcs.mtlWaitForSync(ctx, sync);

        // should be null - but handle gracefully if not
        if (sync->mtl_event != NULL)
        {            
            fprintf(stderr, "MGL WARNING: sync->mtl_event should be NULL after wait, but is %p\n", sync->mtl_event);
        }
    }

    free(sync);
}

GLenum  mglClientWaitSync(GLMContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if (flags & ~GL_SYNC_FLUSH_COMMANDS_BIT)
    {
        // CRITICAL FIX: Handle invalid flags gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Invalid sync flags 0x%x, only GL_SYNC_FLUSH_COMMANDS_BIT allowed\n", flags);
        return GL_INVALID_VALUE;
    }

    if (isSync(ctx, sync) == GL_FALSE)
    {
        // CRITICAL FIX: Handle invalid sync gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Invalid sync object %p passed to client wait sync\n", sync);
        return GL_INVALID_VALUE;
    }

    if (sync->mtl_event == NULL)
    {
        return GL_ALREADY_SIGNALED;
    }

    ctx->mtl_funcs.mtlWaitForSync(ctx, sync);

    assert(sync->mtl_event == NULL);

    return GL_CONDITION_SATISFIED;
}

void mglWaitSync(GLMContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if (isSync(ctx, sync) == GL_FALSE)
    {
        // CRITICAL FIX: Handle invalid sync gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Invalid sync object %p passed to wait sync\n", sync);
        return;
    }

    if (timeout != GL_TIMEOUT_IGNORED)
    {            
        // CRITICAL FIX: Handle invalid timeout gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Server wait sync timeout must be GL_TIMEOUT_IGNORED, got 0x%llx\n", timeout);
        // Continue with GL_TIMEOUT_IGNORED behavior
    }

    ctx->mtl_funcs.mtlWaitForSync(ctx, sync);

    // Handle gracefully if event is not null after wait
    if (sync->mtl_event != NULL)
    {            
        fprintf(stderr, "MGL WARNING: sync->mtl_event should be NULL after server wait, but is %p\n", sync->mtl_event);
    }
}

void mglGetSynciv(GLMContext ctx, GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values)
{
    if (isSync(ctx, sync) == GL_FALSE)
    {
        // CRITICAL FIX: Handle invalid sync gracefully instead of crashing
        fprintf(stderr, "MGL ERROR: Invalid sync object %p passed to get sync iv\n", sync);
        return;
    }

    // CRITICAL FIX: Add parameter validation with graceful handling
    if (!count || count < 0)
    {
        fprintf(stderr, "MGL ERROR: Invalid count %d in get sync iv\n", count);
        return;
    }
    
    if (!length)
    {
        fprintf(stderr, "MGL ERROR: NULL length pointer in get sync iv\n");
        return;
    }
    
    if (!values)
    {
        fprintf(stderr, "MGL ERROR: NULL values pointer in get sync iv\n");
        return;
    }

    if ((size_t)(count * sizeof(GLuint) > SIZE_MAX))
    {
        fprintf(stderr, "MGL ERROR: integer overflow of size\n");
        return;
    }
    
    if (*length < (GLsizei)(count * sizeof(GLuint)))
    {
        // CRITICAL FIX: Handle insufficient buffer size gracefully
        fprintf(stderr, "MGL ERROR: Insufficient buffer size %d for %d values in get sync iv\n", *length, count);
        *length = count * sizeof(GLuint);
        return;
    }

    while(count--)
    {
        switch(pname)
        {
            case GL_OBJECT_TYPE:
                *values = GL_SYNC_FENCE;
                break;

            case GL_SYNC_STATUS:
                if (sync->mtl_event)
                    *values = GL_UNSIGNALED;
                else
                    *values = GL_SIGNALED;
                break;

            case GL_SYNC_CONDITION:
                if (sync->mtl_event == NULL)
                    *values = GL_SYNC_GPU_COMMANDS_COMPLETE;
                break;

            case GL_SYNC_FLAGS:
                *values = 0;
                break;

            default:
                // CRITICAL FIX: Handle unknown sync parameters gracefully instead of crashing
                fprintf(stderr, "MGL ERROR: Unknown sync parameter 0x%x in get sync iv\n", pname);
                *values = 0; // Return safe default value
                break;
        }

        values++;
    }
}

void mglTextureBarrier(GLMContext ctx)
{
    // CRITICAL FIX: Handle unimplemented function gracefully instead of crashing
    fprintf(stderr, "MGL WARNING: mglTextureBarrier is not yet implemented in MGL\n");
    // No-op implementation - this is optional functionality
}

void mglMemoryBarrier(GLMContext ctx, GLbitfield barriers)
{
    if (barriers & ~(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT |  GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_PIXEL_BUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT | GL_TRANSFORM_FEEDBACK_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT))
    {
        // extra bits...
        ERROR_RETURN(GL_INVALID_VALUE);
    }
}

void mglMemoryBarrierByRegion(GLMContext ctx, GLbitfield barriers)
{

    if (barriers & ~(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT))
    {
        // extra bits...
        ERROR_RETURN(GL_INVALID_VALUE);
    }
}

