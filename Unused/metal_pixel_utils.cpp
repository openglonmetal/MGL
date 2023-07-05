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
 * metal_pixel_utils.cpp
 * MGL
 *
 */

#include <typeinfo>

#include "metal_pixel_utils.h"

#define MGL_API_AVAILABLE(...) __API_AVAILABLE_GET_MACRO(__VA_ARGS__,__API_AVAILABLE7, __API_AVAILABLE6, __API_AVAILABLE5, __API_AVAILABLE4, __API_AVAILABLE3, __API_AVAILABLE2, __API_AVAILABLE1, 0)(__VA_ARGS__)

#define MGL_API_UNAVAILABLE(...) __API_UNAVAILABLE_GET_MACRO(__VA_ARGS__,__API_UNAVAILABLE7,__API_UNAVAILABLE6, __API_UNAVAILABLE5, __API_UNAVAILABLE4,__API_UNAVAILABLE3,__API_UNAVAILABLE2,__API_UNAVAILABLE1, 0)(__VA_ARGS__)

typedef enum MTLPixelFormat_t {
    MTLPixelFormatInvalid = 0,

    /* Normal 8 bit formats */

    MTLPixelFormatA8Unorm      = 1,

    MTLPixelFormatR8Unorm                            = 10,
    MTLPixelFormatR8Unorm_sRGB MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 11,
    MTLPixelFormatR8Snorm      = 12,
    MTLPixelFormatR8Uint       = 13,
    MTLPixelFormatR8Sint       = 14,

    /* Normal 16 bit formats */

    MTLPixelFormatR16Unorm     = 20,
    MTLPixelFormatR16Snorm     = 22,
    MTLPixelFormatR16Uint      = 23,
    MTLPixelFormatR16Sint      = 24,
    MTLPixelFormatR16Float     = 25,

    MTLPixelFormatRG8Unorm                            = 30,
    MTLPixelFormatRG8Unorm_sRGB MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 31,
    MTLPixelFormatRG8Snorm                            = 32,
    MTLPixelFormatRG8Uint                             = 33,
    MTLPixelFormatRG8Sint                             = 34,

    /* Packed 16 bit formats */

    MTLPixelFormatB5G6R5Unorm MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 40,
    MTLPixelFormatA1BGR5Unorm MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 41,
    MTLPixelFormatABGR4Unorm  MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 42,
    MTLPixelFormatBGR5A1Unorm MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 43,

    /* Normal 32 bit formats */

    MTLPixelFormatR32Uint  = 53,
    MTLPixelFormatR32Sint  = 54,
    MTLPixelFormatR32Float = 55,

    MTLPixelFormatRG16Unorm  = 60,
    MTLPixelFormatRG16Snorm  = 62,
    MTLPixelFormatRG16Uint   = 63,
    MTLPixelFormatRG16Sint   = 64,
    MTLPixelFormatRG16Float  = 65,

    MTLPixelFormatRGBA8Unorm      = 70,
    MTLPixelFormatRGBA8Unorm_sRGB = 71,
    MTLPixelFormatRGBA8Snorm      = 72,
    MTLPixelFormatRGBA8Uint       = 73,
    MTLPixelFormatRGBA8Sint       = 74,

    MTLPixelFormatBGRA8Unorm      = 80,
    MTLPixelFormatBGRA8Unorm_sRGB = 81,

    /* Packed 32 bit formats */

    MTLPixelFormatRGB10A2Unorm = 90,
    MTLPixelFormatRGB10A2Uint  = 91,

    MTLPixelFormatRG11B10Float = 92,
    MTLPixelFormatRGB9E5Float = 93,

    MTLPixelFormatBGR10A2Unorm  MGL_API_AVAILABLE(macos(10.13), ios(11.0)) = 94,

    MTLPixelFormatBGR10_XR      MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(10.0)) = 554,
    MTLPixelFormatBGR10_XR_sRGB MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(10.0)) = 555,

    /* Normal 64 bit formats */

    MTLPixelFormatRG32Uint  = 103,
    MTLPixelFormatRG32Sint  = 104,
    MTLPixelFormatRG32Float = 105,

    MTLPixelFormatRGBA16Unorm  = 110,
    MTLPixelFormatRGBA16Snorm  = 112,
    MTLPixelFormatRGBA16Uint   = 113,
    MTLPixelFormatRGBA16Sint   = 114,
    MTLPixelFormatRGBA16Float  = 115,

    MTLPixelFormatBGRA10_XR      MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(10.0)) = 552,
    MTLPixelFormatBGRA10_XR_sRGB MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(10.0)) = 553,

    /* Normal 128 bit formats */

    MTLPixelFormatRGBA32Uint  = 123,
    MTLPixelFormatRGBA32Sint  = 124,
    MTLPixelFormatRGBA32Float = 125,

    /* Compressed formats. */

    /* S3TC/DXT */
    MTLPixelFormatBC1_RGBA              MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 130,
    MTLPixelFormatBC1_RGBA_sRGB         MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 131,
    MTLPixelFormatBC2_RGBA              MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 132,
    MTLPixelFormatBC2_RGBA_sRGB         MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 133,
    MTLPixelFormatBC3_RGBA              MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 134,
    MTLPixelFormatBC3_RGBA_sRGB         MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 135,

    /* RGTC */
    MTLPixelFormatBC4_RUnorm            MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 140,
    MTLPixelFormatBC4_RSnorm            MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 141,
    MTLPixelFormatBC5_RGUnorm           MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 142,
    MTLPixelFormatBC5_RGSnorm           MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 143,

    /* BPTC */
    MTLPixelFormatBC6H_RGBFloat         MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 150,
    MTLPixelFormatBC6H_RGBUfloat        MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 151,
    MTLPixelFormatBC7_RGBAUnorm         MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 152,
    MTLPixelFormatBC7_RGBAUnorm_sRGB    MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0), ios(14.0)) = 153,

    /* PVRTC */
    MTLPixelFormatPVRTC_RGB_2BPP        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 160,
    MTLPixelFormatPVRTC_RGB_2BPP_sRGB   MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 161,
    MTLPixelFormatPVRTC_RGB_4BPP        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 162,
    MTLPixelFormatPVRTC_RGB_4BPP_sRGB   MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 163,
    MTLPixelFormatPVRTC_RGBA_2BPP       MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 164,
    MTLPixelFormatPVRTC_RGBA_2BPP_sRGB  MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 165,
    MTLPixelFormatPVRTC_RGBA_4BPP       MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 166,
    MTLPixelFormatPVRTC_RGBA_4BPP_sRGB  MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 167,

    /* ETC2 */
    MTLPixelFormatEAC_R11Unorm          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 170,
    MTLPixelFormatEAC_R11Snorm          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 172,
    MTLPixelFormatEAC_RG11Unorm         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 174,
    MTLPixelFormatEAC_RG11Snorm         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 176,
    MTLPixelFormatEAC_RGBA8             MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 178,
    MTLPixelFormatEAC_RGBA8_sRGB        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 179,

    MTLPixelFormatETC2_RGB8             MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 180,
    MTLPixelFormatETC2_RGB8_sRGB        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 181,
    MTLPixelFormatETC2_RGB8A1           MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 182,
    MTLPixelFormatETC2_RGB8A1_sRGB      MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 183,

    /* ASTC */
    MTLPixelFormatASTC_4x4_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 186,
    MTLPixelFormatASTC_5x4_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 187,
    MTLPixelFormatASTC_5x5_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 188,
    MTLPixelFormatASTC_6x5_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 189,
    MTLPixelFormatASTC_6x6_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 190,
    MTLPixelFormatASTC_8x5_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 192,
    MTLPixelFormatASTC_8x6_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 193,
    MTLPixelFormatASTC_8x8_sRGB         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 194,
    MTLPixelFormatASTC_10x5_sRGB        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 195,
    MTLPixelFormatASTC_10x6_sRGB        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 196,
    MTLPixelFormatASTC_10x8_sRGB        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 197,
    MTLPixelFormatASTC_10x10_sRGB       MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 198,
    MTLPixelFormatASTC_12x10_sRGB       MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 199,
    MTLPixelFormatASTC_12x12_sRGB       MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 200,

    MTLPixelFormatASTC_4x4_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 204,
    MTLPixelFormatASTC_5x4_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 205,
    MTLPixelFormatASTC_5x5_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 206,
    MTLPixelFormatASTC_6x5_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 207,
    MTLPixelFormatASTC_6x6_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 208,
    MTLPixelFormatASTC_8x5_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 210,
    MTLPixelFormatASTC_8x6_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 211,
    MTLPixelFormatASTC_8x8_LDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 212,
    MTLPixelFormatASTC_10x5_LDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 213,
    MTLPixelFormatASTC_10x6_LDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 214,
    MTLPixelFormatASTC_10x8_LDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 215,
    MTLPixelFormatASTC_10x10_LDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 216,
    MTLPixelFormatASTC_12x10_LDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 217,
    MTLPixelFormatASTC_12x12_LDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(8.0)) = 218,


    // ASTC HDR (High Dynamic Range) Formats
    MTLPixelFormatASTC_4x4_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 222,
    MTLPixelFormatASTC_5x4_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 223,
    MTLPixelFormatASTC_5x5_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 224,
    MTLPixelFormatASTC_6x5_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 225,
    MTLPixelFormatASTC_6x6_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 226,
    MTLPixelFormatASTC_8x5_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 228,
    MTLPixelFormatASTC_8x6_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 229,
    MTLPixelFormatASTC_8x8_HDR          MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 230,
    MTLPixelFormatASTC_10x5_HDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 231,
    MTLPixelFormatASTC_10x6_HDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 232,
    MTLPixelFormatASTC_10x8_HDR         MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 233,
    MTLPixelFormatASTC_10x10_HDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 234,
    MTLPixelFormatASTC_12x10_HDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 235,
    MTLPixelFormatASTC_12x12_HDR        MGL_API_AVAILABLE(macos(11.0), macCatalyst(14.0), ios(13.0)) = 236,

    /*!
     @constant MTLPixelFormatGBGR422
     @abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
     @discussion This format is equivalent to YUY2, YUYV, yuvs, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_REV_APPLE.   The component order, from lowest addressed byte to highest, is Y0, Cb, Y1, Cr.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
     */
    MTLPixelFormatGBGR422 = 240,

    /*!
     @constant MTLPixelFormatBGRG422
     @abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
     @discussion This format is equivalent to UYVY, 2vuy, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_APPLE. The component order, from lowest addressed byte to highest, is Cb, Y0, Cr, Y1.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
     */
    MTLPixelFormatBGRG422 = 241,

    /* Depth */
    MTLPixelFormatDepth16Unorm          MGL_API_AVAILABLE(macos(10.12), ios(13.0)) = 250,
    MTLPixelFormatDepth32Float  = 252,

    /* Stencil */
    MTLPixelFormatStencil8        = 253,

    /* Depth Stencil */
    MTLPixelFormatDepth24Unorm_Stencil8  MGL_API_AVAILABLE(macos(10.11), macCatalyst(13.0)) MGL_API_UNAVAILABLE(ios) = 255,
    MTLPixelFormatDepth32Float_Stencil8  MGL_API_AVAILABLE(macos(10.11), ios(9.0)) = 260,

    MTLPixelFormatX32_Stencil8  MGL_API_AVAILABLE(macos(10.12), ios(10.0)) = 261,
    MTLPixelFormatX24_Stencil8  MGL_API_AVAILABLE(macos(10.12), macCatalyst(13.0)) MGL_API_UNAVAILABLE(ios) = 262,

} MTLPixelFormat;


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

#define _mS(_value_, _bits_, _shift_) ((_value_ >> _shift_) & bitmasks[_bits_])

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

#define _mSL(_value_, _bits_, _shift_) ((_value_ & bitmasks[_bits_]) << _shift_)

static inline void pack_5_6_5(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b;

    r = _mSL(pixel, 5, 10);
    g = _mSL(pixel, 6, 6);
    b = _mSL(pixel, 5, 0);

    *dst = r | g | b;
}

static inline void pack_5_6_5_rev(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b;

    r = _mSL(pixel, 5, 0);
    g = _mSL(pixel, 6, 6);
    b = _mSL(pixel, 5, 10);

    *dst = r | g | b;
}

static inline void pack_4_4_4_4(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b, a;

    r = _mSL(pixel, 4, 12);
    g = _mSL(pixel, 4, 8);
    b = _mSL(pixel, 4, 4);
    a = _mSL(pixel, 4, 0);

    *dst = r | g | b | a;
}

static inline void pack_4_4_4_4_rev(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b, a;

    r = _mSL(pixel, 4, 0);
    g = _mSL(pixel, 4, 4);
    b = _mSL(pixel, 4, 8);
    a = _mSL(pixel, 4, 12);

    *dst = r | g | b | a;
}

static inline void pack_5_5_5_1(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b, a;

    r = _mSL(pixel, 5, 11);
    g = _mSL(pixel, 5, 6);
    b = _mSL(pixel, 5, 1);
    a = _mSL(pixel, 1, 0);

    *dst = r | g | b | a;
}

static inline void pack_5_5_5_1_rev(GLushort pixel, GLushort *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint16_t r, g, b, a;

    r = _mSL(pixel, 5, 0);
    g = _mSL(pixel, 5, 1);
    b = _mSL(pixel, 5, 6);
    a = _mSL(pixel, 1, 11);

    *dst = r | g | b | a;
}

static inline void pack_8_8_8_8(GLuint pixel, GLuint *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint32_t r, g, b, a;

    r = _mSL(pixel, 8, 24);
    g = _mSL(pixel, 8, 16);
    b = _mSL(pixel, 8, 8);
    a = _mSL(pixel, 8, 0);

    *dst = r | g | b | a;
}

static inline void pack_8_8_8_8_rev(GLuint pixel, GLuint *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint32_t r, g, b, a;

    r = _mSL(pixel, 8, 0);
    g = _mSL(pixel, 8, 8);
    b = _mSL(pixel, 8, 16);
    a = _mSL(pixel, 8, 24);

    *dst = r | g | b | a;
}

static inline void pack_10_10_10_2(GLuint pixel, GLuint *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint32_t r, g, b, a;

    r = _mSL(pixel, 10, 22);
    g = _mSL(pixel, 10, 12);
    b = _mSL(pixel, 10, 2);
    a = _mSL(pixel, 2, 0);

    *dst = r | g | b | a;
}

static inline void pack_10_10_10_2_rev(GLuint pixel, GLuint *dst)
{
    static const GLushort bitmasks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff,0x1ff, 0x3ff, 0x7ff};
    uint32_t r, g, b, a;

    r = _mSL(pixel, 10, 0);
    g = _mSL(pixel, 10, 10);
    b = _mSL(pixel, 10, 20);
    a = _mSL(pixel, 2, 30);

    *dst = r | g | b | a;
}

template <typename T, typename S>
bool copy_len_check(S *src, T *dst, size_t len, GLenum type, GLenum check_type)
{
    if (type != check_type)
    {
        return false;
    }

    for(int i=0; i<len; i++)
        dst[i] = src[i];

    return true;
}

template <typename T, typename S>
void copy_len(S *src, T *dst, size_t len)
{
    for(int i=0; i<len; i++)
        dst[i] = src[i];
}

template <typename S, typename T>
void copy_len(S *src, T *dst, size_t len, GLuint num_dst_components,  GLboolean swap_components)
{
    size_t cnt;

    cnt = len * num_dst_components;

    if (swap_components)
    {
        switch(num_dst_components)
        {
            case 1:
                for(int i=0; i<cnt; i++)
                {
                    dst[i] = src[i];
                }
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

            default:
                assert(0);
        }
    }
    else
    {
        switch(num_dst_components)
        {
            case 1:
                for(int i=0; i<cnt; i++)
                {
                    dst[i] = src[i];
                }
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

            default:
                assert(0);
        }
    }
}

template <typename S, typename T>
void convert_pack_len(S *src, T *dst, size_t len,
                        void (*pack)(S pixel, T *dst))
{
    for(int i=0; i<len; i++)
    {
        pack(src[i], &dst[i]);
    }
}

template <typename S, typename T>
void convert_len(S *src, T *dst, size_t len, GLenum format, GLenum type, GLuint num_src_components, GLuint src_comp_size, GLuint num_dst_components, GLuint dst_comp_size, GLboolean swap_components)
{
    switch(type)
    {
        case GL_FLOAT: copy_len(src, (GLfloat *)dst, num_dst_components, swap_components); break;

        case GL_UNSIGNED_BYTE: copy_len(src, (GLubyte *)dst, num_dst_components, swap_components); break;
        case GL_BYTE: copy_len(src, (GLbyte *)dst, num_dst_components, swap_components); break;

        case GL_UNSIGNED_SHORT: copy_len(src, (GLushort *)dst, num_dst_components, swap_components); break;
        case GL_SHORT: copy_len(src, (GLshort *)dst, num_dst_components, swap_components); break;

        case GL_UNSIGNED_INT: copy_len(src, (GLuint *)dst, num_dst_components, swap_components); break;
        case GL_INT: copy_len(src, (GLint *)dst, num_dst_components, swap_components); break;

        case GL_UNSIGNED_SHORT_5_6_5: copy_pack_len(src, (GLushort *)dst, len, pack_5_6_5); break;
        case GL_UNSIGNED_SHORT_5_6_5_REV: copy_pack_len(src, (GLushort *)dst, len, pack_5_6_5_rev); break;

        case GL_UNSIGNED_SHORT_4_4_4_4: copy_pack_len(src, (GLushort *)dst, len, pack_4_4_4_4); break;
        case GL_UNSIGNED_SHORT_4_4_4_4_REV: copy_pack_len(src, (GLushort *)dst, len, pack_4_4_4_4_rev); break;

        case GL_UNSIGNED_SHORT_5_5_5_1: copy_pack_len(src, (GLushort *)dst, len, pack_5_5_5_1); break;
        case GL_UNSIGNED_SHORT_1_5_5_5_REV: copy_pack_len(src, (GLushort *)dst, len, pack_5_5_5_1_rev); break;

        case GL_UNSIGNED_INT_8_8_8_8: copy_len(src, (GLuint *)dst, len); break;
        case GL_UNSIGNED_INT_8_8_8_8_REV: copy_pack_len(src, (GLushort *)dst, len, pack_8_8_8_8_rev); break;

        case GL_UNSIGNED_INT_10_10_10_2: copy_pack_len(src, (GLushort *)dst, len, pack_10_10_10_2); break;
        case GL_UNSIGNED_INT_2_10_10_10_REV: copy_pack_len(src, (GLushort *)dst, len, pack_10_10_10_2_rev); break;

        default:
            assert(0);
    }
}

template <typename S, typename T>
bool convertPackedToDstFormat(S *src, T *dst, size_t len, GLenum mtl_format, GLenum format, GLenum type, GLuint num_src_components, GLuint src_comp_size, GLuint num_dst_components, GLuint dst_comp_size, GLboolean swap_components)
{
    // converts packed src types to dest types
    switch(mtl_format)
    {
        /* Packed 16 bit formats */
        case MTLPixelFormatB5G6R5Unorm: convert_unpack_len((GLushort *)src, dst, len, format, type, num_dst_components, dst_comp_size, unpack_5_6_5); break;
        case MTLPixelFormatA1BGR5Unorm: convert_unpack_len((GLushort *)src, dst, len, format, type, num_dst_components, dst_comp_size, unpack_5_5_5_1_rev); break;
        case MTLPixelFormatABGR4Unorm: convert_unpack_len((GLushort *)src, dst, len, format, type, num_dst_components, dst_comp_size, unpack_4_4_4_4_rev); break;
        case MTLPixelFormatBGR5A1Unorm: convert_unpack_len((GLushort *)src, dst, len, format, type, num_dst_components, dst_comp_size, unpack_5_5_5_1); break;

        /* Packed 32 bit formats */
        // just copy no conversion possible yet
        case MTLPixelFormatRGB10A2Unorm: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_UNSIGNED_INT_10_10_10_2); break;
        case MTLPixelFormatRGB10A2Uint: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_UNSIGNED_INT_10_10_10_2); break;
        case MTLPixelFormatRG11B10Float: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_R11F_G11F_B10F); break;
        case MTLPixelFormatRGB9E5Float: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_RGB9_E5); break;
        case MTLPixelFormatBGR10A2Unorm: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_UNSIGNED_INT_2_10_10_10_REV); break;
        case MTLPixelFormatBGR10_XR: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_UNSIGNED_INT); break;
        case MTLPixelFormatBGR10_XR_sRGB: return copy_len_check((GLuint *)src, (GLuint *)dst, len, type, GL_UNSIGNED_INT); break;

        default:
            assert(0);
            break;
    }

    return true;
}

#define FLT(_NUM_COMP_, _COMP_SIZE_)  is_float = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; break
#define UINT(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; break
#define COMP(_NUM_COMP_, _COMP_SIZE_) is_compressed = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; break
#define INT(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; break

#define UINT_PACKED(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; is_packed = true; break

#define UINT_NORM(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; break
#define INT_NORM(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; break
#define UINT_NORM_REV(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; break
#define INT_NORM_REV(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; break

#define UINT_NORM_PACKED(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; is_packed = true; break
#define INT_NORM_PACKED(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; is_packed = true; break
#define UINT_NORM_REV_PACKED(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; is_packed = true; break
#define INT_NORM_REV_PACKED(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; is_packed = true; break

#define UINT_SRGB(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; is_srgb = true; break
#define COMP_SRGB(_NUM_COMP_, _COMP_SIZE_) is_compressed = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; is_srgb = true; break
#define INT_SRGB(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; is_srgb = true; break
#define UINT_NORM_SRGB(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; is_srgb = true; break
#define INT_NORM_SRGB(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; is_srgb = true; break
#define UINT_NORM_REV_SRGB(_NUM_COMP_, _COMP_SIZE_) is_uint = true; num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; is_srgb = true; break
#define INT_NORM_REV_SRGB(_NUM_COMP_, _COMP_SIZE_)  num_src_components = _NUM_COMP_; src_comp_size = _COMP_SIZE_; normalize = true; swap_components = true; is_srgb = true; break


bool pixelConvertMTLFormatToGLFormatType(GLMContext ctx, MTLPixelFormat mtl_format, GLenum format, GLenum type, const void *src, void *dst, size_t len)
{
    bool is_float, is_uint, is_compressed, is_packed, swap_components, normalize, is_srgb;
    GLuint num_src_components, src_comp_size;
    GLuint num_dst_components, dst_comp_size;

    is_float = 0;
    is_uint = 0;
    is_compressed = 0;
    is_packed = 0;
    swap_components = 0;
    normalize = 0;
    is_srgb = 0;

    switch(mtl_format)
    {
        case MTLPixelFormatInvalid:
            return false;

        /* Normal 8 bit formats */
        case MTLPixelFormatA8Unorm: UINT_NORM(1,1);
        case MTLPixelFormatR8Unorm_sRGB: UINT_NORM_SRGB(1,1);
        case MTLPixelFormatR8Snorm: INT_NORM(1,1);
        case MTLPixelFormatR8Uint: UINT(1,1);
        case MTLPixelFormatR8Sint: INT(1,1);

        /* Normal 16 bit formats */
        case MTLPixelFormatR16Unorm: UINT_NORM(1,2);
        case MTLPixelFormatR16Snorm: INT_NORM(1,2);
        case MTLPixelFormatR16Uint: UINT(1,2);
        case MTLPixelFormatR16Sint: INT(1,1);
        case MTLPixelFormatR16Float: FLT(1,2);

        case MTLPixelFormatRG8Unorm: UINT_NORM(2,1);
        case MTLPixelFormatRG8Unorm_sRGB: UINT_NORM_SRGB(2,1);
        case MTLPixelFormatRG8Snorm: INT_NORM(2,1);
        case MTLPixelFormatRG8Uint: UINT(2,1);
        case MTLPixelFormatRG8Sint: INT(2,1);

        /* Normal 32 bit formats */
        case MTLPixelFormatR32Uint: UINT(1,4);
        case MTLPixelFormatR32Sint: INT(1,4);
        case MTLPixelFormatR32Float: FLT(1,4);
        case MTLPixelFormatRG16Unorm: UINT_NORM(2,2);
        case MTLPixelFormatRG16Snorm: INT_NORM(2,2);
        case MTLPixelFormatRG16Uint: UINT(2,2);
        case MTLPixelFormatRG16Sint: INT(2,2);
        case MTLPixelFormatRG16Float: FLT(2,2);

        case MTLPixelFormatRGBA8Unorm: UINT_NORM(4,1);
        case MTLPixelFormatRGBA8Unorm_sRGB: UINT_NORM_SRGB(4,1);
        case MTLPixelFormatRGBA8Snorm: INT_NORM(4,1);
        case MTLPixelFormatRGBA8Uint: UINT(4,1);
        case MTLPixelFormatRGBA8Sint: INT(4,1);

        case MTLPixelFormatBGRA8Unorm: UINT_NORM_REV(4,1);
        case MTLPixelFormatBGRA8Unorm_sRGB: UINT_NORM_REV_SRGB(4,1);

        /* Normal 64 bit formats */

        case MTLPixelFormatRG32Uint: UINT(2,4);
        case MTLPixelFormatRG32Sint: INT(2,4);
        case MTLPixelFormatRG32Float: FLT(2,4);
        case MTLPixelFormatRGBA16Unorm: UINT_NORM(4,2);
        case MTLPixelFormatRGBA16Snorm: INT_NORM(4,2);
        case MTLPixelFormatRGBA16Uint: UINT(4,2);
        case MTLPixelFormatRGBA16Sint: INT(4,2);
        case MTLPixelFormatRGBA16Float: FLT(4,2);

        /* Normal 128 bit formats */
        case MTLPixelFormatRGBA32Uint: UINT(4,4);
        case MTLPixelFormatRGBA32Sint: INT(4,4);
        case MTLPixelFormatRGBA32Float: FLT(4,4);

        /* Compressed formats. */
        /* S3TC/DXT */
        case MTLPixelFormatBC1_RGBA: COMP(4,0);
        case MTLPixelFormatBC1_RGBA_sRGB: COMP(4,0);
        case MTLPixelFormatBC2_RGBA: COMP(4,0);
        case MTLPixelFormatBC2_RGBA_sRGB: COMP(4,0);
        case MTLPixelFormatBC3_RGBA: COMP(4,0);
        case MTLPixelFormatBC3_RGBA_sRGB: COMP(4,0);

        /* RGTC */
        case MTLPixelFormatBC4_RUnorm: COMP(1,0);
        case MTLPixelFormatBC4_RSnorm: COMP(1,0);
        case MTLPixelFormatBC5_RGUnorm: COMP(4,0);
        case MTLPixelFormatBC5_RGSnorm: COMP(4,0);

        /* BPTC */
        case MTLPixelFormatBC6H_RGBFloat: COMP(3,0);
        case MTLPixelFormatBC6H_RGBUfloat: COMP(3,0);
        case MTLPixelFormatBC7_RGBAUnorm: COMP(4,0);
        case MTLPixelFormatBC7_RGBAUnorm_sRGB: COMP(4,0);

        /* PVRTC */
        case MTLPixelFormatPVRTC_RGB_2BPP: COMP(3,0);
        case MTLPixelFormatPVRTC_RGB_2BPP_sRGB: COMP(3,0);
        case MTLPixelFormatPVRTC_RGB_4BPP: COMP(3,0);
        case MTLPixelFormatPVRTC_RGB_4BPP_sRGB: COMP(3,0);
        case MTLPixelFormatPVRTC_RGBA_2BPP: COMP(4,0);
        case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB: COMP(4,0);
        case MTLPixelFormatPVRTC_RGBA_4BPP: COMP(4,0);
        case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB: COMP(4,0);

        /* ETC2 */
        case MTLPixelFormatEAC_R11Unorm: COMP(1,0);
        case MTLPixelFormatEAC_R11Snorm: COMP(1,0);
        case MTLPixelFormatEAC_RG11Unorm: COMP(2,0);
        case MTLPixelFormatEAC_RG11Snorm: COMP(2,0);
        case MTLPixelFormatEAC_RGBA8: COMP(4,0);
        case MTLPixelFormatEAC_RGBA8_sRGB: COMP(4,0);
        case MTLPixelFormatETC2_RGB8: COMP(3,0);
        case MTLPixelFormatETC2_RGB8_sRGB: COMP(3,0);
        case MTLPixelFormatETC2_RGB8A1: COMP(4,0);
        case MTLPixelFormatETC2_RGB8A1_sRGB: COMP(4,0);

        /* ASTC */
        case MTLPixelFormatASTC_4x4_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_5x4_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_5x5_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_6x5_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_6x6_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_8x5_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_8x6_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_8x8_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_10x5_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_10x6_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_10x8_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_10x10_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_12x10_sRGB: COMP(0,0);
        case MTLPixelFormatASTC_12x12_sRGB: COMP(0,0);

        case MTLPixelFormatASTC_4x4_LDR: COMP(0,0);
        case MTLPixelFormatASTC_5x4_LDR: COMP(0,0);
        case MTLPixelFormatASTC_5x5_LDR: COMP(0,0);
        case MTLPixelFormatASTC_6x5_LDR: COMP(0,0);
        case MTLPixelFormatASTC_6x6_LDR: COMP(0,0);
        case MTLPixelFormatASTC_8x5_LDR: COMP(0,0);
        case MTLPixelFormatASTC_8x6_LDR: COMP(0,0);
        case MTLPixelFormatASTC_8x8_LDR: COMP(0,0);
        case MTLPixelFormatASTC_10x5_LDR: COMP(0,0);
        case MTLPixelFormatASTC_10x6_LDR: COMP(0,0);
        case MTLPixelFormatASTC_10x8_LDR: COMP(0,0);
        case MTLPixelFormatASTC_10x10_LDR: COMP(0,0);
        case MTLPixelFormatASTC_12x10_LDR: COMP(0,0);

            // ASTC HDR (High Dynamic Range) Formats
        case MTLPixelFormatASTC_4x4_HDR: COMP(0,0);
        case MTLPixelFormatASTC_5x4_HDR: COMP(0,0);
        case MTLPixelFormatASTC_5x5_HDR: COMP(0,0);
        case MTLPixelFormatASTC_6x5_HDR: COMP(0,0);
        case MTLPixelFormatASTC_6x6_HDR: COMP(0,0);
        case MTLPixelFormatASTC_8x5_HDR: COMP(0,0);
        case MTLPixelFormatASTC_8x6_HDR: COMP(0,0);
        case MTLPixelFormatASTC_8x8_HDR: COMP(0,0);
        case MTLPixelFormatASTC_10x5_HDR: COMP(0,0);
        case MTLPixelFormatASTC_10x6_HDR: COMP(0,0);
        case MTLPixelFormatASTC_10x8_HDR: COMP(0,0);
        case MTLPixelFormatASTC_10x10_HDR: COMP(0,0);
        case MTLPixelFormatASTC_12x10_HDR: COMP(0,0);
        case MTLPixelFormatASTC_12x12_HDR: COMP(0,0);

        /*!
         @constant MTLPixelFormatGBGR422
         @abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
         @discussion This format is equivalent to YUY2, YUYV, yuvs, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_REV_APPLE.   The component order, from lowest addressed byte to highest, is Y0, Cb, Y1, Cr.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
             */

        case MTLPixelFormatGBGR422: UINT_SRGB(3, 3);

        /*!
         @constant MTLPixelFormatBGRG422
         @abstract A pixel format where the red and green channels are subsampled horizontally.  Two pixels are stored in 32 bits, with shared red and blue values, and unique green values.
         @discussion This format is equivalent to UYVY, 2vuy, or GL_RGB_422_APPLE/GL_UNSIGNED_SHORT_8_8_APPLE. The component order, from lowest addressed byte to highest, is Cb, Y0, Cr, Y1.  There is no implicit colorspace conversion from YUV to RGB, the shader will receive (Cr, Y, Cb, 1).  422 textures must have a width that is a multiple of 2, and can only be used for 2D non-mipmap textures.  When sampling, ClampToEdge is the only usable wrap mode.
         */

        /* Depth */
        case MTLPixelFormatDepth16Unorm: UINT_NORM(1, 2);
        case MTLPixelFormatDepth32Float: FLT(1, 4);

        /* Stencil */
        case MTLPixelFormatStencil8: UINT(1, 1);

        /* Depth Stencil */
        case MTLPixelFormatDepth24Unorm_Stencil8: UINT(1, 4);
        case MTLPixelFormatDepth32Float_Stencil8: UINT(1, 5);
        case MTLPixelFormatX32_Stencil8: UINT(1, 5);
        case MTLPixelFormatX24_Stencil8: UINT(1, 4);

        /* Packed 16 bit formats */
        case MTLPixelFormatB5G6R5Unorm: UINT_NORM_PACKED(1, 2);
        case MTLPixelFormatA1BGR5Unorm: UINT_NORM_PACKED(1, 2);
        case MTLPixelFormatABGR4Unorm: UINT_NORM_PACKED(1, 2);
        case MTLPixelFormatBGR5A1Unorm: UINT_NORM_PACKED(1, 2);

        /* Packed 32 bit formats */
        case MTLPixelFormatRGB10A2Unorm: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatRGB10A2Uint: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatRG11B10Float: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatRGB9E5Float: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatBGR10A2Unorm: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatBGR10_XR: UINT_NORM_PACKED(1, 4);
        case MTLPixelFormatBGR10_XR_sRGB: UINT_NORM_PACKED(1, 4);

        /* Packed 64 bit formats */
        case MTLPixelFormatBGRA10_XR: UINT_NORM_PACKED(1, 8);
        case MTLPixelFormatBGRA10_XR_sRGB: UINT_NORM_PACKED(1, 8);

        default:
            return false;
    }

    assert(num_src_components);
    assert(src_comp_size);


    if (is_packed)
    {
        return convertPackedToDstFormat(src, dst, len, mtl_format, format, type, num_src_components, src_comp_size, num_dst_components, dst_comp_size, swap_components);
    }
    else if (is_compressed)
    {
        copy_len((GLubyte *)src, (GLubyte *)dst, len * num_dst_components);
    }
    else if (is_float)
    {
        copy_len(format, type, (GLfloat *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components);
    }
    else if (is_uint)
    {
        switch(src_comp_size)
        {
            case 1: convert_len(format, type, (GLubyte *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 2: convert_len(format, type, (GLushort *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 4: convert_len(format, type, (GLuint *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 8: convert_len(format, type, (GLuint64 *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            default:
                assert(0);
        }
    }
    else // int
    {
        switch(src_comp_size)
        {
            case 1: convert_len(format, type, (GLbyte *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 2: convert_len(format, type, (GLshort *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 4: convert_len(format, type, (GLint *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            case 8: convert_len(format, type, (GLint64 *)src, dst, len, num_src_components, num_dst_components, dst_comp_size, swap_components); break;
            default:
                assert(0);
        }
    }

    return true;
}

