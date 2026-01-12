# MGL
# OpenGL 4.6 and ES 3.x on Metal

This is a start for porting OpenGL 4.6 and ES 3.x on top of Metal, most of it is functional and has been tested. The tests are functional, not coverage tests so they test the functionality of a path not all the possible permutations.

## So far the following parts of OpenGL work

- Vertex Arrays
- Buffers
- Textures
- Programs
  - Shaders
    - Vertex
    - Fragment
    - Compute
- Samplers
- Alpha Blending
- Depth and Stencil Testing
- Framebuffer Objects
- Drawbuffers
- Most of the draw calls
- Elements / Instancing and such
- Uniforms (partially)
- UBOs

## GLFW support
I modified a version of GLFW to work with MGL, it replaces the default MacOS OpenGL contexts. The changes are included in the repository and should build correctly with the MGL xcode project.

(additional notes from conversy/MGL repo: the modified GLFW version is not mandatory, see below).

## Mapping OpenGL onto Metal
I mapped as much of the functionality of Metal I could into OpenGL 4.6, it's surprising how much I was able to map directly from OpenGL state to Metal. But I suppose Metal evolved to meet market requirements and since OpenGL was in place the same features existed just in another form.

Not all of the functionality for OpenGL is available, but I didn't build this for conformance I just wanted to program OpenGL on the MacOS platform.

Metal Hiearchy

    Command Queue
        Command Buffer
            Render Pipeline State <- created with RenderPipelineDescriptor
                Render Encoder  <-- created with RenderPassDescriptor
                    RenderPassDescriptor
                        colorAttachments
                            pixelFormat
                            sourceRGBBlendFactor
                            sourceAlphaBlendFactor
                            destinationRGBBlendFactor
                            destinationAlphaBlendFactor
                            alphaBlendOperation
                            writeMask
                        depthAttachment
                            pixelFormat
                        stencilAttachment
                            pixelFormat
                        visibilityResultBuffer
                        renderTargetArrayLength
                        defaultRasterSampleCount
                        samplePositions
                        renderTargetWidth
                        renderTargetHeight

                    
                DirectState for Render Encoder
                    Vertex Function
                    Fragment Function
                    Vertex Descriptor
                    Vertex Buffers
                    Vertex Textures
                    Viewport
                    FrontFacingWinding
                    CullMode
                    DepthClipMode
                    DepthBias
                    ScissorRect
                    TriangleFillMode
                    Fragment Buffers
                    Fragment Textures
                    Fragment Samplers
                    BlendColor
                    StencilReferenceValue
                    VisibilityResultMode
                    ColorStoreMode
                    DepthStoreMode
                    DepthStoreAction
                    StencilStoreAction
                    DepthStoreActionOptions
                    StencilStoreActionOptions
                    
                Descriptor State used to change render ecoder
                    Depth Stencil State <- created with DepthStencil Descriptor
                        DepthStencilPipeline Descriptor
                            DepthStencil Descriptor
                                readMask
                                writeMask
                                depthCompareFunction
                                depthWriteEnabled
                                stencilCompareFunction
                                stencilFailureOperation
                                depthFailureOperation
                                depthStencilPassOperation
                                frontFaceStencil <- created wtih StencilDescriptor
                                backFaceStencil <- created wtih StencilDescriptor
                                
                    

## Parsing the OpenGL 4.6 XML spec
In the beginning I used ezxml to parse the gl.xml file for all the enums and functions, then printed out one giant file with all the functions. I then used the same parser to create the dispatch tables and data structures. As each functional part was built I separated blocks of functions into these functions like buffers / textures / shaders / programs.

## SPIRV to Metal
I really couldn't have done this project without all the SPIRV support from Khronos, once I found out I could translate GLSL into Metal using some of the SPIRV tools this project became a reality. There are some parts in GLSL that Metal just doesn't support like Geometry Shaders, I think the way forward on support for Geometry Shaders is to translate the vertex and geometry shaders into LLVM from SPIRV then execute the results from processing them on the CPU through a passthrough vertex shader in the pipeline. Some parts of the GLSL spec probably won't map to Metal but more testing and exposure to developers will show what works and what doesn't/

## OpenGL functions and how they work
Each OpenGL function starts in gl_core.c

```C
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLMContext ctx = GET_CONTEXT();

    ctx->dispatch.tex_image2D(ctx, target, level, internalformat, width, height, border, format, type, pixels);
}
```

glTexImage2D calls into a dispatch table which lands on a mgl equivalent 

```C
void mglTexImage2D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    Texture *tex;
    GLuint face;
    GLboolean is_array;
    GLboolean proxy;

    face = 0;
    is_array = false;
    proxy = false;

    switch(target)
    {
        case GL_TEXTURE_2D:
            break;

        case GL_PROXY_TEXTURE_2D:
        case GL_PROXY_TEXTURE_CUBE_MAP:
            proxy = true;
            break;

        case GL_PROXY_TEXTURE_1D_ARRAY:
            is_array = true;
            proxy = true;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            break;

        case GL_PROXY_TEXTURE_RECTANGLE:
            proxy = true;
            ERROR_CHECK_RETURN(level==0, GL_INVALID_OPERATION);
            break;

        case GL_TEXTURE_RECTANGLE:
            ERROR_CHECK_RETURN(level==0, GL_INVALID_OPERATION);
            break;

        default:
            ERROR_RETURN(GL_INVALID_ENUM);
    }

    ERROR_CHECK_RETURN(level >= 0, GL_INVALID_VALUE);

    // verifyFormatType sets the error
    ERROR_CHECK_RETURN(verifyInternalFormatAndFormatType(ctx, internalformat, format, type), 0);

    ERROR_CHECK_RETURN(width >= 0, GL_INVALID_VALUE);
    ERROR_CHECK_RETURN(height >= 0, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(border == 0, GL_INVALID_VALUE);

    tex = getTex(ctx, 0, target);

    ERROR_CHECK_RETURN(tex, GL_INVALID_OPERATION);

    tex->access = GL_READ_ONLY;

    createTextureLevel(ctx, tex, face, level, is_array, internalformat, width, height, 1, format, type, (void *)pixels, proxy);
}
```

The mgl function checks input parameters then modifies the OpenGL state and marks it dirty. I use macros for error checking parameters... because I got sick of writing.

```C
if (expr == false) {
  set error
  return;
}
```

## Common use of code for most OpenGL calls
Most functions are like mglTexImage2D, there a lot of common entry points which check parameters then call a function like createTextureLevel() which is used by all the TexImage calls to do the actual work.

## OpenGL state translated to Metal using glDrawArrays as an example 
All state changes are evaluated on direct commands, these are commands that will issue a command to the GPU for things like drawing primitives, copying data or executing compute kernels.

Looking at glDrawArrays..

glDrawArrays lands on mglDrawArrays which does parameter testing and calls the Objective C interface through a jump table in the context. This lands you in MGLRenderere.m

```C
void mglDrawArrays(GLMContext ctx, GLenum mode, GLint first, GLsizei count)
{
    ERROR_CHECK_RETURN(check_draw_modes(mode), GL_INVALID_ENUM);

    ERROR_CHECK_RETURN(count > 1, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(validate_vao(ctx), GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(validate_program(ctx), GL_INVALID_VALUE);

    ctx->mtl_funcs.mtlDrawArrays(ctx, mode, first, count);
}
```

Which lands you in Objective C land, we do a C interface call to Objecitve C here

```C
void mtlDrawArrays(GLMContext glm_ctx, GLenum mode, GLint first, GLsizei count)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArrays: glm_ctx mode: mode first: first count: count];
}
```

And now we speak Objective C

```C
-(void) mtlDrawArrays: (GLMContext) ctx mode:(GLenum) mode first: (GLint) first count: (GLsizei) count
{
    MTLPrimitiveType primitiveType;

    RETURN_ON_FAILURE([self processGLState: true]);

    primitiveType = getMTLPrimitiveType(mode);
    assert(primitiveType != 0xFFFFFFFF);

     [_currentRenderEncoder drawPrimitives: primitiveType
                              vertexStart: first
                              vertexCount: count];
}
```

## processGLState does all the state mapping from OpenGL to Metal
On OpenGL drawing calls the OpenGL state is processed for any dirty state in processGLState, you have to do this before each draw command to ensure changed state is captured into Metal.

State is transferred from OpenGL to Metal state using a Metal command queue with a MTLRenderCommandEncoder. Occasionally a new MTLRenderCommandEncoder will need to be built if the state changes need to modify the MTLRenderPipelineState so neRenderEncoder is called to create a new encoder with the current OpenGL state.

Most of the work is done in MGLRenderer.m, at first it can look like a giant piece of code. But it's simple once you figure out the mapping process.

# binding buffers and textures from shader layouts
You have to bind all the buffers and textures to Metal, there are mapping operations you need to do to transfer OpenGL buffers / textures to Metal. Since all of this is driven by the GLSL shader in 4.5 which requires you to map your bindings and locations

An example vertex shader defines a vertex buffer object at location 0 and a uniform buffer at binding 0.  These are not the same location, but relative to the type.

    const char* vertex_shader =
    GLSL(450 core,
         layout(location = 0) in vec3 position;

         layout(binding = 0) uniform matrices
         {
             mat4 rotMatrix;
         };

         void main() {
            gl_Position = rotMatrix * vec4(position, 1.0);
         }
    );

Each binding point in OpenGL is a 2D array one for each type, like GL_ARRAY_BUFFER

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

Indexes an array like

    buffers[GL_ARRAY_BUFFER].buffer = vbo

So internal to processGLState the vertex buffer array is taken apart, buffers and attributes mapped then we walk through uniforms / textures through their binding points in the GL state and map these to Metal state.

Once you walk a glDrawArrays call through processGLState, you will get a jist of how this all works.

## Build
See BUILD.md

## Where to start
Use the Xcode MGL project to build your own tests and projects... start by building test_mgl_glfw, this is a chunk of test code I used to get most of the functionality up and running. Xcode has all the debugging tools and won't leave you wondering WTF is that assert about, throwing your hands up and walking away without learning anything about the internals of OpenGL or contributing to this project.

## Performance
I updated most of the immutible objects to allocate metal object up front, this is how to avoid the deferred allocation used in OpenGL and increase performance. The performance on simple tests using a FBO / draw element instance / uniform update in a simple test I wrote is negligiable.. you had to run the loop over 100,000,000 times to extract any difference.
So.. it should be good.

## Missing functions
There are a lot of missing functions, if you open up XCode and look at the project you will see many functions defined and laid out but are just bracketed around an assert(0); If you want functionality just work with the XCode project with test_mgl_glfw.. pick a test like test_2d_array_textures and walk through each function using the debugger and you will get a gist on how it all works.
This is the best way to start adding functionality, until you open it up it will remain a black box. From there just read the GL spec for the function you need, add it in bits by building a test for it and verifying the functionality.

## Why the focus on 4.6 functionality?
OpenGL is huge, and the effort was intended capture 4.6 functionality rather than try to implement it all (and that includes a lot of 3.x functionality). This is a good path to embed functionality into the base then add in all the older functionality later using the paths you want rather than hacking apart older paths to make modern OpenGL functionality.

## Why add ES 3.x support?
Well because people were asking for it, ES 3.x is a subset of the OpenGL core. It builds along side the OpenGL 4.6 core with the ES 3.x restictions in place and a separate library for just ES 3.x functions to live in. I am not a ES 3.x user but I hope to get more testing done on ES 3.x soon.

## Contributing
If you want to contribute that would be great, it's all written in C.. in the same style all of the OpenGL framework from Apple was written in. If you don't like the coding style, don't change it. Just follow the same coding style and put your efforts into testing and functionality.

We really need people to contribute to the functionality and testing rather than trying to just build MGL and see if it works with their application. If you can write C code you can probably figure out how MGL works and contribute.

Its a great way to learn the internals of OpenGL and Metal at the same time, and if you are a college student.. or even an expirienced coder its going to look great on a resume that you actually did work on OpenGL internals and its not just a black box to you. A typical driver for a graphics driver will have hundreds of thousands of lines of code and its all state driven regardless of what they say about new interfaces like Metal or Vulkan, because hardware doesn't match these interfaces directly you have to evaluate state like OpenGL does but at a more abstract level.

## Future
I will continue to implement functionality as needed, I had a goal of implementing OpenGL 4.6 functional paths for buffers, textures, shaders (vertex, fragment, and compute shaders) to release it. Now that that is done my own application uses it and it works fine. But more tests and bugs need to written to bring it to a stage that it can be just compiled and distributed with a Makefile.

## Questions?
You can reach me at sandstormsoftware@gmail.com for more information, it would be great to see this used by others and developed into a full fledged project anyone can use.
  
Cheers
  
Mike
  
  


