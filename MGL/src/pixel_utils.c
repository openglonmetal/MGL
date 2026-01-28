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
 * pixel_utils.c
 * MGL
 *
 */
 
#include <Availability.h>

#include "pixel_utils.h"
#include "glm_context.h"

// Legacy format defines not in core profile headers
#ifndef GL_ALPHA
#define GL_ALPHA                          0x1906
#endif
#ifndef GL_LUMINANCE
#define GL_LUMINANCE                      0x1909
#endif

#ifndef GL_ALPHA8UI_EXT
#define GL_ALPHA8UI_EXT                   0x8D7E
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA                0x190A
#endif
#ifndef GL_ALPHA8
#define GL_ALPHA8                         0x803C
#endif
#ifndef GL_ALPHA16
#define GL_ALPHA16                        0x803E
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8                     0x8040
#endif
#ifndef GL_LUMINANCE16
#define GL_LUMINANCE16                    0x8048
#endif
#ifndef GL_ALPHA32F_ARB
#define GL_ALPHA32F_ARB                   0x8816
#endif
#ifndef GL_LUMINANCE32F_ARB
#define GL_LUMINANCE32F_ARB               0x8818
#endif
#ifndef GL_LUMINANCE_ALPHA32F_ARB
#define GL_LUMINANCE_ALPHA32F_ARB         0x8819
#endif
#ifndef GL_ALPHA16F_ARB
#define GL_ALPHA16F_ARB                   0x881C
#endif
#ifndef GL_LUMINANCE16F_ARB
#define GL_LUMINANCE16F_ARB               0x881E
#endif
#ifndef GL_LUMINANCE_ALPHA16F_ARB
#define GL_LUMINANCE_ALPHA16F_ARB         0x881F
#endif
#ifndef GL_SR8_EXT
#define GL_SR8_EXT                        0x8FBD
#endif
#ifndef GL_SRG8_EXT
#define GL_SRG8_EXT                       0x8FBE
#endif

GLuint numComponentsForFormat(GLenum format)
{
    switch(format)
    {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        // Legacy single-channel formats
        case GL_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA16:
        case GL_ALPHA32F_ARB:
        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE:
        case GL_LUMINANCE8:
        case GL_LUMINANCE16:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE16F_ARB:
        // Sized R formats (internal formats sometimes passed as format)
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_SR8_EXT:
        case GL_ALPHA8UI_EXT:
        case 0x9014: // GL_ALPHA8_SNORM
        case 0x9018: // GL_ALPHA16_SNORM
            return 1;

        case GL_RG:
        case GL_RG_INTEGER:
        // Legacy two-channel formats
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case 0x9016: // GL_LUMINANCE8_ALPHA8_SNORM
        case 0x901a: // GL_LUMINANCE16_ALPHA16_SNORM
        // Sized RG formats
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_SRG8_EXT:
            return 2;

        case 0x8d7b: // GL_ALPHA8I_EXT
        case 0x8d81: // GL_ALPHA32I_EXT
        case 0x8d87: // GL_ALPHA16I_EXT
        case 0x8d8d: // GL_ALPHA32UI_EXT
        case 0x8d93: // GL_ALPHA16UI_EXT
        case 0x8d72: // GL_ALPHA32UI_EXT
            return 1;

        case GL_RGB:
        case GL_BGR:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
        // Sized RGB formats
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_SRGB8:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGB565:
            return 3;

        case 0x8d75: // alternate GL_RGB8I
        case 0x8d7a: // alternate GL_RGB8UI
        case 0x8d80: // alternate GL_RGB32UI
        case 0x8d86: // alternate GL_RGB16I
        case 0x8d8c: // alternate GL_RGB32I
        case 0x8d92: // alternate GL_RGB16UI
            return 3;

        case GL_RGBA:
        case GL_BGRA:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
        // Sized RGBA formats
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGB5_A1:
        case GL_RGBA4:
            return 4;

        case 0x8d78: // alternate GL_RGBA8UI
        // case 0x8d7e: // alternate GL_RGBA32UI - Duplicate of GL_ALPHA8UI_EXT (1 component)
        case 0x8d84: // alternate GL_RGBA16I
        case 0x8d8a: // alternate GL_RGBA32I
        case 0x8d90: // alternate GL_RGBA16UI
            return 4;

        default:
            // Unknown format - return 4 as safe fallback instead of crashing
            fprintf(stderr, "MGL WARNING: numComponentsForFormat unknown format 0x%x, assuming 4 components\n", format);
            return 4;
    }

    return 0;
}

GLuint sizeForType(GLenum type)
{
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return sizeof(uint8_t);

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return sizeof(uint16_t);

        case GL_UNSIGNED_INT:
        case GL_INT:
            return sizeof(uint32_t);

        case GL_FLOAT:
            return sizeof(float);

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return sizeof(uint8_t);

        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            return sizeof(uint16_t);

        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return sizeof(uint32_t);

        default:
            fprintf(stderr, "MGL WARNING: sizeForType unknown type 0x%x, assuming 4 bytes\n", type);
            return sizeof(uint32_t);
    }

    return 0;
}

GLuint sizeForFormatType(GLenum format, GLenum type)
{
    // Handle type=0 case (used for sized internal formats)
    // The format parameter is actually the internal format in this case
    if (type == 0)
    {            
        switch (format)
        {            
            // Alpha formats (1 component)
            case 0x803c: // GL_ALPHA8
            case 0x8040: // GL_LUMINANCE8
                return 1;
            case 0x803e: // GL_ALPHA16
            case 0x8042: // GL_LUMINANCE16
            case 0x8816: // GL_ALPHA16F_ARB
            case 0x8818: // GL_LUMINANCE16F_ARB
                return 2;
            case 0x881c: // GL_ALPHA32F_ARB
            case 0x881e: // GL_LUMINANCE32F_ARB
                return 4;
            // Luminance-alpha formats (2 components)
            case 0x8045: // GL_LUMINANCE8_ALPHA8
                return 2;
            case 0x8048: // GL_LUMINANCE16_ALPHA16
            case 0x8819: // GL_LUMINANCE_ALPHA16F_ARB
                return 4;
            case 0x881f: // GL_LUMINANCE_ALPHA32F_ARB
                return 8;
            // RGB10_A2UI and SNORM formats
            case 0x8fbd: // GL_RGB10_A2UI
                return 4;
            case 0x8fbe: // GL_RGBA16_SNORM
                return 8;
            // Integer formats
            case 0x8d72: case 0x8d78: // RGBA8I/UI variants
                return 4;
            case 0x8d75: case 0x8d7a: // RGB8I/UI variants
                return 3;
            case 0x8d84: case 0x8d90: // RGBA16I/UI variants
                return 8;
            case 0x8d86: case 0x8d92: // RGB16I/UI variants
                return 6;
            case 0x8d8a: case 0x8d7e: // RGBA32I/UI variants
                return 16;
            case 0x8d8c: case 0x8d80: // RGB32I/UI variants  
                return 12;
            default:
                // Return a reasonable default for unknown internal formats
                return 4;
        }
    }

    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return sizeof(uint8_t) * numComponentsForFormat(format);

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return sizeof(uint16_t) * numComponentsForFormat(format);

        case GL_UNSIGNED_INT:
        case GL_INT:
            return sizeof(uint32_t) * numComponentsForFormat(format);

        case GL_FLOAT:
            return sizeof(float) * numComponentsForFormat(format);

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return sizeof(uint8_t);

        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            return sizeof(uint16_t);

        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return sizeof(uint32_t);

        case GL_HALF_FLOAT:
            return sizeof(uint16_t) * numComponentsForFormat(format);

        default:
            fprintf(stderr, "MGL WARNING: sizeForFormatType unknown type 0x%x, format 0x%x\n", type, format);
            return sizeof(uint32_t) * numComponentsForFormat(format);
    }

    return 0;
}

GLenum verifyInternalFormatType(GLint internalformat, GLenum format, GLenum type)
{
    switch(internalformat)
    {
        // unsized formats
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
            break;

        // sized formats
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_R16F:
        case GL_RG16F:
        case GL_RGB16F:
        case GL_RGBA16F:
        case GL_R32F:
        case GL_RG32F:
        case GL_RGB32F:
        case GL_RGBA32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            break;

        // compressed types
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            break;

        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            if(format == GL_DEPTH_COMPONENT)
                return GL_INVALID_OPERATION;
            break;

        default:
            return GL_INVALID_ENUM;
    }

    switch(format)
    {
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_BGR:
        case GL_RGBA:
        case GL_BGRA:
        case GL_RED_INTEGER:
        case GL_RG_INTEGER:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
            break;

        default:
            return GL_INVALID_ENUM;
    }

    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            if(format != GL_RGB)
                return GL_INVALID_OPERATION;
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            if (format != GL_RGBA || format != GL_BGRA)
                return GL_INVALID_OPERATION;
            break;
    }

    return true;
}

GLboolean validFormat(GLuint format)
{
    switch(format)
    {
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_BGR:
        case GL_RGBA:
        case GL_BGRA:
        case GL_RED_INTEGER:
        case GL_RG_INTEGER:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
            return true;

        default:
            return false;
    }

    return false;
}

GLboolean validFormatType(GLuint format, GLuint type)
{
    RETURN_FALSE_ON_FAILURE(validFormat(format));

    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            return true;

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_SHORT_5_6_5:
            RETURN_FALSE_ON_FAILURE(format == GL_RGB);
            break;

        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            RETURN_FALSE_ON_FAILURE(format == GL_BGR);
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_10_10_10_2:
            RETURN_FALSE_ON_FAILURE(format == GL_RGBA);
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            RETURN_FALSE_ON_FAILURE(format == GL_BGRA);
            break;

        default:
            return false;
    }

    return true;
}

GLboolean validInternalFormat(GLint internalformat)
{
    switch(internalformat)
    {
        // unsized formats
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
            break;

        // sized formats
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_R16F:
        case GL_RG16F:
        case GL_RGB16F:
        case GL_RGBA16F:
        case GL_R32F:
        case GL_RG32F:
        case GL_RGB32F:
        case GL_RGBA32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            break;

        // compressed types
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            break;

        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            break;

        default:
            return false;
    }

    return true;
}

#define bitsToBytes(_bits_) ((_bits_ % 8 ? _bits_ / 8 + 1 : _bits_ / 8))
GLuint sizeForInternalFormat(GLenum internalformat, GLenum format, GLenum type)
{
    // return size in bytes
    switch(internalformat)
    {
        case GL_R3_G3_B2:
            return bitsToBytes(8);

        case GL_RGB4:
            return bitsToBytes(12);

        case GL_RGB5:
            return bitsToBytes(15);

        case GL_RGB8:
            return bitsToBytes(24);

        case GL_RGB10:
            return bitsToBytes(30);

        case GL_RGB12:
            return bitsToBytes(12);

        case GL_RGB16:
            return bitsToBytes(48);

        case GL_RGBA2:
            return bitsToBytes(8);

        case GL_RGBA4:
            return bitsToBytes(16);

        case GL_RGB5_A1:
            return bitsToBytes(16);

        case GL_RGBA8:
            return bitsToBytes(32);

        case GL_RGB10_A2:
            return bitsToBytes(32);

        case GL_RGBA12:
            return bitsToBytes(36);

        case GL_RGBA16:
            return bitsToBytes(48);

        case GL_COMPRESSED_RGB:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RGBA:
            return 0;   // return 0 on compressed

        case GL_DEPTH_COMPONENT16:
            return bitsToBytes(12);

        case GL_DEPTH_COMPONENT24:
            return bitsToBytes(12);

        case GL_DEPTH_COMPONENT32:
            return bitsToBytes(12);

        case GL_SRGB:
            return bitsToBytes(12);

        case GL_SRGB8:
            return bitsToBytes(12);

        case GL_SRGB_ALPHA:
            return bitsToBytes(12);

        case GL_SRGB8_ALPHA8:
            return bitsToBytes(12);

        case GL_COMPRESSED_SRGB:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SRGB_ALPHA:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RED:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RG:
            return 0;   // return 0 on compressed

        case GL_RGBA32F:
            return bitsToBytes(128);

        case GL_RGB32F:
            return bitsToBytes(96);

        case GL_RGBA16F:
            return bitsToBytes(64);

        case GL_RGB16F:
            return bitsToBytes(48);

        case GL_R11F_G11F_B10F:
            return bitsToBytes(32);

        case GL_RGB9_E5:
            return bitsToBytes(32);

        case GL_RGBA32UI:
            return bitsToBytes(128);

        case GL_RGB32UI:
            return bitsToBytes(96);

        case GL_RGBA16UI:
            return bitsToBytes(64);

        case GL_RGB16UI:
            return bitsToBytes(48);

        case GL_RGBA8UI:
            return bitsToBytes(32);

        case GL_RGB8UI:
            return bitsToBytes(24);

        case GL_RGBA32I:
            return bitsToBytes(128);

        case GL_RGB32I:
            return bitsToBytes(96);

        case GL_RGBA16I:
            return bitsToBytes(64);

        case GL_RGB16I:
            return bitsToBytes(48);

        case GL_RGBA8I:
            return bitsToBytes(32);

        case GL_RGB8I:
            return bitsToBytes(24);

        case GL_DEPTH_COMPONENT32F:
            return bitsToBytes(32);

        case GL_DEPTH32F_STENCIL8:
            return bitsToBytes(40);

        case GL_DEPTH24_STENCIL8:
            return bitsToBytes(32);

        case GL_STENCIL_INDEX1:
            return 1; // bitsToBytes(1);

        case GL_STENCIL_INDEX4:
            return 1; // bitsToBytes(4);

        case GL_STENCIL_INDEX8:
            return bitsToBytes(8);

        case GL_STENCIL_INDEX16:
            return bitsToBytes(16);

        case GL_COMPRESSED_RED_RGTC1:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RG_RGTC2:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            return 0;   // return 0 on compressed

        case GL_R8:
            return bitsToBytes(8);

        case GL_R16:
            return bitsToBytes(16);

        case GL_RG8:
            return bitsToBytes(16);

        case GL_RG16:
            return bitsToBytes(32);

        case GL_R16F:
            return bitsToBytes(16);

        case GL_R32F:
            return bitsToBytes(32);

        case GL_RG16F:
            return bitsToBytes(32);

        case GL_RG32F:
            return bitsToBytes(32);

        case GL_R8I:
            return bitsToBytes(8);

        case GL_R8UI:
            return bitsToBytes(8);

        case GL_R16I:
            return bitsToBytes(16);

        case GL_R16UI:
            return bitsToBytes(16);

        case GL_R32I:
            return bitsToBytes(32);

        case GL_R32UI:
            return bitsToBytes(32);

        case GL_RG8I:
            return bitsToBytes(16);

        case GL_RG8UI:
            return bitsToBytes(24);

        case GL_RG16I:
            return bitsToBytes(16);

        case GL_RG16UI:
            return bitsToBytes(16);

        case GL_RG32I:
            return bitsToBytes(64);

        case GL_RG32UI:
            return bitsToBytes(64);

        case GL_R8_SNORM:
            return bitsToBytes(8);

        case GL_RG8_SNORM:
            return bitsToBytes(16);

        case GL_RGB8_SNORM:
            return bitsToBytes(24);

        case GL_RGBA8_SNORM:
            return bitsToBytes(32);

        case GL_R16_SNORM:
            return bitsToBytes(16);

        case GL_RG16_SNORM:
            return bitsToBytes(32);

        case GL_RGB16_SNORM:
            return bitsToBytes(48);

        case GL_RGBA16_SNORM:
            return bitsToBytes(64);

        case GL_RGB10_A2UI:
            return bitsToBytes(32);

        case GL_RGB565:
            return bitsToBytes(16);


        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
            return 0;   // return 0 on compressed

        default:
            if (internalformat)
            {
                // we didn't get a sized internal format use the internalformat
                // and the src type to figure out a generic size
                return sizeForFormatType(internalformat, type);
            }
            else
            {
                // we didn't get a sized internal format use the src format
                // and the src type to figure out a generic size
                return sizeForFormatType(format, type);
            }
    }

    return 0;
}

GLuint bicountForFormatType(GLenum format, GLenum type, GLenum component)
{
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return 8;

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return 16;

        case GL_UNSIGNED_INT:
        case GL_INT:
            return 32;

        case GL_FLOAT:
            return 32;

        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return 8;

        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            switch(component)
            {
                case GL_RED: return 5;
                case GL_GREEN: return 6;
                case GL_BLUE: return 5;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            switch(component)
            {
                case GL_RED: return 4;
                case GL_GREEN: return 4;
                case GL_BLUE: return 4;
                case GL_ALPHA: return 4;
            }
            break;


        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            switch(component)
            {
                case GL_RED: return 5;
                case GL_GREEN: return 5;
                case GL_BLUE: return 5;
                case GL_ALPHA: return 1;
            }
            break;

            return 16;

        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return 8;

        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            switch(component)
            {
                case GL_RED: return 10;
                case GL_GREEN: return 10;
                case GL_BLUE: return 10;
                case GL_ALPHA: return 2;
            }
            break;

        default:
            assert(0);
    }

    return 0;
}

GLuint bitcountForInternalFormat(GLenum internalformat, GLenum component)
{
    // return size in bytes
    switch(internalformat)
    {
        case GL_R3_G3_B2:
            switch(component)
            {
                case GL_RED: return 3;
                case GL_GREEN: return 3;
                case GL_BLUE: return 2;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB4:
            switch(component)
            {
                case GL_RED: return 4;
                case GL_GREEN: return 4;
                case GL_BLUE: return 42;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB5:
            switch(component)
            {
                case GL_RED: return 5;
                case GL_GREEN: return 5;
                case GL_BLUE: return 5;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB8:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 8;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB10:
            switch(component)
            {
                case GL_RED: return 10;
                case GL_GREEN: return 10;
                case GL_BLUE: return 10;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB12:
            switch(component)
            {
                case GL_RED: return 12;
                case GL_GREEN: return 12;
                case GL_BLUE: return 12;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB16:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 16;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA2:
            switch(component)
            {
                case GL_RED: return 2;
                case GL_GREEN: return 2;
                case GL_BLUE: return 2;
                case GL_ALPHA: return 2;
            }
            break;

        case GL_RGBA4:
            switch(component)
            {
                case GL_RED: return 4;
                case GL_GREEN: return 4;
                case GL_BLUE: return 4;
                case GL_ALPHA: return 4;
            }
            break;

        case GL_RGB5_A1:
            switch(component)
            {
                case GL_RED: return 5;
                case GL_GREEN: return 5;
                case GL_BLUE: return 5;
                case GL_ALPHA: return 1;
            }
            break;

        case GL_RGBA8:
            return 9;

        case GL_RGB10_A2:
            switch(component)
            {
                case GL_RED: return 10;
                case GL_GREEN: return 10;
                case GL_BLUE: return 10;
                case GL_ALPHA: return 2;
            }
            break;

        case GL_RGBA12:
            return 12;

        case GL_RGBA16:
            return 16;

        case GL_COMPRESSED_RGB:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RGBA:
            return 0;   // return 0 on compressed

        case GL_DEPTH_COMPONENT16:
            return 16;

        case GL_DEPTH_COMPONENT24:
            return 24;

        case GL_DEPTH_COMPONENT32:
            return 32;

        case GL_SRGB:
            return 8;

        case GL_SRGB8:
            return 8;

        case GL_SRGB_ALPHA:
            return 8;

        case GL_SRGB8_ALPHA8:
            return 8;

        case GL_COMPRESSED_SRGB:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SRGB_ALPHA:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RED:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RG:
            return 0;   // return 0 on compressed

        case GL_RGBA32F:
            return 32;

        case GL_RGB32F:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 32;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA16F:
            return 16;

        case GL_RGB16F:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 16;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R11F_G11F_B10F:
            switch(component)
            {
                case GL_RED: return 11;
                case GL_GREEN: return 11;
                case GL_BLUE: return 11;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB9_E5:
            switch(component)
            {
                case GL_RED: return 9;
                case GL_GREEN: return 9;
                case GL_BLUE: return 9;
                case GL_ALPHA: return 5;
            }
            break;

        case GL_RGBA32UI:
            return 32;

        case GL_RGB32UI:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 32;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA16UI:
            return 16;

        case GL_RGB16UI:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 16;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA8UI:
            return 8;

        case GL_RGB8UI:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 8;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA32I:
            return 32;

        case GL_RGB32I:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 32;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA16I:
            return 16;

        case GL_RGB16I:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 16;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA8I:
            return 8;

        case GL_RGB8I:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 8;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_DEPTH_COMPONENT32F:
            return 32;

        case GL_DEPTH32F_STENCIL8:
            switch(component)
            {
                case GL_DEPTH: return 32;
                case GL_STENCIL: return 8;
            }
            break;

        case GL_DEPTH24_STENCIL8:
            switch(component)
            {
                case GL_DEPTH: return 24;
                case GL_STENCIL: return 8;
            }
            break;

        case GL_STENCIL_INDEX1:
            return 1; // retBits(1);

        case GL_STENCIL_INDEX4:
            return 1; // retBits(4);

        case GL_STENCIL_INDEX8:
        case GL_STENCIL_INDEX16:
        case GL_COMPRESSED_RED_RGTC1:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_RG_RGTC2:
            return 0;   // return 0 on compressed

        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            return 0;   // return 0 on compressed

        case GL_R8:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R16:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG8:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG16:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R16F:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R32F:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG16F:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG32F:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R8I:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R8UI:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R16I:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R16UI:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R32I:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R32UI:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG8I:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG8UI:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG16I:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG16UI:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG32I:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 32;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG32UI:
            switch(component)
            {
                case GL_RED: return 32;
                case GL_GREEN: return 32;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_R8_SNORM:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG8_SNORM:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB8_SNORM:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 8;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA8_SNORM:
            return 8;

        case GL_R16_SNORM:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 0;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RG16_SNORM:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 0;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGB16_SNORM:
            switch(component)
            {
                case GL_RED: return 16;
                case GL_GREEN: return 16;
                case GL_BLUE: return 16;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_RGBA16_SNORM:
            return 16;

        case GL_RGB10_A2UI:
            switch(component)
            {
                case GL_RED: return 8;
                case GL_GREEN: return 8;
                case GL_BLUE: return 8;
                case GL_ALPHA: return 2;
            }
            break;

        case GL_RGB565:
            switch(component)
            {
                case GL_RED: return 5;
                case GL_GREEN: return 6;
                case GL_BLUE: return 5;
                case GL_ALPHA: return 0;
            }
            break;

        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
            return 0;   // return 0 on compressed

        default:
            return 0;
    }

    return 0;
}

GLenum internalFormatForGLFormatType(GLenum format, GLenum type)
{
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
            switch(format)
            {
                case GL_RED: return GL_R8;
                case GL_RG: return GL_RG8;
                case GL_RGB: return GL_RGB8;
                case GL_BGR: return GL_RGB8;  /* BGR treated as RGB */
                case GL_RGBA: return GL_RGBA8;
                case GL_BGRA: return GL_RGBA8;  /* BGRA treated as RGBA */
                default:
                    return 0;
            }
            break;

        case GL_BYTE:
            switch(format)
            {
                case GL_RED: return GL_R8_SNORM;
                case GL_RG: return GL_RG8_SNORM;
                case GL_RGB: return GL_RGB8_SNORM;
                case GL_RGBA: return GL_RGBA8_SNORM;
                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_SHORT:
            switch(format)
            {
                case GL_RED: return GL_R16;
                case GL_RG: return GL_RG16;
                case GL_RGB: return GL_RGB16;
                case GL_RGBA: return GL_RGBA16;
                default:
                    return 0;
            }
            break;

        case GL_SHORT:
            switch(format)
            {
                case GL_RED: return GL_R16_SNORM;
                case GL_RG: return GL_RG16_SNORM;
                case GL_RGB: return GL_RGB16_SNORM;
                case GL_RGBA: return GL_RGBA16_SNORM;
                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_INT:
            switch(format)
            {
                case GL_RED: return GL_R32UI;
                case GL_RG: return GL_RG32UI;
                case GL_RGB: return GL_RGB32UI;
                case GL_RGBA: return GL_RGBA32UI;
                default:
                    return 0;
            }
            break;

        case GL_INT:
            switch(format)
            {
                case GL_RED: return GL_R32I;
                case GL_RG: return GL_RG32I;
                case GL_RGB: return GL_RGB32I;
                case GL_RGBA: return GL_RGBA32I;
                default:
                    return 0;
            }
            break;

        case GL_FLOAT:
            switch(format)
            {
                case GL_RED: return GL_R32F;
                case GL_RG: return GL_RG32F;
                case GL_RGB: return GL_RGB32F;
                case GL_RGBA: return GL_RGBA32F;
                case GL_DEPTH_COMPONENT: return GL_DEPTH_COMPONENT32F;
                case GL_DEPTH_STENCIL: return GL_DEPTH32F_STENCIL8;

                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
            return 0;

        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return 0;

        case GL_UNSIGNED_SHORT_5_6_5:
            return GL_RGB565;

        case GL_UNSIGNED_SHORT_5_6_5_REV:
            return 0;

        case GL_UNSIGNED_SHORT_4_4_4_4:
            return GL_RGBA4;

        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            return 0;

        case GL_UNSIGNED_INT_8_8_8_8:
            return GL_RGBA8;

        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return GL_RGBA8;

        default:
            assert(0);
    }
}

MTLPixelFormat mtlFormatForGLInternalFormat(GLenum internal_format)
{
    switch(internal_format)
    {
        case GL_RGB4:
        case GL_RGB5:
            return MTLPixelFormatRGBA8Unorm;  // Upconvert to RGBA8
            
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            return MTLPixelFormatRGBA8Unorm;  // Metal doesn't have RGB-only formats
            
        case GL_RGBA2:
            return MTLPixelFormatRGBA8Unorm;  // Upconvert
            
        case GL_RGBA4:
            return MTLPixelFormatABGR4Unorm;
            
        case GL_RGB5_A1:
            return MTLPixelFormatBGR5A1Unorm;

        case GL_RGBA8:
            return MTLPixelFormatRGBA8Unorm;    // working format
            //return MTLPixelFormatBGRA8Unorm;    // working format

        case GL_R3_G3_B2:
            return MTLPixelFormatRGBA8Unorm;    // Upconvert to RGBA8

        case GL_ALPHA8UI_EXT:
            return MTLPixelFormatR8Uint;        // Map Alpha Integer to Red Integer (best effort)

        case GL_RGB10_A2:
            return MTLPixelFormatRGB10A2Unorm;    // working format

        case GL_RGBA12:
            return MTLPixelFormatInvalid;

        case GL_RGBA16:
            return MTLPixelFormatRGBA16Unorm;    // working format

        case GL_COMPRESSED_RGB:
            if (__builtin_available(macOS 11.0, *))
            {
                return MTLPixelFormatETC2_RGB8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_RGBA8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_DEPTH_COMPONENT16:
            return MTLPixelFormatDepth16Unorm;

        case GL_DEPTH_COMPONENT24:
            // Apple Silicon doesn't support 24-bit depth, use 32-bit float instead
            return MTLPixelFormatDepth32Float;

        case GL_DEPTH_COMPONENT32:
            return MTLPixelFormatDepth32Float;

        case GL_SRGB:
            return MTLPixelFormatInvalid;

        case GL_SRGB8:
            return MTLPixelFormatRGBA8Unorm_sRGB;

        case GL_SRGB_ALPHA:
            return MTLPixelFormatInvalid;

        case GL_SRGB8_ALPHA8:
            return MTLPixelFormatRGBA8Unorm_sRGB;

        case GL_COMPRESSED_SRGB:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB_ALPHA:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RED:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_RG11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_RGBA32F:
            return MTLPixelFormatRGBA32Float;

        case GL_RGB32F:
            return MTLPixelFormatRGBA32Float;

        case GL_RGBA16F:
            return MTLPixelFormatRGBA16Float;

        case GL_RGB16F:
            return MTLPixelFormatRGBA16Float;

        case GL_R11F_G11F_B10F:
            return MTLPixelFormatRG11B10Float;

        case GL_RGB9_E5:
            return MTLPixelFormatRGB9E5Float;

        case GL_RGBA32UI:
            return MTLPixelFormatRGBA32Uint;

        case GL_RGB32UI:
            return MTLPixelFormatRGBA32Uint;

        case GL_RGBA16UI:
            return MTLPixelFormatRG32Uint;

        case GL_RGB16UI:
            return MTLPixelFormatRG32Uint;

        case GL_RGBA8UI:
            return MTLPixelFormatRGBA8Uint;

        case GL_RGB8UI:
            return MTLPixelFormatRGBA8Sint;

        case GL_RGBA32I:
            return MTLPixelFormatRGBA32Sint;

        case GL_RGB32I:
            return MTLPixelFormatRGBA32Sint;

        case GL_RGBA16I:
            return MTLPixelFormatRGBA16Sint;

        case GL_RGB16I:
            return MTLPixelFormatRGBA16Sint;

        case GL_RGBA8I:
            return MTLPixelFormatRGBA8Uint;

        case GL_RGB8I:
            return MTLPixelFormatRGBA8Uint;

        case GL_DEPTH_COMPONENT32F:
            return MTLPixelFormatDepth32Float;

        case GL_DEPTH32F_STENCIL8:
            return MTLPixelFormatDepth32Float_Stencil8;

        case GL_DEPTH24_STENCIL8:
            // MTLPixelFormatX24_Stencil8 (262) is NOT supported on Apple Silicon
            // Use Depth32Float_Stencil8 instead
            return MTLPixelFormatDepth32Float_Stencil8;

        case GL_STENCIL_INDEX1:
            return MTLPixelFormatInvalid;

        case GL_STENCIL_INDEX4:
            return MTLPixelFormatInvalid;

        case GL_STENCIL_INDEX8:
            return MTLPixelFormatStencil8;

        case GL_STENCIL_INDEX16:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RED_RGTC1:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_SIGNED_RED_RGTC1:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RG_RGTC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_R11Snorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_R8:
            return MTLPixelFormatR8Unorm;

        case GL_R16:
            return MTLPixelFormatR16Unorm;

        case GL_RG8:
            return MTLPixelFormatRG8Unorm;

        case GL_RG16:
            return MTLPixelFormatRG16Unorm;

        case GL_R16F:
            return MTLPixelFormatRG16Float;

        case GL_R32F:
            return MTLPixelFormatR32Float;

        case GL_RG16F:
            return MTLPixelFormatRG16Float;

        case GL_RG32F:
            return MTLPixelFormatRG32Float;

        case GL_R8I:
            return MTLPixelFormatR8Sint;

        case GL_R8UI:
            return MTLPixelFormatR16Uint;

        case GL_R16I:
            return MTLPixelFormatR16Sint;

        case GL_R16UI:
            return MTLPixelFormatR16Uint;

        case GL_R32I:
            return MTLPixelFormatR32Sint;

        case GL_R32UI:
            return MTLPixelFormatR32Uint;

        case GL_RG8I:
            return MTLPixelFormatRG32Sint;

        case GL_RG8UI:
            return MTLPixelFormatRG8Uint;

        case GL_RG16I:
            return MTLPixelFormatRG16Sint;

        case GL_RG16UI:
            return MTLPixelFormatRG16Uint;

        case GL_RG32I:
            return MTLPixelFormatRG32Sint;

        case GL_RG32UI:
            return MTLPixelFormatRG32Uint;

        case GL_R8_SNORM:
            return MTLPixelFormatR8Snorm;

        case GL_RG8_SNORM:
            return MTLPixelFormatRG8Snorm;

        case GL_RGB8_SNORM:
            return MTLPixelFormatRGBA8Snorm;

        case GL_RGBA8_SNORM:
            return MTLPixelFormatRGBA8Snorm;

        case GL_R16_SNORM:
            return MTLPixelFormatR16Snorm;

        case GL_RG16_SNORM:
            return MTLPixelFormatRG16Snorm;

        case GL_RGB16_SNORM:
            return MTLPixelFormatRGBA16Snorm;

        case GL_RGBA16_SNORM:
            return MTLPixelFormatRGBA16Snorm;

        case GL_RGB10_A2UI:
            return MTLPixelFormatRGB10A2Uint;

        case GL_RGB565:
            return MTLPixelFormatB5G6R5Unorm;  // Closest match

        // Legacy unsized formats - map to sized equivalents
        case GL_RED:
            return MTLPixelFormatR8Unorm;
            
        case GL_RGBA:
            return MTLPixelFormatRGBA8Unorm;
            
        case GL_RGB:
            return MTLPixelFormatRGBA8Unorm;  // No RGB-only format in Metal
            
        // Legacy luminance/alpha formats - map to R/RG
        case GL_ALPHA8:
        case GL_LUMINANCE8:
            return MTLPixelFormatR8Unorm;
            
        case GL_ALPHA16:
        case GL_LUMINANCE16:
            return MTLPixelFormatR16Unorm;
            
        case GL_ALPHA32F_ARB:
        case GL_LUMINANCE32F_ARB:
            return MTLPixelFormatR32Float;
            
        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE16F_ARB:
            return MTLPixelFormatR16Float;
            
        case GL_LUMINANCE_ALPHA32F_ARB:
            return MTLPixelFormatRG32Float;
            
        case GL_LUMINANCE_ALPHA16F_ARB:
            return MTLPixelFormatRG16Float;
            
        case 0x8045: // GL_LUMINANCE8_ALPHA8
            return MTLPixelFormatRG8Unorm;
            
        // Note: 0x8048 (GL_LUMINANCE16_ALPHA16) already handled by GL_LUMINANCE16 case above
        // due to incorrect macro definition
            
        // sRGB R/RG formats
        case GL_SR8_EXT:
            return MTLPixelFormatR8Unorm_sRGB;
            
        case GL_SRG8_EXT:
            return MTLPixelFormatRG8Unorm_sRGB;

        case GL_COMPRESSED_RGBA_BPTC_UNORM:
            return MTLPixelFormatBC7_RGBAUnorm;

        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
            return MTLPixelFormatBC7_RGBAUnorm_sRGB;

        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
            return MTLPixelFormatBC6H_RGBFloat;

        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            return MTLPixelFormatBC6H_RGBUfloat;

        case GL_COMPRESSED_RGB8_ETC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatETC2_RGB8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ETC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatETC2_RGB8A1;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatETC2_RGB8A1_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            if (__builtin_available(macOS 11.0, *))
            {
                return MTLPixelFormatEAC_RGBA8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_RGBA8_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_R11_EAC:
            if (__builtin_available(macOS 11.0, *))
            {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_R11_EAC:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_R11Snorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG11_EAC:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_RG11Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG11_EAC:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatEAC_RG11Snorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        // Unsized base formats - virglrenderer may pass these
        case GL_RG:
            return MTLPixelFormatRG8Unorm;
            
        case GL_DEPTH_COMPONENT:
            return MTLPixelFormatDepth32Float;
            
        case GL_DEPTH_STENCIL:
            return MTLPixelFormatDepth32Float_Stencil8;
            
        case GL_STENCIL_INDEX:
            return MTLPixelFormatStencil8;

        // Additional integer formats (alternate enum values used by some implementations)
        case 0x8d72: // GL_ALPHA32UI_EXT
            return MTLPixelFormatR32Uint;
        case 0x8d75: // alternate GL_RGB8I
            return MTLPixelFormatRGBA8Sint;
        case 0x8d78: // alternate GL_RGBA8UI
            return MTLPixelFormatRGBA8Uint;
        case 0x8d7a: // alternate GL_RGB8UI
            return MTLPixelFormatRGBA8Uint;
        case 0x8d7b: // GL_ALPHA8I_EXT
            return MTLPixelFormatR8Sint;
        // case 0x8d7e: // GL_ALPHA8UI_EXT - Duplicate
        //    return MTLPixelFormatR8Uint;
        case 0x8d80: // GL_LUMINANCE8UI_EXT
            return MTLPixelFormatR8Uint;
        case 0x8d81: // GL_ALPHA32I_EXT
            return MTLPixelFormatR32Sint;
        case 0x8d84: // alternate GL_RGBA16I
            return MTLPixelFormatRGBA16Sint;
        case 0x8d86: // alternate GL_RGB16I
            return MTLPixelFormatRGBA16Sint;
        case 0x8d87: // GL_ALPHA16I_EXT
            return MTLPixelFormatR16Sint;
        case 0x8d8a: // alternate GL_RGBA32I
            return MTLPixelFormatRGBA32Sint;
        case 0x8d8c: // alternate GL_RGB32I
            return MTLPixelFormatRGBA32Sint;
        
        // SNORM formats
        case 0x9014: // GL_ALPHA8_SNORM
            return MTLPixelFormatR8Snorm;
        case 0x9016: // GL_LUMINANCE8_ALPHA8_SNORM
            return MTLPixelFormatRG8Snorm;
        case 0x9018: // GL_ALPHA16_SNORM
            return MTLPixelFormatR16Snorm;
        case 0x901a: // GL_LUMINANCE16_ALPHA16_SNORM
            return MTLPixelFormatRG16Snorm;
        case 0x8d8d: // GL_ALPHA32I_EXT
            return MTLPixelFormatR32Sint;
        case 0x8d90: // alternate GL_RGBA16UI
            return MTLPixelFormatRGBA16Uint;
        case 0x8d92: // alternate GL_RGB16UI
            return MTLPixelFormatRGBA16Uint;
        case 0x8d93: // GL_ALPHA16UI_EXT
            return MTLPixelFormatR16Uint;

        default:
            // Unknown formats - likely Mesa/Gallium internal format enums or capability probes
            // Return Invalid to indicate format not supported (don't use fallback for probes)
            // Only warn for formats that look like real GL formats (not obvious enum values)
            if (internal_format >= 0x1 && internal_format <= 0x2000)
            {            
                // Low values might be legacy GL formats - warn about these
                static unsigned warned_formats[64] = {0};
                static int warned_count = 0;
                int already_warned = 0;
                for (int i = 0; i < warned_count && i < 64; i++)
                {            
                    if (warned_formats[i] == internal_format)
                    {
                        already_warned = 1;
                        break;
                    }
                }
                if (!already_warned && warned_count < 64)
                {            
                    warned_formats[warned_count++] = internal_format;
                    fprintf(stderr, "MGL WARNING: mtlFormatForGLInternalFormat unknown format 0x%x\n", internal_format);
                }
            }
            // For 0x8Dxx and 0x90xx ranges - these are often internal/capability probes
            // Silently return Invalid to indicate "not supported"
            return MTLPixelFormatInvalid;
    }

    return MTLPixelFormatInvalid;
}

MTLPixelFormat mtlPixelFormatForGLFormatType(GLenum gl_format, GLenum gl_type)
{
    switch(gl_type)
    {
        case GL_UNSIGNED_BYTE:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR8Uint;
                case GL_RG: return MTLPixelFormatRG8Uint;
                case GL_RGBA: return MTLPixelFormatRGBA8Unorm;
                default:
                    return 0;
            }
            break;

        case GL_BYTE:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR8Sint;
                case GL_RG: return MTLPixelFormatRG8Sint;
                case GL_RGBA: return MTLPixelFormatRGBA8Sint;
                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_SHORT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR16Uint;
                case GL_RG: return MTLPixelFormatRG16Uint;
                case GL_RGBA: return MTLPixelFormatRGBA16Uint;
                default:
                    return 0;
            }
            break;

        case GL_SHORT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR16Sint;
                case GL_RG: return MTLPixelFormatRG16Sint;
                case GL_RGBA: return MTLPixelFormatRGBA16Sint;
                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_INT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR32Uint;
                case GL_RG: return MTLPixelFormatRG32Uint;
                case GL_RGBA: return MTLPixelFormatRGBA32Uint;
                case GL_BGRA: return MTLPixelFormatRGBA32Uint;
                default:
                    return 0;
            }
            break;

        case GL_INT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR32Uint;
                case GL_RG: return MTLPixelFormatRG32Uint;
                case GL_RGBA: return MTLPixelFormatRGBA32Uint;
                default:
                    return 0;
            }
            break;

        case GL_FLOAT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR32Float;
                case GL_RG: return MTLPixelFormatRG32Float;
                case GL_RGBA: return MTLPixelFormatRGBA32Float;
                case GL_DEPTH_COMPONENT: return MTLPixelFormatDepth32Float;
                case GL_DEPTH_STENCIL: return MTLPixelFormatDepth24Unorm_Stencil8;

                default:
                    return 0;
            }
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
            return 0;

        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return 0;

        case GL_UNSIGNED_SHORT_5_6_5:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatB5G6R5Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_UNSIGNED_SHORT_5_6_5_REV:
            if (__builtin_available(macOS 11.0, *))
            {            
                return MTLPixelFormatA1BGR5Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            return 0;

        case GL_UNSIGNED_INT_8_8_8_8:
            return MTLPixelFormatRGBA8Unorm;

        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return MTLPixelFormatBGRA8Unorm;

        case GL_UNSIGNED_INT_10_10_10_2:
            return MTLPixelFormatRGB10A2Unorm;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return MTLPixelFormatBGR10A2Unorm;
            return 0;

        default:
            assert(0);
    }
}

MTLPixelFormat mtlPixelFormatForGLTex(Texture * tex)
{
    MTLPixelFormat mtl_format;
    GLenum internal_format;

    assert(tex);

    internal_format = tex->internalformat;
    assert(internal_format);

    mtl_format = mtlFormatForGLInternalFormat(internal_format);
    assert(mtl_format);

    return mtl_format;
}

