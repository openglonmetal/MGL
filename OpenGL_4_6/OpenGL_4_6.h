//
//  OpenGL_4_6.h
//  OpenGL_4_6
//
//  Created by Michael Larson on 1/17/26.
//

#ifndef _OPENGL_4_6_H
#define _OPENGL_4_6_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#undef OPENGL_ENUM_AVAILABLE
#undef OPENGL_ENUM_DEPRECATED
#undef OPENGL_ENUM_DEPRECATED_MSG

#include "OpenGL_4_6/glcorearb.h"
#include <OpenGL_4_6/MGLContext.h>
#include <OpenGL_4_6/MGLRenderer.h>

GLuint sizeForFormatType(GLenum format, GLenum type);
GLuint bicountForFormatType(GLenum format, GLenum type, GLenum component);
void MGLget(GLMContext ctx, GLenum param, GLuint *data);
bool pixelConvertToInternalFormat(GLMContext ctx, GLenum internalformat, GLenum format, GLenum type, const void *src, void *dst, size_t len);

GLMContext MGLgetCurrentContext(void);
void MGLsetCurrentContext(GLMContext ctx);


#ifdef __cplusplus
}
#endif

#endif // _OPENGL_4_6_H
