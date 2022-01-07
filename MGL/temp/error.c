//
//  error.c
//  mgl
//
//  Created by Michael Larson on 11/2/21.
//

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "error.h"


GLenum  mglGetError(GLMContext ctx)
{
    GLenum err;

    err = ctx->state.error;

    ctx->state.error = GL_NO_ERROR;

    return err;
}


void error_func(GLMContext ctx, const char *func, GLenum error)
{
    printf("GL Error func: %s type: %d\n", func, error);

    if (ctx->state.error)
        return;

    ctx->state.error = error;

    if (ctx->assert_on_error)
        assert(0);
}
