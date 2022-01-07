//
//  MGLRenderer.h
//  mgl
//
//  Created by Michael Larson on 11/8/21.
//

#ifndef MGLRenderer_h
#define MGLRenderer_h

#ifdef __OBJC__

#ifndef __GLM_CONTEXT_
#define __GLM_CONTEXT_
typedef struct GLMContextRec_t *GLMContext;
#endif

@interface MGLRenderer : NSObject

- (void) createMGLRendererAndBindToContext: (GLMContext) glm_ctx view: (NSView *) view;

@end
MTLPixelFormat mtlPixelFormatForGLFormatType(GLenum gl_format, GLenum gl_type);
#else

#ifdef __cplusplus
extern "C" {
#endif

GLenum mtlPixelFormatForGLFormatType(GLenum gl_format, GLenum gl_type);

#ifdef __cplusplus
}
#endif

#endif // #ifdef __OBJC__




#endif /* MGLRenderer_h */
