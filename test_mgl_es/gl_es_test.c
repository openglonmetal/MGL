//
//  gl_es_test.c
//  test_mgl_es
//
//  Created by Michael Larson on 1/14/26.
//

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>

#include "gl_es_test.h"

#define GL_SILENCE_DEPRECATION

#include "GLES/gl.h"
#include "GLES/glext.h"
#include "GLES3/gl3.h"
#include "GLES3/gl31.h"
#include "GLES3/gl32.h"


#define GLSL_ES(version, shader) "#version " #version "es\n" #shader

GLuint bindDataToVBO(GLenum target, size_t size, void *ptr, GLenum usage)
{
    GLuint vbo = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(target, vbo);
    glBufferData(target, size, ptr, usage);
    glBindBuffer(target, 0);

    return vbo;
}

GLuint createAndBindVAO(void)
{
    GLuint new_vao;

    glGenVertexArrays(1, &new_vao);
    glBindVertexArray(new_vao);

    return new_vao;
}

void bindAttribute(GLuint index, GLuint target, GLuint vbo, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    glEnableVertexAttribArray(index);
    glBindBuffer(target, vbo);
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
}

GLuint compileGLSLProgram(GLenum shader_count, ...)
{
    va_list argp;
    va_start(argp, shader_count);
    GLuint type;
    const char *src;
    GLuint shader;

    GLuint shader_program = glCreateProgram();

    for(int i=0; i<shader_count; i++)
    {
        type = va_arg(argp, GLuint);
        src = va_arg(argp, const char *);
        assert(src);

        shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        glAttachShader(shader_program, shader);
    }
    
    glLinkProgram(shader_program);

    va_end(argp);

    return shader_program;
}

GLuint createTexture(GLenum target, GLsizei width, GLsizei height, GLsizei depth, const void *pixels, GLint level, GLint internalformat, GLenum format, GLenum type)
{
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    switch(target)
    {
        case GL_TEXTURE_2D:
            glTexImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(target, level, internalformat, width, height, depth, 0, format, type, pixels);
            break;

        case GL_TEXTURE_CUBE_MAP:
            glTexImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
            break;

    }
    glBindTexture(target, 0);

    return tex;
}

void bufferSubData(GLenum target, GLuint buffer, GLsizei size, const void *ptr)
{
    glBindBuffer(target, buffer);
    glBufferSubData(target, 0, size, ptr);
    glBindBuffer(target, 0);
}

void test_draw_arrays(void)
{
    GLuint vbo = 0, vao = 0;
    
    const char* vertex_shader =
    GLSL_ES(310,
        layout(location = 0) in vec3 vPos;
        
        void main()
        {
            gl_Position = vec4(vPos, 1.0f);
        }
     );
    
    const char* fragment_shader =
    GLSL_ES(310,
        precision mediump float;
        out vec4 fragColor;
            
        void main()
        {
            fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    );
    
    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    vbo = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    vao = createAndBindVAO();

    bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);

    GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void test_gl_es(unsigned window_width, unsigned window_height)
{
    glViewport(0, 0, window_width, window_height);
    
    test_draw_arrays();
}
