//
//  pixel_utils.h
//  MGL
//
//  Created by Michael Larson on 12/20/21.
//

#ifndef pixel_utils_h
#define pixel_utils_h

#include <glcorearb.h>

GLuint numComponentsForFormat(GLenum format);
GLuint sizeForType(GLenum type);
GLuint sizeForFormatType(GLenum format, GLenum type);
GLuint sizeForInternalFormat(GLenum internalformat, GLenum format, GLenum type);
GLuint bicountForFormatType(GLenum format, GLenum type, GLenum component);
GLenum internalFormatForGLFormatType(GLenum format, GLenum type);


#endif /* pixel_utils_h */
