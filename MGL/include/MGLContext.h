/*
  MGLContext.h
  mgl

  Created by Michael Larson on 12/17/21.
*/

#ifndef MGLContext_h
#define MGLContext_h

// probably a reason for this... can't remember
#ifndef __GLM_CONTEXT_
#define __GLM_CONTEXT_
typedef struct GLMContextRec_t *GLMContext;
#endif

enum {
    MGL_PIXEL_FORMAT,
    MGL_PIXEL_TYPE,
    MGL_DEPTH_FORMAT,
    MGL_DEPTH_TYPE,
    MGL_STENCIL_FORMAT,
    MGL_STENCIL_TYPE,
    MGL_CONTEXT_FLAGS
};

#ifdef __cplusplus
extern "C" {
#endif

GLMContext createGLMContext(GLenum format, GLenum type,
                            GLenum depth_format, GLenum depth_type,
                            GLenum stencil_format, GLenum stencil_type);

GLuint sizeForFormatType(GLenum format, GLenum type);
GLuint bicountForFormatType(GLenum format, GLenum type, GLenum component);

GLMContext MGLgetCurrentContext(void);
void MGLsetCurrentContext(GLMContext ctx);

// MGLswapBuffers can take NULL for the ctx, in this case it will use the current ctx
void MGLswapBuffers(GLMContext ctx);

// MGLget can take NULL for the ctx, in this case it will use the current ctx
void MGLget(GLMContext ctx, GLenum param, GLuint *data);

#ifdef __cplusplus
};
#endif

#endif
