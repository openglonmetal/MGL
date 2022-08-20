//
//  buffers.h
//  MGL
//
//  Created by Toshit Chawda on 8/19/22.
//

#ifndef buffers_h
#define buffers_h

#include <mach/arm/kern_return.h>

kern_return_t initBufferData(GLMContext ctx, Buffer *ptr, GLsizeiptr size, const void *data);
Buffer *newBuffer(GLMContext ctx, GLenum target, GLuint buffer);

#endif /* buffers_h */
