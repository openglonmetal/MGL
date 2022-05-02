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

GLuint numComponentsForFormat(GLenum format)
{
    switch(format)
    {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
            return 1;

        case GL_RG:
        case GL_RG_INTEGER:
            return 2;

        case GL_RGB:
        case GL_BGR:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
            return 3;

        case GL_RGBA:
        case GL_BGRA:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
            return 4;

        default:
            assert(0);
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
            assert(0);
    }

    return 0;
}

GLuint sizeForFormatType(GLenum format, GLenum type)
{
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

        default:
            assert(0);
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
                case GL_RGBA: return GL_RGBA8;
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
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
            return MTLPixelFormatInvalid;

        case GL_RGBA8:
            //return MTLPixelFormatRGBA8Unorm;    // working format
            return MTLPixelFormatBGRA8Unorm;    // working format

        case GL_RGB10_A2:
            return MTLPixelFormatRGB10A2Unorm;    // working format

        case GL_RGBA12:
            return MTLPixelFormatInvalid;

        case GL_RGBA16:
            return MTLPixelFormatRGBA16Unorm;    // working format

        case GL_COMPRESSED_RGB:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_DEPTH_COMPONENT16:
            return MTLPixelFormatDepth16Unorm;

        case GL_DEPTH_COMPONENT24:
            return MTLPixelFormatInvalid;

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
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB_ALPHA:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RED:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG:
            if (__builtin_available(macOS 11.0, *)) {
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
            return MTLPixelFormatX24_Stencil8;

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
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            if (__builtin_available(macOS 11.0, *)) {
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
            return MTLPixelFormatInvalid;

        case GL_RGB565:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RGBA_BPTC_UNORM:
            return MTLPixelFormatBC7_RGBAUnorm;

        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
            return MTLPixelFormatBC7_RGBAUnorm_sRGB;

        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
            return MTLPixelFormatBC6H_RGBFloat;

        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            return MTLPixelFormatBC6H_RGBUfloat;

        case GL_COMPRESSED_RGB8_ETC2:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ETC2:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8A1;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8A1_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8_sRGB;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_R11_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_R11_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Snorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG11_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RG11Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG11_EAC:
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RG11Snorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        default:
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
            if (__builtin_available(macOS 11.0, *)) {
                return MTLPixelFormatB5G6R5Unorm;
            } else {
                // Fallback on earlier versions
                return MTLPixelFormatInvalid;
            }

        case GL_UNSIGNED_SHORT_5_6_5_REV:
            if (__builtin_available(macOS 11.0, *)) {
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

