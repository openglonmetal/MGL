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
 * pixel_pack.cpp
 * MGL
 *
 */

#include "pixel_utils.h"
#include "glm_context_macros.h"

// finish this at some later time
#if 0
#define _mS(_value_, _bits_, _shift_) ((_value_ >> _shift_) & bitmasks[_bits_])

GLushort convertToHalfFloat(GLfloat f)
{
    GLuint uf, *ufptr, sign, exponent, mantisa;
    GLushort us;

    ufptr = (GLuint *)&f;
    uf = *ufptr;

    sign = (uf >> 31) & 0x1;
    exponent = (uf >> 24) & 0xff;
    mantisa = (uf & 0xffffff);
    exponent &= 0x1f;
    mantisa >>= 3;

    us = (sign << 15) | (exponent << 10) | mantisa;

    return us;
}

template <typename T>
static inline void unpack_3_3_2(GLubyte pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b;

    r = _mS(pixel, 3, 5);
    g = _mS(pixel, 3, 2);
    b = _mS(pixel, 2, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
    }
}

template <typename T>
static inline void unpack_3_3_2_rev(GLubyte pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b;

    r = _mS(pixel, 3, 0);
    g = _mS(pixel, 3, 2);
    b = _mS(pixel, 2, 5);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
    }

}

template <typename T>
static inline void unpack_5_6_5(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b;

    r = _mS(pixel, 5, 11);
    g = _mS(pixel, 6, 5);
    b = _mS(pixel, 5, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
    }
}

template <typename T>
static inline void unpack_5_6_5_rev(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b;

    r = _mS(pixel, 5, 0);
    g = _mS(pixel, 6, 5);
    b = _mS(pixel, 5, 11);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
    }
}

template <typename T>
static inline void unpack_4_4_4_4(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 4, 12);
    g = _mS(pixel, 4, 8);
    b = _mS(pixel, 4, 4);
    a = _mS(pixel, 4, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_4_4_4_4_rev(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 4, 0);
    g = _mS(pixel, 4, 4);
    b = _mS(pixel, 4, 8);
    a = _mS(pixel, 4, 12);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_5_5_5_1(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 5, 11);
    g = _mS(pixel, 5, 6);
    b = _mS(pixel, 5, 1);
    a = _mS(pixel, 1, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_5_5_5_1_rev(GLushort pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 5, 0);
    g = _mS(pixel, 5, 1);
    b = _mS(pixel, 5, 6);
    a = _mS(pixel, 1, 11);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_8_8_8_8(GLuint pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 8, 24);
    g = _mS(pixel, 8, 16);
    b = _mS(pixel, 8, 8);
    a = _mS(pixel, 8, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_8_8_8_8_rev(GLuint pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 8, 0);
    g = _mS(pixel, 8, 8);
    b = _mS(pixel, 8, 16);
    a = _mS(pixel, 8, 24);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_10_10_10_2(GLuint pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 10, 22);
    g = _mS(pixel, 10, 12);
    b = _mS(pixel, 10, 2);
    a = _mS(pixel, 2, 0);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename T>
static inline void unpack_10_10_10_2_rev(GLuint pixel, GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa, T *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint8_t r, g, b, a;

    r = _mS(pixel, 10, 0);
    g = _mS(pixel, 10, 10);
    b = _mS(pixel, 10, 20);
    a = _mS(pixel, 2, 30);

    if (typeid(T) == typeid(GLfloat))
    {
        dst[0] = (T)r * sr;
        dst[1] = (T)g * sg;
        dst[2] = (T)b * sb;
        dst[3] = (T)a * sa;
    }
    else
    {
        dst[0] = (T)r;
        dst[1] = (T)g;
        dst[2] = (T)b;
        dst[3] = (T)a;
    }
}

template <typename S, typename T>
void convert_len(S *src, T *dst, size_t len, GLfloat scale, GLuint num_dst_components, GLboolean swap_components)
{
    size_t cnt;

    cnt = len * num_dst_components;

    if (swap_components)
    {
        switch (num_dst_components)
        {
        case 1:
            for(int i=0; i<len; i++)
                dst[i] = src[i] * scale;
            break;
        case 2:
            for(int i=0; i<cnt; i+=2)
            {
                dst[i] = src[i+1] * scale;
                dst[i] = src[i] * scale;
            }
            break;

        case 3:
            for(int i=0; i<len; i+=3)
            {
                dst[i] = src[i+2] * scale;
                dst[i+1] = src[i+1] * scale;
                dst[i+2] = src[i] * scale;
            }
            break;

        case 4:
            for(int i=0; i<len; i+=4)
            {
                dst[i] = src[i+3] * scale;
                dst[i+1] = src[i+2] * scale;
                dst[i+2] = src[i+1] * scale;
                dst[i+3] = src[i] * scale;
            }
            break;
        }
    }
    else
    {
        switch (num_dst_components)
        {
        case 1:
            for(int i=0; i<len; i++)
                dst[i] = src[i] * scale;
            break;
        case 2:
            for(int i=0; i<cnt; i+=2)
            {
                dst[i] = src[i] * scale;
                dst[i+1] = src[i+1] * scale;
            }
            break;

        case 3:
            for(int i=0; i<len; i+=3)
            {
                dst[i] = src[i] * scale;
                dst[i+1] = src[i+1] * scale;
                dst[i+2] = src[i+2] * scale;
            }
            break;

        case 4:
            for(int i=0; i<len; i+=4)
            {
                dst[i] = src[i] * scale;
                dst[i+1] = src[i+1] * scale;
                dst[i+2] = src[i+2] * scale;
                dst[i+3] = src[i+3] * scale;
            }
            break;
        }
    }
}

template <typename S, typename T>
void convert_unpack_len(S *src, T *dst, size_t len,
                        void (*unpack)(S pixel, GLfloat sr, GLfloat sb, GLfloat sg, GLfloat sa, T *dst),
                        GLfloat sr, GLfloat sg, GLfloat sb, GLfloat sa)
{
    for(int i=0; i<len; i++)
    {
        unpack(src[i], sr, sg, sb, sa, &dst[i]);
    }
}

template <typename T>
void convertToFloat(GLenum format, GLenum type, const T *src, GLfloat *dst, size_t len, GLuint num_dst_components, GLuint dst_comp_size, GLboolean swap_components)
{
    GLfloat inv_4;
    GLfloat inv_8;
    GLfloat inv_16;
    GLfloat inv_32;
    GLfloat inv_64;
    GLfloat inv_127;
    GLfloat inv_255;
    GLfloat inv_1023;
    GLfloat inv_65535;
    GLfloat inv_32767;
    double inv_4294967295;
    double inv_2147483647;

    inv_4 = 1.0 / 4.0;
    inv_8 = 1.0 / 8.0;
    inv_16 = 1.0 / 16.0;
    inv_32 = 1.0 / 32.0;
    inv_64 = 1.0 / 64.0;

    inv_127 = 1.0 / 127.0;
    inv_255 = 1.0 / 255.0;

    inv_1023 = 1.0 / 1023.0;

    inv_65535 = 1.0 / 65535.0;
    inv_32767 = 1.0 / 32767.0;
    inv_4294967295 = 1.0 / 4294967295.0;
    inv_2147483647 = 1.0 / 2147483647.0;

    switch(type)
    {
        case GL_UNSIGNED_BYTE: convert_len((GLubyte *)src, dst, len, inv_255, num_dst_components, swap_components); break;
        case GL_BYTE: convert_len((GLbyte *)src, dst, len, inv_127, num_dst_components, swap_components); break;

        case GL_UNSIGNED_SHORT: convert_len((GLushort *)src, dst, len, inv_65535, num_dst_components, swap_components); break;
        case GL_SHORT: convert_len((GLshort *)src, dst, len, inv_32767, num_dst_components, swap_components); break;

        case GL_UNSIGNED_INT: convert_len((GLuint *)src, dst, len, inv_4294967295, num_dst_components, swap_components); break;
        case GL_INT: convert_len((GLint *)src, dst, len, inv_2147483647, num_dst_components, swap_components); break;

        case GL_FLOAT: convert_len((GLfloat *)src, dst, len, 1.0, num_dst_components, swap_components); break;

        case GL_UNSIGNED_BYTE_3_3_2: convert_unpack_len((GLubyte *)src, dst, len, unpack_3_3_2, inv_8, inv_8, inv_4, 0.0f); break;
        case GL_UNSIGNED_BYTE_2_3_3_REV: convert_unpack_len((GLubyte *)src, dst, len, unpack_3_3_2_rev, inv_8, inv_8, inv_4, 0.0f); break;

        case GL_UNSIGNED_SHORT_5_6_5: convert_unpack_len((GLushort *)src, dst, len, unpack_5_6_5, inv_32, inv_64, inv_32, 0.0f); break;
        case GL_UNSIGNED_SHORT_5_6_5_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_5_6_5_rev, inv_32, inv_64, inv_32, 0.0f); break;

        case GL_UNSIGNED_SHORT_4_4_4_4: convert_unpack_len((GLushort *)src, dst, len, unpack_4_4_4_4, inv_16, inv_16, inv_16, inv_16); break;
        case GL_UNSIGNED_SHORT_4_4_4_4_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_4_4_4_4_rev, inv_16, inv_16, inv_16, inv_16); break;

        case GL_UNSIGNED_SHORT_5_5_5_1: convert_unpack_len((GLushort *)src, dst, len, unpack_5_5_5_1, inv_32, inv_32, inv_32, 1.0f); break;
        case GL_UNSIGNED_SHORT_1_5_5_5_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_5_5_5_1_rev, inv_32, inv_32, inv_32, 1.0f); break;

        case GL_UNSIGNED_INT_8_8_8_8: convert_unpack_len((GLuint *)src, dst, len, unpack_8_8_8_8, inv_255, inv_255, inv_255, inv_255); break;
        case GL_UNSIGNED_INT_8_8_8_8_REV: convert_unpack_len((GLuint *)src, dst, len, unpack_8_8_8_8_rev, inv_255, inv_255, inv_255, inv_255); break;

        case GL_UNSIGNED_INT_10_10_10_2: convert_unpack_len((GLuint *)src, dst, len, unpack_10_10_10_2, inv_1023, inv_1023, inv_1023, inv_4); break;
        case GL_UNSIGNED_INT_2_10_10_10_REV: convert_unpack_len((GLuint *)src, dst, len, unpack_10_10_10_2_rev, inv_1023, inv_1023, inv_1023, inv_4); break;

        default:
            assert(0);
    }
}

void pixelConvertToFloat(GLenum internalformat, GLenum format, GLenum type, GLuint num_dst_components, GLuint dst_comp_size, GLboolean swap_components, const void *src, GLfloat *dst, size_t len)
{
    GLfloat *temp;
    size_t block_size;
    ;

    block_size = 1024 / num_dst_components;

    temp = (GLfloat *)malloc(1024*sizeof(GLfloat));

    // convert to temp float format
    while(len > 0)
    {
        if (dst_comp_size == 4)
        {
            // full floats
            convertToFloat(format, type, src, dst, len, num_dst_components, dst_comp_size, swap_components);
        }
        else if (dst_comp_size == 2)
        {
            // half floats
            assert(0);
        }
        else
        {
            assert(0);
        }

        if (len > block_size)
            len -= block_size;
        else
            len = 0;

        assert(len >= 0);
    }

    free(temp);
}

template <typename T, typename S>
void convert_len(T *src, S *dst, size_t len, GLuint num_dst_components, GLboolean swap_components)
{
    size_t cnt;

    cnt = len * num_dst_components;

    if (swap_components)
    {
        switch (num_dst_components)
        {
        case 1:
            for(int i=0; i<cnt; i++)
                dst[i] = src[i];
            break;
        case 2:
            for(int i=0; i<cnt; i+=2)
            {
                dst[i] = src[i+1];
                dst[i+1] = src[i];
            }
            break;

        case 3:
            for(int i=0; i<cnt; i+=3)
            {
                dst[i] = src[i+2];
                dst[i+1] = src[i+1];
                dst[i+2] = src[i];
            }
            break;

        case 4:
            for(int i=0; i<cnt; i+=4)
            {
                dst[i] = src[i+3];
                dst[i+1] = src[i+2];
                dst[i+2] = src[i+1];
                dst[i+3] = src[i];
            }
            break;
        }
    }
    else
    {
        switch (num_dst_components)
        {
        case 1:
            for(int i=0; i<cnt; i++)
                dst[i] = src[i];
            break;
        case 2:
            for(int i=0; i<cnt; i+=2)
            {
                dst[i] = src[i];
                dst[i+1] = src[i+1];
            }
            break;

        case 3:
            for(int i=0; i<cnt; i+=3)
            {
                dst[i] = src[i];
                dst[i+1] = src[i+1];
                dst[i+2] = src[i+2];
            }
            break;

        case 4:
            for(int i=0; i<cnt; i+=4)
            {
                dst[i] = src[i];
                dst[i+1] = src[i+1];
                dst[i+2] = src[i+2];
                dst[i+3] = src[i+3];
            }
            break;
        }
    }
}

template <typename S, typename T>
void convert_unpack_len(S *src, T *dst, size_t len,
                        void (*unpack)(S pixel, GLfloat sr, GLfloat sb, GLfloat sg, GLfloat sa, T *dst))
{
    for(int i=0; i<len; i++)
    {
        unpack(src[i], 0, 0, 0, 0, &dst[i]);
    }
}

template <typename T>
void convertToDstFormat(GLenum format, GLenum type, const void *src, T *dst, size_t len, GLuint num_dst_components, GLuint dst_comp_size, GLboolean swap_components)
{
    switch(type)
    {
        case GL_UNSIGNED_BYTE: convert_len((GLubyte *)src, dst, len, num_dst_components, swap_components); break;
        case GL_BYTE: convert_len((GLbyte *)src, dst, len, num_dst_components, swap_components); break;

        case GL_UNSIGNED_SHORT: convert_len((GLushort *)src, dst, len, num_dst_components, swap_components); break;
        case GL_SHORT: convert_len((GLshort *)src, dst, len, num_dst_components, swap_components); break;

        case GL_UNSIGNED_INT: convert_len((GLuint *)src, dst, len, num_dst_components, swap_components); break;
        case GL_INT: convert_len((GLint *)src, dst, len, num_dst_components, swap_components); break;

        case GL_FLOAT: convert_len((GLfloat *)src, dst, len, num_dst_components, swap_components); break;

        case GL_UNSIGNED_BYTE_3_3_2: convert_unpack_len((GLubyte *)src, dst, len, unpack_3_3_2); break;
        case GL_UNSIGNED_BYTE_2_3_3_REV: convert_unpack_len((GLubyte *)src, dst, len, unpack_3_3_2_rev); break;

        case GL_UNSIGNED_SHORT_5_6_5: convert_unpack_len((GLushort *)src, dst, len, unpack_5_6_5); break;
        case GL_UNSIGNED_SHORT_5_6_5_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_5_6_5_rev); break;

        case GL_UNSIGNED_SHORT_4_4_4_4: convert_unpack_len((GLushort *)src, dst, len, unpack_4_4_4_4); break;
        case GL_UNSIGNED_SHORT_4_4_4_4_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_4_4_4_4_rev); break;

        case GL_UNSIGNED_SHORT_5_5_5_1: convert_unpack_len((GLushort *)src, dst, len, unpack_5_5_5_1); break;
        case GL_UNSIGNED_SHORT_1_5_5_5_REV: convert_unpack_len((GLushort *)src, dst, len, unpack_5_5_5_1_rev); break;

        case GL_UNSIGNED_INT_8_8_8_8: convert_unpack_len((GLuint *)src, dst, len, unpack_8_8_8_8); break;
        case GL_UNSIGNED_INT_8_8_8_8_REV: convert_unpack_len((GLuint *)src, dst, len, unpack_8_8_8_8_rev); break;

        case GL_UNSIGNED_INT_10_10_10_2: convert_unpack_len((GLuint *)src, dst, len, unpack_10_10_10_2); break;
        case GL_UNSIGNED_INT_2_10_10_10_REV: convert_unpack_len((GLuint *)src, dst, len, unpack_10_10_10_2_rev); break;

        default:
            assert(0);
            break;
    }
}

GLboolean swapComponents(GLenum format)
{
    switch(format)
    {
        case GL_BGR:
        case GL_BGR_INTEGER:
        case GL_BGRA:
        case GL_BGRA_INTEGER:
            return true;
    }

    return false;
}

#define FLT(_NUM_COMP_, _COMP_SIZE_)  is_float = true; num_dst_components = _NUM_COMP_; dst_comp_size = _COMP_SIZE_; break
#define UINT(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_dst_components = _NUM_COMP_; dst_comp_size = _COMP_SIZE_; break
#define COMP(_NUM_COMP_, _COMP_SIZE_) is_compressed = true; num_dst_components = _NUM_COMP_; dst_comp_size = _COMP_SIZE_; break
#define INT(_NUM_COMP_, _COMP_SIZE_)  num_dst_components = _NUM_COMP_; dst_comp_size = _COMP_SIZE_; break

bool pixelConvertToInternalFormat(GLMContext ctx, GLenum internalformat, GLenum format, GLenum type, const void *src, void *dst, size_t len)
{
    bool is_float, is_uint, is_compressed;
    GLuint num_src_components, num_dst_components, dst_comp_size;
    bool swap_components;

    assert(len);

    // figure out destination format
    is_uint = false;
    is_float = false;
    is_compressed = false;
    num_dst_components = 0;
    dst_comp_size = 0;

    switch(internalformat)
    {
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB12:
        case GL_RGBA2:
        case GL_RGB5_A1:
        case GL_RGBA4:
        case GL_RGB10:
        case GL_RGBA12:
        case GL_ALPHA4:
        case GL_ALPHA12:
        case GL_LUMINANCE4:
        case GL_LUMINANCE12:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_INTENSITY12:
        case GL_INTENSITY4:
        case GL_STENCIL_INDEX1:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
            return false; // unsupported at this time

        case GL_RGB565: UINT(3, 16);
        case GL_RGB10_A2UI: UINT(4,32);
        case GL_R11F_G11F_B10F: FLT(3,32);
        case GL_RGB10_A2: UINT(4, 32);
        case GL_RGB9_E5: UINT(4,32);
            assert(0); // need to implement
            break;

        case GL_RGB8: UINT(3, 1);
        case GL_RGB16: UINT(3, 2);
        case GL_RGBA8: UINT(4, 1);
        case GL_RGBA16: UINT(4, 2);

        case GL_ALPHA8: UINT(1, 1);
        case GL_ALPHA16: UINT(1, 2);

        case GL_LUMINANCE8: UINT(1, 1);
        case GL_LUMINANCE16: UINT(1, 2);
        case GL_LUMINANCE8_ALPHA8: UINT(2, 2);
        case GL_LUMINANCE16_ALPHA16: UINT(2, 2);

        case GL_INTENSITY8: UINT(1, 1);
        case GL_INTENSITY16: UINT(1, 2);

        case GL_COMPRESSED_RGB: COMP(3,0);
        case GL_COMPRESSED_RGBA: COMP(4,0);

        case GL_DEPTH_COMPONENT16: UINT(1,2);
        case GL_DEPTH_COMPONENT24: UINT(1,3);
        case GL_DEPTH_COMPONENT32: UINT(1,4);

        case GL_SRGB: UINT(3,0);
        case GL_SRGB8: UINT(3,1);
        case GL_SRGB_ALPHA: UINT(4,0);
        case GL_SRGB8_ALPHA8: UINT(4,1);
        case GL_COMPRESSED_SRGB: COMP(3,0);
        case GL_COMPRESSED_SRGB_ALPHA: COMP(4,0);
        case GL_COMPRESSED_RED: COMP(1,0);
        case GL_COMPRESSED_RG: COMP(2,0);

        case GL_RGBA32F: FLT(4,4);
        case GL_RGB32F: FLT(3,4);
        case GL_RGBA16F: FLT(4,2);
        case GL_RGB16F: FLT(3,2);
        case GL_RGBA32UI: UINT(4,4);
        case GL_RGB32UI: UINT(3,4);
        case GL_RGBA16UI: UINT(4,2);
        case GL_RGB16UI: UINT(3,2);
        case GL_RGBA8UI: UINT(4,1);
        case GL_RGB8UI: UINT(3,1);
        case GL_RGBA32I: INT(4,4);
        case GL_RGB32I: INT(3,4);
        case GL_RGBA16I: INT(4,2);
        case GL_RGB16I: INT(3,2);
        case GL_RGBA8I: INT(4,1);
        case GL_RGB8I: INT(3,1);

        case GL_DEPTH_COMPONENT32F: FLT(1,4);
        case GL_DEPTH32F_STENCIL8: FLT(2,4);
        case GL_DEPTH24_STENCIL8: UINT(2,3);

        case GL_STENCIL_INDEX4:
        case GL_STENCIL_INDEX8: UINT(1,1);
        case GL_STENCIL_INDEX16: UINT(1,2);

        case GL_COMPRESSED_RED_RGTC1: COMP(1,0);
        case GL_COMPRESSED_SIGNED_RED_RGTC1: COMP(1,0);
        case GL_COMPRESSED_RG_RGTC2: COMP(2,0);
        case GL_COMPRESSED_SIGNED_RG_RGTC2: COMP(2,0);

        case GL_R8: UINT(1,1);
        case GL_R16: UINT(1,2);
        case GL_RG8: UINT(2,1);
        case GL_RG16: UINT(2,2);
        case GL_R16F: FLT(1,2);
        case GL_R32F: FLT(1,4);
        case GL_RG16F: FLT(2,2);
        case GL_RG32F: FLT(2,4);

        case GL_R8I: INT(1,1);
        case GL_R8UI: UINT(1,1);
        case GL_R16I: INT(1,2);
        case GL_R16UI: UINT(1,2);
        case GL_R32I: INT(1,4);
        case GL_R32UI: UINT(1,4);
        case GL_RG8I: INT(2,1);
        case GL_RG8UI: UINT(2,1);
        case GL_RG16I: INT(2,2);
        case GL_RG16UI: UINT(2,2);
        case GL_RG32I: INT(2,4);
        case GL_RG32UI: UINT(2,4);

        case GL_R8_SNORM: INT(1,1);
        case GL_RG8_SNORM: INT(2,1);
        case GL_RGB8_SNORM: INT(3,1);
        case GL_RGBA8_SNORM: INT(4,1);
        case GL_R16_SNORM: INT(1,2);
        case GL_RG16_SNORM: INT(2,2);
        case GL_RGB16_SNORM: INT(3,2);
        case GL_RGBA16_SNORM: INT(4,2);

        case GL_COMPRESSED_RGBA_BPTC_UNORM: COMP(4,0);
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: COMP(4,0);
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: COMP(3,0);
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: COMP(3,0);
        case GL_COMPRESSED_RGB8_ETC2: COMP(2,0);
        case GL_COMPRESSED_SRGB8_ETC2: COMP(3,0);
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: COMP(4,0);
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: COMP(4,0);
        case GL_COMPRESSED_RGBA8_ETC2_EAC: COMP(4,0);
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: COMP(4,0);
        case GL_COMPRESSED_R11_EAC: COMP(1,2);
        case GL_COMPRESSED_SIGNED_R11_EAC: COMP(1,2);
        case GL_COMPRESSED_RG11_EAC: COMP(2,2);
        case GL_COMPRESSED_SIGNED_RG11_EAC: COMP(2,2);
            break;

        default:
            return false;
    }

    assert(num_dst_components);
    assert(dst_comp_size);

    swap_components = swapComponents(format);

    num_src_components = numComponentsForFormat(format);

    if (num_src_components != num_dst_components)
    {
        return false;
    }

    if (is_float)
    {
        pixelConvertToFloat(internalformat, format, type, num_dst_components, dst_comp_size, swap_components, src, (GLfloat *)dst, len);
    }
    else if (is_uint)
    {
        switch(dst_comp_size)
        {
            case 1:
                convertToDstFormat(format, type, src, (GLubyte *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
            case 2:
                convertToDstFormat(format, type, src, (GLushort *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
            case 4:
                convertToDstFormat(format, type, src, (GLuint *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
        }
    }
    else if (is_compressed)
    {
        convertToDstFormat(format, type, (GLubyte *)src, (GLubyte *)dst, len, num_dst_components, dst_comp_size, swap_components);
        assert(0);
    }
    else // is int
    {
        switch(dst_comp_size)
        {
            case 1:
                convertToDstFormat(format, type, (GLbyte *)src, (GLubyte *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
            case 2:
                convertToDstFormat(format, type, (GLshort *)src, (GLubyte *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
            case 4:
                convertToDstFormat(format, type, (GLint *)src, (GLubyte *)dst, len, num_dst_components, dst_comp_size, swap_components);
                break;
        }
    }

    return true;
}

#endif // finish at later time
