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
 * main.c
 * test_glfw_mgl
 *
 */

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

#if !defined(TEST_MGL_GLFW) && !defined(TEST_MGL_SDL)
#define TEST_MGL_GLFW 1
#endif

#if TEST_MGL_GLFW
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#define SWAP_BUFFERS MGLswapBuffers((GLMContext) glfwGetWindowUserPointer(window));
#endif

#if TEST_MGL_SDL
#include <SDL.h>
#include <SDL_syswm.h>
#define SWAP_BUFFERS MGLswapBuffers((GLMContext) SDL_GetWindowData(window,"MGLRenderer"));
#define GLFWwindow SDL_Window
SDL_Event sdlevent;
#define glfwPollEvents() SDL_PollEvent(&sdlevent)
#define glfwWindowShouldClose(window) (sdlevent.type==SDL_QUIT ||  (sdlevent.type==SDL_WINDOWEVENT && sdlevent.window.event==SDL_WINDOWEVENT_CLOSE))
#endif

extern "C" {
#include "MGLContext.h"
}
#include "MGLRenderer.h"
#define SWAP_BUFFERS MGLswapBuffers((GLMContext) glfwGetWindowUserPointer(window));

//#define SWAP_BUFFERS glfwSwapBuffers(window);

// change main.c to main.cpp to use glm...
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
#include <glm/gtc/matrix_transform.hpp>

//extern "C" int sizeForFormatType(GLenum format, GLenum type, GLenum internalformat);

#define GLSL(version, shader) "#version " #version "\n" #shader

#define DEF_PIXEL_FOR_TYPE(_type_) _type_ *pixel = (_type_ *)ptr

#define WR_NORM_PIXELR(_type_, _scale_, _r_) {*pixel++ = (_type_)(_r_*_scale_);}
#define WR_NORM_PIXELRG(_type_, _scale_, _r_, _g_) {*pixel++ = (_type_)(_r_*_scale_);*pixel++ = (_type_)(_g_*_scale_);}
#define WR_NORM_PIXELRGB(_type_, _scale_, _r_, _g_, _b_) {*pixel++ = (_type_)(_r_*_scale_);*pixel++ = (_type_)(_g_*_scale_);*pixel++ = (_type_)(_b_*_scale_);}
#define WR_NORM_PIXELRGBA(_type_, _scale_, _r_, _g_, _b_, _a_) {*pixel++ = (_type_)(_r_*_scale_);*pixel++ = (_type_)(_g_*_scale_);*pixel++ = (_type_)(_b_*_scale_);*pixel++ = (_type_)(_a_*_scale_);}

#define WR_NORM_PIXEL_FORMAT(_format_, _type_, _scale_) \
{  DEF_PIXEL_FOR_TYPE(_type_); \
switch(_format_) {  \
case GL_RED: WR_NORM_PIXELR(_type_, _scale_, r); break; \
case GL_RG:  WR_NORM_PIXELRG(_type_, _scale_, r, g); break; \
case GL_RGB:  WR_NORM_PIXELRGB(_type_, _scale_, r, g, b); break; \
case GL_RGBA:  WR_NORM_PIXELRGBA(_type_, _scale_, r, g, b, a); break; \
default: assert(0); } }

#define WR_PIXELR(_type_, _r_, _scale_) {*pixel++ = (_type_)_r_;}
#define WR_PIXELRG(_type_, _r_, _g_, _scale_) {*pixel++ = (_type_)_r_;*pixel++ = (_type_)_g_;}
#define WR_PIXELRGB(_type_, _r_, _g_, _b_, _scale_) {*pixel++ = (_type_)_r_;*pixel++ = (_type_)_g_;*pixel++ = (_type_)_b_;}
#define WR_PIXELRGBA(_type_, _r_, _g_, _b_, _a_, _scale_) {*pixel++ = (_type_)_r_;*pixel++ = (_type_)_g_;*pixel++ = (_type_)_b_;*pixel++ = (_type_)_a_;}

#define WR_PIXEL_FORMAT(_format_, _type_, _scale_) \
{  DEF_PIXEL_FOR_TYPE(_type_); \
switch(_format_) {  \
case GL_RED: WR_PIXELR(int8_t, r, _scale_); break; \
case GL_RG:  WR_PIXELRG(int8_t, r, g, _scale_); break; \
case GL_RGB:  WR_PIXELRGB(int8_t, r, g, b, _scale_); break; \
case GL_RGBA:  WR_PIXELRGBA(int8_t, r, g, b, a, _scale_); break; \
default: assert(0); } }

void write_pixel(GLenum format, GLenum type, void *ptr, float r, float g, float b, float a)
{
    r = glm::clamp(r, 0.0f, 1.0f);
    g = glm::clamp(g, 0.0f, 1.0f);
    b = glm::clamp(b, 0.0f, 1.0f);
    a = glm::clamp(a, 0.0f, 1.0f);

    assert(r <= 1.0f);
    assert(g <= 1.0f);
    assert(b <= 1.0f);
    assert(a <= 1.0f);

    assert(r >= 0.0f);
    assert(g >= 0.0f);
    assert(b >= 0.0f);
    assert(a >= 0.0f);

    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        {
            WR_NORM_PIXEL_FORMAT(format, uint8_t, 255.0);
            break;
        }

        case GL_BYTE:
        {
            WR_NORM_PIXEL_FORMAT(format, int8_t, 127.0);
            break;
        }

        case GL_UNSIGNED_SHORT:
        {
            WR_NORM_PIXEL_FORMAT(format, uint16_t, (float)(2^16-1));
            break;
        }

        case GL_SHORT:
        {
            WR_NORM_PIXEL_FORMAT(format, int16_t, (float)(2^15-1));
            break;
        }

        case GL_UNSIGNED_INT:
        {
            WR_NORM_PIXEL_FORMAT(format, uint32_t, (float)(2^32-1));
            break;
        }

        case GL_INT:
        {
            WR_NORM_PIXEL_FORMAT(format, int32_t, (float)(2^31-1));
            break;
        }

        case GL_FLOAT:
        {
            WR_PIXEL_FORMAT(format, float, 1.0f);
            break;
        }

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            assert(0);

        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            assert(0);
            break;

        case GL_UNSIGNED_INT_8_8_8_8:
            WR_NORM_PIXEL_FORMAT(format, uint8_t, 255.0f);
            break;

        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            assert(0);
            break;

        default:
            assert(0);
    }
}

typedef struct RGBA_Pixel_t {
    uint8_t r,g,b,a;
} RGBA_Pixel;

void *gen3DTexturePixels(GLenum format, GLenum type, GLuint repeat, GLuint width, GLuint height, GLint depth)
{
    GLuint  pixel_size;
    size_t  buffer_size;
    void    *buffer;
    RGBA_Pixel *ptr;

    assert(format == GL_RGBA);
    assert(type == GL_UNSIGNED_BYTE);

    pixel_size = sizeForFormatType(format, type);//, 0);

    buffer_size = pixel_size * width;
    buffer_size *= height;
    buffer_size *= depth;

    // Allocate directly from VM because... 3d textures can be big
    kern_return_t err;
    vm_address_t buffer_data;
    err = vm_allocate((vm_map_t) mach_task_self(),
                      (vm_address_t*) &buffer_data,
                      buffer_size,
                      VM_FLAGS_ANYWHERE);
    assert(err == 0);
    assert(buffer_data);

    buffer = (void *)buffer_data;

    ptr = (RGBA_Pixel *)buffer;

    float r,g,b;
    float dr,dg,db;

    dr = 1.0/width;
    dg = 1.0/height;
    db = 1.0/depth;

    r = 0;
    g = 0;
    b = 0;

    for(int z=0; z<depth; z++)
    {
        for(int y=0; y<height; y++)
        {
            for(int x=0; x<width; x++)
            {
                ptr->r = r * 255;
                ptr->g = g * 255;
                ptr->b = b * 255;
                ptr->a = 255;

                ptr++;

                r += dr;
            }

            r = 0;
            g += dg;
        }

        g = 0;
        b += db;
    }

    return buffer;
}

void HSVtoRGB(float H, float S,float V, float *r, float *g, float *b)
{
    if(H>360 || H<0 || S>100 || S<0 || V>100 || V<0)
    {
        return;
    }

    float s = S/100;
    float v = V/100;
    float C = s*v;
    float X = C*(1-abs(fmod(H/60.0, 2)-1));

    if(H >= 0 && H < 60)
    {
        *r = C;*g = X;*b = 0;
    }
    else if(H >= 60 && H < 120)
    {
        *r = X;*g = C;*b = 0;
    }
    else if(H >= 120 && H < 180)
    {
        *r = 0;*g = C;*b = X;
    }
    else if(H >= 180 && H < 240)
    {
        *r = 0;*g = X;*b = C;
    }
    else if(H >= 240 && H < 300)
    {
        *r = X;*g = 0;*b = C;
    }
    else
    {
        *r = C;*g = 0;*b = X;
    }
}

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

float clamp(float a, float min, float max)
{
    a = MAX(a, min);
    a = MIN(a, max);

    return a;
}

void *genTexturePixels(GLenum format, GLenum type, GLuint repeat, GLuint width, GLuint height, GLint depth=1, GLboolean is_array=false)
{
    GLuint  pixel_size;
    size_t  buffer_size;
    void    *buffer;
    uint8_t *ptr;

    pixel_size = sizeForFormatType(format, type);//, 0);

    buffer_size = pixel_size * width;

    buffer_size *= height;

    if (depth)
        buffer_size *= depth;

    buffer = malloc(buffer_size);
    assert(buffer);

    ptr = (uint8_t *)buffer;

    float r, g, b;
    float dr, dg, db;

    if (is_array)
    {
        r = 0.0;
        g = 0.0;
        b = 0.0;
    }
    else
    {
        r = 1.0;
        g = 1.0;
        b = 1.0;
    }

    dr = 1.0 / width;

    if (height > 1)
        dg = 1.0 / height;
    else
        dg = 0.0;

    if (depth > 1)
        db = 1.0 / depth;
    else
        db = 0.0;

    for(int z=0; z<depth; z++)
    {
        for(int y=0; y<height; y++)
        {
            for(int x=0; x<width; x++)
            {
                r = clamp(r, 0.0f, 1.0f);
                g = clamp(g, 0.0f, 1.0f);
                b = clamp(b, 0.0f, 1.0f);

                if (y & repeat)
                {
                    if (x & repeat)
                    {
                        write_pixel(format, type, ptr, r, g, b, 1.0);
                    }
                    else
                    {
                        write_pixel(format, type, ptr, 0.0, 0.0, 0.0, 1.0);
                    }
                }
                else
                {
                    if ((x & repeat) == 0)
                    {
                        write_pixel(format, type, ptr, r, g, b, 1.0);
                    }
                    else
                    {
                        write_pixel(format, type, ptr, 0.0, 0.0, 0.0, 1.0);
                    }
                }

                if (is_array)
                {
                    r += dr;
                }

                //GLuint *hexptr;
                //hexptr = (GLuint *)ptr;
                //printf("0x%X\n",*hexptr);

                ptr = ptr + pixel_size;
            }

            if (is_array)
            {
                r = 0.0;
                g += dg;
            }
        }

        if (is_array)
        {
            g = 0.0;
            b += db;
        }
    }

    return buffer;
}

GLuint bindDataToVBO(GLenum target, size_t size, void *ptr, GLenum usage)
{
    GLuint vbo = 0;

    glGenBuffers(1, &vbo);
    glBindBuffer(target, vbo);
    glBufferData(target, size, ptr, usage);
    glBindBuffer(target, 0);

    return vbo;
}

void bindVBOToAttrib(GLuint vbo, GLuint index, GLint size, GLenum type, GLboolean normalized=false, GLsizei stride=0, const void *ptr=NULL)
{
    glEnableVertexAttribArray(index);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(index, size, type, normalized, stride, ptr);
}

GLuint bindVAO(GLuint vao=0)
{
    if(vao)
    {
        glBindVertexArray(vao);
    }
    else
    {
        glCreateVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }

    return vao;
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

GLuint createTexture(GLenum target, GLsizei width, GLsizei height=1, GLsizei depth=1, const void *pixels=NULL, GLint level=0, GLint internalformat=GL_RGBA8, GLenum format=GL_RGBA, GLenum type=GL_UNSIGNED_BYTE)
{
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    switch(target)
    {
        case GL_TEXTURE_1D:
            glTexImage1D(target, level, internalformat, width, 0, format, type, pixels);
            break;

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


int test_clear(GLFWwindow* window, int width, int height)
{
    glClearColor(0.5, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    SWAP_BUFFERS;

    return 0;
}

int test_draw_arrays(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0;

    const char* vertex_shader =
    "#version 450 core\n"
    "layout(location = 0) in vec3 position;"
    "void main() {"
    "  gl_Position = vec4(position, 1.0);"
    "}";

    const char* fragment_shader =
    "#version 450 core\n"
    "layout(location = 0) out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    SWAP_BUFFERS;

    return 0;
}

int test_draw_elements(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, elem_vbo = 0;

    const char* vertex_shader =
    "#version 450 core\n"
    "layout(location = 0) in vec3 position;"
    "void main() {"
    "  gl_Position = vec4(position, 1.0);"
    "}";

    const char* fragment_shader =
    "#version 450 core\n"
    "layout(location = 0) out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

    GLfloat points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    GLuint indices[] = {0, 1, 2};

    glGenBuffers(1, &elem_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    //glVertexArrayElementBuffer(vao, elem_vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_vbo);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    SWAP_BUFFERS;

    return 0;
}

int test_draw_range_elements(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, elem_vbo = 0;

    const char* vertex_shader =
    "#version 450 core\n"
    "layout(location = 0) in vec3 position;"
    "void main() {"
    "  gl_Position = vec4(position, 1.0);"
    "}";

    const char* fragment_shader =
    "#version 450 core\n"
    "layout(location = 0) out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

    GLfloat points[] = {
        0.0f,  0.5f,  0.0f,
        0.0f,  0.5f,  0.0f,
        0.0f,  0.5f,  0.0f,
        0.0f,  0.5f,  0.0f, // start @ 3
        0.5f, -0.5f,  0.0f,
       -0.5f, -0.5f,  0.0f, // end @ 5
        0.0f,  0.5f,  0.0f,
        0.0f,  0.5f,  0.0f,
        0.0f,  0.5f,  0.0f,
    };

    GLuint indices[] = {3, 4, 5};

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &elem_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glVertexArrayElementBuffer(vao, elem_vbo);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)

    glDrawRangeElements(GL_TRIANGLES, 3, 5, 3, GL_UNSIGNED_INT, 0);

    SWAP_BUFFERS;

    return 0;
}

int test_draw_arrays_instanced(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0;

    const char* vertex_shader =
    "#version 450\n"
    "layout(location = 0) in vec3 position;"
    "layout(location = 0) out vec4 color;"
    "void main() {"
    "  vec3 instance_position = position;"
    "  if (gl_InstanceID == 1) instance_position.x += 0.1;"
    "  else if (gl_InstanceID == 2) instance_position.x += 0.2;"
    "  else if (gl_InstanceID == 3) instance_position.x += 0.3;"
    "  if (gl_InstanceID == 1) color = vec4(1.0, 0.0, 0.0, 1.0);"
    "  else if (gl_InstanceID == 2) color = vec4(0.0, 1.0, 0.0, 1.0);"
    "  else if (gl_InstanceID == 3) color = vec4(0.0, 0.0, 1.0, 1.0);"
    "  else color = vec4(0.5, 0.0, 0.5, 1.0);"
    "  gl_Position = vec4(instance_position, 1.0);"
    "}";

    const char* fragment_shader =
    "#version 450\n"
    "layout(location = 0) in vec4 color;"
    "layout(location = 0) out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = color;"
    "}";

    float points[] = {
       0.0f,  0.0f,  0.0f,
       0.5f,  0.0f,  0.0f,
       0.0f, -0.5f,  0.0f
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 4);

    SWAP_BUFFERS;

    return 0;
}

int test_uniform_buffer(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
         layout(location = 0) in vec3 position;

         layout(binding = 0) uniform matrices
         {
             mat4 rotMatrix;
         };

         void main() {
            gl_Position = rotMatrix * vec4(position, 1.0);
         }
    );

    const char* fragment_shader =
    GLSL(450 core,
         layout(location = 0) out vec4 frag_colour;

         void main() {
            frag_colour = vec4(0.5, 0.0, 0.5, 1.0);
        }
    );

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    mat4  rotZ = glm::identity<mat4>();
    float angle = M_1_PI / 6;
    rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

    ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(mat4),&rotZ[0][0], GL_STATIC_DRAW);

    bindVBOToAttrib(vbo, 0, 3, GL_FLOAT);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    SWAP_BUFFERS;

    return 0;
}

int test_1D_textures(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in float in_texcords;
        layout(location = 0) out float out_texcoords;
        layout(binding = 0) uniform matrices
        {
            mat4 rotZ;
        };
        void main() {
            gl_Position = rotZ * vec4(position, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in float in_texcords;
        layout(location = 0) out vec4 frag_colour;

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, vec2(in_texcords, 1.0f));

            frag_colour = tex_color;
        }
    );

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f,
       0.0f,  0.5f,  0.0f,
    };

    float texcoords[] = {
        0.0f,
        0.5f,
        1.0f,
    };

    //
    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), points, GL_STATIC_DRAW);
    tex_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 3 * sizeof(float), texcoords, GL_STATIC_DRAW);

    mat4  rotZ = glm::identity<mat4>();
    float angle = M_1_PI / 6;
    rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

    mat_ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(mat4),&rotZ[0][0], GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bindVBOToAttrib(vbo, 0, 3, GL_FLOAT);
    bindVBOToAttrib(tex_vbo, 1, 1, GL_FLOAT);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

    // generate 1d texture
    GLuint tex;
    tex = createTexture(GL_TEXTURE_1D, 256, 1, 1, genTexturePixels(GL_RGBA, GL_FLOAT, 0x10, 256, 1));
    glBindTexture(GL_TEXTURE_1D, tex);

    glViewport(0, 0, width, height);

    glDrawBuffer(GL_FRONT);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    // crashes metal
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_LINE_STRIP, 0, 4);

        SWAP_BUFFERS;

        angle += (M_PI / 180);

        rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        bufferSubData(GL_UNIFORM_BUFFER, mat_ubo, sizeof(mat4), &rotZ[0][0]);

        glfwPollEvents();
    }
    
    return 0;
}

int test_2D_textures(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, col_vbo = 0, tex_vbo = 0, mat_ubo = 0, scale_ubo = 0, col_att_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 in_color;
        layout(location = 2) in vec2 in_texcords;

        layout(location = 0) out vec4 out_color;
        layout(location = 1) out vec2 out_texcoords;

        layout(binding = 0) uniform matrices
        {
            mat4 rotMatrix;
        };

        layout(binding = 1) uniform scale
        {
               float pos_scale;
        };

        void main() {
            gl_Position = rotMatrix * vec4(position, 1.0);
            out_color = vec4(in_color, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec4 in_color;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        layout(binding = 2) uniform color_att
        {
            float att;
        };

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            // frag_colour = in_color * att;
            frag_colour = in_color * att * tex_color;
            //frag_colour = tex_color;
        }
    );

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    float color[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    float texcoords[] = {
        0.5f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &col_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), color, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &tex_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), texcoords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mat4  rotZ = glm::identity<mat4>();

    float angle = M_1_PI / 6;

    rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

    glGenBuffers(1, &mat_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, mat_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4),&rotZ[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    float scale = -1.0;
    glGenBuffers(1, &scale_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, scale_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float), &scale, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    float att = 1.0;
    glGenBuffers(1, &col_att_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, col_att_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float), &att, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    // clear currently bound buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    GLuint scale_loc = glGetUniformBlockIndex(shader_program, "scale");
    assert(scale_loc == 1);

    GLuint color_att_loc = glGetUniformBlockIndex(shader_program, "color_att");
    assert(color_att_loc == 2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, scale_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, col_att_ubo);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 genTexturePixels(GL_RGBA, GL_UNSIGNED_BYTE, 0x10, 256,256));

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.2, 0.2, 0.2, 0.0);

    float att_delta = 0.01;

    while (!glfwWindowShouldClose(window))
    {
        //GLsync  sync;

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        //sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        SWAP_BUFFERS;

        angle += (M_PI / 180);

        rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        //glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);

        glBindBuffer(GL_UNIFORM_BUFFER, mat_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), &rotZ[0][0]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        att += att_delta;
        if (att > 1.0)
        {
            att = 1.0;
            att_delta *= -1.0;
        }
        else if (att < 0.0)
        {
            att = 0.0;
            att_delta *= -1.0;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, col_att_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &att);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glfwPollEvents();
    }

    return 0;
}

int test_3D_textures(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 in_texcords;

        layout(location = 0) out vec3 out_texcoords;

        layout(binding = 0) uniform matrices
        {
            mat4 mvp;
        };

        void main() {
            gl_Position = mvp * vec4(position, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler3D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            frag_colour = tex_color;
            //frag_colour = vec4(1.0,1.0,1.0, 1.0);
            //frag_colour = vec4(in_texcords,1.0);
        }
    );

    float points[] = {
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f
    };

    float texcoords[] = {
         0.0f, 0.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         0.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 0.0f,
         0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 1.0f,
         0.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 0.0f,
         0.0f, 0.0f, 0.0f,
         0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 1.0f,
         0.0f, 0.0f, 1.0f,
         0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 1.0f,
         0.0f, 0.0f, 1.0f,
         1.0f, 0.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 0.0f,
         1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 0.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
         1.0f, 1.0f, 1.0f,
         0.0f, 1.0f, 0.0f,
         0.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         0.0f, 1.0f, 1.0f,
         1.0f, 0.0f, 1.0f
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER, 36 * 3 * sizeof(float), points, GL_MAP_WRITE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glCreateBuffers(1, &tex_vbo);
    glNamedBufferStorage(tex_vbo, 36 * 3 * sizeof(float), texcoords, GL_MAP_WRITE_BIT);

    float angle = M_1_PI / 6;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(60.0f), (float) width / (float)height, 0.1f, 100.0f);

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

#define _A 6
    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(_A/4,_A/2,_A), // Camera is at (10,10,10), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

    glCreateBuffers(1, &mat_ubo);
    glNamedBufferStorage(mat_ubo, sizeof(mat4), &mvp[0][0], GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 3 * sizeof(float));
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glEnableVertexArrayAttrib(vao, 0);

    glVertexArrayVertexBuffer(vao, 1, tex_vbo, 0, 3 * sizeof(float));
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 1);
    glEnableVertexArrayAttrib(vao, 1);

    glBindVertexArray(vao);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

#define _3d_size 128
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, _3d_size, _3d_size, _3d_size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 gen3DTexturePixels(GL_RGBA, GL_UNSIGNED_BYTE, 0x10, _3d_size, _3d_size, _3d_size));

    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glViewport(0, 0, width, height);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        SWAP_BUFFERS;

        angle += (M_PI / 360);

        Model = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(1, 0, 0));
        Model = glm::rotate(Model, angle * 2, glm::vec3(0, 1, 0));
        Model = glm::rotate(Model, angle * 4, glm::vec3(0, 0, 1));

        mvp = Projection * View * Model;

        glNamedBufferSubData(mat_ubo, 0, sizeof(mat4), &mvp[0][0]);

        glfwPollEvents();
    }

    return 0;
}

int test_2D_array_textures(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, ebo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec2 out_texcoords;
        layout(location = 1) flat out int out_instanceID;

        layout(binding = 0) uniform matrices
        {
            mat4 mvp;
        };

        void main() {
            float z;

            z = gl_InstanceID * -0.5;

            gl_Position = mvp * vec4(in_position, z, 1.0);
            out_texcoords = in_texcords;
            out_instanceID = gl_InstanceID;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_texcords;
        layout(location = 1) flat in int in_instanceID;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler2DArray image;

        void main() {
            vec4 tex_color = texture(image, vec3(in_texcords, in_instanceID));

            frag_colour = tex_color;
        }
    );

    float points[] = {
        -1.0f,-1.0f,
        -1.0f, 1.0f,
         1.0f, 1.0f,
         1.0f,-1.0f,
    };

    unsigned short elements[] = {
        0, 1, 3, 2
    };

    float texcoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(points), points, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(elements), elements, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &tex_vbo);
    glNamedBufferStorage(tex_vbo, sizeof(points), texcoords, GL_MAP_WRITE_BIT);

    float angle = M_1_PI / 6;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(60.0f), (float) width / (float)height, 0.1f, 100.0f);

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

#undef _A
#define _A 4
    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(_A/4,_A/2,_A), // Camera is at (10,10,10), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

    glCreateBuffers(1, &mat_ubo);
    glNamedBufferStorage(mat_ubo, sizeof(mat4), &mvp[0][0], GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glEnableVertexArrayAttrib(vao, 0);

    glVertexArrayVertexBuffer(vao, 1, tex_vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 1);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayElementBuffer(vao, ebo);

    glBindVertexArray(vao);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

#define _2d_array_size 128
#define _2d_array_depth 8

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

    // test tex storage path
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8,_2d_array_size, _2d_array_size, _2d_array_depth);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // test subimage path
    GLuint *pixels;
    pixels = (GLuint *)genTexturePixels(GL_RGBA, GL_UNSIGNED_BYTE, 0x08, _2d_array_size, _2d_array_size, _2d_array_depth, true);

    size_t image_size;
    image_size = _2d_array_size * _2d_array_size; // image size in pixels

    // and test pbo unpack path
    GLuint pbo;
    glCreateBuffers(1, &pbo);
    glNamedBufferStorage(pbo, image_size * _2d_array_depth * sizeof(uint32_t), pixels, GL_MAP_WRITE_BIT);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

    for(int i=0; i<_2d_array_depth; i++)
    {
        size_t offset;

        offset = i * image_size * 4;

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _2d_array_size, _2d_array_size, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)offset);
    }

    glViewport(0, 0, width, height);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0, _2d_array_depth);

        SWAP_BUFFERS;

        angle += (M_PI / 360);

        Model = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        mvp = Projection * View * Model;

        glNamedBufferSubData(mat_ubo, 0, sizeof(mat4), &mvp[0][0]);

        glfwPollEvents();
    }

    return 0;
}

int test_textures(GLFWwindow* window, int width, int height, int mipmap, int use_gen_mipmap, GLuint anisotropic_level=0, GLuint min_filter=GL_NEAREST, GLuint mag_filter=GL_NEAREST)
{
    GLuint vbo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec2 out_texcoords;

        layout(binding = 0) uniform matrices
        {
            mat4 mvp;
        };

        void main() {
            gl_Position = mvp * vec4(position, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            frag_colour = tex_color;
        }
    );

#undef _A
#define _A  10

    float points[] = {
        -_A, 0, -_A,
        -_A, 0,  _A,
         _A, 0, -_A,
         _A, 0,  _A
    };

    float texcoords[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1
    };

    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), points, GL_STATIC_DRAW);
    tex_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), texcoords, GL_STATIC_DRAW);

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(_A/4,_A/2,_A), // Camera is at (10,10,10), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

    mat_ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(mat4),&mvp[0][0], GL_STATIC_DRAW);

    GLuint vao = 0;
    vao = bindVAO();

    bindVBOToAttrib(vbo, 0, 3, GL_FLOAT);
    bindVBOToAttrib(tex_vbo, 1, 2, GL_FLOAT);

    // clear currently bound buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    GLuint texsize;

    texsize = 1024;

    if (mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

        if (anisotropic_level)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisotropic_level);
        }

        if (use_gen_mipmap)
        {

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texsize, texsize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         genTexturePixels(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0x8, texsize, texsize));

            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            GLuint size;

            size = texsize;

            for(int i=0; size>0; i++)
            {
                glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             genTexturePixels(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0x8 >> i, size, size));

                size >>= 1;
            }

        }
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texsize, texsize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     genTexturePixels(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0x8, texsize, texsize));
    }

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.4, 0.4, 0.4, 0.0);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SWAP_BUFFERS;

        glBindBuffer(GL_UNIFORM_BUFFER, mat_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), &mvp[0][0]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glfwPollEvents();
    }

    return 0;
}

GLuint draw_to_framebuffer(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0;

    float points[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    const char* vertex_shader =
    GLSL(450 core,
         layout(location = 0) in vec2 position;
         layout(location = 0) out vec2 texCoord;

         void main(void)
         {
            gl_Position = vec4( position.xy, 0.0, 1.0 );
            gl_Position = sign( gl_Position );

            texCoord = (vec2( gl_Position.x, gl_Position.y )
                      + vec2( 1.0 ) ) / vec2( 2.0 );
         }
    );

    const char* fragment_shader =
    GLSL(450 core,
         layout(location = 0) in vec2 texCoord;
         layout(location = 0) out vec4 frag_colour;

         void main(void)
         {
             ivec2 size = ivec2(16,16);
             float total = floor(texCoord.x*float(size.x)) +
                           floor(texCoord.y*float(size.y));
             bool isEven = mod(total,2.0)==0.0;

             vec4 col1 = vec4(0.0,0.0,0.0,1.0);
             vec4 col2 = vec4(1.0,1.0,1.0,1.0);

             frag_colour = (isEven)? col1:col2;
         }
    );

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bindVBOToAttrib(vbo, 0, 2, GL_FLOAT);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint tex_attachment;
    tex_attachment = createTexture(GL_TEXTURE_2D, 256, 256);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_attachment, 0);

    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            printf("good job\n");
            break;

        default:
            assert(0);
    }

    glViewport(0, 0, 256, 256);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glFinish();

    glDrawBuffer(GL_FRONT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    return tex_attachment;
}

int test_framebuffer(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, col_vbo = 0, tex_vbo = 0, mat_ubo = 0, scale_ubo = 0, col_att_ubo = 0;

    const char* vertex_texture_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 in_color;
        layout(location = 2) in vec2 in_texcords;

        layout(location = 0) out vec4 out_color;
        layout(location = 1) out vec2 out_texcoords;

        layout(binding = 0) uniform matrices
        {
            mat4 rotMatrix;
        };

        layout(binding = 1) uniform scale
        {
               float pos_scale;
        };

        void main() {
            gl_Position = rotMatrix * vec4(position, 1.0);
            out_color = vec4(in_color, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_texture_shader =
    GLSL(450 core,
        layout(location = 0) in vec4 in_color;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        layout(binding = 2) uniform color_att
        {
            float att;
        };

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            //frag_colour = in_color * att * tex_color;
            //frag_colour = tex_color;
            frag_colour = in_color;
        }
    );

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    float color[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    float texcoords[] = {
        0.5f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    // draw to a texture using an FBO
    GLuint tex_attachment;
    tex_attachment = draw_to_framebuffer(window, width, height);
    glBindTexture(GL_TEXTURE_2D, tex_attachment);

    //
    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    col_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), color, GL_STATIC_DRAW);
    tex_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 6 * sizeof(float), texcoords, GL_STATIC_DRAW);

    mat4  rotZ = glm::identity<mat4>();
    float angle = M_1_PI / 6;
    rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

    float scale = -1.0;
    float att = 1.0;

    mat_ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(mat4),&rotZ[0][0], GL_STATIC_DRAW);
    scale_ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(float), &scale, GL_STATIC_DRAW);
    col_att_ubo = bindDataToVBO(GL_UNIFORM_BUFFER, sizeof(float), &att, GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bindVBOToAttrib(vbo, 0, 3, GL_FLOAT);
    bindVBOToAttrib(col_vbo, 1, 3, GL_FLOAT);
    bindVBOToAttrib(tex_vbo, 2, 2, GL_FLOAT);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_texture_shader, GL_FRAGMENT_SHADER, fragment_texture_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    GLuint scale_loc = glGetUniformBlockIndex(shader_program, "scale");
    assert(scale_loc == 1);

    GLuint color_att_loc = glGetUniformBlockIndex(shader_program, "color_att");
    assert(color_att_loc == 2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, scale_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, col_att_ubo);

    glViewport(0, 0, width, height);

    glUseProgram(shader_program);

    glDrawBuffer(GL_FRONT);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    float att_delta = 0.01;

    // bind and render texture to framebuffer texture
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        SWAP_BUFFERS;

        angle += (M_PI / 180);

        rotZ = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        bufferSubData(GL_UNIFORM_BUFFER, mat_ubo, sizeof(mat4), &rotZ[0][0]);

        att += att_delta;
        if (att > 1.0)
        {
            att = 1.0;
            att_delta *= -1.0;
        }
        else if (att < 0.0)
        {
            att = 0.0;
            att_delta *= -1.0;
        }

        bufferSubData(GL_UNIFORM_BUFFER, col_att_ubo, sizeof(float), &att);

        glfwPollEvents();
    }

    return 0;
}

int test_readpixels(GLFWwindow* window, int width, int height)
{
    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 in_color;
        layout(location = 2) in vec2 in_texcords;

        layout(location = 0) out vec4 out_color;
        layout(location = 1) out vec2 out_texcoords;

        void main() {
            gl_Position = vec4(position, 1.0);
            out_color = vec4(in_color, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec4 in_color;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            frag_colour = in_color * tex_color;
        }
    );

    GLuint vbo = 0, col_vbo = 0, tex_vbo = 0;

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

    float color[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    float texcoords[] = {
        0.5f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    col_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 9 * sizeof(float), color, GL_STATIC_DRAW);
    tex_vbo = bindDataToVBO(GL_ARRAY_BUFFER, 6 * sizeof(float), texcoords, GL_STATIC_DRAW);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bindVBOToAttrib(vbo, 0, 3, GL_FLOAT);
    bindVBOToAttrib(col_vbo, 1, 3, GL_FLOAT);
    bindVBOToAttrib(tex_vbo, 2, 2, GL_FLOAT);

    GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint tex;

    tex = createTexture(GL_TEXTURE_2D, 256, 256, 0, genTexturePixels(GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0x10, 256, 256));
    glBindTexture(GL_TEXTURE_2D, tex);

    glViewport(0, 0, width, height);

    glBindVertexArray(vao);

    glUseProgram(shader_program);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    uint8_t *buf;

    buf = (uint8_t *)malloc(256*256*4);

    glReadBuffer(GL_FRONT);

    glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buf);

    glFlush();

    return 0;
}

const char* compute_shader1 =
GLSL(450 core,
     layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

     layout(rgba8, binding = 0) uniform image2D img_output_0;
     layout(rgba8, binding = 1) uniform image2D img_output_1;
     layout(rgba8, binding = 2) uniform image2D img_output_2;
     layout(rgba8, binding = 3) uniform image2D img_output_3;
     layout(rgba8, binding = 4) uniform image2D img_output_4;
     layout(rgba8, binding = 5) uniform image2D img_output_5;

     layout(binding = 6) buffer Cells { int cells[]; };

     layout(binding = 7) uniform Params
     {
        int cell_end_y;
     };

     layout(binding = 8) buffer AtomicBuffer {atomic_int count};

     void main()
     {
        int gid_x;
        int gid_y;
        int gid_z;

        ivec2 image_size;

        gid_x = gl_GlobalInvocationID.x;
        gid_y = gl_GlobalInvocationID.y;
        gid_z = gl_GlobalInvocationID.z;

        image_size = imageSize(img_output_0);

        // cell data is fed into face 0
        if (gid_z == 0)
        {
            if (gid_y == 0)
            {
                // serial copy
                for(int y=image_size.y-1; y>0; y--)
                {
                    vec4 pixel;

                    pixel = imageLoad(img_output_0, ivec2(gid_x, y));
                    imageStore(image_output_0, ivec2(gid_x, y+1), pixel);
                }

                // perform rule on y == 0
            }
            else if (gid_y >= cell_end_y)
            {
                // play game of life
                int count;
                vec4 n[9];
                vec4 pixel;

                count = 0;

                for(int y=-1; y<2; y++)
                {
                    for(int x=-1; x<2; x++)
                    {
                        n[y*3+x] = imageLoad(img_output_0, ivec(gid_x + x, gid_y + y));
                        if(n[y*3+x].w)
                            count++;
                    }
                }

                if (count == 3)
                {
                    pixel = vec4(1.0,1.0,1.0,1.0);

                    imageStore(img_output_0, ivec(gid_x, gid_y), pixel);
                }
                else if (count == 2)
                {
                    pixel = imageLoad(img_output_0, ivec(gid_x, gid_y));

                    if (pixel.w != 1.0)
                    {
                        pixel = vec4(0.0,0.0,0.0,0.0);
                    }

                    imageStore(img_output_0, ivec(gid_x, gid_y), pixel);
                }
                else
                {
                    pixel = vec4(0.0,0.0,0.0,0.0);

                    imageStore(img_output_0, ivec(gid_x, gid_y), pixel);
                }
            }
        }
        else
        {
        }
     }
);

int test_compute_shader(GLFWwindow* window, int width, int height)
{
    const char* compute_shader =
    GLSL(450 core,
         layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

         // image bindings
         layout(rgba8, binding = 0) uniform image2D img_output_0;
         layout(rgba8, binding = 0) uniform image2D img_output_1;

         // buffer binding 0
         layout(binding = 0) buffer Cells { int cells[]; };

         // uniform binding 0
         layout(binding = 0) uniform Params
         {
            int cell_end_y;
         };

         layout(binding = 0) uniform atomic_uint count;

         uint hash( uint x )
         {
             x += ( x << 10u );
             x ^= ( x >>  6u );
             x += ( x <<  3u );
             x ^= ( x >> 11u );
             x += ( x << 15u );
             return x;
         }

         float random( float f )
         {
             const uint mantissaMask = 0x007FFFFFu;
             const uint one          = 0x3F800000u;

             uint h = hash( floatBitsToUint( f ) );
             h &= mantissaMask;
             h |= one;

             float  r2 = uintBitsToFloat( h );
             return r2 - 1.0;
         }

         void main()
         {
            // play game of life (like)
            uint count;
            vec4 n[9];
            vec4 pixel;

            uint gid_x;
            uint gid_y;
            uint gid_z;

            ivec2 image_size;

            gid_x = gl_GlobalInvocationID.x;
            gid_y = gl_GlobalInvocationID.y;
            gid_z = gl_GlobalInvocationID.z;

            image_size = imageSize(img_output_0);

            count = 0;

            if (gid_z == 0)
            {
                for(int y=0; y<3; y++)
                {
                    for(int x=0; x<3; x++)
                    {
                        n[y * 3 + x] = imageLoad(img_output_0, ivec2(gid_x + x - 1, gid_y + y - 1));

                        if ((x != 1) && (y != 1))
                        {
                            if(n[y * 3 + x].w == 1.0)
                                count++;
                        }
                    }
                }

                if (count == 3)
                {
                    pixel = vec4(n[4].w, n[4].w, n[4].w, 1.0);
                }
                else if (count == 2)
                {
                    pixel = imageLoad(img_output_0, ivec2(gid_x, gid_y));

                    if (pixel.w != 1.0)
                    {
                        pixel = vec4(0.0,0.0,0.0,0.0);
                    }
                }
                else if (count == 0)
                {
                    float r;
                    float delta;

                    delta = 1.0 / 10.0;

                    r = random(gid_x) * random(gid_y) * random(delta);

                    if (r < 0.25)
                        n[4].w = n[4].w + r;

                    pixel = vec4(n[4].w, n[4].w, n[4].w, n[4].w);
                }
                else
                {
                    pixel = vec4(n[4].w / 4.0);
                }

                imageStore(img_output_0, ivec2(gid_x, gid_y), pixel);
            }
            else
            {
                //pixel = imageLoad(img_output_1, ivec2(gid_x, gid_y));
                //imageStore(img_output_0, ivec2(gid_x, gid_y), pixel);
            }
        }
    );

    GLuint compute_program;
    compute_program = compileGLSLProgram(1, GL_COMPUTE_SHADER, compute_shader);
    assert(compute_program);

    GLuint sbo;
    glCreateBuffers(1, &sbo);
    glNamedBufferStorage(sbo, 1024 * sizeof(float), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_CLIENT_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sbo);

    GLuint pbo;
    GLuint cell_end_y;
    cell_end_y = height / 2;
    glCreateBuffers(1, &pbo);
    glNamedBufferStorage(pbo, sizeof(GLuint), &cell_end_y, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_CLIENT_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, pbo);

    GLuint abo;
    glCreateBuffers(1, &abo);
    glNamedBufferStorage(abo, sizeof(GLuint), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_CLIENT_STORAGE_BIT);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, abo);

    GLuint sampler;
    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // dimensions of the image
    int tex_w = 256, tex_h = 256;
    GLuint textures[2];
    glCreateTextures(GL_TEXTURE_2D, 2, textures);
    for(int i=0; i<2; i++)
    {
        glTextureStorage2D(textures[i], 1, GL_RGBA8, tex_w, tex_h);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_UNSIGNED_BYTE,
                        genTexturePixels(GL_RGBA, GL_UNSIGNED_BYTE, 0x10, tex_w, tex_h));
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    }

    glUseProgram(compute_program);
    glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 2);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glFinish();

    GLuint vbo = 0, ebo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec2 out_texcoords;

        layout(binding = 0) uniform matrices
        {
            mat4 mvp;
        };

        void main() {
            gl_Position = mvp * vec4(position, 1.0);
            out_texcoords = in_texcords;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_texcords;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler2D image;

        void main() {
            vec4 tex_color = texture(image, in_texcords);

            frag_colour = tex_color;
            //frag_colour = vec4(1.0,1.0,1.0, 1.0);
            //frag_colour = vec4(in_texcords,1.0);
        }
    );

    float points[] = {
        -1.0f,-1.0f,
        -1.0f, 1.0f,
         1.0f, 1.0f,
         1.0f,-1.0f,
    };

    unsigned short elements[] = {
        0, 1, 3, 2
    };

    float texcoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(points), points, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(elements), elements, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &tex_vbo);
    glNamedBufferStorage(tex_vbo, sizeof(points), texcoords, GL_MAP_WRITE_BIT);

    float angle = M_1_PI / 6;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(60.0f), (float) width / (float)height, 0.1f, 100.0f);

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

#undef _A
#define _A 1
    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(_A,_A,_A), // Camera is at (10,10,10), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,0,1)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

    glCreateBuffers(1, &mat_ubo);
    glNamedBufferStorage(mat_ubo, sizeof(mat4), &mvp[0][0], GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glEnableVertexArrayAttrib(vao, 0);

    glVertexArrayElementBuffer(vao, ebo);

    glVertexArrayVertexBuffer(vao, 1, tex_vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 1);
    glEnableVertexArrayAttrib(vao, 1);

    glBindVertexArray(vao);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);

    glViewport(0, 0, width, height);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shader_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);
        // glBindTexture(GL_TEXTURE_2D, textures[0]);
        // glBindSampler(GL_TEXTURE0 + 1, sampler);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

        SWAP_BUFFERS;

        angle += (M_PI / 360) * 0.25;

        Model = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        mvp = Projection * View * Model;

        glNamedBufferSubData(mat_ubo, 0, sizeof(mat4), &mvp[0][0]);

        glFlush();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, pbo);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, abo);

        for(int i=0; i<2; i++)
        {
            glBindSampler(GL_TEXTURE0 + i, sampler);
            glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
        }
        glBindSampler(GL_TEXTURE0, 0);

        glUseProgram(compute_program);
        glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 2);

        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glFlush();

        glfwPollEvents();
    }

    return 0;
}

int test_2D_array_textures_perf_mon(GLFWwindow* window, int width, int height)
{
    GLuint vbo = 0, ebo = 0, tex_vbo = 0, mat_ubo = 0;

    const char* vertex_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in vec2 in_texcords;

        layout(location = 0) out vec2 out_texcoords;
        layout(location = 1) flat out int out_instanceID;

        layout(binding = 0) uniform matrices
        {
            mat4 mvp;
        };

        void main() {
            float z;

            z = gl_InstanceID * -0.5;

            gl_Position = mvp * vec4(in_position, z, 1.0);
            out_texcoords = in_texcords;
            out_instanceID = gl_InstanceID;
        }
    );

    const char* fragment_shader =
    GLSL(450 core,
        layout(location = 0) in vec2 in_texcords;
        layout(location = 1) flat in int in_instanceID;

        layout(location = 0) out vec4 frag_colour;

        uniform sampler2DArray image;

        void main() {
            vec4 tex_color = texture(image, vec3(in_texcords, in_instanceID));

            frag_colour = tex_color;
        }
    );

    float points[] = {
        -1.0f,-1.0f,
        -1.0f, 1.0f,
         1.0f, 1.0f,
         1.0f,-1.0f,
    };

    unsigned short elements[] = {
        0, 1, 3, 2
    };

    float texcoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    // generate a FBO
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint tex_attachment;
    tex_attachment = createTexture(GL_TEXTURE_2D, width, height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_attachment, 0);

    GLuint depth_attachment;
    glGenRenderbuffers(1, &depth_attachment);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_attachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_attachment);

    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            printf("good job\n");
            break;

        default:
            assert(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(points), points, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(elements), elements, GL_MAP_WRITE_BIT);

    glCreateBuffers(1, &tex_vbo);
    glNamedBufferStorage(tex_vbo, sizeof(points), texcoords, GL_MAP_WRITE_BIT);

    float angle = M_1_PI / 6;

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(60.0f), (float) width / (float)height, 0.1f, 100.0f);

    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

#undef _A
#define _A 4
    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(_A/4,_A/2,_A), // Camera is at (10,10,10), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

    glCreateBuffers(1, &mat_ubo);
    glNamedBufferStorage(mat_ubo, sizeof(mat4), &mvp[0][0], GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 0, 0);
    glEnableVertexArrayAttrib(vao, 0);

    glVertexArrayVertexBuffer(vao, 1, tex_vbo, 0, 2 * sizeof(float));
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 1);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayElementBuffer(vao, ebo);

    glBindVertexArray(vao);

    GLuint shader_program;
    shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
    glUseProgram(shader_program);

    GLuint matrices_loc = glGetUniformBlockIndex(shader_program, "matrices");
    assert(matrices_loc == 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mat_ubo);

#define _2d_array_size 128
#define _2d_array_depth 8

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

    // test tex storage path
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8,_2d_array_size, _2d_array_size, _2d_array_depth);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // test subimage path
    GLuint *pixels;
    pixels = (GLuint *)genTexturePixels(GL_RGBA, GL_UNSIGNED_BYTE, 0x08, _2d_array_size, _2d_array_size, _2d_array_depth, true);

    size_t image_size;
    image_size = _2d_array_size * _2d_array_size; // image size in pixels

    // and test pbo unpack path
    GLuint pbo;
    glCreateBuffers(1, &pbo);
    glNamedBufferStorage(pbo, image_size * _2d_array_depth * sizeof(uint32_t), pixels, GL_MAP_WRITE_BIT);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

    for(int i=0; i<_2d_array_depth; i++)
    {
        size_t offset;

        offset = i * image_size * 4;

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _2d_array_size, _2d_array_size, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)offset);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glViewport(0, 0, width, height);

    glClearColor(0.2, 0.2, 0.2, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    int count = 100000000;

    while (count--)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0, _2d_array_depth);

        angle += (M_PI / 360);

        Model = glm::rotate(glm::identity<mat4>(), angle, glm::vec3(0, 0, 1));

        mvp = Projection * View * Model;

        glNamedBufferSubData(mat_ubo, 0, sizeof(mat4), &mvp[0][0]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_FRONT);

    return 0;
}
void error_callback(int error_code, const char* description)
{
    fprintf(stderr, "%s\n", description);
    //exit(EXIT_FAILURE);
}

#if TEST_MGL_GLFW
int main_glfw(int argc, const char * argv[])
{
    glfwSetErrorCallback (error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    //glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);

    // force MGL
    //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    //glfwWindowHint(GLFW_DEPTH_BITS, 32);

    fprintf(stderr, "creating window...\n");

    GLFWwindow* window = glfwCreateWindow(600, 600, "MGL Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }	

    GLMContext glm_ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
    void *renderer = CppCreateMGLRendererAndBindToContext (glfwGetCocoaWindow (window), glm_ctx); // FIXME should do something later with the renderer
    if (!renderer)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    MGLsetCurrentContext(glm_ctx);
    glfwSetWindowUserPointer(window, glm_ctx);

    //glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwGetError(NULL);

    int width, height;

    glfwGetWindowSize(window, &width, &height);

    fprintf(stderr, "setup complete. testing...\n");

    // test_clear(window, width, height);
    // test_draw_arrays(window, width, height);
    // test_draw_elements(window, width, height);
    // test_draw_range_elements(window, width, height);
    // test_draw_arrays_instanced(window, width, height);
    // test_uniform_buffer(window, width, height);
    // test_1D_textures(window, width, height);
    // test_1D_array_textures(window, width, height);
    // test_2D_textures(window, width, height);
    // test_3D_textures(window, width, height);
    // test_2D_array_textures(window, width, height);
    // test_textures(window, width, height, 0, 0);
    // test_textures(window, width, height, 0, 0, 0, GL_NEAREST, GL_NEAREST);
    // test_textures(window, width, height, 0, 0, 0, GL_LINEAR, GL_LINEAR);
    // test_textures(window, width, height, 1, 1, 0, GL_LINEAR_MIPMAP_NEAREST);
    // test_textures(window, width, height, 1, 1, 8, GL_LINEAR_MIPMAP_NEAREST);
    // test_framebuffer(window, width, height);
    // test_readpixels(window, width, height);
    test_compute_shader(window, width, height);

    //test_2D_array_textures_perf_mon(window, width, height);


    glfwTerminate();

    return 0;
}
#endif

#if TEST_MGL_SDL
int main_sdl(int argc, const char * argv[])
{
    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "creating window...\n");

    SDL_GL_LoadLibrary ("/Users/conversy/recherche/istar/code/misc/MGL/build/libmgl.dylib"); // this will make SDL_GL_GetProcAdress work as expected

    SDL_Window * window = SDL_CreateWindow (
        "MGL Test", 
        0,0,600,600,
          SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_ALLOW_HIGHDPI
        | SDL_WINDOW_OPENGL
        //| SDL_WINDOW_METAL
        );

    if( window == NULL ) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }

    GLMContext glm_ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
    MGLsetCurrentContext(glm_ctx);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if( !SDL_GetWindowWMInfo(window, &info)) {
        fprintf(stderr, "Couldn't GetWindowWMInfo: %s %s:%d\n", SDL_GetError(), __FILE__, __LINE__);
        exit(EXIT_FAILURE);
      }
    assert (info.subsystem==SDL_SYSWM_COCOA);
    void * renderer = CppCreateMGLRendererAndBindToContext (info.info.cocoa.window, glm_ctx);
    if (!renderer)
    {
        fprintf(stderr, "Couldn't create MGL renderer\n");   
        exit(EXIT_FAILURE);
    }
    SDL_SetWindowData (window, "MGLRenderer", glm_ctx);
    assert(SDL_GetWindowData(window, "MGLRenderer")==glm_ctx);

    SDL_GL_SetSwapInterval(0);

    int width, height;

    SDL_GetWindowSize(window, &width, &height);

    int wscaled, hscaled;

    SDL_GL_GetDrawableSize(window, &wscaled, &hscaled);
    printf("%d %d\n",width, wscaled);

    glViewport(0, 0, wscaled, hscaled);

    fprintf(stderr, "setup complete. testing...\n");

    // test_clear(window, width, height);
    // test_draw_arrays(window, width, height);
    // test_draw_elements(window, width, height);
    // test_draw_range_elements(window, width, height);
    // test_draw_arrays_instanced(window, width, height);
    // test_uniform_buffer(window, width, height);
    // test_1D_textures(window, width, height);
    // test_1D_array_textures(window, width, height);
    test_2D_textures(window, width, height);
    // test_3D_textures(window, width, height);
    // test_2D_array_textures(window, width, height);
    // test_textures(window, width, height, 0, 0);
    // test_textures(window, width, height, 0, 0, 0, GL_NEAREST, GL_NEAREST);
    // test_textures(window, width, height, 0, 0, 0, GL_LINEAR, GL_LINEAR);
    // test_textures(window, width, height, 1, 1, 0, GL_LINEAR_MIPMAP_NEAREST);
    // test_textures(window, width, height, 1, 1, 8, GL_LINEAR_MIPMAP_NEAREST);
    // test_framebuffer(window, width, height);
    // test_readpixels(window, width, height);
    // test_compute_shader(window, width, height);

    //test_2D_array_textures_perf_mon(window, width, height);

    SDL_Quit();

    return 0;
}
#endif

int main(int argc, const char * argv[])
{
#if TEST_MGL_GLFW
    return main_glfw(argc, argv);
#endif
#if TEST_MGL_SDL
    return main_sdl(argc, argv);
#endif
}

