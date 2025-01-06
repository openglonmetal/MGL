//
//  buffers.h
//  MGL
//
//  Created by Toshit Chawda on 8/19/22.
//

#ifndef buffers_h
#define buffers_h

#if defined (__i386__) || defined (__x86_64__)
#include <mach/i386/kern_return.h>
#else
#include <mach/arm/kern_return.h>
#endif

kern_return_t initBufferData(GLMContext ctx, Buffer *ptr, GLsizeiptr size, const void *data, bool isUniformConstant);
Buffer *newBuffer(GLMContext ctx, GLenum target, GLuint name);

#endif /* buffers_h */
