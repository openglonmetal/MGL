//
//  Header.h
//  MGL Tests
//
//  Created by Michael Larson on 1/4/25.
//

#ifndef Header_h
#define Header_h

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

#define GLSL(version, shader) "#version " #version "\n" #shader

void write_pixel(GLenum format, GLenum type, void *ptr, float r, float g, float b, float a);

void *gen3DTexturePixels(GLenum format, GLenum type, GLuint repeat, GLuint width, GLuint height, GLint depth);

void HSVtoRGB(float H, float S,float V, float *r, float *g, float *b);

float clamp(float a, float min, float max);

void *genTexturePixels(GLenum format, GLenum type, GLuint repeat, GLuint width, GLuint height, GLint depth=1, GLboolean is_array=false);

GLuint bindDataToVBO(GLenum target, size_t size, void *ptr, GLenum usage);

GLuint bindVAO(GLuint vao=0);

void bindAttribute(GLuint index, GLuint target, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

GLuint compileGLSLProgram(GLenum shader_count, ...);

GLuint createTexture(GLenum target, GLsizei width, GLsizei height=1, GLsizei depth=1, const void *pixels=NULL, GLint level=0, GLint internalformat=GL_RGBA8, GLenum format=GL_RGBA, GLenum type=GL_UNSIGNED_BYTE);

void bufferSubData(GLenum target, GLuint buffer, GLsizei size, const void *ptr);

#endif /* Header_h */
