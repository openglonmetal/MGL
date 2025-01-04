//
//  MGL_Tests.m
//  MGL Tests
//
//  Created by Michael Larson on 1/3/25.
//

#import <XCTest/XCTest.h>

@interface MGL_Tests : XCTestCase

@end

@implementation MGL_Tests

- (void)setUp
{
    GLFWwindow *window;
    
    glfwSetErrorCallback (error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    //glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    // force MGL
    //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    //glfwWindowHint(GLFW_DEPTH_BITS, 32);

    fprintf(stderr, "creating window...\n");

    window = glfwCreateWindow(width, width, name, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    GLMContext glm_ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
    void *renderer = CppCreateMGLRendererAndBindToContext (glfwGetCocoaWindow (window), glm_ctx); // FIXME should do something later with the renderer
    if (!renderer)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    MGLsetCurrentContext(glm_ctx);
    glfwSetWindowUserPointer(window, glm_ctx);

    //glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwGetError(NULL);

    glfwGetWindowSize(window, &width, &height);
    
    // hidpi
    width *= 2;
    height *= 2;
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
