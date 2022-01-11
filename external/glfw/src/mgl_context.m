/*
 * Michael Larson on 1/6/2022
 *
 * mgl_context.m
 * GLFW
 *
 */

#import <QuartzCore/QuartzCore.h>

#include <MGLContext.h>
#include "internal.h"
#include <MGLRenderer.h>

#include <unistd.h>
#include <math.h>

#define GL_BGRA                           0x80E1
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_DEPTH_COMPONENT                0x1902
#define GL_FLOAT                          0x1406


GLMContext createGLMContext(GLenum format, GLenum type,
                        GLenum depth_format, GLenum depth_type,
                        GLenum stencil_format, GLenum stencil_type);

void MGLsetCurrentContext(GLMContext ctx);
void MGLswapBuffers(GLMContext ctx);

static void makeContextCurrentMGL(_GLFWwindow* window)
{
    @autoreleasepool {

    if (window)
    {
        MGLsetCurrentContext(window->context.mgl.ctx);

        _glfwPlatformSetTls(&_glfw.contextSlot, window);
    }
    else
    {
        // just so we have jump tables
        MGLsetCurrentContext(createGLMContext(0, 0,
                                              0, 0,
                                              0, 0));
    }

    } // autoreleasepool
}

static void swapBuffersMGL(_GLFWwindow* window)
{
    MGLswapBuffers(window->context.mgl.ctx);
}

static void swapIntervalMGL(int interval)
{

}

static int extensionSupportedMGL(const char* extension)
{
    // There are no MGL extensions
    return GLFW_FALSE;
}

static GLFWglproc getProcAddressMGL(const char* procname)
{
    GLFWproc symbol;

    assert(_glfw.mgl.handle);

    symbol = _glfwPlatformGetModuleSymbol(_glfw.mgl.handle, procname);
    assert(symbol);

    return symbol;
}

static void destroyContextMGL(_GLFWwindow* window)
{
    @autoreleasepool {


    } // autoreleasepool
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize OpenGL support
//
GLFWbool _glfwInitMGL(void)
{
    if (_glfw.mgl.handle)
        return GLFW_TRUE;

    _glfw.mgl.handle = _glfwPlatformLoadModule("libmgl.dylib");
    assert(_glfw.mgl.handle);

    if (_glfw.mgl.handle == NULL)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "MGL: Failed to locate libmgl.dylib");
        return GLFW_FALSE;
    }

    return GLFW_TRUE;
}

// Terminate OpenGL support
//
void _glfwTerminateMGL(void)
{
}

// Create the OpenGL context
//
GLFWbool _glfwCreateContextMGL(_GLFWwindow* window,
                                const _GLFWctxconfig* ctxconfig,
                                const _GLFWfbconfig* fbconfig)
{
    if (ctxconfig->client == GLFW_OPENGL_ES_API)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "MGL: OpenGL ES is not available on macOS");
        return GLFW_FALSE;
    }

    if (ctxconfig->major < 4)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "MGL: OpenGL 4.6 and above supported on MGL");
        return GLFW_FALSE;
    }

    if (ctxconfig->minor < 6)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "MGL: OpenGL 4.6 and above supported on MGL");
        return GLFW_FALSE;
    }

    window->context.mgl.ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                                               GL_DEPTH_COMPONENT, GL_FLOAT,
                                               0, 0);
    assert(window->context.mgl.ctx);

    if (window->context.mgl.ctx == nil)
    {
        _glfwInputError(GLFW_VERSION_UNAVAILABLE,
                        "MGL: Failed to create MGL context");
        return GLFW_FALSE;
    }

    [window->ns.view wantsLayer];

    MGLRenderer *renderer = [[MGLRenderer alloc] init];
    assert(renderer);

    window->context.mgl.renderer = renderer;

    [window->context.mgl.renderer createMGLRendererAndBindToContext: window->context.mgl.ctx view: window->ns.view];

    //[window->context.mgl.object setView: window->ns.view];

    window->context.makeCurrent = makeContextCurrentMGL;
    window->context.swapBuffers = swapBuffersMGL;
    window->context.swapInterval = swapIntervalMGL;
    window->context.extensionSupported = extensionSupportedMGL;
    window->context.getProcAddress = getProcAddressMGL;
    window->context.destroy = destroyContextMGL;

    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI void * glfwGetMGLContext(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(nil);

    if (_glfw.platform.platformID != GLFW_PLATFORM_COCOA)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,
                        "MGL: Platform not initialized");
        return nil;
    }

    if (window->context.source != GLFW_NATIVE_CONTEXT_API)
    {
        _glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL);
        return nil;
    }

    return window->context.mgl.ctx;
}

