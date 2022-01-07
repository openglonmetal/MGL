//
//  error.h
//  MGL
//
//  Created by Michael Larson on 12/20/21.
//

#ifndef error_h
#define error_h

#include "glm_context.h"

void error_func(GLMContext ctx, const char *func, GLenum error);

GLenum  mglGetError(GLMContext ctx);

#endif /* error_h */
