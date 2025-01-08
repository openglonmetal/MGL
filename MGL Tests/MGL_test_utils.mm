//
//  MGL_test_utils.mm
//  MGL
//
//  Created by Michael Larson on 1/4/25.
//


#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdbool.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#include "MGL_test_utils.h"

#include "MGLContext.h"
#include "MGLRenderer.h"

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

void *genTexturePixels(GLenum format, GLenum type, GLuint repeat, GLuint width, GLuint height, GLint depth, GLboolean is_array)
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

GLuint bindVAO(GLuint vao)
{
    if(vao)
    {
        glBindVertexArray(vao);
    }
    else
    {
        GLuint new_vao;

        glCreateVertexArrays(1, &new_vao);
        glBindVertexArray(new_vao);

        return new_vao;
    }

    return vao;
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


// Copyright(c) 2019 Nick Klingensmith (@koujaku). All rights reserved.
//
// This work is licensed under the terms of the MIT license.
// For a copy of this license, see < https://opensource.org/licenses/MIT >
bool tga_write(const char *filename, uint32_t width, uint32_t height, uint8_t *dataBGRA, uint8_t dataChannels, uint8_t fileChannels)
{
    FILE *fp = NULL;
    
    fp = fopen(filename, "wb");
    if (fp == NULL)
        return false;

    // You can find details about TGA headers here: http://www.paulbourke.net/dataformats/tga/
    uint8_t header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0, (uint8_t)(width%256), (uint8_t)(width/256), (uint8_t)(height%256), (uint8_t)(height/256), (uint8_t)(fileChannels*8), 0x20 };
    fwrite(&header, 18, 1, fp);

    for (uint32_t i = 0; i < width*height; i++)
    {
        for (uint32_t b = 0; b < fileChannels; b++)
        {
            fputc(dataBGRA[(i*dataChannels) + (b%dataChannels)], fp);
        }
    }
    
    fclose(fp);
    
    return true;
}
