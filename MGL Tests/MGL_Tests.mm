//
//  MGL_Tests.m
//  MGL Tests
//
//  Created by Michael Larson on 1/3/25.
//

#import <XCTest/XCTest.h>

#include <mach/mach_vm.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#include <stdbool.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

extern "C" {
#include "MGLContext.h"
}
#include "MGLRenderer.h"

#import "MGL_test_utils.h"


@interface MGL_Tests : XCTestCase
{
    NSApplication *m_application;
    NSWindow *m_window;
    GLMContext m_glm_ctx;
    MGLRenderer *m_renderer;
}
- (GLuint) winWidth;
- (GLuint) winHeight;
@end

@implementation MGL_Tests

- (NSRect) windowFrame
{
    return [m_window contentLayoutRect];
}

- (GLuint) winWidth
{
    return [self windowFrame].size.width;
}

- (GLuint) winHeight
{
    return [self windowFrame].size.height;
}

- (void)setUp
{
    dispatch_async(dispatch_get_main_queue(), ^{
        // Initialize NSApplication
        if (NSApp == NULL)
        {
            self->m_application = [NSApplication sharedApplication];
        }

        // Create and configure the NSWindow
        self->m_window = [[NSWindow alloc] initWithContentRect:NSMakeRect(32, 32, 512, 512)
                                                     styleMask:(NSWindowStyleMaskTitled |
                                                                NSWindowStyleMaskClosable)
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
        
        // Run the main loop temporarily to simulate user interaction
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];
        
        // Show the window
        [self->m_window makeKeyAndOrderFront:nil];
        [self->m_window makeMainWindow];
                
        [self->m_window setBackgroundColor: [NSColor blackColor]];
        
        [self->m_window display];
        [self->m_window makeKeyAndOrderFront:nil];
        
        [NSApp activateIgnoringOtherApps:YES];
        
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];

        self->m_glm_ctx = createGLMContext(GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0);
        self->m_renderer = [[MGLRenderer alloc] initMGLRendererFromContext: self->m_glm_ctx andBindToWindow: self->m_window];
        
        if (!self->m_renderer)
        {
            exit(EXIT_FAILURE);
        }
        
        MGLsetCurrentContext(self->m_glm_ctx);
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];
    });
}

- (void)tearDown
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self->m_window = nil;
    });

    self->m_renderer = NULL;
    self->m_glm_ctx = NULL;
    
    [super tearDown];
}

#if 0
- (void)testGLClear
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        glClearColor(0.5, 0.2, 0.2, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        MGLswapBuffers(NULL);
        
        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:60.0 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

- (void)testGLDrawArrays
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        GLuint vbo = 0, vao = 0;

        const char* vertex_shader =
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position, 1.0);
            }
        );

        const char* fragment_shader =
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             void main() {
                frag_colour = vec4(0.5, 0.0, 0.5, 1.0);
            }
        );

        float points[] = {
           0.0f,  0.5f,  0.0f,
           0.5f, -0.5f,  0.0f,
          -0.5f, -0.5f,  0.0f
        };

        vbo = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
        vao = bindVAO();

        bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);

        GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
        glUseProgram(shader_program);

        glViewport(0, 0, [self winWidth], [self winHeight]);

        glClearColor(0.2, 0.2, 0.2, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
            
        MGLswapBuffers(NULL);

        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:120 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}
#endif

- (void)testGLDrawArraysUniform1i
{
    XCTestExpectation *expectation = [self expectationWithDescription:@"Task on main thread completed"];
    
    // Ensure everything runs on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        GLuint vbo = 0, vao = 0;

        const char* vertex_shader =
        GLSL(460,
             layout(location = 0) in vec3 position;
             void main() {
                gl_Position = vec4(position.x, position.y, position.z, 1.0);
            }
        );
        const char* fragment_shader =
        GLSL(460,
             layout(location = 0) out vec4 frag_colour;
             layout(location = 0) uniform int mp;
             void main() {
                frag_colour = vec4(0.0, 0.0, float(mp)/100.0, 1.0);
            }
        );

        float points[] = {
           0.0f,  0.5f,  0.0f,
           0.5f, -0.5f,  0.0f,
          -0.5f, -0.5f,  0.0f
        };

        vbo = bindDataToVBO(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
        vao = bindVAO();

        bindAttribute(0, GL_ARRAY_BUFFER, vbo, 3, GL_FLOAT, false, 0, NULL);

        GLuint shader_program = compileGLSLProgram(2, GL_VERTEX_SHADER, vertex_shader, GL_FRAGMENT_SHADER, fragment_shader);
        glUseProgram(shader_program);

        glViewport(0, 0, [self winWidth], [self winHeight]);

        GLint mp_loc = glGetUniformLocation(shader_program, "mp");
        std::cout << mp_loc << std::endl;
        
        int a = 0;
        int e = 1;
        
        glClearColor(0.2, 0.2, 0.2, 0.0);
        
        glClear(GL_COLOR_BUFFER_BIT);
        MGLswapBuffers(NULL);

        glUseProgram(shader_program);
        
        int count;
        count = 100;
        while(count--)
        {
            glBindVertexArray(vao);
            glUniform1i(mp_loc, a);
            
            glClearColor(0.2, 0.2, (float)a/100.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            MGLswapBuffers(NULL);
            
            a += e;
            if(a>=100){e=-1;}
            if(a==0){e=1;}
        }
        
        [expectation fulfill];
    });
    
    // Wait for the expectation
    [self waitForExpectationsWithTimeout:120.0 handler:^(NSError * _Nullable error) {
        XCTAssertNil(error, @"The task did not complete in time");
    }];
}

#if 1
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
#endif

@end
