//
//  pixel_utils.c
//  mgl
//
//  Created by Michael Larson on 11/19/21.
//

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
