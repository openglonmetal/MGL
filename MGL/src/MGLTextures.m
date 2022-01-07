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
 * MGLTextures.m
 * MGL
 *
 */

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#import "glm_context.h"

// file should be deprecated, moved to pixel_utils so it can be seen from C

#if 0
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
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_BYTE:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR8Sint;
                case GL_RG: return MTLPixelFormatRG8Sint;
                case GL_RGBA: return MTLPixelFormatRGBA8Sint;
                default:
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_UNSIGNED_SHORT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR16Uint;
                case GL_RG: return MTLPixelFormatRG16Uint;
                case GL_RGBA: return MTLPixelFormatRGBA16Uint;
                default:
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_SHORT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR16Sint;
                case GL_RG: return MTLPixelFormatRG16Sint;
                case GL_RGBA: return MTLPixelFormatRGBA16Sint;
                default:
                    return MTLPixelFormatInvalid;
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
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_INT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR32Uint;
                case GL_RG: return MTLPixelFormatRG32Uint;
                case GL_RGBA: return MTLPixelFormatRGBA32Uint;
                default:
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_FLOAT:
            switch(gl_format)
            {
                case GL_RED: return MTLPixelFormatR32Float;
                case GL_RG: return MTLPixelFormatRG32Float;
                case GL_RGBA: return MTLPixelFormatRGBA32Float;
                case GL_DEPTH_COMPONENT: return MTLPixelFormatDepth32Float;
                case GL_DEPTH_STENCIL: return MTLPixelFormatDepth32Float_Stencil8;

                default:
                    return MTLPixelFormatInvalid;
            }
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
            return MTLPixelFormatInvalid;

        case GL_UNSIGNED_BYTE_2_3_3_REV:
            return MTLPixelFormatInvalid;

        case GL_UNSIGNED_SHORT_5_6_5:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatB5G6R5Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_UNSIGNED_SHORT_5_6_5_REV:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatA1BGR5Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            return MTLPixelFormatInvalid;

        case GL_UNSIGNED_INT_8_8_8_8:
            return MTLPixelFormatRGBA8Unorm;

        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return MTLPixelFormatBGRA8Unorm;

        case GL_UNSIGNED_INT_10_10_10_2:
            return MTLPixelFormatRGB10A2Unorm;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return MTLPixelFormatBGR10A2Unorm;

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
            return MTLPixelFormatRGBA8Unorm;    // working format

        case GL_RGB10_A2:
            return MTLPixelFormatRGB10A2Unorm;    // working format

        case GL_RGBA12:
            return MTLPixelFormatInvalid;

        case GL_RGBA16:
            return MTLPixelFormatRGBA16Unorm;    // working format

        case GL_COMPRESSED_RGB:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8;
            } else {
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
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB_ALPHA:
            return MTLPixelFormatInvalid;

        case GL_COMPRESSED_RED:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG:
            if (@available(macOS 11.0, *)) {
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
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG_RGTC2:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Snorm;
            } else {
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
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ETC2:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8A1;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatETC2_RGB8A1_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RGBA8_sRGB;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_R11_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_R11_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_R11Snorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_RG11_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RG11Unorm;
            } else {
                return MTLPixelFormatInvalid;
            }

        case GL_COMPRESSED_SIGNED_RG11_EAC:
            if (@available(macOS 11.0, *)) {
                return MTLPixelFormatEAC_RG11Snorm;
            } else {
                return MTLPixelFormatInvalid;
            }
    }

    return MTLPixelFormatInvalid;
}

MTLPixelFormat mtlPixelFormatForGLTex(Texture * tex)
{
    MTLPixelFormat mtl_format;
    GLenum internal_format;

    assert(tex);

    internal_format = tex->internalformat;

    mtl_format = mtlFormatForGLInternalFormat(internal_format);

    if (mtl_format == MTLPixelFormatInvalid)
    {
        GLuint format, type;

        printf("Internal format 0x%x failed\n", internal_format);

        format = tex->format;
        type = tex->type;

        printf("format 0x%x type 0x%x\n", format, type);

        return mtlPixelFormatForGLFormatType(format, type);
    }

    // cant get here
    return mtl_format;
}

#endif

