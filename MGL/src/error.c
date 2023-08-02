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
 * error.h
 * MGL
 *
 */

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

struct token_string
{
   GLuint Token;
   const char* String;
};

#define ERR_MSG(error) {error, #error}

static const struct token_string Errors[]=
{
   ERR_MSG(GL_NO_ERROR),
   ERR_MSG(GL_INVALID_ENUM),
   ERR_MSG(GL_INVALID_VALUE),
   ERR_MSG(GL_INVALID_OPERATION),
   ERR_MSG(GL_STACK_OVERFLOW),
   ERR_MSG(GL_STACK_UNDERFLOW),
   ERR_MSG(GL_OUT_OF_MEMORY),
   ERR_MSG(GL_INVALID_FRAMEBUFFER_OPERATION),
   ERR_MSG(GL_CONTEXT_LOST),
   // ERR_MSG(GL_TABLE_TOO_LARGE),

   { ~0, NULL } /* end of list indicator */
};


GLAPI const GLubyte* APIENTRY gluErrorString(GLenum errorCode)
{
   int i;

   for (i=0; Errors[i].String; i++)
   {
      if (Errors[i].Token==errorCode)
      {
         return (const GLubyte*) Errors[i].String;
      }
   }

   printf("unknown GL error %d\n", errorCode);
   return (const GLubyte*)0;
}


void error_func(GLMContext ctx, const char *funcname, const char *filename, unsigned int line, GLenum error)
{
    printf("GL Error func: %s type: %d name: %s file: %s:%d\n", funcname, error, gluErrorString(error), filename, line);

    if (ctx->state.error)
        return;

    ctx->state.error = error;

    if (ctx->assert_on_error)
        assert(0);
}
