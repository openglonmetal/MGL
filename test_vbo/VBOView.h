//
//  TestVBOView.h
//  test_vbo
//
//  Created by Michael Larson on 1/25/26.
//

#import "MGLView.h"

NS_ASSUME_NONNULL_BEGIN

enum {
    kDrawArrays = 0,
    kDrawElements,
    kDrawRangeElements,
    kDrawArraysInstanced,
    kDrawElementsInstanced,
    kRandomPrimitiveType,
    kMaxPrimitiveDrawType
};

/* list of draw calls to be tested
 glm_ctx->mtl_funcs.mtlDrawArrays = mtlDrawArrays;
 glm_ctx->mtl_funcs.mtlDrawElements = mtlDrawElements;
 glm_ctx->mtl_funcs.mtlDrawRangeElements = mtlDrawRangeElements;
 
 glm_ctx->mtl_funcs.mtlDrawArraysInstanced = mtlDrawArraysInstanced;
 glm_ctx->mtl_funcs.mtlDrawElementsInstanced = mtlDrawElementsInstanced;
 
 glm_ctx->mtl_funcs.mtlDrawElementsBaseVertex = mtlDrawElementsBaseVertex;
 glm_ctx->mtl_funcs.mtlDrawRangeElementsBaseVertex = mtlDrawRangeElementsBaseVertex;
 glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseVertex = mtlDrawElementsInstancedBaseVertex;
 glm_ctx->mtl_funcs.mtlMultiDrawElementsBaseVertex = mtlMultiDrawElementsBaseVertex;
 glm_ctx->mtl_funcs.mtlDrawArraysIndirect = mtlDrawArraysIndirect;
 glm_ctx->mtl_funcs.mtlDrawElementsIndirect = mtlDrawElementsIndirect;
 glm_ctx->mtl_funcs.mtlDrawArraysInstancedBaseInstance = mtlDrawArraysInstancedBaseInstance;
 glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseInstance = mtlDrawElementsInstancedBaseInstance;
 glm_ctx->mtl_funcs.mtlDrawElementsInstancedBaseVertexBaseInstance = mtlDrawElementsInstancedBaseVertexBaseInstance;

 glm_ctx->mtl_funcs.mtlMultiDrawArrays = mtlMultiDrawArrays;
 glm_ctx->mtl_funcs.mtlMultiDrawElements = mtlMultiDrawElements;
 glm_ctx->mtl_funcs.mtlMultiDrawElementsBaseVertex = mtlMultiDrawElementsBaseVertex;
 glm_ctx->mtl_funcs.mtlMultiDrawArraysIndirect = mtlMultiDrawArraysIndirect;
 glm_ctx->mtl_funcs.mtlMultiDrawElementsIndirect = mtlMultiDrawElementsIndirect;

 
 */
@interface VBOView : MGLView
{
    BOOL m_first_pass;
    
    GLboolean m_4x_mode;
    
    GLboolean m_instanced_mode;
    
    GLuint m_vao, m_vbo[16], m_elements[16];
    
    GLuint m_shader_program[16];

    GLuint m_instanced_shader_program;

    GLuint m_primiteDrawType;
    
    int m_a;
    int m_e;
}

-(void)setPrimitveDrawType: (GLuint) type;
-(void)set3x_4x_mode: (GLboolean) mode;

@end

NS_ASSUME_NONNULL_END
