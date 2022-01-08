# MGL
OpenGL 4.6 on Metal

This is a start for porting OpenGL 4.6 on top of Metal, most of it is functional and has been tested. The tests are functional, not coverage tests so they test the functionality of a path not all the possible permutations.

So far the following work

Vertex Arrays
Buffers
Textures
Programs
  Shaders
    Vertex
    Fragment
    Compute
Samplers
Alpha Blending
Depth and Stencil Testing
Framebuffer Objects
Drawbuffers
Most of the draw calls
Elements / Instancing and such

I modified a version of GLFW to work with MGL, it replaces the default MacOS OpenGL contexts.

I mapped as much of the functionality of Metal I could into OpenGL 4.6, it's surprising how much I was able to map directly from OpenGL state to Metal. But I suppose Metal evolved to meet market requirements and since OpenGL was in place the same features existed just in another form.


In the beginning I used ezxml to parse the gl.xml file for all the enums and functions, then printed out one giant file with all the functions. I then used the same parser to create the dispatch tables and data structures.

Each OpenGL function starts in gl_core.c

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    GLMContext ctx = GET_CONTEXT();

    ctx->dispatch.tex_image2D(ctx, target, level, internalformat, width, height, border, format, type, pixels);
}

glTexImage2D calls into a dispatch table which lands on a mgl equivalent 

void mglTexImage2D(GLMContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
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

The mgl function checks input parameters then modifies the OpenGL state and marks it dirty.

Most functions are like mglTexImage2D, there a lot of common entry points which check parameters then call a function like createTextureLevel() which is used by all the TexImage calls to do the actual work.

All state changes are evaluated on direct commands, these are commands that will issue a command to the GPU for things like drawing primitives, copying data or executing compute kernels.

Looking at glDrawArrays..

glDrawArrays lands on mglDrawArrays which does parameter testing and calls the Objective C interface through a jump table in the context. This lands you in MGLRenderere.m

void mglDrawArrays(GLMContext ctx, GLenum mode, GLint first, GLsizei count)
{
    ERROR_CHECK_RETURN(check_draw_modes(mode), GL_INVALID_ENUM);

    ERROR_CHECK_RETURN(count > 1, GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(validate_vao(ctx), GL_INVALID_VALUE);

    ERROR_CHECK_RETURN(validate_program(ctx), GL_INVALID_VALUE);

    ctx->mtl_funcs.mtlDrawArrays(ctx, mode, first, count);
}

Which lands you in Objective C land, we do a C interface call to Objecitve C here

void mtlDrawArrays(GLMContext glm_ctx, GLenum mode, GLint first, GLsizei count)
{
    // Call the Objective-C method using Objective-C syntax
    [(__bridge id) glm_ctx->mtl_funcs.mtlObj mtlDrawArrays: glm_ctx mode: mode first: first count: count];
}

And now we speak Objective C

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

On GL calls the OpenGL state is processed for any dirty state in processGLState, you have to do this before each draw command to ensure changed state is captured into Metal.

State is transferred from OpenGL to Metal state using a Metal command queue with a MTLRenderCommandEncoder. Occationally a new MTLRenderCommandEncoder will need to be build if the state changes need to modifie the MTLRenderPipelineState so neRenderEncoder is called to create a new encoder with the current OpenGL state.

Most of the work is done in MGLRenderer.m, at first it can look like a giant piece of code. But it's simple.

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









There is an Xcode project build most of this, but you will need to clone a bunch of external projects from GitHub.

In MGL/external there is a file

clone_external.sh

Run this and all the external repositories will be downloaded.

"drwxr-xr-x  68 developer  staff  2176 Jan  7 15:06 SPIRV-Cross"
"drwxr-xr-x  19 developer  staff   608 Jan  7 15:07 SPIRV-Headers"
"drwxr-xr-x  35 developer  staff  1120 Jan  7 15:10 SPIRV-Tools"
"-rwxr-xr-x   1 developer  staff   503 Jan  7 15:17 clone_external.sh"
"drwxr-xr-x  13 developer  staff   416 Jan  7 14:58 ezxml"
"drwxr-xr-x  17 developer  staff   544 Dec 19 16:54 glew"
"drwxr-xr-x  15 developer  staff   480 Jan  7 15:06 glfw"
"drwxr-xr-x  16 developer  staff   512 Dec 21 13:59 glm"
"drwxr-xr-x  43 developer  staff  1376 Jan  7 15:12 glslang"

You will need to build the following
SPIRV-Cross
SPIRV-Headers
SPIRV-Tools
glslang

Each is distributed by Khronos with a CMake style build

For each of the projects above build you will need to do the following

cd <Project>
mkdir build
cd build
cmake ..
make
make install

Once installed in /usr/local the Xcode project should be able to build all the required dependencies for MGL.


Start by building test_mgl_glfw, this is a chunk of test code I used to get most of the functionality up and running.
  
  
If you want to contribute that would be great, it's all written in C.. in the same style all of the OpenGL framework from Apple was written in. If you don't like the coding style, don't change it. Just follow the same coding style and put your efforts into testing and functionality.
  

You can reach me at sandstormsoftware@gmail.com for more information, it would be great to see this used by others and developed into a full fledged project anyone can use.
  
Cheers
  
Mike
  
  


